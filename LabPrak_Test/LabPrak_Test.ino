#include <HX711_ADC.h>

HX711_ADC loadCells[] = {HX711_ADC(2, 3), HX711_ADC(6, 7), HX711_ADC(8, 9), HX711_ADC(12, 13)};
float measurements[4];
float offsets[4];

int count = 1000;
long stabilization = 2000;

void readData() {
    for (int i = 0; i < sizeof(loadCells) / sizeof(*loadCells); i++) {
        loadCells[i].update();
        measurements[i] = loadCells[i].getRareData() - offsets[i];
    }
}

void setup() {
    Serial.begin(9600);

    for (int i = 0; i < sizeof(loadCells) / sizeof(*loadCells); i++) {
        loadCells[i].begin();

        loadCells[i].start(stabilization, true);

        loadCells[i].update();

        // zero out load cells
        offsets[i] = loadCells[i].getRareData();
    }
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