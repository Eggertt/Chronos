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
  if (new_PWM > PWM_max)
    new_PWM = PWM_max;
  if (new_PWM < -PWM_max)
    new_PWM = -PWM_max;

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

  f.printf("kf %.6g;\n", wheel_PID_pars.Kf);
  f.printf("kc %.6g;\n", wheel_PID_pars.Kc);
  f.printf("ki %.6g;\n", wheel_PID_pars.Ki);
  f.printf("kd %.6g;\n", wheel_PID_pars.Kd);
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
  else if (frame.command_is("kf"))
  {
    wheel_PID_pars.Kf = frame.value;
    serial_commands.send_command("kf", wheel_PID_pars.Kf);
  }
  else if (frame.command_is("kc"))
  {
    wheel_PID_pars.Kc = frame.value;
    serial_commands.send_command("kc", wheel_PID_pars.Kc);
  }
  else if (frame.command_is("ki"))
  {
    wheel_PID_pars.Ki = frame.value;
    serial_commands.send_command("ki", wheel_PID_pars.Ki);
  }
  else if (frame.command_is("kd"))
  {
    wheel_PID_pars.Kd = frame.value;
    serial_commands.send_command("kd", wheel_PID_pars.Kd);
  }
  else if (frame.command_is("pl"))
  {
    load_pars();
  }
  else if (frame.command_is("pr"))
  {
    serial_commands.send_command("kf", wheel_PID_pars.Kf);
    serial_commands.send_command("kc", wheel_PID_pars.Kc);
    serial_commands.send_command("ki", wheel_PID_pars.Ki);
    serial_commands.send_command("kd", wheel_PID_pars.Kd);
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

  } // Put here more commands...
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

