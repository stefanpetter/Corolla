#include <SPI.h>          //Library for using SPI Communication 
#include <mcp2515.h>      //Library for using CAN Communication

#define CAN_ID_STEERING_WHEEL 0xAA
#define CAN_ID_TRANSMISSION_MODULE 0xAB
#define CAN_ID_REAR_LIGHT_RIGHT 0xB0


int data[7] = {0, 0, 0, 0, 0, 0, 0};

// CAR Related variables
int currentShiftPosition;

bool light_front_left;
bool light_front_left_highbeam;
bool light_front_right;
bool light_front_right_highbeam;
bool light_rear_left;
bool light_rear_left_brakelight;
bool light_rear_right;
bool light_rear_reight_brakelight;
bool light_interior_steeringwheel;

struct can_frame canMsg;
MCP2515 mcp2515(10);

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

    if(serialInput == 'S'){
        if(!light_interior_steeringwheel){
          light_interior_steeringwheel = true;

          data[0] = 2;
          data[1] = 255;
          data[2] = 255;
          send_can_message(CAN_ID_STEERING_WHEEL, data);
        
          Serial.println("00:PUSH:STEERINGWHEEL-LIGHTS:ON:##");
        } else {
          light_interior_steeringwheel = false;

          data[0] = 2;
          data[1] = 0;
          data[2] = 0;
          send_can_message(CAN_ID_STEERING_WHEEL, data);
        
          Serial.println("00:PUSH:STEERINGWHEEL-LIGHTS:OFF:##");
        }
        
        
    } 
  }
}

void checkCanMessages() {
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) // To receive data (Poll Read)
  {
    Serial.print(canMsg.can_id, HEX);
    
    switch(canMsg.can_id){
      case CAN_ID_STEERING_WHEEL:              // STEERING WHEEL 
        
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
                currentShiftPosition--;
                data[0] = 1;
                data[1] = currentShiftPosition;
                send_can_message(CAN_ID_TRANSMISSION_MODULE, data);
                break;
              case 3:                               // BTN MODE
                Serial.print(":BTN-VOICE");
                break;
              case 4:                               // BTN OK
                Serial.print(":BTN-VOLUME");
                break;
              case 5:                               // BTN FLIPPER LEFT
                Serial.print(":BTN-FLIPPER-RIGHT");
                currentShiftPosition++;
                data[0] = 1;
                data[1] = currentShiftPosition;
                send_can_message(CAN_ID_TRANSMISSION_MODULE, data);
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
        break;
      case CAN_ID_TRANSMISSION_MODULE:
        if(canMsg.data[0] == 0)                 // PULL
        {
          Serial.print(":PULL");

          switch(canMsg.data[1]){                 
              case 0:                              
                break;
          }
          
        } else {                                //PUSH
            Serial.print(":PUSH");
            
            switch(canMsg.data[1]){               // WHICH BUTTON
              case 1:                               // BTN OK
                currentShiftPosition = 
                break;
    }

    Serial.println(":##");
  }
}

void setup() 
{
  Serial.begin(115200);
  SPI.begin();               //Begins SPI communication
  
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS,MCP_8MHZ); //Sets CAN at speed 500KBPS and Clock 8MHz
  mcp2515.setNormalMode();
}

void loop() 
{
  checkSerialInput(); 
  checkCanMessages();
}
