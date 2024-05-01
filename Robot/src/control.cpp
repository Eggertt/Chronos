#include <Arduino.h>
#include "robot.h"
#include "state_machines.h"
//#include "util.h"
#include "IRLine.h"
#include "config.h"

IRLine_t IRLine;

state_machines_t state_machines;



class main_fsm_t: public state_machine_t
{

  // MODO FNR
  virtual void next_state_rules(void) 
  {
    
    robot.tis = millis() - robot.tes;
    
    // Rules for the state evolution
    /*if(robot.state == start && robot.tof_dist > 0.10 && robot.prev_tof_dist < 0.05 && robot.tis > 3000){
      robot.rel_s = 0;
      //robot.setState(start);

    }*/if(robot.state == GO){
      robot.IRLine.crosses = 0;
      robot.rel_theta = 0;
      //robot.setState(FLINE);
    
    }else if(robot.state == FLINE && robot.IRLine.crosses >= 1){
      robot.IRLine.crosses = 0;
      robot.rel_theta = 0;
      robot.rel_s = 0;

      robot.setState(GO);
    
    }else if(robot.state == 60 && robot.rel_s > 0.03){
      robot.setState(TLEFT);

    }else if(robot.state == TLEFT && robot.rel_theta > radians (90)){
      robot.IRLine.crosses = 0;

      Serial.print("State:");
      Serial.println(robot.state);

      if(robot.wasDropPick == 0){
        robot.setState(GO);

      }else if(robot.wasDropPick == 1){
        // robot.integral = 0;
        robot.setState(FLINE);
        robot.wasDropPick = 0;

      }  
    
    }else if(robot.state == 70 && robot.rel_s > 0.03){
      robot.setState(TRIGHT);

    }else if(robot.state == TRIGHT && robot.rel_theta < radians (-90)){
      robot.IRLine.crosses = 0;

      Serial.print("State:");
      Serial.println(robot.state);

      if(robot.wasDropPick == 0){
        robot.setState(GO);

      }else if(robot.wasDropPick == 1){
        robot.setState(FLINE);
        robot.wasDropPick = 0;

      }  
    
    }else if(robot.state == PICKB && robot.tof_dist < 0.04){ 

      Serial.print("State:");
      Serial.println(robot.state);
      robot.setState(41); 
    
    }else if(robot.state == 41 && robot.tis > robot.tis_DROPB){ //1000
      robot.rel_s = 0;

      Serial.print("State:");
      Serial.println(robot.state);
      robot.setState(42); 
    
    }else if(robot.state == 42 && robot.rel_s < -robot.rel_sDROPB){ //0.08
      robot.rel_theta = 0;

      Serial.print("State:");
      Serial.println(robot.state);
    //   robot.setState(43); 
    
    // }
    // else if(robot.state == 43 && robot.rel_theta > radians(170)){
      robot.rel_theta = 0;
      robot.IRLine.crosses = 0;

      robot.setState(GO); 
    
    }
    else if(robot.state == PICKG && robot.tof_dist < 0.04){ 

      Serial.print("State:");
      Serial.println(robot.state);
      robot.setState(51); 
    
    }else if(robot.state == 51 && robot.tis > robot.tis_DROPG){ //1700
      robot.rel_s = 0;

      Serial.print("State:");
      Serial.println(robot.state);
      robot.setState(52); 
    
    }else if(robot.state == 52 && robot.rel_s < -robot.rel_sDROPG){ //0.17
      robot.rel_theta = 0;

      Serial.print("State:");
      Serial.println(robot.state);
    //   robot.setState(53); 
    
    // }
    // else if(robot.state == 53 && robot.rel_theta < radians(-170)){
      robot.rel_theta = 0;
      robot.IRLine.crosses = 0;

      robot.setState(GO);
    
    }else if(robot.state == DROPB && robot.tis > robot.tis_DROPB){
      robot.rel_s = 0;
      robot.rel_theta = 0;
      //robot.setState(22);

      Serial.print("State:");
      Serial.println(robot.state);
      robot.setState(21);
    
    }else if(robot.state == 21 && robot.rel_s < -robot.rel_sDROPB){
      robot.rel_theta = 0;

      Serial.print("State:");
      Serial.println(robot.state);
      //robot.setState(22);
    
    //}
    //else if(robot.state == 22 && robot.rel_theta > radians(170)){
      robot.IRLine.crosses = 0;
      robot.rel_theta = 0;

      robot.setState(GO);

    
    }
    else if(robot.state == LEFTLINE && robot.rel_theta > radians(80)){
      robot.IRLine.crosses = 0;
      robot.rel_theta = 0;

      Serial.print("State:");
      Serial.println(robot.state);
      robot.setState(FLINE);
    
    }else if(robot.state == RIGHTLINE && robot.rel_theta < radians (-80)){
      robot.IRLine.crosses = 0;
      robot.rel_theta = 0;

      Serial.print("State:");
      Serial.println(robot.state);
      robot.setState(FLINE);
    
    } 
    
    else if(robot.state == DROPG && robot.tis > robot.tis_DROPG){
      robot.rel_s = 0;
      robot.rel_theta = 0;

      Serial.print("State:");
      Serial.println(robot.state);
      robot.setState(31);
    
    }else if(robot.state == 31 && robot.rel_s < -robot.rel_sDROPG){
    robot.rel_theta = 0;

    Serial.print("State:");
    Serial.println(robot.state);
    //robot.setState(32);
    
    //}else if(robot.state == 32 && robot.rel_theta < radians(-170)){
      robot.IRLine.crosses = 0;
      robot.rel_theta = 0;

      robot.setState(GO);
    
    }else if(robot.state == TURNBACK && robot.rel_theta < radians(-190)){
      robot.IRLine.crosses = 0;
      robot.rel_theta = 0;

      if(robot.wasDropPick == 0){
        robot.setState(GO);

      }else if(robot.wasDropPick == 1){
        robot.setState(FLINE);
        robot.wasDropPick = 0;
      }
    
    } else if(robot.state == 102){

    } else if(robot.state == TEST){

    }
    









    // ============================================================================================
    
    // SAIDAS DOS ESTADOS
    if (robot.state == GO){
      //robot.setRobotVW(0,0);
    
    }else if(robot.state == FLINE){  
      
      robot.rel_theta = 0;                  
      robot.followLine();
    
    }else if(robot.state == TLEFT){  
      //robot.rel_theta = 0;
      robot.setRobotVW(robot.rightLeft_v, -robot.rightLeft_w); //v was 0.02 and w was w
    
    }else if(robot.state == TRIGHT){  
      robot.setRobotVW(0, robot.rightLeft_w); //v was 0.02 and w was 2
    
    }else if(robot.state == PICKB) {  //follow until find box
      robot.followLine();
    
    }else if(robot.state == 41){  //iman on and get box
      robot.solenoid_PWM = 180;
      robot.followLine();
    
    }else if(robot.state == 42){  //go back a little
      robot.setRobotVW(-0.1, 0); //v was 0.12
    
    }else if(robot.state == 43){  //turn 180
      robot.setRobotVW(0, -2.5); //w was 3
    
    }else if(robot.state == PICKG){  //follow until find box
       robot.followLine();
    
    }else if(robot.state == 51){  //iman on and get box
      robot.solenoid_PWM = 180;
      robot.followLine();
    
    }else if(robot.state == 52){  //go back a little
      robot.setRobotVW(-0.1, 0); //v was 0.12
    
    }else if(robot.state == 53){  //turn 180
      robot.setRobotVW(0, 2.5); //w was 3
    //  } else if (robot.state == 14) {  //follow until find cross
    //    robot.followLine();
    
    }else if(robot.state == DROPB){  
      //  robot.followLineRight(, robot.follow_v, robot.follow_k);
      robot.followLine();
    
    }else if(robot.state == 21){  //iman off and go back
      robot.solenoid_PWM = 0;
      robot.setRobotVW(-0.1, 0);
    
    }else if(robot.state == 22){  //turn 180
      robot.setRobotVW(0, -2.5); // w was 3
    //  } else if (robot.state == 23) {  
    //    robot.followLine();   
    
    }else if(robot.state == LEFTLINE){  
      robot.setRobotVW(robot.rightLeftLine_v, -robot.rightLeftLine_w); //was v = 0.02 and w = 2.5
    
    }else if(robot.state == RIGHTLINE){  
      robot.setRobotVW(robot.rightLeftLine_v, robot.rightLeftLine_w); //was v = 0.02 and w = 2.5
    
    }else if(robot.state == START){         // Robot GOed            
      robot.solenoid_PWM = 0;
      robot.setRobotVW(0, 0);
    
    }else if(robot.state == DROPG){  
      //  robot.followLineRight(, robot.follow_v, robot.follow_k);
      robot.followLine();
    
    }else if(robot.state == 31){  //iman off and go back
      robot.solenoid_PWM = 0;
      robot.setRobotVW(-0.1, 0);
    
    }else if(robot.state == 32){  //turn 180
      robot.setRobotVW(0, 2.5); // w was 3
    
    }else if(robot.state == TURNBACK){  //turn 180
      robot.setRobotVW(0, 2.5); // w was 3

    }else if(robot.state == 102){
      robot.followLine();
      //robot.setRobotVW(0.1, 0);
    }else if(robot.state == TEST){

    }
  };


};

main_fsm_t main_fsm;

void init_control(robot_t& robot)
{
  robot.pfsm = &main_fsm;
  robot.setState(START);
  //robot.setState(TEST);
  //robot.setState(102);


  //main_fsm.set_new_state(8);
  //main_fsm.update_state();
}

void control(robot_t& robot)
{
  robot.control_mode = cm_pid;
  main_fsm.calc_next_state();
  main_fsm.update_state();
  main_fsm.do_enter_state_actions();
  main_fsm.do_state_actions();
}