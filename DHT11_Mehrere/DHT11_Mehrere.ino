
#include <DHT.h>  // DHT-Bibliothek einbinden

#define DHT1_Pin 6   // Datenpin des DHT11(1) ist Pin 6
#define DHT2_Pin 8   // Datenpin des DHT11(2) ist Pin 8
#define DHT3_Pin 10  // Datenpin des DHT11(3) ist Pin 10
#define DHT4_Pin 13  // Datenpin des DHT11(3) ist Pin 13

// Je nachdem, ob Sensoren vom Typ DHT11 oder DHT 22 benutzt werden die entsprechende "DHT_Typ"-Zeile benutzen
// Hier wird der DHT11 benutzt

#define DHT_Typ DHT11  // DHT 11 als Sensortyp festlegen
//#define DHT_Typ DHT22              // DHT 22 als Sensortyp festlegen

DHT dht1(DHT1_Pin, DHT_Typ);  // Sensor 1 initialisieren
DHT dht2(DHT2_Pin, DHT_Typ);  // Sensor 2 initialisieren
DHT dht3(DHT3_Pin, DHT_Typ);  // Sensor 3 initialisieren
DHT dht4(DHT4_Pin, DHT_Typ);  // Sensor 4 initialisieren

void setup() {
    Serial.begin(9600);
    dht1.begin();  // Sensor 1 starten
    dht2.begin();  // Sensor 2 starten
    dht3.begin();  // Sensor 3 starten
    dht4.begin();  // Sensor 4 starten
}

void loop() {
    if (Serial.available()) {
        Serial.read();
        // Da das Display nur 2 Zeilen hat, werden die 3 Sensoren nacheinander ausgelesen und angezeigt

        float h1 = dht1.readHumidity();     // Auslesen der Luftfeuchtigkeit (Sensor 1)
        float t1 = dht1.readTemperature();  // Auslesen der Temperatur (Sensor 2)

        // Serial.print("T1   ");
        Serial.println(t1, 2);
        delay(10);

        float h2 = dht2.readHumidity();     // Auslesen der Luftfeuchtigkeit (Sensor 2)
        float t2 = dht2.readTemperature();  // Auslesen der Temperatur (Sensor 2)
        // Serial.print("T2   ");
        Serial.println(t2, 2);
        delay(10);

        float h3 = dht3.readHumidity();     // Auslesen der Luftfeuchtigkeit (Sensor 2)
        float t3 = dht3.readTemperature();  // Auslesen der Temperatur (Sensor 2)
        // Serial.print("T3   ");
        Serial.println(t3, 2);
        delay(10);

        float h4 = dht4.readHumidity();
        float t4 = dht4.readTemperature();
        // Serial.print("T4   ");
        Serial.println(t4, 2);
        delay(10);
    }
}
