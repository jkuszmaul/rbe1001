#include <Servo.h>
#include "Encoder.h"

class Drive {
 public:
   Drive(int left, int right, double reduction);
   void Stop();
   void Arc(double dist, double radius);
   void Straight(double dist);
   void Step();
   void WriteMotors(double left_power, double right_power); // -1.0 to 1.0
   double LeftEnc() { return left_enc_.read() * red_; }
   double RightEnc() { return right_enc_.read() * red_; }
 private:
   double LeftVel() { return (LeftEnc() - prev_left_) / kStepTime; }
   double RightVel() { return (RightEnc() - prev_right_) / kStepTime; }

   double lratio_;
   double rratio_;
   double goal_; // meters.
   double correction_; // left / right such that same output = same speed.
   // Previous positions; for velocity calcs.
   double prev_right_;
   double prev_left_;
   double kStepTime = 0.01; // seconds.

   static const int kNoMotion;
   static const int kRadius; //radius of drivetrain.

   double red_; // = meters / tick.
   Servo left_, right_;
   Encoder left_enc_, right_enc_;
};
