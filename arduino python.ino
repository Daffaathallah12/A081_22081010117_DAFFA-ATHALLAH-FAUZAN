/*
iTCLab Internet-Based Temperature Control Lab Firmware
Jeffrey Kantor, Initial Version
John Hedengren, Modified
Oct 2017
Basuki Rahmat, Modified
April 2022

Firmware untuk iTCLab ESP32.
Fungsi:
- Mengontrol pemanas (Q1, Q2) dengan rentang 0-100%.
- Membaca suhu (T1, T2) dan memberikan output dalam derajat Celcius.
- Menampilkan versi firmware.
- Mengontrol LED dengan PWM.
- Melakukan shutdown jika suhu melebihi batas tertentu.
*/

#include <Arduino.h>

// Konstanta
const String vers = "1.04";               // Versi firmware
const int baud = 115200;                  // Kecepatan baud serial
const char sp = ' ';                      // Pemisah perintah
const char nl = '\n';                     // Terminator perintah
const int pinT1 = 34;                     // Pin untuk T1
const int pinT2 = 35;                     // Pin untuk T2
const int pinQ1 = 32;                     // Pin untuk Q1
const int pinQ2 = 33;                     // Pin untuk Q2
const int pinLED = 26;                    // Pin untuk LED
const int freq = 5000;                    // Frekuensi PWM
const int ledChannel = 0;                 // Saluran PWM untuk LED
const int Q1Channel = 1;                  // Saluran PWM untuk Q1
const int Q2Channel = 2;                  // Saluran PWM untuk Q2
const int resolution = 8;                 // Resolusi PWM
const double upper_temperature_limit = 59.0; // Batas suhu maksimum (°C)
const int n = 10;                         // Jumlah pengukuran suhu untuk averaging

// Variabel Global
char Buffer[64];                          // Buffer untuk input serial
String cmd;                               // Perintah dari serial
double pv = 0;                            // Nilai parameter perintah
float level = 0;                          // Level LED (0-100%)
double Q1 = 0;                            // Nilai Q1 (0-100%)
double Q2 = 0;                            // Nilai Q2 (0-100%)
int iwrite = 0;                           // Nilai integer untuk PWM

// Fungsi untuk parsing serial input
void parseSerial(void) {
    int ByteCount = Serial.readBytesUntil(nl, Buffer, sizeof(Buffer) - 1);
    if (ByteCount > 0) {
        Buffer[ByteCount] = '\0'; // Tambahkan null terminator
        String read_ = String(Buffer);
        memset(Buffer, 0, sizeof(Buffer)); // Reset buffer

        // Pisahkan perintah dan data
        int idx = read_.indexOf(sp);
        if (idx > 0) {
            cmd = read_.substring(0, idx);
            cmd.trim();
            cmd.toUpperCase();
            String data = read_.substring(idx + 1);
            data.trim();
            pv = data.toFloat();
        }
    }
}

// Fungsi untuk mengeksekusi perintah
void dispatchCommand(void) {
    if (cmd == "Q1") {
        Q1 = max(0.0, min(100.0, pv));
        iwrite = int(Q1 * 2.55); // Konversi ke rentang 0-255
        ledcWrite(Q1Channel, iwrite);
        Serial.println(Q1);
    } else if (cmd == "Q2") {
        Q2 = max(0.0, min(100.0, pv));
        iwrite = int(Q2 * 2.55);
        ledcWrite(Q2Channel, iwrite);
        Serial.println(Q2);
    } else if (cmd == "T1") {
        float mV = 0.0, degC = 0.0;
        for (int i = 0; i < n; i++) {
            mV += analogRead(pinT1) * 0.322265625; // ADC ke tegangan (mV)
        }
        degC = (mV / n) / 10.0; // Konversi ke derajat Celcius
        Serial.println(degC);
    } else if (cmd == "T2") {
        float mV = 0.0, degC = 0.0;
        for (int i = 0; i < n; i++) {
            mV += analogRead(pinT2) * 0.322265625;
        }
        degC = (mV / n) / 10.0;
        Serial.println(degC);
    } else if (cmd == "VER") {
        Serial.println("TCLab Firmware Version " + vers);
    } else if (cmd == "LED") {
        level = max(0.0, min(100.0, pv));
        iwrite = int(level * 2.55);
        ledcWrite(ledChannel, iwrite);
        Serial.println(level);
    } else if (cmd == "X") {
        ledcWrite(Q1Channel, 0);
        ledcWrite(Q2Channel, 0);
        Serial.println("Stop");
    }
}

// Fungsi untuk memeriksa suhu dan mematikan pemanas jika terlalu tinggi
void checkTemp(void) {
    float mV = (float) analogRead(pinT1) * 0.322265625;
    float degC = mV / 10.0;
    if (degC >= upper_temperature_limit) {
        ledcWrite(Q1Channel, 0);
        ledcWrite(Q2Channel, 0);
        Serial.println("High Temp 1: " + String(degC));
    }

    mV = (float) analogRead(pinT2) * 0.322265625;
    degC = mV / 10.0;
    if (degC >= upper_temperature_limit) {
        ledcWrite(Q1Channel, 0);
        ledcWrite(Q2Channel, 0);
        Serial.println("High Temp 2: " + String(degC));
    }
}

// Fungsi setup
void setup() {
    Serial.begin(baud);
    while (!Serial) {
        ; // Tunggu hingga serial terhubung
    }

    // Konfigurasi saluran PWM
    ledcSetup(Q1Channel, freq, resolution);
    ledcAttachPin(pinQ1, Q1Channel);
    ledcSetup(Q2Channel, freq, resolution);
    ledcAttachPin(pinQ2, Q2Channel);
    ledcSetup(ledChannel, freq, resolution);
    ledcAttachPin(pinLED, ledChannel);

    // Set nilai awal PWM ke 0
    ledcWrite(Q1Channel, 0);
    ledcWrite(Q2Channel, 0);
    ledcWrite(ledChannel, 0);
}

// Fungsi loop utama
void loop() {
    parseSerial();
    dispatchCommand();
    checkTemp();
}
