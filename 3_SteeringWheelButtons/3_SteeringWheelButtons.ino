#include <SPI.h>
#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#include <mcp2515.h>    

SSD1306AsciiWire oled;

struct can_frame canMsg; 
MCP2515 mcp2515(10);                 // SPI CS Pin 10 

unsigned long debouncePressDelay = 50;
unsigned long debounceHoldDelay = 750;

// VARIABLES LEFT BUTTON MODULE
#define leftBtnOk 8
#define leftBtnMode 7
#define leftBtnFlipper A2
#define leftRotaryInputRight A7
#define leftRotaryInputLeft A6
#define leftLeds 6

unsigned long lastDebounceTimeLeftBtnOk = 0; 
unsigned long lastDebounceTimeLeftBtnMode = 0; 
unsigned long lastDebounceTimeLeftBtnFlipper = 0;

bool LeftBtnOkPressed = false;
bool LeftBtnModePressed = false;
bool LeftBtnFlipperPressed = false;
bool LeftLedsOn = false;

bool LeftBtnOkPressTriggered = false;
bool LeftBtnOkHoldTriggered = false;
bool LeftBtnModePressTriggered = false;
bool LeftBtnModeHoldTriggered = false;
bool LeftBtnFlipperPressTriggered = false;
bool LeftBtnFlipperHoldTriggered = false;

int LeftRotaryInputRightValue;
int LeftRotaryInputLeftValue;

bool LeftRotaryInHighRange = false;
int LeftRotaryTimeOut = 0;

int currentLeft = 0;
int currentRight = 0;
bool onlyCheckUp = false;
bool onlyCheckDown = false;

// VARIABLES RIGHT BUTTON MODULE
#define rightBtnVoice 4
#define rightBtnVolume 3
#define rightBtnFlipper A1
#define rightLeds 5

unsigned long lastDebounceTimeRightBtnVoice = 0; 
unsigned long lastDebounceTimeRightBtnVolume = 0; 
unsigned long lastDebounceTimeRightBtnFlipper = 0;

bool RightBtnVoicePressed = false;
bool RightBtnVolumePressed = false;
bool RightBtnFlipperPressed = false;
bool RightLedsOn = false;

bool RightBtnVoicePressTriggered = false;
bool RightBtnVoiceHoldTriggered = false;
bool RightBtnVolumePressTriggered = false;
bool RightBtnVolumeHoldTriggered = false;
bool RightBtnFlipperPressTriggered = false;
bool RightBtnFlipperHoldTriggered = false;

char shiftArray[7] = {'R','N','1','2','3','4','5'};
int currentShiftPosition = 1;
int currentShiftPositionTimeOut = 2000;

// DIRECTION, BUTTON, ACTION, NA, NA, NA, NA
int data[7] = {0, 0, 0, 0, 0, 0, 0};

void displayShiftChange(int currentShiftPosition){
  oled.clear();
  oled.set1X();

  if(currentShiftPosition > 1){
    oled.print("     --  ");
    oled.print(shiftArray[(currentShiftPosition - 2)]);
    oled.println("  --");
  } else {
    oled.println(" ");
  }

  if(currentShiftPosition > 0){
    oled.print("      --  ");
    oled.print(shiftArray[(currentShiftPosition - 1)]);
    oled.println("  --");
  } else {
    oled.println(" ");
  }
  oled.println(" ");

  oled.set2X();
  oled.print("--|| ");
  oled.print(shiftArray[currentShiftPosition]);
  oled.println(" ||--");

  oled.set1X();
  oled.println(" ");
  if(currentShiftPosition < 6){
    oled.print("      --  ");
    oled.print(shiftArray[(currentShiftPosition + 1)]);
    oled.println("  --");
  } else {
    oled.println(" ");
  }

  if(currentShiftPosition < 5){
    oled.print("     --  ");
    oled.print(shiftArray[(currentShiftPosition + 2)]);
    oled.println("  --");
  } else {
    oled.println(" ");
  }
}

void send_can_message(int data[7]){
  canMsg.can_id  = 0xAA;           //CAN id as 0x036
  canMsg.can_dlc = 8;   
  //CAN data length as 8
  for(int i = 0; i < 8; i++){
    canMsg.data[i] = data[i];
  }
  
  mcp2515.sendMessage(&canMsg);     //Sends the CAN message
}

