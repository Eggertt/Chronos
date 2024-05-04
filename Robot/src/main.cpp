// 2023 Paulo Costa
// Periodic Interrupts for Quadrature Encoder reading and serial commands example
// It also has the PWM generation code to drive an H-bridge like the DRV8212PDSGR

#include <Arduino.h>

#include <WiFi.h>

// #include <RPi_Pico_TimerInterrupt.h>
// #include <ArduinoOTA.h>

#include <WiFiUdp.h>
#include <LittleFS.h>

int destNode = -1;
bool provaMode, pacoMode;
bool stp1 = 0, stp2 = 0;

#define max_wifi_str 32

char ssid[max_wifi_str];
char password[max_wifi_str];

const char *fname_wifi = "/wifi.txt";

int udp_on, ip_on;

WiFiUDP Udp;
unsigned int localUdpPort = 4224; // local port to listen on
                    //port was 4224
#define UDP_MAX_SIZE 512
uint8_t UdpInPacket[UDP_MAX_SIZE];  // buffer for incoming packets
uint8_t UdpOutPacket[UDP_MAX_SIZE]; // buffer for outgoing packets
int UdpBufferSize = UDP_MAX_SIZE;

// Select the timer you're using, from ITimer0(0)-ITimer3(3)
// Init RPI_PICO_Timer
// RPI_PICO_Timer ITimer1(1);

#define ENC1_A 2
#define ENC1_B 3

#define ENC2_A 0
#define ENC2_B 1

#define TEST_PIN 27

#define digitalWriteFast(pin, val) (val ? sio_hw->gpio_set = (1 << pin) : sio_hw->gpio_clr = (1 << pin))
#define digitalReadFast(pin) (((1 << pin) & sio_hw->gpio_in) >> pin)

#define pinIsHigh(pin, pins) (((1 << pin) & pins) >> pin)

// bool timer_handler(struct repeating_timer *t)
//{
//
// }

// PWM stuff

// SENSOR DE INFRARED

#define RXD2 9 // Era IRrecvPIN
char irrecvbuffer[6];
char irrecvdata;
byte irrecvbuffer_index = -1;

#define MOTOR1A_PIN 16
#define MOTOR1B_PIN 17

#define MOTOR2A_PIN 14
#define MOTOR2B_PIN 15

#define SOLENOID_PIN_A 12
#define SOLENOID_PIN_B 13

int debug;

void setMotorPWM(int new_PWM, int pin_a, int pin_b)
{
  int PWM_max = 200;
  if (new_PWM >  PWM_max) new_PWM =  PWM_max;
  if (new_PWM < -PWM_max) new_PWM = -PWM_max;

  if (new_PWM == 0)
  { // Both outputs 0 -> A = H, B = H
    analogWrite(pin_a, 255);
    analogWrite(pin_b, 255);
  }
  else if (new_PWM > 0)
  {
    analogWrite(pin_a, 255 - new_PWM);
    analogWrite(pin_b, 255);
  }
  else
  {
    analogWrite(pin_a, 255);
    analogWrite(pin_b, 255 + new_PWM);
  }
}

#include <VL53L0X.h>
VL53L0X tof;

#include "robot.h"

void init_control(robot_t &robot);
void control(robot_t &robot);

PID_pars_t wheel_PID_pars;

#define ADC_IN_PIN 28

#define MUXA_PIN 18
#define MUXB_PIN 19
#define MUXC_PIN 20

#define TINY_CTRL_PIN 21

static void adc_set_channel(int channel)
{
  gpio_put_masked(digitalPinToBitMask(MUXA_PIN) | digitalPinToBitMask(MUXB_PIN) | digitalPinToBitMask(MUXC_PIN), channel << MUXA_PIN);
  // digitalWrite(MUXA_PIN, channel & 1);
  // digitalWrite(MUXB_PIN, (channel >> 1) & 1);
  // digitalWrite(MUXC_PIN, (channel >> 2) & 1);
}

uint16_t read_adc(int channel)
{
  adc_set_channel(channel); // Switch external MUX to the desired channel
  delayMicroseconds(100);
  return analogRead(A2); // The mux connects to analog input A2
}

void readIRSensors(void)
{
  byte c; // Read the five IR sensors using the AD converter
  for (c = 0; c < IRSENSORS_COUNT; c++)
  {
    robot.IRLine.IR_values[(IRSENSORS_COUNT - 1) - c] = 1023 - read_adc(3 + c);
  }
}

