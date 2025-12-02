/* === COLLECTOR FINAL - MULTI NODE (NODE 1 & NODE 2) === */
#include <Arduino.h>

#define BLYNK_TEMPLATE_ID   "TMPLxxxxxx"
#define BLYNK_TEMPLATE_NAME "Your_PRoject_Name"
#define BLYNK_AUTH_TOKEN    "YOur_Auth_TOKEN" 
#define BLYNK_PRINT Serial 

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <esp_now.h>
#include <BlynkSimpleEsp32.h>
#include <Crypto.h>
#include <ChaChaPoly.h>
#include <Curve25519.h>
#include <WiFiManager.h>

uint8_t myPrivKey[32] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //  <-- GANTI DENGAN KEY
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //      YANG DIHASILKAN KEYGEN.INO
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

uint8_t node1PubKey[32] = { 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //  <-- GANTI DENGAN KEY
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //      YANG DIHASILKAN KEYGEN.INO
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

uint8_t node2PubKey[32] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //  <-- GANTI DENGAN KEY
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //      YANG DIHASILKAN KEYGEN.INOS
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

ChaChaPoly chacha;
uint8_t sharedKeyNode1[32]; 
uint8_t sharedKeyNode2[32];

struct __attribute__((packed)) SensorData { float temp; float hum; int soil_pct; char node_id[8]; };
struct __attribute__((packed)) SecurePacket { uint8_t iv[12]; uint8_t tag[16]; uint8_t ciphertext[sizeof(SensorData)]; };

SecurePacket packet;
SensorData receivedData;
SensorData tempBuffer;
volatile bool dataReady = false;

// --- DEKRIPSI ---
bool tryDecrypt(uint8_t* key, const uint8_t* data) {
    memcpy(&packet, data, sizeof(SecurePacket));
    chacha.clear(); chacha.setKey(key, 32); chacha.setIV(packet.iv, 12);
    chacha.decrypt((uint8_t*)&tempBuffer, packet.ciphertext, sizeof(SensorData));
    return chacha.checkTag(packet.tag, 16); 
}

// --- CALLBACK ---
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    if (len != sizeof(SecurePacket)) return;

    bool decrypted = false;

    // Cek apakah dari Node 1?
    if (tryDecrypt(sharedKeyNode1, incomingData)) {
        decrypted = true;
    } 
    // Cek apakah dari Node 2?
    else if (tryDecrypt(sharedKeyNode2, incomingData)) {
        decrypted = true;
    }

    if (decrypted) {
        memcpy(&receivedData, &tempBuffer, sizeof(SensorData));
        dataReady = true;
    } else {
        // Hanya print kalau gagal dekripsi (biar tau ada yang nyoba masuk)
        // Serial.println("[GAGAL] Kunci Tidak Cocok"); 
    }
}

void setup() {
    Serial.begin(115200);
    
    Curve25519::eval(sharedKeyNode1, myPrivKey, node1PubKey); // Kunci buat Node 1
    Curve25519::eval(sharedKeyNode2, myPrivKey, node2PubKey); // Kunci buat Node 2

    WiFiManager wm;
    wm.setConfigPortalTimeout(180); 
    if(!wm.autoConnect("SETUP-COLLECTOR", "rahasia123")) ESP.restart();
    
    Blynk.config(BLYNK_AUTH_TOKEN);
    Blynk.connect();

    Serial.println("\n--------------------------------");
    Serial.print("MAC COLLECTOR : "); Serial.println(WiFi.macAddress());
    Serial.printf("CHANNEL WIFI  : %d \n", WiFi.channel());
    Serial.println("--------------------------------\n");

    if (esp_now_init() != ESP_OK) return;
    esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
    Serial.println("Collector Ready (Multi-Node).");
}

void loop() {
    Blynk.run();
    if (dataReady) {
        dataReady = false;
        Serial.printf("[DATA MASUK] ID:%s T:%.1f H:%.1f S:%d%%\n", 
            receivedData.node_id, receivedData.temp, receivedData.hum, receivedData.soil_pct);
        
        if (Blynk.connected()) {
            // KIRIM SESUAI NODE ID
            if (strcmp(receivedData.node_id, "NODE_01") == 0) {
                Blynk.virtualWrite(V0, receivedData.temp);
                Blynk.virtualWrite(V1, receivedData.hum);
                Blynk.virtualWrite(V2, receivedData.soil_pct);
                Serial.println("    >> Sent to Blynk (Node 1)");
            }
            else if (strcmp(receivedData.node_id, "NODE_02") == 0) {
                Blynk.virtualWrite(V3, receivedData.temp);
                Blynk.virtualWrite(V4, receivedData.hum);
                Blynk.virtualWrite(V5, receivedData.soil_pct);
                Serial.println("    >> Sent to Blynk (Node 2)");
            }
        }
    }
}