void setup() {
  pinMode(leftLeds, OUTPUT);
  pinMode(leftBtnOk, INPUT_PULLUP);
  pinMode(leftBtnMode, INPUT_PULLUP);
  pinMode(leftBtnFlipper, INPUT_PULLUP);
  pinMode(leftRotaryInputLeft, INPUT);
  pinMode(leftRotaryInputRight, INPUT);

  pinMode(rightLeds, OUTPUT);
  pinMode(rightBtnVoice, INPUT_PULLUP);
  pinMode(rightBtnVolume, INPUT_PULLUP);
  pinMode(rightBtnFlipper, INPUT_PULLUP);

  SPI.begin();

  Wire.begin();
  Wire.setClock(400000L);

  mcp2515.reset();                          
  mcp2515.setBitrate(CAN_500KBPS,MCP_8MHZ); //Sets CAN at speed 500KBPS and Clock 8MHz 
  mcp2515.setNormalMode();                  //Sets CAN at normal mode

  oled.begin(&Adafruit128x64, 0x3C);
  oled.setFont(Adafruit5x7);
  oled.clear();
  
  displayShiftChange(currentShiftPosition);

  analogWrite(leftLeds, 0);
  analogWrite(rightLeds, 0);

  if(analogRead(leftRotaryInputLeft) > 75){ 
    currentLeft = 1;
    currentRight = 1;
    LeftRotaryInHighRange = true;
  }
}
                              

void loop() {

  //POLLING CURRENT GEAR JUST TO BE ON THE SAFE SIDE
  if(currentShiftPositionTimeOut > 0){
    currentShiftPositionTimeOut--;
  } else {
    // Request current shift position
    data[0] = 0; // PULL DATA
    data[1] = 0; // CURRENT GEAR
    send_can_message(data);
    
    currentShiftPositionTimeOut = 2000;
  }

  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) // To receive data (Poll Read)
  {
    if(canMsg.can_id == 0xAA){  //ONLY ACT ON MESSAGES FOR STEERING WHEEL
       if(canMsg.data[0] == 1){ //GEAR POSITION
         if(currentShiftPosition != canMsg.data[1]){
            currentShiftPosition = canMsg.data[1];
            displayShiftChange(currentShiftPosition);
         }
       }
       if(canMsg.data[0] == 2){ //LEDS
          analogWrite(leftLeds, canMsg.data[1]);
          analogWrite(rightLeds, canMsg.data[2]);
       }
    }
  }

  
