#include <HX711_ADC.h>

HX711_ADC loadCell[] = {HX711_ADC(2, 3), HX711_ADC(6, 7), HX711_ADC(8, 9), HX711_ADC(12, 13)};
float measurement[4];
float offset[4];

int count = 1000;
long stabilisation = 2000;
boolean tare = true;

void readData() {
    for (int i = 0; i < 4; i++) {
        measurements[i] = 0;
    }

    for (int i = 0; i < count; i++) {
        for (int j = 0; j < 4; j++) {
            loadCells[j].update();
        }

        for (int j = 0; j < 4; j++) {
            measurement[j] += loadCell[j].getRareData() - offset[j];
        }

        delay(1);
    }

    for (int i = 0; i < 4; i++) {
        measurements[i] /= count * 2000;
    }
}

void loop() {
    if (Serial.available()) {
        readData();

        Serial.read();

        delay(10);

        Serial.println(1); // for identification

        for (int i = 0; i < 4; i++) {
            delay(10);
            Serial.println(measurements[i]);
        }
    }
}

void setup() {
    Serial.begin(9600);

    for (int i = 0; i < 4; i++) {
        loadCell[i].begin();
    }

    for (int i = 0; i < 4; i++) {
        loadCell[i].start(stabilisation, tare);
    }

    readData();

    for (int i = 0; i < 4; i++) {
        offset[i] = measurement[i];
    }

    delay(500);
}
