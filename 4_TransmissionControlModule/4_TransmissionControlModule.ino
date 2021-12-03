#include <SPI.h>
#include <mcp2515.h>    

#define CAN_ID_TRANSMISSION_MODULE 0xAB

int currentShiftPosition;
struct can_frame canMsg; 
int data[7] = {0, 0, 0, 0, 0, 0, 0};

MCP2515 mcp2515(10);                 // SPI CS Pin 10 

void send_can_message(int data[7]){
  canMsg.can_id  = CAN_ID_TRANSMISSION_MODULE;
  canMsg.can_dlc = 8;   
  //CAN data length as 8
  for(int i = 0; i < 8; i++){
    canMsg.data[i] = data[i];
  }
  
  mcp2515.sendMessage(&canMsg);     //Sends the CAN message
}

int getCurrentShiftPosition(){


  return 0;
}

void setup() {
  SPI.begin();
  Serial.begin(115200);

  mcp2515.reset();                          
  mcp2515.setBitrate(CAN_500KBPS,MCP_8MHZ); //Sets CAN at speed 500KBPS and Clock 8MHz 
  mcp2515.setNormalMode();                  //Sets CAN at normal mode

  currentShiftPosition = getCurrentShiftPosition();
}
                              

void loop() {

  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) // To receive data (Poll Read)
  {
    if(canMsg.data[0] == 0)                 // PULL
    {
      Serial.print(":PULL");

      switch(canMsg.data[1]){                 // WHICH REQUEST
        case 0:                               // CURRENT GEAR
          Serial.print(":CURRENT-GEAR");
          data[0] = 1;
          data[1] = 1;
          data[2] = currentShiftPosition;
          send_can_message(canMsg.can_id, data);
          break;
      } 
    }
  }
}
