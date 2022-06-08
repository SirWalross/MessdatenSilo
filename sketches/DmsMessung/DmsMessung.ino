#include <HX711_ADC.h>

// Ein Objekt mit dem Konstruktor aus der Bibliothek erstellen
HX711_ADC LoadCell(2, 3);
HX711_ADC LoadCell2(6, 7);
HX711_ADC LoadCell3(8, 9);
HX711_ADC LoadCell4(12, 13);

// Globale Variablen intialisieren

static float Offset  = 0;
static float Offset2 = 0;
static float Offset3 = 0;
static float Offset4 = 0;

int anzahl         = 100;
int caltime        = 2;
float calFac       = 2429;
float calFac2      = 2406;
float calFac3      = 2442;
float calFac4      = 2454;
long stabilisation = 2000;
boolean t_are      = true;
float messwertges;
float messwertges2;
float messwertges3;
float messwertges4;

float messwert;
float messwert2;
float messwert3;
float messwert4;

void regelbetrieb(HX711_ADC zelle, HX711_ADC zelle2, HX711_ADC zelle3, HX711_ADC zelle4) {
    float messwertAlt  = messwert;
    float messwertAlt2 = messwert2;
    float messwertAlt3 = messwert3;
    float messwertAlt4 = messwert4;
    messwert           = 0;
    messwert2          = 0;
    messwert3          = 0;
    messwert4          = 0;

    for (int i = 1; i <= anzahl; i++) {
        // Eingänge der Pins neu auslesen

        zelle.update();
        zelle2.update();
        zelle3.update();
        zelle4.update();

        // Messwert aufsummieren

        messwert  = messwert + (getDaten(Offset, zelle));
        messwert2 = messwert2 + (getDaten(Offset2, zelle2));
        messwert3 = messwert3 + (getDaten(Offset3, zelle3));
        messwert4 = messwert4 + (getDaten(Offset4, zelle4));

        // messwert=getDaten(Offset);
        // Serial.println(messwert);
        // 1 Millisekunden warten((1/1000) Sekunde)
        delay(1);
    }

    // Berechne Mittelwert der letzten 1000 Messwerte aus der
    // letzten Sekunde

    messwert  = messwert / anzahl;
    messwert2 = messwert2 / anzahl;
    messwert3 = messwert3 / anzahl;
    messwert4 = messwert4 / anzahl;

    messwert  = messwert / calFac;
    messwert2 = messwert2 / calFac2;
    messwert3 = messwert3 / calFac3;
    messwert4 = messwert4 / calFac4;

    // Methode für schreiben in Datenbank
    //  messwert=epsilonumgebung(messwert,messwertAlt);
    //  messwert2=epsilonumgebung(messwert2,messwertAlt2);
    //  messwert3=epsilonumgebung(messwert3,messwertAlt3);
    //  messwert4=epsilonumgebung(messwert4,messwertAlt4);

    // Es fehlt noch die kalbrierte Umrechnung auf Mikrostrain.
    //  Serial.println("Die Daten betragen ");
    //  Serial.println(messwert,1);
    //  Serial.println(Offset,1);
    // delay(1000);
}
// Methode zum berechnen der Spannung
float getDaten(float offset, HX711_ADC zelle) {
    float Offsetnew = offset;

    // Hilfsvariablen initialisieren
    float data;

    // Methode getRareData() aus der Bibliothek abzüglich dem vorher
    // gemessenen Offset-Wert

    data = (float)zelle.getRareData() - Offsetnew;

    // Gemessene Spannung als Rückgabewert

    return data;
}

// Hier steht die Methode zum schreiben in eine Datenbank

void writeInData() {
    // Wenn eine serielle Verbindung verfügbar ist soll das Programm ausgefuehrt
    // werden
    if (Serial.available()) {
        // Der Arduino wartet auf eine Anfrage vom Pi
        Serial.read();
        // Damit der Pi die Gelegenheit bekommt sich auf das Einlesen der Messwerte
        // vorzubereiten wird ein Delay von 10ms eingeführt
        delay(10);

        Serial.println(1);
        delay(10);
        // In diesem Testskript wird eine zufällige Zahl zwischen 0 und 100
        // festgelegt

        regelbetrieb(LoadCell, LoadCell2, LoadCell3, LoadCell4);

        // Die Zahl wird dann an den Pi übermittelt
        // Serial.print("Zelle 1   ");
        Serial.println(messwert);
        delay(10);
        // Serial.print("Zelle 2   ");
        Serial.println(messwert2);
        delay(10);
        // Serial.print("Zelle 3    ");
        Serial.println(messwert3);
        delay(10);
        // Serial.print("Zelle 4    ");
        Serial.println(messwert4);
        delay(10);
    }
}

float epsilonumgebung(float hierMesswert, float hierMesswertAlt) {
    if (hierMesswert < hierMesswertAlt + 0.25 && hierMesswert > hierMesswertAlt - 0.25) {
        hierMesswert = hierMesswertAlt;
    }
    return hierMesswert;
}
// In der Setup steht die bestimmung des Nullpunkts

void setup() {
    Serial.begin(9600);

    // Serial.println(Offset);

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

    // Messwert aufsummieren

    Offset  = getDaten(0, LoadCell);
    Offset2 = getDaten(0, LoadCell2);
    Offset3 = getDaten(0, LoadCell3);
    Offset4 = getDaten(0, LoadCell4);

    delay(500);

    /*
     //Eingänge der Pins neu auslesen
     float Offsetsum;
     caltime=caltime*1000;
     for (int i=1; i<=caltime; i++){
     delay(1);
  }
   Offset=Offsetsum/caltime;
   */
}
// In der loop steht der Regelbetrieb der Waage

void loop() {
    writeInData();
}

// mögliche Nachbesserungen
// millis() läuft nach 50 Tagen über und geht zurück auf 0
// millis(): https://www.arduino.cc/reference/de/language/functions/time/millis/