const char *encToString(uint8_t enc)
{
  switch (enc)
  {
  case ENC_TYPE_NONE:
    return "NONE";
  case ENC_TYPE_TKIP:
    return "WPA";
  case ENC_TYPE_CCMP:
    return "WPA2";
  case ENC_TYPE_AUTO:
    return "AUTO";
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

#include "direction.h"
#include "node.h"
#include "config.h"
#include "scheduler.h"

Scheduler scheduler(25);
std::vector<std::string> instructions;
int instructionCounter = 0;
int pathCounter = 0;
Box currentBox;
Box newBox;
Port currentPort;
Box boxes[4];

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
  commands_list.register_command("m2", &(robot.PWM_2_req)); // The 'm2' command sets the PWM value for motor 1

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

  // strcpy(ssid, "TP-Link_4B12");
  // strcpy(password, "23893481");
  strcpy(ssid, "BDWireless");
  strcpy(password, "BFFDDB8849");

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
  Serial.println("");
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

  set_interval(0.040); // In seconds
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

      Serial.printf("                                                 Serial2.available() = ");
      Serial.println(Serial2.available());
      if (Serial2.available()) {
          irrecvdata = Serial2.read();
          if(irrecvdata == 'U') {
            Serial.println("!\n");
            Serial.println("!\n");
            Serial.println("!\n");
            Serial.println("!\n");
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
                robot.setState(SETUP1);
                robot.IRLine.crosses = 0;
              }
            } else {
              Serial.write(irrecvdata);
              Serial.println();
            }
        }
      }

      // ========== pacoMode ==========

      // Serial.printf("                                 Serial2.available() = ");
      // Serial.println(Serial2.available());
      // if (Serial2.available())
      // {
      //   irrecvdata = Serial2.read();
      //   if (irrecvdata == 'U')
      //   {
      //     Serial.println("!\n");
      //     Serial.println("!\n");
      //     Serial.println("!\n");
      //     Serial.println("!\n");
      //     Serial.println("!\n");
      //   }
      //   else if (irrecvdata == 'R')
      //   {
      //     irrecvbuffer_index = 0;
      //   }
      //   else if (irrecvbuffer_index >= 0)
      //   {
      //     irrecvbuffer[irrecvbuffer_index] = irrecvdata;
      //     if (irrecvbuffer[0] == '5')
      //     {
      //       irrecvbuffer_index++;
      //       if (irrecvbuffer_index >= 3)
      //       {
      //         irrecvbuffer[irrecvbuffer_index] = '\0';
      //         irrecvbuffer_index = -1;
      //         Serial.println("Received Instruction!");
      //         Serial.println(irrecvbuffer);
      //         stp2 = 1;
      //         Serial.println("Stp2 = TRUE");
      //         robot.IRLine.crosses = 0;
      //       }
      //     }
      //     else
      //     {
      //       Serial.write(irrecvdata);
      //       Serial.println();
      //     }
      //   }
      // }
          // irrecvbuffer[0] = 'R';
          // irrecvbuffer[1] = '5';
          // irrecvbuffer[2] = 'I';
          // irrecvbuffer[3] = '1';
          // irrecvbuffer[4] = '<';
          // irrecvbuffer[5] = '\0';
          // irrecvbuffer_index = -1;
          // Serial.println("Received Instruction!");
          // Serial.println(irrecvbuffer);
          // robot.setState(SETUP1);
          // stp2 = 1;
          // Serial.println("Stp2 = TRUE");
          // robot.IRLine.crosses = 0;
    }

    // if Setup, create boxes and start the setup for the Dijkstra
    if (robot.state == SETUP1)
    {
      for (size_t i = 0; i < 4; i++)
      {
        Box box;
        box.pos = i;
        box.num = i;
        box.status = WAINTING;
        if (irrecvbuffer[i] == 'w')
          box.color = RED;

        else if (irrecvbuffer[i] == 'u')
          box.color = GREEN;

        else if (irrecvbuffer[i] == 'o')
          box.color = BLUE;

        boxes[i] = box;
      }

      scheduler.setUp(boxes, 4);
      currentBox = scheduler.getBox();
      currentPort = scheduler.getAvailablePort(currentBox);
      instructions = scheduler.getRoute();
      // Serial.println("Aqui");
      for (auto inst : instructions)
      {
        Serial.println(inst.c_str());
      }

      robot.setState(STOP);
    }

    if (stp2)
    {

      Serial.println("ESTOU NO SETUP");

      if (irrecvbuffer[1] == '0')
      {

        destNode = irrecvbuffer[2] - 48;
      }
      else if (irrecvbuffer[1] == '1')
      {

        destNode = 10 + (irrecvbuffer[2] - 48);
      }
      else if (irrecvbuffer[1] == '2')
      {

        destNode = 20 + (irrecvbuffer[2] - 48);
      }
      else if (irrecvbuffer[1] == '3')
      {

        destNode = 30 + (irrecvbuffer[2] - 48);
      }

      Serial.println("Morri depois do if");

      instructions = scheduler.getPaco(destNode);
      Serial.println("Morri depois do getpaco");
      instructionCounter = 0;
      pathCounter = 0;
      Serial.println("Morri depois de zerar coisas");
      // Serial.println("Aqui");
      for (auto inst : instructions)
      {
        Serial.println(inst.c_str());
      }

      Serial.println("Morri depois do FOR");

      robot.setState(STOP);

      Serial.println("Morri depois de entrar em STOP");

      stp2 = 0;

      Serial.println("Morri depois do stp2 = 0");
    }

    if (robot.state == STOP)
    {

      Serial.println("ESTOU NO SCHEDULER!");

      // if (instructionCounter == instructions.size()) {
      // if (currentBox.status == HOLDING) {
      // instructions = scheduler.getRoute(currentBox, currentPort);
      //} else {
      // instructions = scheduler.getRoute();
      // if(scheduler.flag = 1) robot.solenoid_robot.state = 0;
      //}
      // instructionCounter = 0;
      // pathCounter = 0;
      //}

      if (instructions[instructionCounter] == "Line" && (instructions[instructionCounter + 1] != "Drop" || instructions[instructionCounter + 1] != "Pick"))
      {
        Serial.println("Case FLINE");
        robot.setState(FLINE);
      } else if (pathCounter <= 0 && instructions[instructionCounter] == "Right" && (instructions[instructionCounter + 1] == "Left" && instructions[instructionCounter + 2] == "Pick")){
        // instructions[instructionCounter] = "Line";
        // instructions[instructionCounter + 1] = "Right";
        // instructions[instructionCounter + 2] = "Left";
        // instructions[instructionCounter + 3] = "Pick";
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
        Serial.println("Case STOP");
        robot.setState(STOP);
      } else if (instructions[instructionCounter] == "Left" && (instructions[instructionCounter + 1] == "Pick" || instructions[instructionCounter + 1] == "Drop")){
        Serial.println("Case TLEFT");

        robot.setState(TLEFT);
        if (robot.compassCount == 0)
          robot.compassCount = 3;
        else
          robot.compassCount--;

        Serial.printf("compassCount: %d\n", robot.compassCount);
      } else if (instructions[instructionCounter] == "Right" && (instructions[instructionCounter + 1] == "Pick" || instructions[instructionCounter + 1] == "Drop")){
        Serial.println("Case TRIGHT");
        robot.setState(TRIGHT);

        if (robot.compassCount == 3)
          robot.compassCount = 0;
        else
          robot.compassCount++;

        Serial.printf("compassCount: %d\n", robot.compassCount);
        
        
      } else if (instructions[instructionCounter] == "Pick"){
        if (destNode == 0 || destNode == 1 || destNode == 2 || destNode == 3){
          robot.setState(PICKB);
          robot.compassCount = 2;
          Serial.printf("compassCount: %d\n", robot.compassCount);

        } else if (destNode == 16 || destNode == 23 || destNode == 12 || destNode == 19){
          robot.setState(PICKBV);
          robot.compassCount = 1;
          Serial.printf("compassCount: %d\n", robot.compassCount);
        }
        // Assuming that after this robot.state the box is picked
        if (currentBox.color == RED)
        {
          int counterGreen = 0;
          for (auto pick : scheduler.deliveryNodesB)
          {
            pick.occupied = false;
            Serial.printf("deliveryNodesB[");
            Serial.print(counterGreen);
            Serial.printf("].occupied = ");
            Serial.println(scheduler.deliveryNodesB[counterGreen].occupied);
          }
        }
        currentBox.status = HOLDING;
        Serial.println("Case START");
        instructionCounter++;
      } else if (instructions[instructionCounter] == "Drop"){
        Serial.println("Case DROP");

        if (destNode == 31 || destNode == 32 || destNode == 33 || destNode == 34){
          robot.setState(DROPB);
          robot.compassCount = 0;
          Serial.printf("compassCount: %d\n", robot.compassCount);

        } else if (destNode == 15 || destNode == 22 || destNode == 11 || destNode == 18) {
          robot.setState(DROPBV);
           robot.compassCount = 3;
          Serial.printf("compassCount: %d\n", robot.compassCount);
          Serial.println("CAIXA VERDE OU VERMELHA");

        }

        currentBox.status = DELIVERED;
        currentPort = scheduler.getAvailablePort(currentBox);
        currentPort.occupied = true;
        // scheduler.updatePorts(currentPort);
        currentBox = scheduler.getBox();
        
      } else if (instructions[instructionCounter] == "Left") {
        Serial.println("Case LEFTLINE");
        robot.setState(LEFTLINE);

        if (robot.compassCount == 0)
          robot.compassCount = 3;
        else
          robot.compassCount--;

        Serial.printf("compassCount: %d\n", robot.compassCount);
        
      } else if (instructions[instructionCounter] == "Right"){
        Serial.println("Case RIGHTLINE");
        robot.setState(RIGHTLINE);
        if (robot.compassCount == 3)
          robot.compassCount = 0;
        else
          robot.compassCount++;

        Serial.printf("compassCount: %d\n", robot.compassCount);
        
      } else if (instructions[instructionCounter] == "Error"){
        Serial.println("CASE ERROR -> State = START");
        robot.setState(START);
      }

      instructionCounter++;
    }

    // Debug information
    // prints
    // serial_commands.send_command("dte", delta);

    // serial_commands.send_command("M1", robot.PWM_1);
    // serial_commands.send_command("M2", robot.PWM_2);

    // serial_commands.send_command("e1", robot.enc1);
    // serial_commands.send_command("e2", robot.enc2);

    // serial_commands.send_command("Vbat", robot.battery_voltage);

    // serial_commands.send_command("ve", robot.ve);
    // serial_commands.send_command("we", robot.we);

    // serial_commands.send_command("v1ref", robot.v1ref);
    // serial_commands.send_command("v2ref", robot.v2ref);
    // Serial.println("TESTE");

    serial_commands.send_command("iman", robot.solenoid_PWM);

    // serial_commands.send_command("mode", robot.control_mode);

    serial_commands.send_command("st", robot.state);

    serial_commands.send_command("theta", robot.rel_theta);

    // serial_commands.send_command("IP", WiFi.localIP().toString().c_str());
    // serial_commands.send_command("IR0", robot.IRLine.IR_values[0]);
    // serial_commands.send_command("IR1", robot.IRLine.IR_values[1]);
    // serial_commands.send_command("IR2", robot.IRLine.IR_values[2]);
    // serial_commands.send_command("IR3", robot.IRLine.IR_values[3]);
    // serial_commands.send_command("IR4", robot.IRLine.IR_values[4]);

    serial_commands.send_command("tof", robot.tof_dist);
    // serial_commands.send_command("fk", robot.follow_k);
    // serial_commands.send_command("fv", robot.follow_v);

    // serial_commands.send_command("pr", robot.IRLine.pos_right);
    // serial_commands.send_command("pl", robot.IRLine.pos_left);

    // serial_commands.send_command("ki", wheel_PID_pars.Ki);

    // Serial.print(" cmd: ");
    Serial.print(serial_commands.frame.command);
    // Serial.print("; ");

    // serial_commands.send_command("dbg", debug);
    // Serial.print(" loop: ");
    // Serial.println(micros() - loop_micros);
    // serial_commands.send_command("loop", micros() - loop_micros);

    serial_commands.flush();
    Serial.println();
  }
}