uint32_t encodeIRSensors(void)
{
  byte c;                                           // Encode five IR sensors with 6 bits for each sensor
  uint32_t result = robot.IRLine.IR_values[0] >> 4; // From 10 bits to 6 bits
  for (c = 1; c < 5; c++)
  {
    result = (result << 6) | (robot.IRLine.IR_values[c] >> 4);
  }
  return result;
}

uint32_t interval, last_cycle;
uint32_t loop_micros;

void set_interval(float new_interval)
{
  interval = new_interval * 1000000L; // In microseconds
  robot.dt = new_interval;            // In seconds
  wheel_PID_pars.dt = robot.dt;
}

// Remote commands

#include "gchannels.h"

gchannels_t udp_commands;
gchannels_t serial_commands;
commands_list_t commands_list;

const char *pars_fname = "pars.cfg";

void save_pars(void)
{
  File f;

  f = LittleFS.open(pars_fname, "w+");
  if (!f)
  {
    Serial.print("Error: could not create/open the file ");
    Serial.println(pars_fname);
    return;
  }

  // f.printf("kf %.6g;\n", wheel_PID_pars.Kf);
  // f.printf("kc %.6g;\n", wheel_PID_pars.Kc);
  // f.printf("ki %.6g;\n", wheel_PID_pars.Ki);
  // f.printf("kd %.6g;\n", wheel_PID_pars.Kd);

  f.printf("kf %.6g;\n", robot.IRkp);
  f.printf("kc %.6g;\n", robot.IRki);
  f.printf("ki %.6g;\n", robot.IRkd);

  f.close();
}

void load_pars(void)
{
  File f;
  f = LittleFS.open(pars_fname, "R");
  if (!f)
  {
    Serial.print("Error: could not create/open the file ");
    Serial.println(pars_fname);
    return;
  }

  int c;
  byte b;

  while (1)
  {
    c = f.read(&b, 1);
    if (c != 1)
      break;
    serial_commands.process_char(b);
  }
  f.close();
}


#include "direction.h"
#include "node.h"
#include "config.h"
#include "scheduler.h"

std::vector<std::string> instructions;
Scheduler scheduler(25);
int instructionCounter = 0;
int pathCounter = 0;
Box currentBox;
Box newBox;
Port currentPort;
Box boxes[4];

