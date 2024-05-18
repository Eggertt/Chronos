#include <Arduino.h>
#include <iostream>
#include <array>
#include <string>
#include <algorithm> // Para std::shuffle
#include <random> 

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <EEPROM.h>

#include <ezButton.h>


// W - Starting condition (primeiro char do vetor enviado)
// U - Ping message
// w - Red Part
// u - Green part
// o - Blue part

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//Defines para OLED
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 32

#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//Defines pra logica do IR
#define TXD2 14
#define IR_CARRIER 12
#define RXD2 2

char irrecvbuffer[5];
char irrecvdata;
byte irrecvbuffer_index = -1;

unsigned long currentMillis;
unsigned long previousMillis_instruction = 0;
unsigned long previousMillis_ping = 50;
unsigned long interval_instruction = 0;
unsigned long interval_ping = 100;

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

//Defines pra logica dos botoes
#define debounceTime 50

#define whiteButtonPin 19
#define blueButtonPin 18
#define greenButtonPin 5
#define redButtonPin 17

ezButton whiteButton(whiteButtonPin);
ezButton blueButton(blueButtonPin);
ezButton greenButton(greenButtonPin);
ezButton redButton(redButtonPin);

unsigned long currentWhiteMillis;
unsigned long lastWhiteMillis;
unsigned long whiteButtonInterval = 2000;

bool setOrder = false;
int whiteArrayTracker = 0;


const int EEPROM_ADDRESS = 0;

std::vector<char> sendArray = {'x', 'x', 'x', 'x'};
std::vector<char> OLEDsendArray = {'x', 'x', 'x', 'x'};
std::vector<char> whiteArray = {'x', 'x', 'x', 'x'};
std::vector<char> OLEDwhiteArray = {'x', 'x', 'x', 'x'};
std::string bufferString, whiteString, OLEDstring, OLEDwhiteString;
const char* sendString; 
const char* showOLEDsendString;
const char* sendWhiteString;
const char* showOLEDwhiteString;


void saveWhiteArray(){
  for (size_t i=0; i<whiteArray.size(); i++){
    EEPROM.write(EEPROM_ADDRESS + i, whiteArray[i]);
  }

  EEPROM.commit();

}

void readFromEEPROM(){
  for(int i=0; i<whiteArray.size(); i++){
    whiteArray[i] = EEPROM.read(EEPROM_ADDRESS + i);
  }

  Serial.print("Valores lidos da EEPROM:");
  for(char c : whiteArray){
    Serial.print(c);
  }
  Serial.println();
}


void refreshOLEDwhiteString(){
    OLEDwhiteString = std::string(OLEDwhiteArray.begin(), OLEDwhiteArray.end());
    showOLEDwhiteString = OLEDwhiteString.c_str();

    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextSize(1);
    display.print(showOLEDwhiteString);
    display.display(); 

}

void convertSendArray(std::vector<char> array){
  for(int i = 0; i < sizeof(array); i++){

    if(sendArray[i] == 'o') OLEDsendArray[i] = 'B';
    else if(sendArray[i] == 'u') OLEDsendArray[i] = 'G';
    else if(sendArray[i] == 'w') OLEDsendArray[i] = 'R';

  }

  OLEDstring = std::string(OLEDsendArray.begin(), OLEDsendArray.end());

  showOLEDsendString = OLEDstring.c_str();

}

void sendInstruction(){

  display.clearDisplay();

  std::random_shuffle(sendArray.begin(), sendArray.end());

  convertSendArray(sendArray);

  sendArray.insert(sendArray.begin(), 'W');

  bufferString = std::string(sendArray.begin(), sendArray.end());

  sendString = bufferString.c_str();

  Serial2.write(sendString);
}



