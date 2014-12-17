#include <PPM.h>
#include <Servo.h>
#include "Encoder.h"
#include "drive.h"

PPM ppm(2);

Drive *drive;


// 8 and 9 don't work.
#define LEFT 10
#define RIGHT 11
#define CONVEYOR1 6
#define CONVEYOR2 7
#define INTAKE 5
#define BASS 4
#define BLOCK 13

Servo lift1, lift2;
Servo intake;
Servo block;
Servo bass;

void setup() {
  lift1.attach(CONVEYOR1, 1000, 2000);
  lift2.attach(CONVEYOR2, 1000, 2000);
  block.attach(BLOCK);
  bass.attach(BASS);
  intake.attach(INTAKE, 1000, 2000);
  drive = new Drive(LEFT, RIGHT, 0.0693 * PI / 360.0);
  Serial.begin(115200);
}

double averageDist(Drive drivebase) {
  return (drivebase.RightEnc() + drivebase.LeftEnc()) / 2.0;
}

void autonomous(unsigned long time) {
  while (0 == ppm.getChannel(1)) continue;

  unsigned long startTime = millis();

  time *= 1000;

  while (millis() - startTime <= time) {
    // Run Auto!
    //Serial.println("Autonomous");
    //delay(50);
  }
}

void writeLift(bool on) {
  if (on) {
    lift1.write(0);
    lift2.write(180);
  }
  else {
    lift1.write(90);
    lift2.write(90);
  }
}

void writeIntake(bool on) {
  if (on) intake.write(0);
  else intake.write(90);
}

void writeBlock(bool on) {
  if (on) block.write(0);
  else block.write(180);
}

void dropBass(bool drop) {
  if (drop) bass.write(140);
  else bass.write(0);
}

void teleop(unsigned long time) {
  unsigned long startTime2 = millis();
  time *= 1000;
  dropBass(false);
  while (millis() - startTime2 <= time) {
    for (int i = 1; i <= 6; i++) {
      Serial.print(ppm.getChannel(i));
      Serial.print(" \t");
    }
    if (ppm.getChannel(5) < 30) {
      writeLift(true);
      writeIntake(true);
      Serial.print("Intaking");
      Serial.print(" \t");
    }
    else {
      writeLift(false);
      writeIntake(false);
      Serial.print("Not Intake");
      Serial.print(" \t");
    }
    Serial.println();
    if (ppm.getChannel(6) < 30) writeBlock(false);
    else writeBlock(true);
    //lift1.write(0);
    //lift2.write(180);
    // Run Teleop!
    double left_power = -(ppm.getChannel(3) - 90.0) / 90.0;
    double right_power = -(ppm.getChannel(2) - 90.0) / 90.0;
    Serial.print("Left: ");
    Serial.print(left_power);
    Serial.print("Right: ");
    Serial.println(right_power);
    drive->left_.write(180 - ppm.getChannel(3));
    drive->right_.write(180 - ppm.getChannel(2));
    delay(100);
  }
  exit(0);
}

void driveArc(double dist, double rad) {
  drive->Arc(dist, rad);
  while (abs(averageDist(*drive)) < abs(dist)) {
    drive->Step();
    delay(10);
    Serial.print("Dist: ");
    Serial.print(averageDist(*drive));
    Serial.print(" ");
  }
}

void driveStraight(double dist) {
  drive->Straight(dist);
  while (abs(averageDist(*drive)) < abs(dist)) {
    drive->Step();
    delay(10);
    Serial.print("Dist: ");
    Serial.print(averageDist(*drive));
    Serial.print(" ");
  }
}

void loop() {
  //autonomous(20);
  //teleop(180);
  writeLift(false);
  writeIntake(true);
  writeBlock(true);
  dropBass(false);
  /* Going backwards...
  dropBass(false);
  driveArc(-0.6, -0.5);
  driveArc(-0.6, 0.5);
  dropBass(true);
  driveStraight(-1.0);
  exit(0);
  */
  driveArc(1.8, 20);
  driveArc(0.06, 0.01);
  dropBass(true);
  driveArc(0.06, 0.01);
  driveStraight(0.4);
  exit(0);
  driveArc(0.35, 0.15);
  driveStraight(0.5);
  driveArc(0.45, -0.2);
  drive->Clear();
  driveStraight(0.8);
  driveArc(0.02, 0.01);
  dropBass(true);
  driveArc(0.01, 0.01);
  driveStraight(0.4);
  dropBass(false);
  driveArc(-0.5, 0.7);
  exit(0);
  for (double i = 0; i < 1.0; i+=0.01) {
    //drive->WriteMotors(i, i);
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