void process_command(command_frame_t frame)
{
  commands_list.process_command(frame);

  if (frame.command_is("mo"))
  { // The 'mo'de command ...
    robot.control_mode = (control_mode_t)frame.value;
  }
  else if (frame.command_is("st"))
  {
    robot.setState(frame.value);
    // robot.pfsm->update_state();
  }
  else if (frame.command_is("dt"))
  {
    set_interval(frame.value);
  }
  else if (frame.command_is("v"))
  {
    robot.v_req = frame.value;
  }
  else if (frame.command_is("w"))
  {
    robot.w_req = frame.value;
  }
  else if (frame.command_is("sl"))
  {
    robot.solenoid_PWM = frame.value;
  }



  else if (frame.command_is("kp"))
  {
    robot.IRkp = frame.value;
    serial_commands.send_command("kp", robot.IRkp);
  }
  else if (frame.command_is("ki"))
  {
    robot.IRki = frame.value;
    serial_commands.send_command("ki", robot.IRki);
  }
  else if (frame.command_is("kd"))
  {
    robot.IRkd = frame.value;
    serial_commands.send_command("kd", robot.IRkd);
  }



  else if (frame.command_is("testKp"))
  {
    robot.testKp = frame.value;
    serial_commands.send_command("testKp", robot.testKp);
  }
  else if (frame.command_is("testKi"))
  {
    robot.testKi = frame.value;
    serial_commands.send_command("testKi", robot.testKi);
  }
  else if (frame.command_is("testKd"))
  {
    robot.testKd = frame.value;
    serial_commands.send_command("testKd", robot.testKd);
  }



  else if (frame.command_is("pl"))
  {
    load_pars();
  }
  else if (frame.command_is("pr"))
  {
    serial_commands.send_command("kp", robot.IRkp);
    serial_commands.send_command("ki", robot.IRki);
    serial_commands.send_command("kd", robot.IRkd);
  }
  else if (frame.command_is("rlv"))
  { 
    robot.rightLeft_v = frame.value;
    serial_commands.send_command("rlv", robot.rightLeft_v);
  }
  else if (frame.command_is("rlw"))
  { 
    robot.rightLeft_w = frame.value;
    serial_commands.send_command("rlw", robot.rightLeft_w);
  }
  else if (frame.command_is("rllv"))
  { 
    robot.rightLeftLine_v = frame.value;
    serial_commands.send_command("rllv", robot.rightLeftLine_v);
  }
  else if (frame.command_is("rllw"))
  { 
    robot.rightLeftLine_w = frame.value;
    serial_commands.send_command("rllw", robot.rightLeftLine_w);
  }
  else if (frame.command_is("er"))
  { 
    robot.integral = frame.value;
    serial_commands.send_command("er", robot.integral);
  }
  else if (frame.command_is("blacks_lvl"))
  { 
    robot.IRLine.black_cross_level = frame.value;
    serial_commands.send_command("blacks_lvl", robot.IRLine.black_cross_level);
  }
  else if (frame.command_is("refreshRate"))
  { 
    robot.refreshRate = frame.value;
    serial_commands.send_command("refreshRate", robot.refreshRate);
  }
  else if (frame.command_is("box"))
  { 
    instructions.clear();
    instructionCounter = 0;
    pathCounter = 0;
    robot.integral = 0;
    robot.IRLine.crosses = 0;
    scheduler.resetPosition();
    strcpy(irrecvbuffer, frame.text);
    serial_commands.send_command("box", irrecvbuffer);
  }
  else if (frame.command_is("ES"))
  { // EEPROM Store
    save_pars(); 
  }
  else if (frame.command_is("fv"))
  {
    robot.follow_v = frame.value;
  }
  else if (frame.command_is("fk"))
  {
    robot.follow_k = frame.value;
  }
  else if (frame.command_is("rsdb"))
  {
    robot.rel_sDROPB = frame.value;
    serial_commands.send_command("rsdb", robot.rel_sDROPB);
  }
  else if (frame.command_is("rsdg"))
  {
    robot.rel_sDROPG = frame.value;
    serial_commands.send_command("rsdg", robot.rel_sDROPG);
  }
  else if (frame.command_is("tdb"))
  {
    robot.tis_DROPB = frame.value;
    serial_commands.send_command("tdb", robot.tis_DROPB);
  }
  else if (frame.command_is("tdg"))
  {
    robot.tis_DROPG = frame.value;
    serial_commands.send_command("tdg", robot.tis_DROPG);
  }
  else if (frame.command_is("output_w"))
  {
    robot.output_w = frame.value;
    serial_commands.send_command("output_w", robot.output_w);
  }
  else if (frame.command_is("ssid"))
  {
    strncpy(ssid, frame.text, max_wifi_str - 1);
    ssid[max_wifi_str - 1] = 0;
  }
  else if (frame.command_is("pass"))
  {
    strncpy(password, frame.text, max_wifi_str - 1);
    password[max_wifi_str - 1] = 0;

    WiFi.end();
    WiFi.begin(ssid, password);

  }
  // else if (frame.command_is("IRkp"))
  // {
  //   robot.IRkp = frame.value;
  //   serial_commands.send_command("kf", robot.IRkp);
  // } 
  // Put here more commands...
}

void init_PIO_dual_encoders(int enc1_pin_A, int enc2_pin_A);
int read_PIO_encoder(int sm);

void read_PIO_encoders(void)
{
  robot.enc1 = read_PIO_encoder(0);
  robot.enc2 = read_PIO_encoder(1);
}

SerialPIO SerialTiny(SerialPIO::NOPIN, 21);

void serial_write(const char *buffer, size_t size)
{
  Serial.write(buffer, size);
  if (udp_on)
  {
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(buffer, size);
    // Serial.print("Sent="); Serial.println(Udp.endPacket());
    Udp.endPacket();
  }
}

const char *encToString(uint8_t enc) {
  switch (enc) {
    case ENC_TYPE_NONE: return "NONE";
    case ENC_TYPE_TKIP: return "WPA";
    case ENC_TYPE_CCMP: return "WPA2";
    case ENC_TYPE_AUTO: return "AUTO";
  }
  return "UNKN";
}

void wifi_list(void)
{
  Serial.printf("Beginning scan at %d\n", millis());
  int cnt = WiFi.scanNetworks();
  if (!cnt)
  {
    Serial.printf("No networks found\n");
  }
  else
  {
    Serial.printf("Found %d networks\n\n", cnt);
    Serial.printf("%32s %5s %2s %4s\n", "SSID", "ENC", "CH", "RSSI");
    for (int i = 0; i < cnt; i++)
    {
      uint8_t bssid[6];
      WiFi.BSSID(i, bssid);
      Serial.printf("%32s %5s %2d %4d\n", WiFi.SSID(i), encToString(WiFi.encryptionType(i)), WiFi.channel(i), WiFi.RSSI(i));
    }
  }
}


