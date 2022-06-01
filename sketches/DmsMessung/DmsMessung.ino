#include <HX711_ADC.h>
#include "cstring"

HX711_ADC loadCells[] = {HX711_ADC(2, 3), HX711_ADC(6, 7), HX711_ADC(8, 9), HX711_ADC(12, 13)};
float measurements[4];
float offsets[4];

int count = 100;
long stabilization = 2000;

void readData() {
    // write the averaged data over 'count' measurements into 'measurements'.

    std::memset(measurements, 0, sizeof measurements); // zero out measurements

    for (int i = 0; i < count; i++) {
        for (int j = 0; j < sizeof(loadCells) / sizeof(*loadCells); j++) {
            loadCells[j].update();
            measurements[j] += loadCells[j].getRareData() - offsets[j];
        }
    }

    for (int i = 0; i < sizeof(loadCells) / sizeof(*loadCells); i++) {
        measurements[i] /= count;
    }
}

void setup() {
    Serial.begin(9600);

    for (int i = 0; i < sizeof(loadCells) / sizeof(*loadCells); i++) {
        loadCells[i].begin();

        loadCells[i].start(stabilization, true);
    }

    // zero out load cells
    readData();
    std::memcpy(offsets, measurements, sizeof offsets);
}

void loop() {
    if (Serial.available()) {
        Serial.read();

        delay(10);

        Serial.println(1);  // for identification

        readData();

        for (int i = 0; i < sizeof(loadCells) / sizeof(*loadCells); i++) {
            Serial.println(measurements[i]);
            delay(10);
        }
    }
}