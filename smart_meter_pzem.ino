#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <PZEM004Tv30.h>
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>

#define BLYNK_TEMPLATE_ID "TMPL6lx4adcu_"
#define BLYNK_TEMPLATE_NAME "Quickstart Template"
#define BLYNK_AUTH_TOKEN "I8lNvlXbALwXfXtRwvDoCNQ5Vc7rT3yJ"
#include <BlynkSimpleEsp8266.h>


// Pin PZEM
#define PZEM_RX_PIN 13  // D7
#define PZEM_TX_PIN 12  // D6

// Pin RELAY
#define RELAY_OUT 14

SoftwareSerial pzemSWSerial(PZEM_RX_PIN, PZEM_TX_PIN);
PZEM004Tv30 pzem(pzemSWSerial);

// Struktur konfigurasi
struct Config {
  String ssid;
  String password;
  float tdl; // Tarif dasar listrik (Rp/kWh)
  String BLYNK_TEMPLATE_ID_STRUCT;
  String BLYNK_TEMPLATE_NAME_STRUCT;
  String BLYNK_AUTH_TOKEN_STRUCT;
} config;

// Akumulasi
float total_kwh = 0;
float total_watt = 0;
float total_rupiah = 0;

BLYNK_WRITE(V9) {
  int state = param.asInt();
  digitalWrite(RELAY_OUT, state);
  Serial.println(state ? "🔛 Relay ON dari Blynk" : "🔴 Relay OFF dari Blynk");
}

bool saveConfig() {
  StaticJsonDocument<512> doc;
  doc["ssid"] = config.ssid;
  doc["password"] = config.password;
  doc["tdl"] = config.tdl;
  doc["BLYNK_TEMPLATE_ID_STRUCT"] = config.BLYNK_TEMPLATE_ID_STRUCT;
  doc["BLYNK_TEMPLATE_NAME_STRUCT"] = config.BLYNK_TEMPLATE_NAME_STRUCT;
  doc["BLYNK_AUTH_TOKEN_STRUCT"] = config.BLYNK_AUTH_TOKEN_STRUCT;

  File file = LittleFS.open("/config.json", "w");
  if (!file) {
    Serial.println("❌ Gagal membuka file untuk menulis");
    return false;
  }
  serializeJsonPretty(doc, file);
  file.close();
  Serial.println("✅ Konfigurasi disimpan.");
  return true;
}

bool loadConfig() {
  if (!LittleFS.begin()) {
    Serial.println("❌ Gagal mount LittleFS");
    return false;
  }
  File file = LittleFS.open("/config.json", "r");
  if (!file) {
    Serial.println("⚠️  File config.json belum ada");
    return false;
  }
  StaticJsonDocument<512> doc;
  DeserializationError err = deserializeJson(doc, file);
  file.close();
  if (err) {
    Serial.println("❌ Gagal parsing config.json");
    return false;
  }
  config.ssid = doc["ssid"] | "";
  config.password = doc["password"] | "";
  config.tdl = doc["tdl"] | 0;
  config.BLYNK_TEMPLATE_ID_STRUCT = doc["BLYNK_TEMPLATE_ID_STRUCT"] | "";
  config.BLYNK_TEMPLATE_NAME_STRUCT = doc["BLYNK_TEMPLATE_NAME_STRUCT"] | "";
  config.BLYNK_AUTH_TOKEN_STRUCT = doc["BLYNK_AUTH_TOKEN_STRUCT"] | "";
  Serial.println("✅ Konfigurasi berhasil dimuat:");
  Serial.println("SSID: " + config.ssid);
  Serial.println("PASSWORD: " + String(config.password));
  Serial.println("TDL: " + String(config.tdl));
  Serial.println("BLYNK_TEMPLATE_ID_STRUCT: " + String(config.BLYNK_TEMPLATE_ID_STRUCT));
  Serial.println("BLYNK_TEMPLATE_NAME_STRUCT: " + String(config.BLYNK_TEMPLATE_NAME_STRUCT));
  Serial.println("BLYNK_AUTH_TOKEN_STRUCT: " + String(config.BLYNK_AUTH_TOKEN_STRUCT));
  return true;
}

void inputConfigFromSerial() {
  Serial.println("\n🔧 Masukkan konfigurasi:");
  Serial.print("SSID WiFi: ");
  while (!Serial.available()) delay(10);
  config.ssid = Serial.readStringUntil('\n'); config.ssid.trim();
  Serial.print("Password WiFi: ");
  while (!Serial.available()) delay(10);
  config.password = Serial.readStringUntil('\n'); config.password.trim();
  Serial.print("TDL (Rp/kWh): ");
  while (!Serial.available()) delay(10);
  config.tdl = Serial.readStringUntil('\n').toFloat();
  Serial.print("BLYNK_TEMPLATE_ID_STRUCT: ");
  while (!Serial.available()) delay(10);
  config.BLYNK_TEMPLATE_ID_STRUCT = Serial.readStringUntil('\n'); config.BLYNK_TEMPLATE_ID_STRUCT.trim();
  Serial.print("BLYNK_TEMPLATE_NAME_STRUCT: ");
  while (!Serial.available()) delay(10);
  config.BLYNK_TEMPLATE_NAME_STRUCT = Serial.readStringUntil('\n'); config.BLYNK_TEMPLATE_NAME_STRUCT.trim();
  Serial.print("BLYNK_AUTH_TOKEN_STRUCT: ");
  while (!Serial.available()) delay(10);
  config.BLYNK_AUTH_TOKEN_STRUCT = Serial.readStringUntil('\n'); config.BLYNK_AUTH_TOKEN_STRUCT.trim();
  saveConfig();
}

