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

const float kDown = -0.11;
const float kUp = 0.2;

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
void loop() {
  if (first) {
    Serial.println("HERE!!!!!");
    Serial.println(potToAngle(470));
    Serial.println(angleToPot(0.0));
    waitForButton(START_BUTTON);
    //arm.write(120);
    delay(50);
    goToAngle(kDown, 10.0);//, 2.0, 50.0);
    delay(1000);
    Serial.println(getAngle());
    waitForButton(START_BUTTON);
    goToAngle(kUp, 10.0, 1.0, 3.0);
    delay(1000);
    first = false;
    start = millis();
    endTime = start + 11.6 * 1000;
    latestTime = endTime + 1*1000;
  }
  Serial.println(getAngle());


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
  else if (millis() > endTime && millis() < latestTime) {
    cur_triggered = BOTH;
  }
  else if ((!(left_photo || right_photo) && ((latestTime + 2) < millis()))) {
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
    // Go down extra far, because we will be driving anyways.
    goToAngle(kDown - 0.15, 30.0, 3.0);
    writeMotors(50, 25);
    delay(600);
  }
  else if (last_triggered == LEFT) {
    if (cur_triggered == NEITHER)
      writeMotors(0, 40); // 30, 40
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
  int millisgood = 0;
  int startgood = 0;
  int start = millis();
  double error = angle - getAngle();
  double prev_error = error;
  double sum = 0.0;
  while ((abs(error) > 0.03) || (millisgood < 1000)) {
    if (millis() > start + 5000) break;
    error = angle - getAngle();
    sum *= 0.99;
    sum += error;
    double diff = error - prev_error;
    double out = kP * error + kI * sum + kD * diff;
    out = (out > 90) ? 90 : ((out < -90) ? -90 : out);
    Serial.print(getAngle());
    Serial.print("\t");
    arm.write(90 - out);
    delay(10);
    if (abs(error) > 0.03)
      millisgood = 0;
    else if (abs(prev_error) > 0.03)
      startgood = millis();
    else // error is good and has been good.
      millisgood = millis() - startgood;
    Serial.print(out);
    Serial.print("\t");
    Serial.println(millisgood);
    prev_error = error;
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
