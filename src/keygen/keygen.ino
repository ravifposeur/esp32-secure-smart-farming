/* ECC KEY PAIR GENERATOR (ALL NODES) */

#include <Arduino.h>
#include <Crypto.h>
#include <Curve25519.h>

void printKey(const char* name, uint8_t* key) {
    Serial.printf("uint8_t %s[32] = {\n  ", name);
    for (int i = 0; i < 32; i++) {
        Serial.printf("0x%02X", key[i]);
        if (i < 31) Serial.print(", ");
        if ((i + 1) % 8 == 0 && i < 31) Serial.print("\n  ");
    }
    Serial.println("\n};\n");
}

void setup() {
    Serial.begin(115200);
    while(!Serial);
    delay(2000);

    Serial.println("   GENERATING VALID ECC KEYS FOR FULL SYSTEM\n");

    uint8_t priv[32];
    uint8_t pub[32];

    Serial.println(" IDENTITAS COLLECTOR ");
    for(int i=0; i<32; i++) priv[i] = esp_random() % 256; 
    Curve25519::eval(pub, priv, 0); 
    
    Serial.println("[PASTE KE FILE: COLLECTOR.INO]");
    printKey("myPrivKey", priv); // Private Key Collector
    
    Serial.println("[PASTE KE FILE: SENDER NODE 1 & NODE 2]");
    printKey("collectorPubKey", pub); // Public Key Collector (Sama untuk semua node)
    
    Serial.println("-------------------------------------------------\n");

    // GENERATE KEY NODE 1
    Serial.println(" IDENTITAS NODE 1 ");
    for(int i=0; i<32; i++) priv[i] = esp_random() % 256;
    Curve25519::eval(pub, priv, 0);
    
    Serial.println("[PASTE KE FILE: SENDER NODE 1]");
    printKey("myPrivKey", priv); // Private Key Sender 1
    
    Serial.println("[PASTE KE FILE: COLLECTOR.INO]");
    printKey("node1PubKey", pub); // Public Key Sender 1

    Serial.println("-------------------------------------------------\n");

    // GENERATE KEY NODE 2
    Serial.println(" IDENTITAS NODE 2 ");
    for(int i=0; i<32; i++) priv[i] = esp_random() % 256;
    Curve25519::eval(pub, priv, 0);
    
    Serial.println("[PASTE KE FILE: SENDER NODE 2]");
    printKey("myPrivKey", priv); // Private Key Sender 2
    
    Serial.println("[PASTE KE FILE: COLLECTOR.INO]");
    printKey("node2PubKey", pub); // Public Key Sender 2

    Serial.println("=================================================");
    Serial.println("SELESAI. SILAKAN COPY SELURUH LOG DI ATAS.");
    Serial.println("JANGAN LUPA: Public Key Collector SAMA untuk semua Node.");
    Serial.println("=================================================");
}

void loop() {}