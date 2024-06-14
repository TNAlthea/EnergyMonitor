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

#if !defined(BUZZER_PIN)
#define BUZZER_PIN 2
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
float voltage, current, energy, frequency, pf;
float power = 0;
float powerLimit = 10;

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

bool ignoreFirstMsg = false;  //ignore first mqtt message, assume it always retained

char mqttDeviceStatusTopic[64] = "2024/2005021/esp32/device_status/";
char mqttElectricalDataTopic[64] = "2024/2005021/esp32/telemetry/";
char mqttRelayTopic[64] = "2024/2005021/esp32/relay/";
char mqttServerTopic[64] = "2024/2005021/esp32/server/";  //message from server
char mqttPowerLimitTopic[64] = "2024/2005021/esp32/config/power/";

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

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  relayFunction();

  strcat(mqttDeviceStatusTopic, WiFi.macAddress().c_str());
  strcat(mqttElectricalDataTopic, WiFi.macAddress().c_str());
  strcat(mqttRelayTopic, WiFi.macAddress().c_str());
  strcat(mqttServerTopic, WiFi.macAddress().c_str());
  strcat(mqttPowerLimitTopic, WiFi.macAddress().c_str());

  client.subscribe(mqttRelayTopic);
  client.subscribe(mqttServerTopic);
  client.subscribe(mqttPowerLimitTopic);
}

void loop() {
  client.loop();

  currentMillis = millis();
  if (currentMillis - startMillis >= 5000) {
    if (power <= powerLimit) {
      pzemFunction();
      deviceStatus();
    } else {
      overloadAlert();
    }
    startMillis = currentMillis;
  }
}

void overloadAlert() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Power");
  lcd.setCursor(0, 1);
  lcd.print("Overload!");

  digitalWrite(BUZZER_PIN, HIGH);
  delay(1000);
  digitalWrite(BUZZER_PIN, LOW);
  delay(1000);

  relay1_on = false;
  relay2_on = false;
  relayStatus();

  // JSON object
  StaticJsonDocument<64> doc;
  doc["status"] = "overload";

  // Serialize JSON to a string
  char buffer[64];
  serializeJson(doc, buffer);

  // Publish the serialized JSON string
  client.publish(mqttPowerLimitTopic, buffer, false);
}

void clearRetainedMessages() {
  client.publish(mqttElectricalDataTopic, "", true);
  client.publish(mqttRelayTopic, "", true);
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

void deviceStatus() {
  char payload[10] = "active";
  client.publish(mqttDeviceStatusTopic, payload, false);

  StaticJsonDocument<64> doc;
  doc["status"] = "normal";

  // Serialize JSON to a string
  char buffer[32];
  serializeJson(doc, buffer);

  client.publish(mqttPowerLimitTopic, buffer, false);
}

void relayStatus() {
  // relay status
  StaticJsonDocument<64> doc;
  doc["relay_1"] = relay1_on ? "active" : "inactive";
  doc["relay_2"] = relay2_on ? "active" : "inactive";

  // Serialize JSON to a string
  char buffer[64];
  serializeJson(doc, buffer);

  client.publish(mqttRelayTopic, buffer, false);
}

void relayFunction() {
  digitalWrite(relay1, relay1_on ? HIGH : LOW);
  digitalWrite(relay2, relay2_on ? HIGH : LOW);
}

void pzemFunction() {
  float voltage = pzem.voltage();
  if (isnan(voltage)) {
    Serial.println("Error reading voltage");
  }

  float current = pzem.current();
  if (isnan(current)) {
    Serial.println("Error reading current");
  }

  power = pzem.power();
  if (isnan(power)) {
    Serial.println("Error reading power");
  }

  float energy = pzem.energy();
  if (isnan(energy)) {
    Serial.println("Error reading energy");
  }

  float frequency = pzem.frequency();
  if (isnan(frequency)) {
    Serial.println("Error reading frequency");
  }

  float pf = pzem.pf();
  if (isnan(pf)) {
    Serial.println("Error reading power factor");
  }

  if (!isnan(voltage) && !isnan(power) && !isnan(current) && !isnan(energy) && !isnan(frequency) && !isnan(pf)) {
    pzemLcdPrint(&voltage, &current, &energy, &frequency, &pf);
    publishData(&voltage, &current, &energy, &frequency, &pf);
  }
}

void pzemLcdPrint(float *voltage, float *current, float *energy, float *frequency, float *powerFactor) {
  lcd.setCursor(0, 0);
  lcd.print("V:");
  lcd.print(*voltage);

  lcd.setCursor(0, 1);
  lcd.print("I:");
  lcd.print(*current);

  lcd.setCursor(9, 0);
  lcd.print("P:");
  lcd.print(power);

  lcd.setCursor(9, 1);
  lcd.print("f:");
  lcd.print(*frequency);
}

void publishData(float *voltage, float *current, float *energy, float *frequency, float *powerFactor) {
  // JSON object
  StaticJsonDocument<200> doc;
  doc["voltage"] = *voltage;
  doc["current"] = *current;
  doc["power"] = power;
  doc["energy"] = *energy;
  doc["frequency"] = *frequency;
  doc["power_factor"] = *powerFactor;

  // Serialize JSON to a string
  char buffer[256];
  serializeJson(doc, buffer);
  // Serialize JSON to a string
  size_t n = serializeJson(doc, buffer, sizeof(buffer));

  // Publish the serialized JSON string
  client.publish(mqttElectricalDataTopic, (const uint8_t *)buffer, n, false);
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
    /* if server want to control/toggle relay */
    if (strcmp(topic, mqttRelayTopic) == 0) {
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
    }
    /* if server asks for relay status */
    else if (strcmp(topic, mqttServerTopic) == 0) {
      if (strcmp(doc["message"], "relay?") == 0) {
        relayStatus();
        Serial.println("server asked relay status");
      }
    }
    /* if server wants to change current limit */
    else if (strcmp(topic, mqttPowerLimitTopic) == 0) {
      if (doc["power_limit"]) {
        float receivedPowerValue = doc["power_limit"].as<float>();
        if (receivedPowerValue != 0) {
          powerLimit = receivedPowerValue;
          StaticJsonDocument<64> doc;
          doc["reset"] = "reset";

          // Serialize JSON to a string
          char buffer[32];
          serializeJson(doc, buffer);
          client.publish(mqttPowerLimitTopic, buffer, false);
        }
      }
      // turn all relays back on, reset power to 0 making the loop() first condition running again
      else if (doc["reset"]) {
        if (strcmp(doc["reset"], "reset") == 0) {
          lcd.clear();
          relay1_on = true;
          relay2_on = true;
          relayStatus();

          power = 0;

          Serial.println("reset");
        }
      }
    }
  } else {
    Serial.print(F("JSON parsing failed: "));
    Serial.println(error.c_str());
  }
}
