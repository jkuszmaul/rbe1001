#include "drive.h"

Drive::Drive(int left, int right, double reduction) :
  red_(reduction),
  left_enc_(19, 20),
  right_enc_(3, 18),
  correction_(1.0)
{
  left_.attach(left);
  right_.attach(right);
}

void Drive::WriteMotors(double left_power, double right_power) {
  const int kMax = 140;
  const int kMin = 40;
  const int kStartDead = 88;
  const int kEndDead = 98;
  // Inputs 0 - 600; outputs 98-139, inclusive.
  const double kc0 = 96.523;
  const double kc1 = 7.13e-2;
  const double kc2 = -3e-4;
  const double kc3 = 5e-7;
  double left1 = abs(left_power * 600.0);
  double left2 = left1 * left1;
  double left3 = left2 * left1;
  double right1 = abs(right_power * 600.0);
  double right2 = right1 * right1;
  double right3 = right2 * right1;
  double leftraw = kc0 + kc1 * left1 + kc2 * left2 + kc3 * left3;
  double rightraw = kc0 + kc1 * right1 + kc2 * right2 + kc3 * right3;
  if (left_power < 0) leftraw *= -1;
  if (right_power < 0) rightraw *= -1;
  
  if (left_power == 0) leftraw = (kStartDead + kEndDead) / 2;
  if (right_power == 0) rightraw = (kStartDead + kEndDead) / 2;

  Serial.print("Req: ");
  Serial.print(right_power * 600);
  Serial.print(" Left Out: ");
  Serial.print((int)leftraw);
  Serial.print(" Right Out: ");
  Serial.print((int)rightraw);

  left_.write((int)leftraw);
  right_.write((int)rightraw);
}

void Drive::Stop() {
  WriteMotors(0.0, 0.0);
}

void Drive::Arc(double dist, double radius/*metersl +=left -=right*/) {
  if (radius != radius) return; // Check for nans.
  double kRadius = .10;
  left_enc_.write(0);
  right_enc_.write(0);
  double leftRadius = radius - kRadius;
  double rightRadius = radius + kRadius;
  lratio_ = leftRadius / radius;// left speed / speed.
  rratio_ = rightRadius / radius;// right speed / speed.
  if (leftRadius == radius) lratio_ = 1.0; // Handle inf.
  if (rightRadius == radius) rratio_ = 1.0; // Handle inf.
  goal_ = dist;
}

void Drive::Straight(double dist) {
  Arc(dist, 1.0f / 0.0f);
}

void Drive::Step() {
  Serial.println();
  double kPSpeed = 1.0;
  double kPRatio = 1.0;
  double speed = kPSpeed * (goal_ - (LeftEnc() + RightEnc()) / 2.0);
  double rightv = RightVel();
  double leftv = LeftVel();
  prev_left_ = LeftEnc();
  prev_right_ = RightEnc();
  Serial.print(rightv);
  Serial.print(" ");
  Serial.print(leftv);
  Serial.print(" ");
  double vratio = (leftv == rightv) ? 1.0 : leftv / rightv;
  double ratioerror = lratio_ / rratio_ - vratio;
  correction_ += kPRatio * ratioerror;
  Serial.print(" Correction: ");
  Serial.print(correction_);
  double left_power = speed * lratio_ * correction_;
  double right_power = speed * rratio_;
  // Scale so that max power = +/-1.0.
  if (right_power) right_power /= abs(left_power);
  if (left_power) left_power /= abs(left_power);
  if (abs(right_power) > 1.0) {
    left_power /= abs(right_power);
    right_power /= abs(right_power);
  }
  Serial.println();
  WriteMotors(left_power, right_power);
}
