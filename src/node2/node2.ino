/* SENDER NODE 2 - ECC VALID KEY */
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h> 
#include <DHT.h>
#include <Crypto.h>
#include <ChaChaPoly.h>
#include <Curve25519.h>

#define NODE_NAME    "NODE_02"  
#define WIFI_CHANNEL 1    // <--- GANTI JADI CHANNEL YANG MUNCUL DI SERIAL COLLECTOR
uint8_t receiverMac[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // <--- GANTI MAC COLLECTOR

uint8_t collectorPubKey[32] = { 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //  <-- GANTI DENGAN KEY
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //      YANG DIHASILKAN KEYGEN.INO
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

uint8_t myPrivKey[32] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //  <-- GANTI DENGAN KEY
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //      YANG DIHASILKAN KEYGEN.INO
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#define DHT_PIN 4
#define DHT_TYPE DHT11
#define SOIL_PIN 34
#define SOIL_PWR_PIN 5

DHT dht(DHT_PIN, DHT_TYPE);
ChaChaPoly chacha;
uint8_t sharedKey[32]; 

struct __attribute__((packed)) SensorData { float temp; float hum; int soil_pct; char node_id[8]; };
struct __attribute__((packed)) SecurePacket { uint8_t iv[12]; uint8_t tag[16]; uint8_t ciphertext[sizeof(SensorData)]; };

SecurePacket packet;
SensorData payload;
esp_now_peer_info_t peerInfo;

void setup() {
    Serial.begin(115200);
    dht.begin();
    pinMode(SOIL_PIN, INPUT); pinMode(SOIL_PWR_PIN, OUTPUT); digitalWrite(SOIL_PWR_PIN, LOW);

    WiFi.mode(WIFI_STA);
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE);
    esp_wifi_set_promiscuous(false);

    // Hitung Kunci Rahasia
    Curve25519::eval(sharedKey, myPrivKey, collectorPubKey);

    if (esp_now_init() != ESP_OK) return;

    memset(&peerInfo, 0, sizeof(peerInfo));
    memcpy(peerInfo.peer_addr, receiverMac, 6);
    peerInfo.channel = WIFI_CHANNEL;  
    peerInfo.encrypt = false;
    peerInfo.ifidx = WIFI_IF_STA;
    
    if (esp_now_add_peer(&peerInfo) != ESP_OK) return;
    
    strcpy(payload.node_id, NODE_NAME);
    Serial.println("SENDER NODE 2 READY.");
}

void loop() {
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    
    digitalWrite(SOIL_PWR_PIN, HIGH); delay(20);
    int raw = analogRead(SOIL_PIN);
    digitalWrite(SOIL_PWR_PIN, LOW);
    
    int soil = map(raw, 4095, 1000, 0, 100); 
    soil = constrain(soil, 0, 100);

    if(isnan(t)) { payload.temp = 0.0; payload.hum = 0.0; }
    else { payload.temp = t; payload.hum = h; }
    payload.soil_pct = soil;

    // Encrypt
    for(int i=0; i<12; i++) packet.iv[i] = esp_random() % 256;
    chacha.clear(); chacha.setKey(sharedKey, 32); chacha.setIV(packet.iv, 12);
    chacha.encrypt(packet.ciphertext, (uint8_t*)&payload, sizeof(SensorData));
    chacha.computeTag(packet.tag, 16);

    // Kirim
    esp_err_t result = esp_now_send(receiverMac, (uint8_t *) &packet, sizeof(SecurePacket));
    
    if (result == ESP_OK) Serial.println(">> Terkirim (NODE 2)");
    else Serial.println(">> Gagal Kirim");
    
    delay(3000); 
}