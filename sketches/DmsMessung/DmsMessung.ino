#include <HX711_ADC.h>

HX711_ADC loadCells[] = {HX711_ADC(2, 3), HX711_ADC(6, 7), HX711_ADC(8, 9), HX711_ADC(12, 13)};
float measurements[4];
float offsets[4];

int count = 1000;
long stabilisation = 2000;

void readData() {
    for (int i = 0; i < 4; i++) {
        measurements[i] = 0;
    }

    for (int i = 0; i < count; i++) {
        for (int j = 0; j < 4; j++) {
            loadCells[j].update();
        }

        for (int j = 0; j < 4; j++) {
            measurements[j] += loadCells[j].getRareData() - offsets[j];
        }

        delay(1);
    }

    for (int i = 0; i < 4; i++) {
        measurements[i] /= count * 2500;
    }
}

void loop() {
    if (Serial.available()) {
        Serial.read();

        delay(20);

        Serial.println(1); // for identification
        
        readData();

        for (int i = 0; i < 4; i++) {
            Serial.println(measurements[i]);
            delay(20);
        }
    }
}

void setup() {
    Serial.begin(9600);

    for (int i = 0; i < 4; i++) {
        loadCells[i].begin();
    }

    for (int i = 0; i < 4; i++) {
        loadCells[i].start(stabilisation, true);
    }

    readData();

    for (int i = 0; i < 4; i++) {
        offsets[i] = measurements[i];
    }

    delay(500);
}