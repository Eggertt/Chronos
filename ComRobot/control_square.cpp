#include <Arduino.h>
#include "robot.h"
#include "state_machines.h"

state_machines_t state_machines;

class main_fsm_t: public state_machine_t
{
  virtual void next_state_rules(void) 
  {
    // Rules for the state evolution
     if(state == 0 && robot.d[0] > 0.2 && robot.last_d[0] < 0.1) {
      set_new_state(1);

    } else if(state == 1 && tis > 1000) {
      set_new_state(2);

    } else if(state == 2 && tis > 1000) {
      set_new_state(3);

    } else if(state == 3 && tis > 1000) {
      set_new_state(4);

    } else if(state == 4 && tis > 1000) {
      set_new_state(1);      

    } else if(state == 202 && tis > 2000) {
      set_new_state(200);
    }

  };


  virtual void state_actions_rules(void)
  {
    int i;
    // Actions in each state
    if (state == 0) {         // Robot Stoped            
      robot.control_mode = cm_kinematics;
      robot.setRobotVVnWReq(0, 0, 0);

    } else if (state == 1) {   
      robot.setRobotVVnWReq(0.2, 0, 0);
      robot.setServoPulse(1.8);

    } else if (state == 2) {   
      robot.setRobotVVnWReq(0, 0.2, 0);

    } else if (state == 3) {   
      robot.setRobotVVnWReq(-0.2, 0, 0);
      robot.setServoPulse(1.5);

    } else if (state == 4) {   
      robot.setRobotVVnWReq(0, -0.2, 0);

    } else if (state == 10) { //Test
      robot.setRobotVVnWReq(0.1, 0, 0);      

    }
  };  
};


main_fsm_t main_fsm;

void init_control(robot_t& robot)
{
  robot.pfsm = &main_fsm;
  main_fsm.set_new_state(0);
  main_fsm.update_state();
}



void control(robot_t& robot)
{
  main_fsm.calc_next_state();
  main_fsm.update_state();
  main_fsm.do_enter_state_actions();
  main_fsm.do_state_actions();
}
