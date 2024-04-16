#include <Arduino.h>

#define TXD2 15//13
#define RXD2 2//12
#define IR_CARRIER 5

char mode = 'E';
//char mode = 'R';

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
    if (Serial2.available()) {
      irrecvdata = Serial2.read();
      Serial.print("Received: ");
      Serial.println(received);
      //Serial.println(char(Serial2.read()));

      if(irrecvdata == 'U') Serial.print("U");
    }
  }

  // Write byte to IR
  else if(mode == 'E'){
    if(currentMillis - previousMillis_ping > interval_ping){
      //Serial2.write(emitted);
      Serial.print("Sent: ");
      //Serial.println(emitted);
      Serial.println(Serial2.write(emitted));

      previousMillis_ping = currentMillis;
    }
  }
}