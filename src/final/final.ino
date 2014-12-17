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
#define LINEL 0
#define LINEM 1
#define LINER 2

Servo lift1, lift2;
Servo intake;
Servo block;
Servo bass;
Servo leftm, rightm;

void setup() {
  lift1.attach(CONVEYOR1, 1000, 2000);
  lift2.attach(CONVEYOR2, 1000, 2000);
  leftm.attach(LEFT, 1000, 2000);
  rightm.attach(RIGHT, 1000, 2000);
  block.attach(BLOCK);
  bass.attach(BASS);
  intake.attach(INTAKE, 1000, 2000);
  Serial.begin(115200);
}

double averageDist(Drive drivebase) {
  return (drivebase.RightEnc() + drivebase.LeftEnc()) / 2.0;
}

void autonomous(unsigned long time) {
  while (0 == ppm.getChannel(1)) continue;
  pinMode(32, INPUT_PULLUP);
  unsigned long startTime = millis();
  writeBlock(true);

  time *= 1000;
  if (digitalRead(32)) {
    Serial.println("Running Boring Auto.");


  dropBass(false);
  writeMotors(90, 90);
  delay(1000);
  double threshold = 450;
  int last_left = false;
  unsigned long endTime = millis() + 10000;
  while (millis() < endTime) {
    bool lefton = analogRead(LINEL) < threshold;
    bool midon = analogRead(LINEM) < threshold;
    bool righton = analogRead(LINER) < threshold;
    if (midon && !lefton && !righton)
      writeMotors(90, 90);
    else if (midon && lefton && !righton) {
      writeMotors(20, 90);
    }
    else if (midon && righton && !lefton) {
      writeMotors(90, 20);
    }
    else if (lefton && !midon && !righton) {
      writeMotors(0, 90);
    }
    else if (righton && !midon && !lefton) {
      writeMotors(90, 0);
    }
    else if (!midon && !righton && !lefton) {
    }
    else // just outsides on, or all off.
      writeMotors(90, 90);
    Serial.print(analogRead(2));
    Serial.print(" ");
    Serial.print(analogRead(1));
    Serial.print(" ");
    Serial.print(analogRead(0));
    Serial.print(" ");
    Serial.println();
    delay(100);
  }
  writeMotors(-90, -90);
  delay(500);
  writeMotors(90, -90);
  delay(500);
  writeIntake(true);
  writeLift(true);
  writeBlock(false);
  writeMotors(70, 70);
  delay(4000);
  writeMotors(-40, -40);
  delay(2000);
  writeMotors(0, 0);
  }
  else {
    leftm.detach();
    rightm.detach();
    drive = new Drive(LEFT, RIGHT, 0.0693 * PI / 360.0);
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

    drive->left_.detach();
    drive->right_.detach();
    leftm.attach(LEFT);
    rightm.attach(RIGHT);
  }


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
  if (on) intake.write(10);
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
    /*for (int i = 1; i <= 6; i++) {
      Serial.print(ppm.getChannel(i));
      Serial.print(" \t");
    }*/
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
    leftm.write(180 - ppm.getChannel(3));
    rightm.write(180 - ppm.getChannel(2));
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

void writeMotors(double left, double right) {
  leftm.write(left + 90);
  rightm.write(right + 90);
}

void loop() {
  dropBass(false);
  writeBlock(false);
  /*
  writeMotors(90, 90);
  delay(1000);
  double threshold = 450;
  int last_left = false;
  unsigned long endTime = millis() + 10000;
  while (millis() < endTime) {
    bool lefton = analogRead(LINEL) < threshold;
    bool midon = analogRead(LINEM) < threshold;
    bool righton = analogRead(LINER) < threshold;
    if (midon && !lefton && !righton)
      writeMotors(90, 90);
    else if (midon && lefton && !righton) {
      writeMotors(20, 90);
    }
    else if (midon && righton && !lefton) {
      writeMotors(90, 20);
    }
    else if (lefton && !midon && !righton) {
      writeMotors(0, 90);
    }
    else if (righton && !midon && !lefton) {
      writeMotors(90, 0);
    }
    else if (!midon && !righton && !lefton) {
    }
    else // just outsides on, or all off.
      writeMotors(90, 90);
    Serial.print(analogRead(2));
    Serial.print(" ");
    Serial.print(analogRead(1));
    Serial.print(" ");
    Serial.print(analogRead(0));
    Serial.print(" ");
    Serial.println();
    delay(100);
  }
  writeMotors(-90, -90);
  delay(500);
  writeMotors(90, -90);
  delay(400);
  writeIntake(true);
  writeLift(true);
  writeBlock(false);
  writeMotors(90, 90);
  delay(2000);
  exit(0);*/
  Serial.println("Start Auto");
  autonomous(20);
  Serial.println("Start Teleop");
  teleop(180);
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