void kirimDataKeBlynk(float voltage, float current, float power, float energy, float frequency, float biaya) {
  Blynk.virtualWrite(V0, power);
  Blynk.virtualWrite(V1, energy);
  Blynk.virtualWrite(V2, biaya);
  Blynk.virtualWrite(V3, voltage);

  char buffer_current[10];
  dtostrf(current, 1, 2, buffer_current);  // 3 angka di belakang koma
  Blynk.virtualWrite(V4, buffer_current);
  Blynk.virtualWrite(V5, frequency);
  Blynk.virtualWrite(V6, total_watt);
  Blynk.virtualWrite(V7, total_kwh);

  char buffer_rupiah[10];
  dtostrf(total_rupiah, 1, 0, buffer_rupiah);  // 3 angka di belakang koma
  Blynk.virtualWrite(V8, buffer_rupiah);
}

void tampilkanDataPZEM() {
  float voltage = pzem.voltage();
  float current = pzem.current();
  float power = pzem.power();
  float energy = pzem.energy();
  float frequency = pzem.frequency();
  float pf = pzem.pf();
  if (isnan(voltage) || isnan(current) || isnan(power) || isnan(energy)) {
    Serial.println("⚠️  Error membaca data dari PZEM!");
    return;
  }

  float sampling_time = 5;
  float energiKWh = (power * sampling_time) / 3600000.0;
  float biaya = energiKWh * config.tdl;

  total_kwh += energy;
  total_watt += power;
  total_rupiah += biaya;

  Serial.println("📊 Data PZEM:");
  Serial.printf("Tegangan : %.2f V\n", voltage);
  Serial.printf("Arus     : %.2f A\n", current);
  Serial.printf("Daya     : %.2f W\n", power);
  Serial.printf("Energi   : %.3f kWh\n", energy);
  Serial.printf("Biaya    : Rp %.2f\n", biaya);
  Serial.printf("Frekuensi: %.1f Hz\n", frequency);
  Serial.printf("PowerFactor: %.2f\n", pf);
  Serial.println("🔁 Akumulasi:");
  Serial.printf("Total Daya   : %.2f W\n", total_watt);
  Serial.printf("Total Energi : %.3f kWh\n", total_kwh);
  Serial.printf("Total Biaya  : Rp %.2f\n", total_rupiah);
  Serial.println();
  kirimDataKeBlynk(voltage, current, power, energy, frequency, biaya);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  pinMode(RELAY_OUT, OUTPUT);
  Serial.println("✅ Siap baca data dari PZEM...");
  digitalWrite(RELAY_OUT, LOW);
  delay(1000);
  digitalWrite(RELAY_OUT, HIGH);
  if (!LittleFS.begin()) {
    Serial.println("Gagal mount LittleFS");
    return;
  }
  if (!loadConfig()) {
    Serial.println("📥 Konfigurasi belum ada atau rusak.");
    inputConfigFromSerial();
  } else {
    Serial.println("\n➡️ Tekan ENTER dalam 5 detik untuk ubah konfigurasi...");
    unsigned long start = millis();
    while (millis() - start < 5000) {
      if (Serial.available()) {
        Serial.read();
        inputConfigFromSerial();
        break;
      }
    }
  }
  WiFi.begin(config.ssid.c_str(), config.password.c_str());
  Serial.print("🔌 Menghubungkan ke WiFi");
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 20) {
    delay(500);
    Serial.print(".");
    retry++;
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("✅ WiFi terhubung: " + WiFi.localIP().toString());
  } else {
    Serial.println("❌ Gagal konek WiFi");
  }
  Blynk.config(config.BLYNK_AUTH_TOKEN_STRUCT.c_str());
  Blynk.connect();
}

void handleSerialInput() {
  static String buffer;
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '<') {
      buffer = "";
    } else if (c == '>') {
      int sep = buffer.indexOf(':');
      if (sep > 0) {
        String key = buffer.substring(0, sep);
        String value = buffer.substring(sep + 1);
        value.trim();
        if (key == "ssid") {
          config.ssid = value;
          Serial.println("✅ SSID diupdate ke: " + value);
        } else if (key == "pass") {
          config.password = value;
          Serial.println("✅ Password diupdate.");
        } else if (key == "tdl") {
          config.tdl = value.toFloat();
          Serial.println("✅ TDL diupdate ke: " + String(config.tdl));
        } else if (key == "template_id") {
          config.BLYNK_TEMPLATE_ID_STRUCT = value;
          Serial.println("✅ BLYNK_TEMPLATE_ID_STRUCT diupdate.");
        } else if (key == "template_name") {
          config.BLYNK_TEMPLATE_NAME_STRUCT = value;
          Serial.println("✅ BLYNK_TEMPLATE_NAME_STRUCT diupdate.");
        } else if (key == "auth") {
          config.BLYNK_AUTH_TOKEN_STRUCT = value;
          Serial.println("✅ BLYNK_AUTH_TOKEN_STRUCT diupdate.");
        } else {
          Serial.println("❓ Tidak dikenali: " + key);
        }
        saveConfig();
      } else {
        Serial.println("❌ Format salah. Gunakan <key:value>");
      }
      buffer = "";
    } else {
      buffer += c;
    }
  }
}

void loop() {
  Blynk.run();
  handleSerialInput();
  static unsigned long lastPzem = 0;
  if (millis() - lastPzem > 5000) {
    lastPzem = millis();
    tampilkanDataPZEM();
  }
}
