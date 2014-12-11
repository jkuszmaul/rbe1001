#include <Servo.h>
#include "Encoder.h"
#include "drive.h"

Encoder *left;
Encoder *right;
Drive *drive;

void setup() {
  //right = new Encoder(3, 18);
  //left = new Encoder(19, 20);
  drive = new Drive(6, 7, 1.0);
  Serial.begin(115200);
  //right->write(0);
  //left->write(0);
}

void loop() {
  drive->Straight(300.0);
  for (int i = 0; i < 100; i++) {
    drive->Step();
    delay(10);
  }
  drive->Stop();
  delay(1000);
  return;
  for (double i = 0; i < 1.0; i+=0.01) {
    drive->WriteMotors(i, i);
    delay(300);
    int rstart = drive->RightEnc();//right->read();
    int lstart = drive->LeftEnc();//left->read();
    delay(100);
    int rend = drive->RightEnc();//right->read();
    int lend = drive->LeftEnc();//left->read();
    Serial.print(" ");
    Serial.print(i);
    Serial.print(" Right Vel: ");
    Serial.print((rend - rstart) * 10);
    Serial.print(" Left Vel: ");
    Serial.print((lend - lstart) * 10);
    Serial.println();
  }
  delay(100);
}
