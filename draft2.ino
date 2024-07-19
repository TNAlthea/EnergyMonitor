#include <LiquidCrystal_I2C.h>
#include <PZEM004Tv30.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

/* Serial dan PIN (RX) (TX) modul sensor listrik PZEM */
#if !defined(PZEM_RX_PIN) && !defined(PZEM_TX_PIN)
#define PZEM_RX_PIN 16
#define PZEM_TX_PIN 17
#endif

#if !defined(PZEM_SERIAL)
#define PZEM_SERIAL Serial2
#endif

/* PIN modul buzzer KY-012 */
#if !defined(BUZZER_PIN)
#define BUZZER_PIN 2
#endif

/* kredensial WIFI */
const char WIFI_SSID[] = "G01";
const char WIFI_PASSWORD[] = "12131225";

/* kredensial MQTT broker */
const char *mqtt_server = "broker.emqx.io";
const char *mqtt_username = "esp32";
const char *mqtt_password = "thesis";

WiFiClient espClient;
PubSubClient client(espClient);

/* variabel PZEM */
PZEM004Tv30 pzem(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN);
float voltage, current, energy, frequency, pf;
float power = 0;
float powerLimit = 400;
bool overload;

/* variabel LCD */
int lcdColumns = 16;
int lcdRows = 2;

/* variabel pin relay */
const int relay1 = 32;
const int relay2 = 33;

// set LCD address, number of columns and rows
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

/* status wifi dan mqtt */
bool wifiConnected = false;
bool mqttConnected = false;

/* status relay */
bool relay1_on = true;
bool relay2_on = true;

/* topic yang akan berlangganan */
char mqttDeviceStatusTopic[64] = "2024/2005021/esp32/device_status/";
char mqttElectricalDataTopic[64] = "2024/2005021/esp32/telemetry/";
char mqttRelayTopic[64] = "2024/2005021/esp32/relay/";
char mqttServerTopic[64] = "2024/2005021/esp32/server/"; 
char mqttPowerLimitTopic[64] = "2024/2005021/esp32/config/power/";

/* variabel detik untuk looping */
unsigned long startMillis;
unsigned long currentMillis;

void setup() {
  startMillis = millis();
  overload = false;

  // Konfigurasi I2C LCD
  lcd.init();
  lcd.backlight();
  Serial.begin(115200);

  wifiConfig();
  mqttConfig();

  // Konfigurasi PIN dan modul
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  relayFunction();

  // Konfigurasi topic yang akan berlangganan
  strcat(mqttDeviceStatusTopic, WiFi.macAddress().c_str());
  strcat(mqttElectricalDataTopic, WiFi.macAddress().c_str());
  strcat(mqttRelayTopic, WiFi.macAddress().c_str());
  strcat(mqttServerTopic, WiFi.macAddress().c_str());
  strcat(mqttPowerLimitTopic, WiFi.macAddress().c_str());

  // Berlangganan topic
  client.subscribe(mqttRelayTopic);
  client.subscribe(mqttServerTopic);
  client.subscribe(mqttPowerLimitTopic);
}

void loop() {
  client.loop();

  currentMillis = millis();
  /* lakukan looping setiap 5 detik */
  if (currentMillis - startMillis >= 5000) {
    /* apabila mqtt tidak terhubung */
    if (!client.connected()) {
      mqttConfig();
    }

    /* apabila beban daya tidak melebihi batas */
    if (overload == false) {
      pzemFunction();
    }

    /* apabila sensor mengalami malfungsi */
    if (isnan(power)) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("PZEM");
      lcd.setCursor(0, 1);
      lcd.print("ERROR");
      Serial.println(overload);
    } 
    /* apabila beban melebihi batas yang diperbolehkan */
    else if ((power > powerLimit) && (!isnan(power))) {
      overloadAlert();
    } 
    /* apabila kondisi normal */
    else {
      pzemLcdPrint();
      deviceStatus();
    }

    startMillis = currentMillis;
  }
}

/* fungsi untuk memberi peringatan ketika beban daya melebihi batasan yang diperbolehkan */
void overloadAlert() {
  overload = true;

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

/* konfigurasi wifi dan koneksi pertama kali */
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

/* konfigurasi MQTT dan koneksi pertama kali */
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

/* menghubungkan kembali device kepada MQTT broker apabila jaringan terputus */
boolean mqttReconnect() {
  String client_id = "esp32-client-";
  client_id += String(WiFi.macAddress());
  if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
    Serial.println("MQTT RECONNECTED");
  }
  return client.connected();
}

/* memberi informasi status device */
void deviceStatus() {
  /* heartbeat status controller */
  char payload[10] = "active";
  client.publish(mqttDeviceStatusTopic, payload, false);

  /* status beban daya saat ini apakah normal/overload */
  StaticJsonDocument<64> doc;
  doc["status"] = "normal";
  // Serialize JSON to a string
  char buffer[32];
  serializeJson(doc, buffer);

  client.publish(mqttPowerLimitTopic, buffer, false);
}

