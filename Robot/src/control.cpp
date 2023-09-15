#include <Arduino.h>
#include "robot.h"
#include "state_machines.h"
//#include "util.h"
#include "IRLine.h"

IRLine_t IRLine;

state_machines_t state_machines;

class main_fsm_t: public state_machine_t
{
  virtual void next_state_rules(void) 
  {
    robot.tis = millis() - robot.tes;

    //robot.setState(4);

    // Rules for the state evolution
    /*if(robot.state == start && robot.tof_dist > 0.10 && robot.prev_tof_dist < 0.05 && robot.tis > 3000){
      robot.rel_s = 0;
      //robot.setState(start);
    }*/if(robot.state == STOP){
      robot.IRLine.crosses = 0;
      robot.rel_theta = 0;
      //robot.setState(FLINE);
      
    } else if(robot.state == FLINE && robot.IRLine.crosses >= 1){
      robot.IRLine.crosses = 0;
      robot.rel_theta = 0;
      robot.setState(STOP);
      
    } else if(robot.state == TLEFT && robot.rel_theta > radians (70)){
      robot.IRLine.crosses = 0;
      robot.setState(STOP);

      
    } else if(robot.state == TRIGHT && robot.rel_theta < radians (-70)){
      robot.IRLine.crosses = 0;
      robot.setState(STOP);  
      
      
    } else if(robot.state == PICKB && robot.tof_dist < 0.04){ 
      robot.setState(41); 
      
      
    } else if(robot.state == 41 && robot.tis > 1000){
      robot.rel_s = 0;
      robot.setState(42); 
      
      
    } else if(robot.state == 42 && robot.rel_s < -0.08){
      robot.rel_theta = 0;
      robot.setState(43); 
      
      
    } else if(robot.state == 43 && robot.rel_theta < radians(-160)){
      robot.rel_theta = 0;
      robot.IRLine.crosses = 0;
      robot.setState(START); 

    } else if(robot.state == PICKBV && robot.tof_dist < 0.04){ 
      robot.setState(51); 
      
      
    } else if(robot.state == 51 && robot.tis > 1700){
      robot.rel_s = 0;
      robot.setState(52); 
      
      
    } else if(robot.state == 52 && robot.rel_s < -0.17){
      robot.rel_theta = 0;
      robot.setState(53); 
      
      
    } else if(robot.state == 53 && robot.rel_theta < radians(-160)){
      robot.rel_theta = 0;
      robot.IRLine.crosses = 0;
      robot.setState(START);

      
    // } else if(robot.state == 14 && IRLine.crosses >= 1){
    //   robot.rel_s = 0;
    //   robot.rel_theta = 0;
    //   robot.setState(stop); 

      
    } else if(robot.state == DROPB && robot.tis > 1000){
      robot.rel_s = 0;
      robot.rel_theta = 0;
      //robot.setState(22);
      robot.setState(21);

      
    } else if(robot.state == 21 && robot.rel_s < -0.08){
    robot.rel_theta = 0;
    robot.setState(22);

      
    } else if(robot.state == 22 && robot.rel_theta < radians(-160)){
      robot.IRLine.crosses = 0;
      robot.rel_theta = 0;
      robot.setState(START);
      

      
    // } else if(robot.state == 23 && IRLine.crosses >= 1){
    //   IRLine.crosses = 0;
    //   robot.setState(stop);
    
      
    } else if(robot.state == LEFTLINE && robot.rel_theta > radians(70)){
      robot.IRLine.crosses = 0;
      robot.rel_theta = 0;
      robot.setState(FLINE);
    
      
    } else if(robot.state == RIGHTLINE && robot.rel_theta < radians (-70)){
      robot.IRLine.crosses = 0;
      robot.rel_theta = 0;
      robot.setState(FLINE);
    
 
    } else if(robot.state == START && robot.tof_dist > 0.10 && robot.prev_tof_dist < 0.05 && robot.tis > 3000){
      //robot.setState(STOP);
    
 
    } else if(robot.state == DROPBV && robot.tis > 1500){
      robot.rel_s = 0;
      robot.rel_theta = 0;
      //robot.setState(22);
      robot.setState(31);

      
    } else if(robot.state == 31 && robot.rel_s < -0.15){
    robot.rel_theta = 0;
    robot.setState(32);

      
    } else if(robot.state == 32 && robot.rel_theta < radians(-160)){
      robot.IRLine.crosses = 0;
      robot.rel_theta = 0;
      robot.setState(START);

    } else if(robot.state == TURNBACK && robot.rel_theta < radians(-160)){
      robot.IRLine.crosses = 0;
      robot.rel_theta = 0;
      robot.setState(101);

    } else if(robot.state == 102 && robot.tis > 50){
      robot.IRLine.crosses = 0;
      robot.rel_theta = 0;
      robot.setState(START);
    }
 
    // ============================================================================================
    // SAIDAS DOS ESTADOS

    if (robot.state == STOP){
      //robot.setRobotVW(0,0);

    } else if (robot.state == FLINE){  
      robot.rel_theta = 0;                  
      robot.followLine();
    
    } else if (robot.state == TLEFT) {  
      //robot.rel_theta = 0;
      robot.setRobotVW(0.04, 1.6); //w was 2
       

    } else if (robot.state == TRIGHT) {  
      robot.setRobotVW(0.04, -1.6); //w was 2
       

    } else if (robot.state == PICKB) {  //follow until find box
      robot.followLine();
       

    } else if (robot.state == 41) {  //iman on and get box
      robot.solenoid_PWM = 180;
      robot.followLine();
       

    } else if (robot.state == 42) {  //go back a little
      robot.setRobotVW(-0.12, 0); //v was 0.1
       

    } else if (robot.state == 43) {  //turn 180
      robot.setRobotVW(0, -2.5); //w was 3



    } else if (robot.state == PICKBV) {  //follow until find box
       robot.followLine();
       

    } else if (robot.state == 51) {  //iman on and get box
      robot.solenoid_PWM = 180;
      robot.followLine();
       

    } else if (robot.state == 52) {  //go back a little
      robot.setRobotVW(-0.12, 0); //v was 0.1
       

    } else if (robot.state == 53) {  //turn 180
      robot.setRobotVW(0, -1.5); //w was 3
       

    //  } else if (robot.state == 14) {  //follow until find cross
    //    robot.followLine();
       

    } else if (robot.state == DROPB) {  
      //  robot.followLineRight(, robot.follow_v, robot.follow_k);
      robot.followLine();

    } else if (robot.state == 21) {  //iman off and go back
      robot.solenoid_PWM = 0;
      robot.setRobotVW(-0.1, 0);
       

    } else if (robot.state == 22) {  //turn 180
      robot.setRobotVW(0, -2.5); // w was 3
       

    //  } else if (robot.state == 23) {  
    //    robot.followLine();   
       

    } else if (robot.state == LEFTLINE) {  
      robot.setRobotVW(0.02, 2.5); //was v = 0.02 and w = 2.5
       

    } else if (robot.state == RIGHTLINE) {  
      robot.setRobotVW(0.02, -2.5); //was v = 0.02 and w = 2.5
       

    } else if (robot.state == START) {         // Robot Stoped            
      robot.solenoid_PWM = 0;
      robot.setRobotVW(0, 0);

    } else if (robot.state == DROPBV) {  
      //  robot.followLineRight(, robot.follow_v, robot.follow_k);
      robot.followLine();

    } else if (robot.state == 31) {  //iman off and go back
      robot.solenoid_PWM = 0;
      robot.setRobotVW(-0.1, 0);
      
    } else if (robot.state == 32) {  //turn 180
      robot.setRobotVW(0, -2.5); // w was 3

    } else if (robot.state == TURNBACK) {  //turn 180
      robot.setRobotVW(0, -2.5); // w was 3
    }
   

    // ==========================================
    
      if (robot.state == 50) { //Test
      robot.setRobotVW(0.1, 0);      

    } else if (robot.state == 100) {
      robot.v_req = 0;
      robot.w_req = 0;
      robot.solenoid_PWM = 180;

    } else if (robot.state == 101) {
      robot.v_req = 0;
      robot.w_req = 0;
      robot.rel_theta = 0;
      robot.IRLine.crosses = 0;

    } else if (robot.state == 102) {
      robot.v_req = 0;
      robot.w_req = 0;

    } else if (robot.state == 200) {
      robot.PWM_1 = 0;
      robot.PWM_2 = 0;

    } else if (robot.state == 201) {
      robot.PWM_1 = robot.PWM_1_req;
      robot.PWM_2 = robot.PWM_2_req;
    
    } else if (robot.state == 202) {
      robot.PWM_1 = robot.PWM_1_req;
      robot.PWM_2 = robot.PWM_2_req;      
    }   
  };  
};

main_fsm_t main_fsm;

void init_control(robot_t& robot)
{
  robot.pfsm = &main_fsm;
  robot.setState(START);
  //robot.setState(100);
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