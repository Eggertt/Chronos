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

#include "state_machines.h"
#include <Arduino.h>

state_machine_t::state_machine_t()
{
  
}

// Set new state
void state_machine_t::set_new_state(int astate)
{
  new_state = astate;
}


void state_machine_t::update_state(void)
{
  if (state != new_state) {  // if the state changed 'tis' is reset
    prev_state = state;
    state = new_state;
    tes = millis();
    tis = 0;
    count = 0;
  }
}

void state_machine_t::calc_next_state(void)
{
  tis = millis() - tes;
  next_state_rules();
}


void state_machine_t::do_enter_state_actions(void)
{
  enter_state_actions_rules();
}

void state_machine_t::do_state_actions(void)
{
  state_actions_rules();
  count++;
}



int state_machines_t::register_state_machine(pstate_machine_t psm)
{
  return 0;
}  
  
void state_machines_t::calc_next_states(void)
{

}

void state_machines_t::do_states_actions(void)
{

}
  
void state_machines_t::do_global_actions(void)
{

}