//  // =================== LEFT BUTTON Ok ===================
  int leftBtnOkReading = digitalRead(leftBtnOk);
  
  if (leftBtnOkReading == 0) { 
    if(LeftBtnOkPressed == false){
      lastDebounceTimeLeftBtnOk = millis(); 
      LeftBtnOkPressed = true;
    }
  } else {
    LeftBtnOkPressed = false;
    LeftBtnOkPressTriggered = false;
    LeftBtnOkHoldTriggered = false;
  }  
  if ((millis() - lastDebounceTimeLeftBtnOk) > debouncePressDelay) {
    if (leftBtnOkReading == 0 && !LeftBtnOkPressTriggered) {
      data[0] = 1; // PUSH DATA
      data[1] = 1; // BTN OK
      data[2] = 1; // ACTION PRESS
      send_can_message(data);
      LeftBtnOkPressTriggered = true;
    }
  }

   if ((millis() - lastDebounceTimeLeftBtnOk) > debounceHoldDelay) {
    if (leftBtnOkReading == 0 && !LeftBtnOkHoldTriggered) {
      data[0] = 1; // PUSH DATA
      data[1] = 1; // BTN OK
      data[2] = 2; // ACTION HOLD
      send_can_message(data);
      LeftBtnOkHoldTriggered = true;
    }
  }

  // =================== LEFT BUTTON Mode ===================
  int leftBtnModeReading = digitalRead(leftBtnMode);
  
  if (leftBtnModeReading == 0) { 
    if(LeftBtnModePressed == false){
      lastDebounceTimeLeftBtnMode = millis(); 
      LeftBtnModePressed = true;
    }
  } else {
    LeftBtnModePressed = false;
    LeftBtnModePressTriggered = false;
    LeftBtnModeHoldTriggered = false;
  }  
  if ((millis() - lastDebounceTimeLeftBtnMode) > debouncePressDelay) {
    if (leftBtnModeReading == 0 && !LeftBtnModePressTriggered) {
      data[0] = 1; // PUSH DATA
      data[1] = 0; // BTN MODE
      data[2] = 1; // ACTION PRESS
      send_can_message(data);
      LeftBtnModePressTriggered = true;
      
    }
  }

   if ((millis() - lastDebounceTimeLeftBtnMode) > debounceHoldDelay) {
    if (leftBtnModeReading == 0 && !LeftBtnModeHoldTriggered) {
      data[0] = 1; // PUSH DATA
      data[1] = 0; // BTN MODE
      data[2] = 2; // ACTION HOLD
      send_can_message(data);
      LeftBtnModeHoldTriggered = true;
    }
  }

  // =================== LEFT BUTTON Flipper ===================
  int leftBtnFlipperReading = analogRead(leftBtnFlipper);
  
  if (leftBtnFlipperReading < 125) { 
    if(LeftBtnFlipperPressed == false){
      lastDebounceTimeLeftBtnFlipper = millis(); 
      LeftBtnFlipperPressed = true;
    }
  } else {
    LeftBtnFlipperPressed = false;
    LeftBtnFlipperPressTriggered = false;
    LeftBtnFlipperHoldTriggered = false;
  }  
  if ((millis() - lastDebounceTimeLeftBtnFlipper) > debouncePressDelay) {
    if (leftBtnFlipperReading < 125 && !LeftBtnFlipperPressTriggered) {
      data[0] = 1; // PUSH DATA
      data[1] = 2; // BTN FLIPPER LEFT
      data[2] = 1; // ACTION PRESS
      send_can_message(data);
      LeftBtnFlipperPressTriggered = true;      
    }
  }

   if ((millis() - lastDebounceTimeLeftBtnFlipper) > debounceHoldDelay) {
    if (leftBtnFlipperReading < 125 && !LeftBtnFlipperHoldTriggered) {
      data[0] = 1; // PUSH DATA
      data[1] = 2; // BTN FLIPPER LEFT
      data[2] = 2; // ACTION HOLD
      send_can_message(data);
      LeftBtnFlipperHoldTriggered = true;     
    }
  }

  // =================== LEFT ROTARY ENCODER ===================
  LeftRotaryInputLeftValue = analogRead(leftRotaryInputLeft);
  LeftRotaryInputRightValue = analogRead(leftRotaryInputRight);

  if(LeftRotaryTimeOut > 0){
    LeftRotaryTimeOut = LeftRotaryTimeOut - 1;
  } else {

      bool skipNextChecks = false;

      // LOW RANGE CHECKS
      if(!LeftRotaryInHighRange && !currentRight == 1 && currentLeft == 0 && LeftRotaryInputLeftValue > 75){
        Serial.println("LOW - Going UP");
        currentLeft = 1;
        onlyCheckUp = true;
      }
      if(!LeftRotaryInHighRange && !currentLeft == 1 && currentRight == 0 && LeftRotaryInputRightValue > 75){
        Serial.println("LOW - Going DOWN");
        currentRight = 1;
        onlyCheckDown = true;
      }

      // HIGH RANGE CHECKS
      if(LeftRotaryInHighRange && !currentRight == 0 && currentLeft == 1 && LeftRotaryInputLeftValue < 175){
        Serial.println("HIGH - Going UP");
        currentLeft = 0;
        onlyCheckUp = true;
      }
      if(LeftRotaryInHighRange && !currentLeft == 0 && currentRight == 1 && LeftRotaryInputRightValue < 175){
        Serial.println("HIGH - Going DOWN");
        currentRight = 0;
        onlyCheckDown = true;
      }

      // LOW RANGE CHECKS
      if(!LeftRotaryInHighRange && onlyCheckUp && currentLeft == 1 && LeftRotaryInputRightValue > 75){
        Serial.println("LOW - Going UP Finished");
        currentRight = 1;
        LeftRotaryInHighRange = true;
        LeftRotaryTimeOut = 50;

        data[0] = 1; // PUSH DATA
        data[1] = 6; // ROTARY UP
        data[2] = 1; // PRESS
        send_can_message(data);
      }
        
      if(!LeftRotaryInHighRange && onlyCheckDown && currentRight == 1 && LeftRotaryInputLeftValue > 75){
        Serial.println("LOW - Going DOWN Finished");
        currentLeft = 1;
        LeftRotaryInHighRange = true;
        LeftRotaryTimeOut = 50;

        data[0] = 1; // PUSH DATA
        data[1] = 7; // ROTARY DOWN
        data[2] = 1; // PRESS
        send_can_message(data);
      }

      // HIGH RANGE CHECKS
      if(LeftRotaryInHighRange && onlyCheckUp && currentLeft == 0 && LeftRotaryInputRightValue < 175){
        Serial.println("HIGH - Going UP Finished");
        currentRight = 0;
        LeftRotaryInHighRange = false;
        LeftRotaryTimeOut = 50;

        data[0] = 1; // PUSH DATA
        data[1] = 6; // ROTARY UP
        data[2] = 1; // PRESS
        send_can_message(data);
      }
        
      if(LeftRotaryInHighRange && onlyCheckDown && currentRight == 0 && LeftRotaryInputLeftValue < 175){
        Serial.println("HIGH - Going DOWN Finished");
        currentLeft = 0;
        LeftRotaryInHighRange = false;
        LeftRotaryTimeOut = 50;

        data[0] = 1; // PUSH DATA
        data[1] = 7; // ROTARY DOWN
        data[2] = 1; // PRESS
        send_can_message(data);
      }
      
  }

