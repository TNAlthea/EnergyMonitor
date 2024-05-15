#include <LiquidCrystal_I2C.h>
#include <PZEM004Tv30.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

#if !defined(PZEM_RX_PIN) && !defined(PZEM_TX_PIN)
#define PZEM_RX_PIN 16
#define PZEM_TX_PIN 17
#endif

#if !defined(PZEM_SERIAL)
#define PZEM_SERIAL Serial2
#endif

const char WIFI_SSID[] = "G01";
const char WIFI_PASSWORD[] = "12131225";

String HOST_PATH = "http://192.168.1.9/energy_monitor.php";

const char *mqtt_server = "broker.emqx.io";
const char *mqtt_username = "esp32";
const char *mqtt_password = "thesis";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

PZEM004Tv30 pzem(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN);
float voltage, current, power, energy, frequency, pf;

int lcdColumns = 16;
int lcdRows = 2;

const int relay1 = 32;
const int relay2 = 33;
// set LCD address, number of columns and rows
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

bool wifiConnected = false;
bool mqttConnected = false;

bool relay1_on = true;
bool relay2_on = true;
char mqttTopicRelay[50] = "2024/2005021/esp32/relay/";


unsigned long startMillis;
unsigned long currentMillis;
void setup() {
  startMillis = millis();

  //configure lcd
  lcd.init();
  lcd.backlight();
  Serial.begin(115200);

  wifiConfig();
  mqttConfig();

  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);

  strcat(mqttTopicRelay, WiFi.macAddress().c_str());
  client.subscribe(mqttTopicRelay);
}

void loop() {
  client.loop();

  currentMillis = millis();
  if (currentMillis - startMillis >= 2000) {
    pzemFunction();
    heartbeatStatus();

    startMillis = currentMillis;
  }
}

void wifiConfig() {
  //configure WIFI and HTTP
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting to wifi...");
    lcd.setCursor(0, 0);
    lcd.print("Connecting");
    lcd.setCursor(0, 1);
    lcd.print("to wifi");
    delay(500);
    Serial.print(".");
  }

  wifiConnected = true;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Wifi");
  lcd.setCursor(0, 1);
  lcd.print("Connected");
  delay(2000);
  lcd.clear();
}

void mqttConfig() {
  //Configure MQTT
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  while (!client.connected()) {
    String client_id = "esp32-client-";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
    lcd.setCursor(0, 0);
    lcd.print("Connecting");
    lcd.setCursor(0, 1);
    lcd.print("MQTT");
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Public EMQX MQTT broker connected");
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("MQTT Fails");
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("MQTT");
  lcd.setCursor(0, 1);
  lcd.print("Connected");
  delay(2000);
  lcd.clear();

  mqttConnected = true;
}

void heartbeatStatus() {
  // device status
  char topic[100] = "2024/2005021/esp32/device_status/";
  strcat(topic, WiFi.macAddress().c_str());
  char payload[10] = "active";
  client.publish(topic, payload);

  // relay status
  StaticJsonDocument<64> doc;
  doc["relay_1"] = relay1_on ? "active" : "inactive";
  doc["relay_2"] = relay2_on ? "active" : "inactive";

  // Serialize JSON to a string
  char buffer[64];
  serializeJson(doc, buffer);
  size_t n = serializeJson(doc, buffer);

  client.publish(mqttTopicRelay, buffer, n);
}

void pzemFunction() {
  float voltage = pzem.voltage();
  if (voltage != NAN) {
    lcd.setCursor(0, 0);
    lcd.print("V:");
    lcd.print(voltage);
  } else {
    Serial.println("Error reading voltage");
  }

  float current = pzem.current();
  if (current != NAN) {
    lcd.setCursor(0, 1);
    lcd.print("I:");
    lcd.print(current);
  } else {
    Serial.println("Error reading current");
  }

  float power = pzem.power();
  if (power != NAN) {
    lcd.setCursor(9, 0);
    lcd.print("P:");
    lcd.print(power);
  } else {
    Serial.println("Error reading power");
  }

  float energy = pzem.energy();
  if (energy == NAN) Serial.println("Error reading energy");

  float frequency = pzem.frequency();
  if (frequency != NAN) {
    lcd.setCursor(9, 1);
    lcd.print("f:");
    lcd.print(frequency);
  } else {
    Serial.println("Error reading frequency");
  }

  float pf = pzem.pf();
  if (pf == NAN) Serial.println("Error reading power factor");

  publishData(&voltage, &power, &current, &energy, &frequency, &pf);
}

void relayFunction() {
  digitalWrite(relay1, relay1_on ? HIGH : LOW);
  digitalWrite(relay2, relay2_on ? HIGH : LOW);
}

void publishData(float *voltage, float *current, float *power, float *energy, float *frequency, float *powerFactor) {
  // JSON object
  StaticJsonDocument<200> doc;
  doc["voltage"] = *voltage;
  doc["current"] = *current;
  doc["power"] = *power;
  doc["energy"] = *energy;
  doc["frequency"] = *frequency;
  doc["power_factor"] = *powerFactor;

  // Serialize JSON to a string
  char buffer[256];
  serializeJson(doc, buffer);
  size_t n = serializeJson(doc, buffer);

  client.publish("2024/2005021/esp32/telemetry", buffer, n);
}


void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  Serial.println("-----------------------");

  StaticJsonDocument<200> doc;
  // Deserialize the JSON payload
  DeserializationError error = deserializeJson(doc, payload, length);

  // Check for parsing errors
  if (!error) {
    // Compare topics using strcmp
    if (strcmp(topic, mqttTopicRelay) == 0) {
      // Extract relay statuses from the JSON document
      if (doc["relay_1"]) {
        const char *relay1_status = doc["relay_1"];
        relay1_on = (strcmp(relay1_status, "active") == 0) ? true : false;
      }
      if (doc["relay_2"]) {
        const char *relay2_status = doc["relay_2"];
        relay2_on = (strcmp(relay2_status, "active") == 0) ? true : false;
      }
      relayFunction();
      delay(1000);
    }
  } else {
    Serial.print(F("JSON parsing failed: "));
    Serial.println(error.c_str());
  }
}
