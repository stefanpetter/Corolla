#include <SPI.h>          //Library for using SPI Communication 
#include <mcp2515.h>      //Library for using CAN Communication

#define BTN1 4
#define BTN2 5
#define LED1 6
#define LED2 7
#define CAN_ID_REAR_LIGHT_RIGHT 0xB0

// DIM, BRAKE, BLINK, NA, NA, NA, NA
int data[7] = {0, 0, 0, 0, 0, 0, 0};

int currentShiftPosition = 3;

int button1State = HIGH;             
int button2State = HIGH;            
int lastButton1State = HIGH;  
int lastButton2State = HIGH; 

unsigned long lastDebounce1Time = 0; 
unsigned long lastDebounce2Time = 0; 
unsigned long debounceDelay = 50;

bool dimlight = false;
bool is_braking = false;
bool is_blinking = false;

struct can_frame canMsg;
MCP2515 mcp2515(10);

void changeGear(int shiftDirection){
  switch(shiftDirection){
    case 1:
      currentShiftPosition++;
      break;
    case 0:
      currentShiftPosition--;
      break;
  }

  //SEND DATA TO STEERING WHEEL
  data[0] = 1;
  data[1] = currentShiftPosition;
  send_can_message(0xAA, data);
}


void setup() 
{
  pinMode(BTN1, INPUT_PULLUP);
  pinMode(BTN2, INPUT_PULLUP);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  
  Serial.begin(115200);
  SPI.begin();               //Begins SPI communication
  
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS,MCP_8MHZ); //Sets CAN at speed 500KBPS and Clock 8MHz
  mcp2515.setNormalMode();
}

void send_can_message(int can_id, int data[7]){
  canMsg.can_id  = can_id;           //CAN id as 0x036
  canMsg.can_dlc = 8;   
  //CAN data length as 8
  for(int i = 0; i < 8; i++){
    canMsg.data[i] = data[i];
  }
  
  mcp2515.sendMessage(&canMsg);     //Sends the CAN message
}

void checkSerialInput() {
  if(Serial.available() > 0){
    char serialInput = Serial.read();

    if(serialInput == 'B'){
        Serial.println("Blinking OFF");
        data[0] = 2;
        data[1] = 255;
        data[2] = 255;
        send_can_message(0xAA, data);
    } 
  }
}

void loop() 
{
  checkSerialInput(); 
  
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) // To receive data (Poll Read)
  {
    Serial.print(canMsg.can_id, HEX);
    
    switch(canMsg.can_id){
      case 0xAA:                                // STEERING WHEEL 
        
        if(canMsg.data[0] == 0)                 // PULL
        {
          Serial.print(":PULL");

          switch(canMsg.data[1]){                 // WHICH REQUEST
              case 0:                               // CURRENT GEAR
                Serial.print(":CURRENT-GEAR");
                data[0] = 1;
                data[1] = currentShiftPosition;
                send_can_message(canMsg.can_id, data);
                break;
          }
          
        } else {                                //PUSH
            Serial.print(":PUSH");
            
            switch(canMsg.data[1]){               // WHICH BUTTON
              case 0:                               // BTN MODE
                Serial.print(":BTN-MODE");
                break;
              case 1:                               // BTN OK
                Serial.print(":BTN-OK");
                break;
              case 2:                               // BTN FLIPPER LEFT
                Serial.print(":BTN-FLIPPER-LEFT");
                changeGear(0);
                break;
              case 3:                               // BTN MODE
                Serial.print(":BTN-VOICE");
                break;
              case 4:                               // BTN OK
                Serial.print(":BTN-VOLUME");
                break;
              case 5:                               // BTN FLIPPER LEFT
                Serial.print(":BTN-FLIPPER-RIGHT");
                changeGear(1);
                break;
              case 6:                               // BTN FLIPPER LEFT
                Serial.print(":ROTARY-UP");
                break;
              case 7:                               // BTN FLIPPER LEFT
                Serial.print(":ROTARY-DOWN");
                break;
            }
            switch(canMsg.data[2]){               // WHICH ACTION
              case 0:                               // ACTION RELEASE
                Serial.print(":RELEASE");
                break;
              case 1:                               // ACTION PRESS
                Serial.print(":PRESS");
                break;
              case 2:                               // ACTION HOLD
                Serial.print(":HOLD");
                break;
            }
        }
        
    }

    Serial.println(":##");
  }
}