void setup() {

  whiteButton.setDebounceTime(debounceTime);
  blueButton.setDebounceTime(debounceTime);
  greenButton.setDebounceTime(debounceTime);
  redButton.setDebounceTime(debounceTime);

  
  Wire.begin();
  EEPROM.begin(512);
  readFromEEPROM();

  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);

  //INFRARED
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
  Serial2.begin(2400, SERIAL_8N1, RXD2, TXD2);

  // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

  //BOTOES
  pinMode(whiteButtonPin, INPUT);
  pinMode(blueButtonPin, INPUT);
  pinMode(greenButtonPin, INPUT);
  pinMode(redButtonPin, INPUT);

  whiteString = std::string(whiteArray.begin(), whiteArray.end());

  sendWhiteString = whiteString.c_str();

  Serial.begin(115200);


}

void loop() {

  currentMillis = millis();

  whiteButton.loop();
  blueButton.loop();
  greenButton.loop();
  redButton.loop();
  
  if(digitalRead(whiteButtonPin) == LOW){
    lastWhiteMillis = millis();

  }if(millis() - lastWhiteMillis >= 2000){
    setOrder = true; 
    whiteArrayTracker = 0;
    whiteArray.clear();
    OLEDwhiteArray.clear();
    whiteArray.push_back('W');

    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextSize(1);
    display.print("Select white array");
    display.display(); 

    
  }
  
    if(blueButton.isPressed()){
      if(setOrder == false && whiteArrayTracker == 0){
        sendArray = {'o', 'o', 'o', 'o'};
        Serial.println("BLUE");

        sendInstruction();

      }else if(setOrder == true && whiteArrayTracker < 4){
        whiteArray.push_back('o');
        OLEDwhiteArray.push_back('B');
        Serial.println("BLUE INTO WHITE");

        refreshOLEDwhiteString();
        whiteArrayTracker++;
        if(whiteArrayTracker == 4) setOrder = false, whiteArrayTracker = 0, saveWhiteArray();

      }

    }else if(greenButton.isPressed()){
      if(setOrder == false && whiteArrayTracker == 0){
        sendArray = {'u', 'u', 'o', 'o'};
        Serial.println("GREEN");

        sendInstruction(); 

      }else if(setOrder == true && whiteArrayTracker < 4){
        whiteArray.push_back('u');
        OLEDwhiteArray.push_back('G');
        Serial.println("GREEN INTO WHITE");

        refreshOLEDwhiteString();
        whiteArrayTracker++;
        if(whiteArrayTracker == 4) setOrder = false, whiteArrayTracker = 0, saveWhiteArray();

      }   

    }else if(redButton.isPressed()){

      if(setOrder == false && whiteArrayTracker == 0){
        sendArray = {'w', 'u', 'u', 'o'};
        Serial.println("RED");

        sendInstruction();

      }else if(setOrder == true && whiteArrayTracker < 4){
        whiteArray.push_back('w');
        OLEDwhiteArray.push_back('R');
        Serial.println("RED INTO WHITE");

        refreshOLEDwhiteString();
        whiteArrayTracker++;
        if(whiteArrayTracker == 4) setOrder = false, whiteArrayTracker = 0, saveWhiteArray();

      }

    }else if(whiteButton.isPressed()){
      sendArray = whiteArray;
      Serial.println("WHITE");

      bufferString = std::string(sendArray.begin(), sendArray.end());
      sendString = bufferString.c_str();

      Serial2.write(sendString);

      
      display.clearDisplay();
      display.setCursor(0,0);
      display.setTextSize(1);
      display.print(showOLEDwhiteString);
      display.display();


      display.clearDisplay();
      display.setCursor(0,10);
      display.setTextSize(2);
      display.print(showOLEDwhiteString);
      display.display();

    }

  
  

  display.setCursor(0,0);
  display.setTextSize(1);
  display.print(showOLEDwhiteString);
  display.display();

  display.setCursor(0,10);
  display.setTextSize(2);
  display.print(showOLEDsendString);
  display.display();

  if(currentMillis - previousMillis_ping > interval_ping){
      Serial2.write('U');
      //Serial.print("Sent: ");
      //Serial.println('U');

      previousMillis_ping = currentMillis;
    }

}






