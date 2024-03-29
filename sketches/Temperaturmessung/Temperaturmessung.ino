#include <DHT.h>

#define DHT1_Pin 6
#define DHT2_Pin 8
#define DHT3_Pin 10
#define DHT4_Pin 13

#define DHT_Typ DHT11

DHT dhts[] = {DHT(DHT1_Pin, DHT_Typ), DHT(DHT2_Pin, DHT_Typ), DHT(DHT3_Pin, DHT_Typ), DHT(DHT4_Pin, DHT_Typ)};
float temp[4];

void setup() {
    Serial.begin(9600);

    for (int i = 0; i < sizeof(dhts) / sizeof(*dhts); i++) {
        dhts[i].begin();
    }
}

void loop() {
    if (Serial.available()) {
        Serial.read();

        delay(40);

        Serial.println(2);  // for identification

        for (int i = 0; i < sizeof(dhts) / sizeof(*dhts); i++) {
            temp[i] = dhts[i].readTemperature();
        }

        for (int i = 0; i < sizeof(dhts) / sizeof(*dhts); i++) {
            delay(40);
            Serial.println(temp[i]);
        }
    }
}
