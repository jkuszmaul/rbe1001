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
/*
Drive::~Drive() {
  left_.detach();
  right_.detach();
}
*/

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
  if (left_power < 0) leftraw = (180 - leftraw);
  if (right_power < 0) rightraw = (180 - rightraw);
  
  if (left_power == 0) leftraw = (kStartDead + kEndDead) / 2;
  if (right_power == 0) rightraw = (kStartDead + kEndDead) / 2;

  /*
  Serial.print("Req: ");
  Serial.print(right_power * 600);
  Serial.print(" Left Out: ");
  Serial.print((int)leftraw);
  Serial.print(" Right Out: ");
  Serial.print((int)rightraw);
  */

  left_.write(leftraw);
  right_.write(rightraw);
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

void Drive::Clear() {
  correction_ = 1.0;
}

void Drive::Step() {
  double kPSpeed = 1.0;
  double kPRatio = 0.01;
  double speed = kPSpeed * (goal_ - (LeftEnc() + RightEnc()) / 2.0);
  double rightv = RightVel();
  double leftv = LeftVel();
  prev_left_ = LeftEnc();
  prev_right_ = RightEnc();
  Serial.print(leftv);
  Serial.print(" ");
  Serial.print(rightv);
  Serial.print(" ");
  double vratio = (leftv == rightv) ? 1.0 : leftv / rightv;
  double rratio = (lratio_ == rratio_) ? 1.0 : lratio_ / rratio_;
  double ratioerror = rratio - vratio;
  ratioerror = (ratioerror > 1.0) ? 1.0 : ratioerror < -1.0 ? -1.0 : ratioerror;
  correction_ += kPRatio * ratioerror;
  correction_ = (correction_ > 2.0) ? 2.0 : correction_ < 0.5 ? 0.5 : correction_;
  Serial.print(" Correction: ");
  Serial.print(correction_);
  double left_power = speed * lratio_;// * correction_;
  double right_power = speed *  rratio_;
  if (rratio > 0) left_power *= correction_;
  else right_power *= correction_;
  // Scale so that max power = +/-1.0.
  if (right_power) right_power /= abs(left_power);
  if (left_power) left_power /= abs(left_power);
  if (abs(right_power) > 1.0) {
    left_power /= abs(right_power);
    right_power /= abs(right_power);
  }
  Serial.print(" Left Power: ");
  Serial.print(left_power);
  Serial.print(" Right Power: ");
  Serial.print(right_power);
  Serial.println();
  WriteMotors(left_power, right_power);
}