void setup()
{
  Serial.println("setup iniciated");
  // Set the pins as input or output as needed
  pinMode(ENC1_A, INPUT_PULLUP);
  pinMode(ENC1_B, INPUT_PULLUP);
  pinMode(ENC2_A, INPUT_PULLUP);
  pinMode(ENC2_B, INPUT_PULLUP);

  pinMode(TEST_PIN, OUTPUT);

  // Motor driver pins
  pinMode(MOTOR1A_PIN, OUTPUT);
  pinMode(MOTOR1B_PIN, OUTPUT);

  pinMode(MOTOR2A_PIN, OUTPUT);
  pinMode(MOTOR2B_PIN, OUTPUT);

  pinMode(SOLENOID_PIN_A, OUTPUT);
  pinMode(SOLENOID_PIN_B, OUTPUT);

  init_PIO_dual_encoders(ENC1_A, ENC2_A);

  // ADC mux pins
  pinMode(MUXA_PIN, OUTPUT);
  pinMode(MUXB_PIN, OUTPUT);
  pinMode(MUXC_PIN, OUTPUT);

  analogReadResolution(10);

  commands_list.register_command("m1", &(robot.PWM_1_req)); // The 'm1' command sets the PWM value for motor 1
  commands_list.register_command("m2", &(robot.PWM_2_req)); // The 'm2' command sets the PWM value for motor 2

  udp_commands.init(process_command, serial_write);

  serial_commands.init(process_command, serial_write);

  robot.pchannels = &serial_commands;

  // Start the serial port with 115200 baudrate
  Serial.begin(115200);

  LittleFS.begin();
  File f;

  /*
  if (!LittleFS.exists(fname_wifi)) {
    f = LittleFS.open(fname_wifi, "w+");
    while (!f) {
        Serial.print("Error opening the file: ");
        Serial.println(fname_wifi);
        delay(500);
    }
   //f.printf("TP-LINK_1DC0\t");
    //f.printf("08840530\n");
    f.close();
  }*/

  f = LittleFS.open(fname_wifi, "r");
  if (!f)
  {
    Serial.println("file open failed");
  }
  int c;
  c = f.readBytesUntil('\t', ssid, max_wifi_str - 1);
  ssid[c] = 0;
  c = f.readBytesUntil('\n', password, max_wifi_str - 1);
  password[c] = 0;
  f.close();

  // strcpy(ssid, "FNR");
  // strcpy(password, "FNR-PCR-2024");
  strcpy(ssid, "eggert");
  strcpy(password, "semsenha");
  // strcpy(ssid, "5DPO-NETWORK");
  // strcpy(password, "5dpo5dpo");

  // Operate in WiFi Station mode
  WiFi.mode(WIFI_STA);

  // Start WiFi with supplied parameters
  WiFi.begin(ssid, password);

  // Print periods on monitor while establishing connection
  /*while (WiFi.status() != WL_CONNECTED) {
    Serial.print(" SSID: ");
    Serial.println(ssid);
    //delay(200);
    uint8_t b;
    if (Serial.available()) {  // Only do this if there is serial data to be read
      b = Serial.read();
      //serial_commands.process_char(b);
    }
  }*/

  // Connection established
  //Serial.println("");
  Serial.print("Pico W is connected to WiFi network ");
  Serial.println(WiFi.SSID());

  // Print IP Address
  Serial.print("Assigned IP Address: ");
  Serial.println(WiFi.localIP());

  ip_on = Udp.begin(localUdpPort);
  Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), localUdpPort);

  int i;
  // All wheeel PID controllers share the same parameters
  wheel_PID_pars.Kf = 12.5;
  wheel_PID_pars.Kc = 20;
  wheel_PID_pars.Ki = 0;
  wheel_PID_pars.Kd = 0.5;
  for (i = 0; i < NUM_WHEELS; i++)
  {
    robot.PID[i].init_pars(&wheel_PID_pars);
  }

  load_pars();

  // Serial2.begin(2400, SERIAL_8N1, RXD2);

  Serial2.setRX(RXD2);
  Serial2.begin(2400);
  SerialTiny.begin();

  // if (ITimer1.attachInterrupt(40000, timer_handler))
  //   Serial.println("Starting ITimer OK, millis() = " + String(millis()));
  // else
  //   Serial.println("Can't set ITimer. Select another freq. or timer");

  // init_OTA();

  // Wire.setSDA(8);
  // Wire.setSCL(9);

  Wire.begin();
  // tof.setAddress(0x22);

  tof.setTimeout(100);
  while (!tof.init())
  {
    Serial.println(F("Failed to detect and initialize VL53L0X!"));
    delay(100);
  }

  // Reduce timing budget to 20 ms (default is about 33 ms)
  // tof.setMeasurementTimingBudget(20000);

  // Start new distance measure
  tof.startContinuous(0);

  //refresh rate
  set_interval(robot.refreshRate); // In seconds | was 0.04
  init_control(robot);

  Serial.println("setup completed");
}

