/* Copyright (c) 2023  Paulo Costa
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE. */

#ifndef ROBOT_H
  #define ROBOT_H
#endif

#include <Arduino.h>
#include <math.h>
#include "PID.h"
#include "IRLine.h"
#include "state_machines.h"
#include "gchannels.h"

#ifndef NUM_WHEELS
#define NUM_WHEELS 2

typedef enum { 
  cm_pwm,
  cm_pid
} control_mode_t;

typedef enum{
  NORTH,   // 0
  EAST,    // 1
  SOUTH,   // 2
  WEST     // 3
}compass;

class robot_t {
  public:
  int enc1, enc2;
  int Senc1, Senc2;
  float w1e, w2e;
  float v1e, v2e;
  float ve, we;
  float ds, dtheta;
  float rel_s, rel_theta;
  float xe, ye, thetae;

  bool wasDropPick = 0;

  int *mypath;
  int mypathsize;

  int compassCount = 0;

  byte state;
  uint32_t tis, tes;
  
  float dt;
  float v, w;
  float v_req, w_req;
  float dv_max, dw_max;
  
  
  float wheel_radius, wheel_dist;
  
  float v1ref, v2ref;
  float w1ref, w2ref;
  float u1, u2;
  int PWM_1, PWM_2;
  int PWM_1_req, PWM_2_req;
  control_mode_t control_mode;

  double  IRkp = 20, IRki = 1, IRkd = 30;
  double prevError = 0; // Erro anterior para cada sensor
  double integral = 0; // Integral para cada sensor
  double derivative = 0; // Derivada para cada sensor

  float follow_v, follow_k;
  float rightLeft_v = 0.06, rightLeft_w = 2; // for tright and tleft
  float rightLeftLine_v = 0.08, rightLeftLine_w = 3; //for leftline and rightline

  float refreshRate = 0.04; //original is 0.04

  bool lastDeliveryException = 0;

  float rel_sDROPB = 0.105, rel_sDROPG = 0.19;
  float tis_DROPB = 1000, tis_DROPG = 2100;
  float tleftAngle = 80;
  
  PID_t PID[NUM_WHEELS];
  float battery_voltage;
  int button_state;

  int solenoid_PWM;
  bool hasBox;

  IRLine_t IRLine;
  float tof_dist, prev_tof_dist;

  int LastTouchSwitch, TouchSwitch;
  state_machine_t* pfsm;
  gchannels_t* pchannels;
  
  robot_t();

  void setState(byte new_state);
  
  void odometry(void);
  void setRobotVW(float Vnom, float Wnom);

  void accelerationLimit(void);
  void VWToMotorsVoltage(void);

  void followLineRight(float Vnom, float K);
  void followLineLeft(float Vnom, float K);
  void followLine();

  void send_command(const char* command, float par);
  void send_command(const char* command, const char* par);

  int IR_sum();
};

extern robot_t robot;

#endif // ROBOT_H
