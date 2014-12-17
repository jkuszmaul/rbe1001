#include <PPM.h>
#include <Servo.h>

PPM ppm(2);


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

void autonomous(unsigned long time) {
  while (0 == ppm.getChannel(1)) continue;
  pinMode(32, INPUT_PULLUP);
  unsigned long startTime = millis();
  writeBlock(true);

  time *= 1000;

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


  while (millis() - startTime <= time) {
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

void writeMotors(double left, double right) {
  leftm.write(left + 90);
  rightm.write(right + 90);
}

void loop() {
  dropBass(false);
  writeBlock(false);
  Serial.println("Start Auto");
  autonomous(20);
  Serial.println("Start Teleop");
  teleop(180);
}
