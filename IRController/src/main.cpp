#include <Arduino.h>

#define TXD2 15//13
#define RXD2 18//12
#define IR_CARRIER 5

char mode = 'E';
// char mode = 'R';

char emitted = 'U';
char received;

char irrecvbuffer[5];
char irrecvdata;
byte irrecvbuffer_index = -1;

unsigned long currentMillis;
unsigned long previousMillis_instruction = 0;
unsigned long previousMillis_ping = 50;
unsigned long interval_instruction = 0;
unsigned long interval_ping = 100;

void setup(){

  // setting PWM properties
  const int freq = 38000;
  const int ledChannel = 0;
  const int resolution = 8;

  // configure PWM functionalitites
  ledcSetup(ledChannel, freq, resolution);

  // attach the channel to the GPIO to be controlled
  ledcAttachPin(IR_CARRIER, ledChannel);
  int dutyCycle = 64;
  ledcWrite(ledChannel, dutyCycle);


  // Init IR Serial
  Serial.begin(9600);
  Serial2.begin(2400, SERIAL_8N1, RXD2, TXD2);

}

void loop(){

  currentMillis = millis();

  // Read bytes from IR
  if (mode == 'R'){
    Serial.printf("                                 Serial2.available() = ");
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
              }
            } else {
              Serial.write(irrecvdata);
              Serial.println();
            }
        }
      }
  }

  // Write byte to IR
  else if(mode == 'E'){
    if(currentMillis - previousMillis_ping > interval_ping){
      Serial2.write(emitted);
      Serial.print("Sent: ");
      Serial.println(emitted);
      

      previousMillis_ping = currentMillis;
    }
  }
}