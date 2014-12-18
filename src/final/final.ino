#include <PPM.h>
#include <Servo.h>
#include <avr/wdt.h>

PPM ppm(2);



// 8, 9, and 12 don't work.
// Ports for various motors and sensors.
#define LEFT 10
#define RIGHT 11
#define CONVEYOR1 6
#define CONVEYOR2 7
#define INTAKE 5
#define BLOCK 13
#define LINEL 0
#define LINEM 1
#define LINER 2

Servo lift1, lift2;
Servo intake;
Servo block;
Servo leftm, rightm;

void setup() {
  // Attach everything necessary.
  lift1.attach(CONVEYOR1, 1000, 2000);
  lift2.attach(CONVEYOR2, 1000, 2000);
  leftm.attach(LEFT, 1000, 2000);
  rightm.attach(RIGHT, 1000, 2000);
  block.attach(BLOCK);
  intake.attach(INTAKE, 1000, 2000);
  Serial.begin(115200);
#ifndef AUTO
  wdt_enable(WDTO_1S);
#endif
}

// autonomous loop.
void autonomous(unsigned long time) {
  // Wait for controller to connect.
  while (0 == ppm.getChannel(1)) continue;

  unsigned long startTime = millis();

  // Prevent balls from falling out.
  writeBlock(true);

  time *= 1000;

  // Go forwards to get out of starting zone.
  writeMotors(90, 90);
  delay(1000);
  // Informatino for line following.
  double threshold = 450;
  int last_left = false;

  // Spend 10 seconds line following.
  unsigned long endTime = millis() + 10000;
  while (millis() < endTime) {
    writeBlock(true);
#ifndef AUTO
    wdt_reset();
#endif
    // Check state of all line sensors. true = on line.
    bool lefton = analogRead(LINEL) < threshold;
    bool midon = analogRead(LINEM) < threshold;
    bool righton = analogRead(LINER) < threshold;
    // The algorithm is as follows: If only middle sensor is on, go straight
    // forwards; if the middle sensor and one of the side sensors is triggered,
    // turn gradually, but if only an edge sensor is triggered, turn more
    // sharply. If no sensors are on the line, fall through and continue doing
    // what we were last doing.
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
    else // just outsides on.
      writeMotors(90, 90);

    // Debugging info.
    Serial.print(analogRead(2));
    Serial.print(" ");
    Serial.print(analogRead(1));
    Serial.print(" ");
    Serial.print(analogRead(0));
    Serial.print(" ");
    Serial.println();
    delay(100);
  }

  // Backup to let bass drop.
  writeMotors(-90, -90);
  delay(500);
  // Turn and go straight to try and pick stuff up.
  writeMotors(90, -90);
  delay(600);
  writeIntake(true);
  writeLift(true);
  writeBlock(false);
  writeMotors(70, 70);
  delay(4000);
  writeMotors(-40, -40);
  delay(2000);
  writeMotors(0, 0);


  // Wait for auto to finish.
  while (millis() - startTime <= time) {
  }
}

// Helper functions. All of these are used to turn various intake and serovs on
// and off.

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

// Teleop code.
void teleop(unsigned long time) {
  unsigned long startTime2 = millis();
  time *= 1000;
  // Run until time runs out.
  while (millis() - startTime2 <= time) {
    if (ppm.getChannel(5) > 140) {
#ifndef AUTO
      while (true) continue;
#endif
    }
#ifndef AUTO
    else wdt_reset();
#endif
    /*for (int i = 1; i <= 6; i++) {
      Serial.print(ppm.getChannel(i));
      Serial.print(" \t");
    }*/
    // Back buttons are tied to intake and blocker respectively. Joystick
    // channels are tied straight to driving. Nothing too exciting.
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
    delay(50);
  }

  // Just exit.
  exit(0);
}

void writeMotors(double left, double right) {
  leftm.write(left + 90);
  rightm.write(right + 90);
}

void loop() {
  writeBlock(false);
  Serial.println("Start Auto");
#ifdef AUTO
  autonomous(20);
#endif
  Serial.println("Start Teleop");
  teleop(180);
}