/* mengatur relay dan memberi informasi status relay */
void relayStatus() {
  // relay status
  StaticJsonDocument<64> doc;
  doc["relay_1"] = relay1_on ? "active" : "inactive";
  doc["relay_2"] = relay2_on ? "active" : "inactive";

  // Serialize JSON to a string
  char buffer[64];
  serializeJson(doc, buffer);

  /* publish status relay saat ini melalui mqtt */
  client.publish(mqttRelayTopic, buffer, false);
}

void relayFunction() {
  digitalWrite(relay1, relay1_on ? HIGH : LOW);
  digitalWrite(relay2, relay2_on ? HIGH : LOW);
}

/*
* Fungsi deteksi pada sensor
*/
void pzemFunction() {
  voltage = pzem.voltage();
  if (isnan(voltage)) {
    Serial.println("Error reading voltage");
  }

  current = pzem.current();
  if (isnan(current)) {
    Serial.println("Error reading current");
  }

  power = pzem.power();
  if (isnan(power)) {
    Serial.println("Error reading power");
  }

  energy = pzem.energy();
  if (isnan(energy)) {
    Serial.println("Error reading energy");
  }

  frequency = pzem.frequency();
  if (isnan(frequency)) {
    Serial.println("Error reading frequency");
  }

  pf = pzem.pf();
  if (isnan(pf)) {
    Serial.println("Error reading power factor");
  }

  if (!isnan(voltage) && !isnan(power) && !isnan(current) && !isnan(energy) && !isnan(frequency) && !isnan(pf)) {
    publishData(&voltage, &current, &energy, &frequency, &pf);
  }
}

/*
*  Menampilkan data dari sensor PZEM ke LCD 
*/
void pzemLcdPrint() {
  lcd.setCursor(0, 0);
  lcd.print("V:");
  lcd.print(voltage);

  lcd.setCursor(0, 1);
  lcd.print("I:");
  lcd.print(current);

  lcd.setCursor(9, 0);
  lcd.print("P:");
  lcd.print(power);

  lcd.setCursor(9, 1);
  lcd.print("f:");
  lcd.print(frequency);
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

/* callback ketika menerima pesan dari topic MQTT yang berlangganan */
void callback(char *topic, byte *payload, unsigned int length) {
  StaticJsonDocument<200> doc;
  // Deserialize the JSON payload
  DeserializationError error = deserializeJson(doc, payload, length);

  // Check for parsing errors
  if (!error) {
    // Compare topics using strcmp
    /* Apabila user ingin mengatur relay */
    if (strcmp(topic, mqttRelayTopic) == 0) {
      /*
      * Received key: "relay_n"
      * Received value: "active" / "inactive"
      * Description: Menyalakan atau mematikan relay
      */
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
    /* Apabila server ingin mengetahui status relay */
    else if (strcmp(topic, mqttServerTopic) == 0) {
      /*
      * Received key: "power_limit"
      * Received value: "relay?"
      * Description: Mengubah batasan (threshold) daya yang diperbolehkan
      */
      if (strcmp(doc["message"], "relay?") == 0) {
        relayStatus();
        Serial.println("server asked relay status");
      }
    }
    /* Apabila user ingin mengubahbatasan (threshold) daya yang diperbolehkan */
    else if (strcmp(topic, mqttPowerLimitTopic) == 0) {
      /*
      * Received key: "power_limit"
      * Received value: {batasan daya}
      * Description: Mengubah batasan (threshold) daya yang diperbolehkan
      */
      if (doc["power_limit"]) {
        float receivedPowerValue = doc["power_limit"].as<float>();
        if (receivedPowerValue != 0) {
          powerLimit = receivedPowerValue;
          StaticJsonDocument<32> doc;
          doc["reset"] = "reset";

          // Serialize JSON to a string
          char buffer[32];
          serializeJson(doc, buffer);
          client.publish(mqttPowerLimitTopic, buffer, false);
        }
      }
      /*
      * Received key: "reset"
      * Received value: "reset"
      * Description: Menyalakan semua relay, mengembalikan nilai power menjadi 0, dan membuat kondisi if pertama pada loop() berjalan kembali
      */
      else if (doc["reset"]) {
        if (strcmp(doc["reset"], "reset") == 0) {
          lcd.clear();
          relay1_on = true;
          relay2_on = true;
          relayStatus();

          power = 0;
          overload = false;

          Serial.println("reset");
        }
      }
    }
  } else {
    Serial.print(F("JSON parsing failed: "));
    Serial.println(error.c_str());
  }
}