void loop()
{
  if (ip_on)
  {
    // ArduinoOTA.handle();

    int packetSize = Udp.parsePacket();
    if (packetSize)
    {
      int i;
      udp_on = 1;
      // receive incoming UDP packets

      // Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
      int len = Udp.read(UdpInPacket, UdpBufferSize - 1);
      if (len > 0)
      {
        UdpInPacket[len] = 0;
      }
      // Serial.printf("UDP packet contents (as string): %s\n", UdpInPacket);

      for (i = 0; i < len; i++)
      {
        udp_commands.process_char(UdpInPacket[i]);
        // Serial.write(UdpInPacket[i]);
        // digitalWrite(LED_BUILTIN, 1);
      }
    }
  }

  uint8_t b;
  if (Serial.available())
  { // Only do this if there is serial data to be read

    b = Serial.read();
    serial_commands.process_char(b);
    Serial.write(b);
  }

  if (SerialTiny.available())
  {
    b = SerialTiny.read();
    robot.battery_voltage = 1e-3 * ((b >> 1) * 50 + 4800);
    robot.button_state = b & 1;
  }

  // To measure the time between loop() calls
  // unsigned long last_loop_micros = loop_micros;

  // Do this only every "interval" miliseconds
  // It helps to clear the switches bounce effect
  uint32_t now = micros();
  uint32_t delta = now - last_cycle;
  if (delta >= interval)
  {
    loop_micros = micros();
    last_cycle = now;
    // last_cycle += interval;

    // Read and process sensors
    read_PIO_encoders();

    robot.odometry();
    // robot.battery_voltage = 7.4; // if it could not be measured...
    readIRSensors();

    robot.IRLine.calcIRLineEdgeLeft();
    robot.IRLine.calcIRLineEdgeRight();
    robot.IRLine.calcCrosses();

    if (tof.readRangeAvailable())
    {
      robot.prev_tof_dist = robot.tof_dist;
      robot.tof_dist = (tof.readRangeMillimeters() * 1e-3) - 0.02;
    }

    // Control the robot here by choosing:
    //   v_req and w_req          when robot.control_mode = cm_pid
    //   PWM_1_req and PWM_1_req  when robot.control_mode = cm_pwm
    control(robot);

    // Calc outputs
    robot.setRobotVW(robot.v_req, robot.w_req);

    // robot.accelerationLimit();
    robot.v = robot.v_req;
    robot.w = robot.w_req;
    robot.VWToMotorsVoltage();

    setMotorPWM(robot.PWM_1, MOTOR1A_PIN, MOTOR1B_PIN);
    setMotorPWM(robot.PWM_2, MOTOR2A_PIN, MOTOR2B_PIN);

    setMotorPWM(robot.solenoid_PWM, SOLENOID_PIN_A, SOLENOID_PIN_B);

    // pedro

    // if Start, wait for IR message
    if (robot.state == START)
    {

      // ========== provaMode ==========

      //Serial.printf("                                                                                                           Serial2.available() = ");
      //Serial.println(Serial2.available());
      if (Serial2.available()) {
          irrecvdata = Serial2.read();
          if(irrecvdata == 'U') {
            Serial.println("!\n");
          } else if(irrecvdata == 'W') {
            irrecvbuffer_index = 0;
          } else if(irrecvbuffer_index >= 0) {
            irrecvbuffer[irrecvbuffer_index] = irrecvdata;
            if(irrecvbuffer[irrecvbuffer_index] == 'w' || irrecvbuffer[irrecvbuffer_index] == 'o' || irrecvbuffer[irrecvbuffer_index] == 'u'){
              irrecvbuffer_index++;
              if(irrecvbuffer_index >= 4) {
                irrecvbuffer[irrecvbuffer_index] = '\0';
                irrecvbuffer_index = -1;
                Serial.println("Received Instruction!");
                Serial.println(irrecvbuffer);
                robot.setState(SETUP);
                robot.IRLine.crosses = 0;
              }
            } else {
              Serial.write(irrecvdata);
              //Serial.println();
            }
        }
      }

    }

    // if Setup, create boxes and start the setup for the Dijkstra
    if (robot.state == SETUP){
      for (size_t i = 0; i < 4; i++){
        Box box;
        box.pos = i;
        box.num = i;
        box.status = WAITING;
        if (irrecvbuffer[i] == 'w')
          box.color = RED;

        else if (irrecvbuffer[i] == 'u')
          box.color = GREEN;

        else if (irrecvbuffer[i] == 'o')
          box.color = BLUE;

        boxes[i] = box;
      }

      scheduler.setUp(boxes, 4);

      robot.setState(NEXT);
    }
    

    if (robot.state == NEXT){
      currentBox = scheduler.getBox();
      currentPort = scheduler.getAvailablePort(currentBox);
      instructions = scheduler.getRoute();
      Serial.print("Picking at: ");
      Serial.println(currentBox.pos);
      Serial.print("Dropping at: ");
      Serial.println(currentPort.pos);
      
      for (auto inst : instructions)
      {
        Serial.println(inst.c_str());
      }

      robot.setState(GO);
    }


    if (robot.state == GO)
    {

      //Serial.println("ESTOU NO SCHEDULER!");

      if (instructionCounter == instructions.size()) {
        if (currentBox.status == HOLDING) {
          instructions = scheduler.getRoute(currentBox, currentPort);
          
          Serial.print("Dropping at: ");
          Serial.println(currentPort.pos);

          
          for (auto inst : instructions)
          {
            Serial.println(inst.c_str());
          }
        } else {
          instructions = scheduler.getRoute();

          Serial.print("Picking at: ");
          Serial.println(currentBox.pos);
          
          for (auto inst : instructions)
          {
            Serial.println(inst.c_str());
          }

        }
          instructionCounter = 0;
          pathCounter = 0;
      }

      if(instructions[instructionCounter] == "Line" && robot.wasDropPick == 1 /*&& currentBox.pos != 19*/){
        Serial.println("Case TURNBACK AFTER DROP/PICK");
        robot.setState(TURNBACK);
        

      }
      else if (instructions[instructionCounter] == "Line" && (instructions[instructionCounter + 1] != "Drop" || instructions[instructionCounter + 1] != "Pick"))
      {
        Serial.println("Case FLINE");
        robot.setState(FLINE);
      } else if (pathCounter <= 0 && instructions[instructionCounter] == "Right" && (instructions[instructionCounter + 1] == "Left" && instructions[instructionCounter + 2] == "Pick")){
        instructions.insert(instructions.begin() + 2, "Line");
        Serial.println("CASO PROBLEMA1");
        pathCounter++;
        robot.setState(FLINE);
        
      } else if (instructions[instructionCounter] == "Right" && (instructions[instructionCounter + 1] == "Line" && instructions[instructionCounter + 2] == "Left" && instructions[instructionCounter + 3] == "Pick")){
        instructions.insert(instructions.begin() + 2, "Line");
        pathCounter++;
        Serial.println("CASO PROBLEMA2");
        robot.setState(FLINE);
        
      } else if (instructions[instructionCounter] == "Right" && (instructions[instructionCounter + 1] == "Line" && instructions[instructionCounter + 2] == "Line" && instructions[instructionCounter + 3] == "Left" && instructions[instructionCounter + 4] == "Pick")){
        Serial.println("CASO PROBLEMA3");
        instructions.insert(instructions.begin() + 2, "Line");
        pathCounter++;
        robot.setState(FLINE);

      } else if (instructions[instructionCounter] == "Line" && (instructions[instructionCounter + 1] == "Drop" || instructions[instructionCounter + 1] == "Pick")){
        Serial.println("Case GO");
        robot.setState(GO);

      } else if (instructions[instructionCounter] == "Left" && (instructions[instructionCounter + 1] == "Pick" || instructions[instructionCounter + 1] == "Drop")){
        Serial.println("Case TLEFT");
        //robot.setState(TLEFT);
        //if(instructions[instructionCounter + 1] == "Drop")robot.tleftAngle = 90;
        robot.setState(60);

        if (robot.compassCount == 0)
          robot.compassCount = 3;
        else
          robot.compassCount--;

        //Serial.printf("compassCount: %d\n", robot.compassCount);
      } else if (instructions[instructionCounter] == "Right" && (instructions[instructionCounter + 1] == "Pick" || instructions[instructionCounter + 1] == "Drop")){
        Serial.println("Case TRIGHT");
        //robot.setState(TRIGHT);
        robot.setState(70);

        if (robot.compassCount == 3)
          robot.compassCount = 0;
        else
          robot.compassCount++;

        //Serial.printf("compassCount: %d\n", robot.compassCount);
        
        
      } else if (instructions[instructionCounter] == "Pick"){
        if (currentBox.pos == 0 || currentBox.pos == 1 || currentBox.pos == 2 || currentBox.pos == 3){
          robot.setState(PICKB);
          robot.wasDropPick = 1;
          robot.compassCount = 2;
          //Serial.printf("compassCount: %d\n", robot.compassCount);

        } else if (currentBox.pos == 16 || currentBox.pos == 23 || currentBox.pos == 12 || currentBox.pos == 19){
          robot.setState(PICKG);
          robot.wasDropPick = 1;
          robot.compassCount = 1;

          if(currentBox.pos == 19){
            robot.lastDeliveryException = 1; 
            robot.tis_DROPB += 200;
          }
          //Serial.printf("compassCount: %d\n", robot.compassCount);
        }
        else
          Serial.println("Failed to start picking phase");

        currentBox.status = HOLDING;
        Serial.println("Case PICK");

        // Serial.print("TestPort "); Serial.print(scheduler.deliveryPorts[0].pos); Serial.print(".occupied = "); Serial.println(scheduler.deliveryPorts[0].occupied);
        // Serial.print("TestPort "); Serial.print(scheduler.deliveryPorts[1].pos); Serial.print(".occupied = "); Serial.println(scheduler.deliveryPorts[1].occupied);
        // Serial.print("TestPort "); Serial.print(scheduler.deliveryPorts[2].pos); Serial.print(".occupied = "); Serial.println(scheduler.deliveryPorts[2].occupied);
        // Serial.print("TestPort "); Serial.print(scheduler.deliveryPorts[3].pos); Serial.print(".occupied = "); Serial.println(scheduler.deliveryPorts[3].occupied);

        // instructionCounter++;
      } else if (instructions[instructionCounter] == "Drop"){
        Serial.println("Case DROP");

        if (currentPort.pos == 31 || currentPort.pos == 32 || currentPort.pos == 33 || currentPort.pos == 34){
          robot.setState(DROPB);
          // robot.tleftAngle = 80;
          robot.wasDropPick = 1;
          robot.compassCount = 0;
          //Serial.printf("compassCount: %d\n", robot.compassCount);

        } else if (currentPort.pos == 15 || currentPort.pos == 22 || currentPort.pos == 11 || currentPort.pos == 18) {
          robot.setState(DROPG);
          robot.wasDropPick = 1;
          robot.compassCount = 3;
          //Serial.printf("compassCount: %d\n", robot.compassCount);
          //Serial.println("CAIXA VERDE OU VERMELHA");

        }
        else 
          Serial.println("Failed to start dropping phase");

          //if(currentPort.pos == 31) scheduler.deliveryPorts[0].occupied = true;
          //else if(currentPort.pos == 32) scheduler.deliveryPorts[1].occupied = true;
          //else if(currentPort.pos == 33) scheduler.deliveryPorts[2].occupied = true;
          //else if(currentPort.pos == 34) scheduler.deliveryPorts[3].occupied = true;

          currentBox.status = DELIVERED;
          currentPort.occupied = true;
          scheduler.updatePorts(currentBox, currentPort);
          currentBox = scheduler.getBox();
          currentPort = scheduler.getAvailablePort(currentBox);
        
      } else if (instructions[instructionCounter] == "Left" && robot.wasDropPick == 0) {
        Serial.println("Case LEFTLINE");
        robot.setState(LEFTLINE);

        if (robot.compassCount == 0)
          robot.compassCount = 3;
        else
          robot.compassCount--;

        //Serial.printf("compassCount: %d\n", robot.compassCount);
        
      } else if (instructions[instructionCounter] == "Right" && robot.wasDropPick == 0){
        Serial.println("Case RIGHTLINE");
        robot.setState(RIGHTLINE);
        if (robot.compassCount == 3)
          robot.compassCount = 0;
        else
          robot.compassCount++;

        //Serial.printf("compassCount: %d\n", robot.compassCount);
        
      } 
      else if(instructions[instructionCounter] == "Left" && robot.wasDropPick == 1){
        Serial.println("Case TRIGHT AFTER DROP/PICK");
        robot.setState(TRIGHT);
        

      } else if(instructions[instructionCounter] == "Right" && robot.wasDropPick == 1){
        Serial.println("Case TLEFT AFTER DROP/PICK");
        robot.setState(TLEFT);
     
      } 
      
      else if (instructions[instructionCounter] == "Error"){
        Serial.println("CASE ERROR -> State = START");
        robot.setState(START);
      } 
      
      
      // else if (instructions[instructionCounter] == "Fim"){
      //   Serial.println("CASE FIM");
      //   robot.setState(NEXT);
      // }

      instructionCounter++;
      //Serial.println("HERE");
    }

    // Debug information
    // prints
    // serial_commands.send_command("dte", delta);

    // // serial_commands.send_command("M1", robot.PWM_1);
    // // serial_commands.send_command("M2", robot.PWM_2);

    // // serial_commands.send_command("e1", robot.enc1);
    // // serial_commands.send_command("e2", robot.enc2);

    // serial_commands.send_command("Vbat", robot.battery_voltage);

    // serial_commands.send_command("ve", robot.battery_voltage); // This should be robot.ve
    // // serial_commands.send_command("we", robot.we);

    // // serial_commands.send_command("v1ref", robot.v1ref);
    // // serial_commands.send_command("v2ref", robot.v2ref);

    // // serial_commands.send_command("iman", robot.solenoid_PWM);

    // // serial_commands.send_command("mode", robot.control_mode);

    serial_commands.send_command("st", robot.state);

    // // serial_commands.send_command("rel_s", robot.rel_s);
    // // Serial.println("");
    // // serial_commands.send_command("theta", robot.rel_theta);
    // // Serial.println("");

    // serial_commands.send_command("IP", WiFi.localIP().toString().c_str());
    // // Serial.println("");
    // serial_commands.send_command("IR0", robot.IRLine.IR_values[0]);
    // serial_commands.send_command("IR1", robot.IRLine.IR_values[1]);
    // serial_commands.send_command("IR2", robot.IRLine.IR_values[2]);
    // serial_commands.send_command("IR3", robot.IRLine.IR_values[3]);
    // serial_commands.send_command("IR4", robot.IRLine.IR_values[4]);
    // Serial.println("");

    // serial_commands.send_command("x", robot.IRLine.crosses);
    // serial_commands.send_command("er", robot.integral);
    // serial_commands.send_command("tof", robot.tof_dist);
    // // serial_commands.send_command("fk", robot.follow_k);
    serial_commands.send_command("fv", robot.follow_v);

    // // serial_commands.send_command("pr", robot.IRLine.pos_right);
    // // serial_commands.send_command("pl", robot.IRLine.pos_left);

    serial_commands.send_command("kp", robot.IRkp);
    serial_commands.send_command("ki", robot.IRki);
    serial_commands.send_command("kd", robot.IRkd);

    serial_commands.send_command("testKp", robot.testKp);
    serial_commands.send_command("testKi", robot.testKi);
    serial_commands.send_command("testKd", robot.testKd);

    // serial_commands.send_command("rlv", robot.rightLeft_v);
    // serial_commands.send_command("rlw", robot.rightLeft_w);
    // serial_commands.send_command("rllv", robot.rightLeftLine_v);
    // serial_commands.send_command("rllw", robot.rightLeftLine_w);

    // serial_commands.send_command("rsdb", robot.rel_sDROPB);
    // serial_commands.send_command("rsdg", robot.rel_sDROPG);
    // serial_commands.send_command("tdb", robot.tis_DROPB);
    // serial_commands.send_command("tdg", robot.tis_DROPG);


    serial_commands.send_command("box", irrecvbuffer);
    // serial_commands.send_command("blacks_lvl", robot.IRLine.black_cross_level);
    serial_commands.send_command("blacks", robot.IRLine.blacks);
    // serial_commands.send_command("wasDP", robot.wasDropPick);

    // serial_commands.send_command("refreshRate", robot.refreshRate);


    Serial.print(">Output_w:");
    Serial.println(robot.output_w);

    Serial.print("Output_w: ");
    Serial.println(robot.output_w);



    // // Serial.print(" cmd: ");
    // // Serial.print(serial_commands.frame.command);
    // // Serial.print("; ");

    // // serial_commands.send_command("dbg", debug);
    // Serial.print(" loop: ");
    // // Serial.println(micros() - loop_micros);
    serial_commands.send_command("loop", micros() - loop_micros);

    serial_commands.flush();
    Serial.println();
  }
}
