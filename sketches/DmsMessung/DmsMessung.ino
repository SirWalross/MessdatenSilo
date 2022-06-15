#include <HX711_ADC.h>

HX711_ADC LoadCell(2, 3);
HX711_ADC LoadCell2(6, 7);
HX711_ADC LoadCell3(8, 9);
HX711_ADC LoadCell4(12, 13);

static float Offset = 0;
static float Offset2 = 0;
static float Offset3 = 0;
static float Offset4 = 0;

int anzahl = 1000;
int caltime = 2;
float calFac = 2500;
float calFac2 = 2500;
float calFac3 = 2500;
float calFac4 = 2500;
long stabilisation = 2000;
boolean t_are = true;
float messwertges;
float messwertges2;
float messwertges3;
float messwertges4;

float messwert;
float messwert2;
float messwert3;
float messwert4;

void regelbetrieb(HX711_ADC zelle, HX711_ADC zelle2, HX711_ADC zelle3, HX711_ADC zelle4) {
    float messwertAlt = messwert;
    float messwertAlt2 = messwert2;
    float messwertAlt3 = messwert3;
    float messwertAlt4 = messwert4;
    messwert = 0;
    messwert2 = 0;
    messwert3 = 0;
    messwert4 = 0;

    for (int i = 1; i <= anzahl; i++) {
        zelle.update();
        zelle2.update();
        zelle3.update();
        zelle4.update();

        messwert = messwert + (getDaten(Offset, zelle));
        messwert2 = messwert2 + (getDaten(Offset2, zelle2));
        messwert3 = messwert3 + (getDaten(Offset3, zelle3));
        messwert4 = messwert4 + (getDaten(Offset4, zelle4));

        delay(1);
    }

    messwert = messwert / anzahl;
    messwert2 = messwert2 / anzahl;
    messwert3 = messwert3 / anzahl;
    messwert4 = messwert4 / anzahl;

    messwert = messwert / calFac;
    messwert2 = messwert2 / calFac2;
    messwert3 = messwert3 / calFac3;
    messwert4 = messwert4 / calFac4;
}

float getDaten(float offset, HX711_ADC zelle) {
    float Offsetnew = offset;

    float data;

    data = (float)zelle.getRareData() - Offsetnew;

    return data;
}

void writeInData() {
    if (Serial.available()) {
        Serial.read();

        delay(10);

        Serial.println(1);
        delay(10);

        regelbetrieb(LoadCell, LoadCell2, LoadCell3, LoadCell4);

        Serial.println(messwert);
        delay(10);

        Serial.println(messwert2);
        delay(10);

        Serial.println(messwert3);
        delay(10);

        Serial.println(messwert4);
    }
}

float epsilonumgebung(float hierMesswert, float hierMesswertAlt) {
    if (hierMesswert < hierMesswertAlt + 0.25 && hierMesswert > hierMesswertAlt - 0.25) {
        hierMesswert = hierMesswertAlt;
    }
    return hierMesswert;
}

void setup() {
    Serial.begin(9600);

    LoadCell.begin();
    LoadCell2.begin();
    LoadCell3.begin();
    LoadCell4.begin();

    LoadCell.start(stabilisation, t_are);
    LoadCell2.start(stabilisation, t_are);
    LoadCell3.start(stabilisation, t_are);
    LoadCell4.start(stabilisation, t_are);

    LoadCell.update();
    LoadCell2.update();
    LoadCell3.update();
    LoadCell4.update();

    Offset = getDaten(0, LoadCell);
    Offset2 = getDaten(0, LoadCell2);
    Offset3 = getDaten(0, LoadCell3);
    Offset4 = getDaten(0, LoadCell4);

    delay(500);
}

void loop() {
    writeInData();
}
