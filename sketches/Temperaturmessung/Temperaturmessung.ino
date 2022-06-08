#include <DHT.h>

DHT dhts[] = {DHT(6, DHT11), DHT(8, DHT11), DHT(10, DHT11), DHT(13, DHT11)};

void setup() {
    Serial.begin(9600);

    for (int i = 0; i < sizeof(dhts) / sizeof(*dhts); i++) {
        dhts[i].begin();
    }
}

void loop() {
    if (Serial.available()) {
        Serial.read();

        delay(10);

        Serial.println(2);  // for identification

        for (int i = 0; i < sizeof(dhts) / sizeof(*dhts); i++) {
            float temp = dhts[i].readTemperature();

            delay(10);
            Serial.println(temp);
        }
    }
}
