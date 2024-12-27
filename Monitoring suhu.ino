#include <Arduino.h>

#define T1       34
#define T2       35
#define LED      26
#define Q1       32
#define Q2       33

float cel, cel1, degC, degC1;
const float batas_suhu_atas = 55;

void setup() {
  Serial.begin(115200);
  
  // Konfigurasi PWM untuk ESP32
  ledcSetup(0, 5000, 8); // Channel 0, frekuensi 5 kHz, resolusi 8 bit
  ledcSetup(1, 5000, 8); // Channel 1
  ledcSetup(2, 5000, 8); // Channel 2
  
  ledcAttachPin(LED, 0);
  ledcAttachPin(Q1, 1);
  ledcAttachPin(Q2, 2);
}

void Q1on() {
  ledcWrite(1, 128); // Duty cycle 50% pada channel 1
}

void Q1off() {
  ledcWrite(1, 0); // Matikan PWM pada channel 1
}

void Q2on() {
  ledcWrite(2, 128); // Duty cycle 50% pada channel 2
}

void Q2off() {
  ledcWrite(2, 0); // Matikan PWM pada channel 2
}

void ledon() {
  ledcWrite(0, 255); // Nyalakan LED (100% duty cycle)
}

void ledoff() {
  ledcWrite(0, 0); // Matikan LED
}

void cektemp() {
  degC = analogRead(T1) * (3.3 / 4095.0); // Sesuaikan dengan resolusi ADC
  cel = degC / 10.0;
  degC1 = analogRead(T2) * (3.3 / 4095.0);
  cel1 = degC1 / 10.0;

  Serial.print("Temperature: ");
  Serial.print(cel);
  Serial.print("°C");
  Serial.print("  ~  ");
  Serial.print(cel1);
  Serial.println("°C");
}

void loop() {
  cektemp();
  
  if (cel > batas_suhu_atas) {
    Q1off();
    ledon();
  } else {
    Q1on();
    ledoff();
  }
  
  if (cel1 > batas_suhu_atas) {
    Q2off();
    ledon();
  } else {
    Q2on();
    ledoff();
  }
  
  delay(100);
}
