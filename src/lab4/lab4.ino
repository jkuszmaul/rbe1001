#include <Servo.h>
// Note:  S8 and S9 don't seem to work.

Servo left; // declare a servo to use for our left motor
Servo right;  //declare a servo to use for our right motor
Servo arm; // Arm.

// The maximum drivepower for either motor.
#define FULL_POWER 90
// The value to write to a motor if we want no movement.
#define ZERO_POWER 90

// Left and Right motor ports.
#define LEFT_MOTOR 6
#define RIGHT_MOTOR 7

// Input pin with left photoresistor.
#define LEFT_PHOTO 23
// Input pin with right photoresistor.
#define RIGHT_PHOTO 25

#define START_BUTTON 30

#define ARM_MOTOR 11
#define ARM_POT 9

const double kDown = -0.0;
const double kUp = 0.2;

// enum which we can use to indicate which side of the robot we are talking about.
enum side {
  LEFT,
  RIGHT,
  NEITHER,
  BOTH
} last_triggered, cur_triggered;

// A function which runs the motors at a given power, from -90 to +90 where zero
// is stopped and positive is forwards.
void writeMotors(int leftPower, int rightPower) {
  // Deals with converting a drivepower (0 - 90) into a number to actually write
  // to the servos.
  left.write(ZERO_POWER + leftPower);
  right.write(ZERO_POWER - rightPower);
}

double potToAngle(double pot /*0 - 1023*/);
double angleToPot(double angle /*radians*/);
double getAngle();
void goToAngle(double angle, double kP, double kI=0, double kD=0);

long long int start, endTime, latestTime;

void setup() {
  Serial.begin(115200); // For debugging.
  // Attach the left and right motors to the appropriate ports.
  left.attach(LEFT_MOTOR);
  right.attach(RIGHT_MOTOR);

  pinMode(LEFT_PHOTO, INPUT);
  pinMode(RIGHT_PHOTO, INPUT);
  
  pinMode(START_BUTTON, INPUT_PULLUP);

  // Arbitrarily chose which direction to start in.
  last_triggered = LEFT;

  arm.attach(ARM_MOTOR, 1000, 2000);
}

boolean first = true;
boolean needto = true;
void loop() {
  if (first) {
    Serial.println("HERE!!!!!");
    Serial.println(potToAngle(470));
    Serial.println(angleToPot(0.0));
    waitForButton(START_BUTTON);
    goToAngle(kDown, 30.0, 5.0);
    waitForButton(START_BUTTON);
    goToAngle(kUp, 30.0, 5.0);
    first = false;
    start = millis();
    endTime = start + 12.5 * 1000;
    latestTime = endTime + 1*1000;
  }


  // Get the sensor values.
  // High if light.
  int left_photo = digitalRead(LEFT_PHOTO);
  int right_photo = digitalRead(RIGHT_PHOTO);

  // Debugging info.
  Serial.print("Left: ");
  Serial.print(left_photo);
  Serial.print(" Right: ");
  Serial.println(right_photo);

  if (left_photo && right_photo) { // straddling the line; no change.
    last_triggered = last_triggered;
    cur_triggered = NEITHER;
  }
  // If it is too late, stop and put it down. If it is too early, don't. If it is in between, the left and right photos should be triggered.
  else if (needto && ((!(left_photo || right_photo) && millis() > endTime && millis() > (latestTime)) || millis() > latestTime)) {
    cur_triggered = BOTH;
    needto = false;
  }
  else if ((!(left_photo || right_photo) && ((latestTime + 5000) < millis()))) {
    cur_triggered = BOTH;
    last_triggered = BOTH;
  }
  else if (!left_photo) { // left sensor on line; turn right.
    last_triggered = LEFT;
    cur_triggered = LEFT;
  }
  else { // right sensor on line; turn left.
    last_triggered = RIGHT;
    cur_triggered = RIGHT;
  }

  // Turn just one motor or the other to stay on the line.
  if (cur_triggered == BOTH) {
    if (last_triggered == BOTH) {
      writeMotors(0, 0);
      while (true) continue;
    }
    writeMotors(30, 30);
    goToAngle(kDown - 0.15, 30.0, 3.0); // Arm down = -0.8 rad; 300 analogRead.
    writeMotors(50, 0);
    delay(100);
  }
  else if (last_triggered == LEFT) {
    if (cur_triggered == NEITHER)
      writeMotors(0, 40);
    else
      writeMotors(0, 50);
  }
  else if (last_triggered == RIGHT) {
    if (cur_triggered == NEITHER)
      writeMotors(40, 0);
    else
      writeMotors(50, 0);
  }

}

const double kLowAngle = -PI / 4.0;
const double kLowInput = 300;
const double kHighAngle = PI / 2.0;
const double kHighInput = 850;

double potToAngle(double pot /*0 - 1023*/) {
  pot -= kLowInput;
  double intercept = kLowAngle;
  double slope = (kHighAngle - kLowAngle) / (kHighInput - kLowInput);
  double angle = intercept + slope * pot;
  return angle;
}

double angleToPot(double angle /*radians*/) {
  angle -= kLowAngle;
  double intercept = kLowInput;
  double slope = (kHighInput - kLowInput) / (kHighAngle - kLowAngle);
  double pot = intercept + slope * angle;
  return pot;
}

double getAngle() {
  return potToAngle(analogRead(ARM_POT));
}

void goToAngle(double angle/*radians*/, double kP, double kI, double kD) {
  double error = angle - getAngle();
  double prev_error = error;
  double sum = 0.0;
  while (abs(error) > 0.02 || abs(prev_error - error) > 0.001) {
    error = angle - getAngle();
    sum *= 0.99;
    sum += error;
    double out = kP * error + kI * sum;
    out = (out > 90) ? 90 : ((out < -90) ? -90 : out);
    Serial.print(getAngle());
    Serial.print("\t");
    Serial.println(out);
    arm.write(90 - out);
    prev_error = error;
    delay(10);
  }
  arm.write(90);
}


// Blocks until the button on port port has gone low, then delays half a second
// to allow the user to get their hand away, and returns.
void waitForButton(int port) {
  while (digitalRead(port)) continue;
  delay(500);
  return;
}