//  // =================== LEFT BUTTON Voice ===================
  int rightBtnVoiceReading = digitalRead(rightBtnVoice);
  
  if (rightBtnVoiceReading == 0) { 
    if(RightBtnVoicePressed == false){
      lastDebounceTimeRightBtnVoice = millis(); 
      RightBtnVoicePressed = true;
    }
  } else {
    RightBtnVoicePressed = false;
    RightBtnVoicePressTriggered = false;
    RightBtnVoiceHoldTriggered = false;
  }  
  if ((millis() - lastDebounceTimeRightBtnVoice) > debouncePressDelay) {
    if (rightBtnVoiceReading == 0 && !RightBtnVoicePressTriggered) {
      data[0] = 1; // PUSH DATA
      data[1] = 3; // BTN VOICE
      data[2] = 1; // PRESS
      send_can_message(data);
      RightBtnVoicePressTriggered = true;
    }
  }

   if ((millis() - lastDebounceTimeRightBtnVoice) > debounceHoldDelay) {
    if (rightBtnVoiceReading == 0 && !RightBtnVoiceHoldTriggered) {
      data[0] = 1; // PUSH DATA
      data[1] = 3; // BTN VOICE
      data[2] = 2; // HOLD
      send_can_message(data);
      RightBtnVoiceHoldTriggered = true;
    }
  }

  // =================== RIGHT BUTTON Volume ===================
  int rightBtnVolumeReading = digitalRead(rightBtnVolume);
  
  if (rightBtnVolumeReading == 0) { 
    if(RightBtnVolumePressed == false){
      lastDebounceTimeRightBtnVolume = millis(); 
      RightBtnVolumePressed = true;
    }
  } else {
    RightBtnVolumePressed = false;
    RightBtnVolumePressTriggered = false;
    RightBtnVolumeHoldTriggered = false;
  }  
  if ((millis() - lastDebounceTimeRightBtnVolume) > debouncePressDelay) {
    if (rightBtnVolumeReading == 0 && !RightBtnVolumePressTriggered) {
      data[0] = 1; // PUSH DATA
      data[1] = 4; // BTN VOLUME
      data[2] = 1; // PRESS
      send_can_message(data);
      RightBtnVolumePressTriggered = true;
    }
  }

   if ((millis() - lastDebounceTimeRightBtnVolume) > debounceHoldDelay) {
    if (rightBtnVolumeReading == 0 && !RightBtnVolumeHoldTriggered) {
      data[0] = 1; // PUSH DATA
      data[1] = 4; // BTN VOLUME
      data[2] = 2; // HOLD
      send_can_message(data);
      RightBtnVolumeHoldTriggered = true;
    }
  }
  // =================== RIGHT BUTTON Flipper ===================
  int rightBtnFlipperReading = analogRead(rightBtnFlipper);
  
  if (rightBtnFlipperReading < 125) { 
    if(RightBtnFlipperPressed == false){
      lastDebounceTimeRightBtnFlipper = millis(); 
      RightBtnFlipperPressed = true;
    }
  } else {
    RightBtnFlipperPressed = false;
    RightBtnFlipperPressTriggered = false;
    RightBtnFlipperHoldTriggered = false;
  }  
  if ((millis() - lastDebounceTimeRightBtnFlipper) > debouncePressDelay) {
    if (rightBtnFlipperReading < 125 && !RightBtnFlipperPressTriggered) {
      data[0] = 1; // PUSH DATA
      data[1] = 5; // BTN FLIPPER RIGHT
      data[2] = 1; // PRESS
      send_can_message(data);
      RightBtnFlipperPressTriggered = true;

      currentShiftPosition++;
      displayShiftChange(currentShiftPosition); 
    }
  }

   if ((millis() - lastDebounceTimeRightBtnFlipper) > debounceHoldDelay) {
    if (rightBtnFlipperReading < 125 && !RightBtnFlipperHoldTriggered) {
      data[0] = 1; // PUSH DATA
      data[1] = 5; // BTN FLIPPER RIGHT
      data[2] = 2; // HOLD
      send_can_message(data);
      RightBtnFlipperHoldTriggered = true;      
    }
  }
}
