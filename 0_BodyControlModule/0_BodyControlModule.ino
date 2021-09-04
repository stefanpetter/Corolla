#include <SPI.h>          //Library for using SPI Communication 
#include <mcp2515.h>      //Library for using CAN Communication

#define BTN1 4
#define BTN2 5
#define LED1 6
#define LED2 7
#define CAN_ID_REAR_LIGHT_RIGHT 0xB0

// DIM, BRAKE, BLINK, NA, NA, NA, NA
int data[7] = {0, 0, 0, 0, 0, 0, 0};

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


void setup() 
{
  pinMode(BTN1, INPUT_PULLUP);
  pinMode(BTN2, INPUT_PULLUP);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  
  Serial.begin(9600);
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
  Serial.println("Message Sent");
}

void loop() 
{
  int btn1reading = digitalRead(BTN1);
  int btn2reading = digitalRead(BTN2);
  
  if (btn1reading != lastButton1State) { lastDebounce1Time = millis(); }
  if (btn2reading != lastButton2State) { lastDebounce2Time = millis(); }
  
  if ((millis() - lastDebounce1Time) > debounceDelay) {
    if (btn1reading != button1State) {
      button1State = btn1reading;
      
      if (button1State == LOW) {
        Serial.println("Button 1 pressed");
        if(!dimlight){ 
          dimlight = true; 
          digitalWrite(LED1, HIGH);
          Serial.println("Turning on light");
          data[0] = 1;
          send_can_message(CAN_ID_REAR_LIGHT_RIGHT, data); 
        } else { 
          dimlight = false; 
          digitalWrite(LED1, LOW); 
          Serial.println("Turning off light");
          data[0] = 0;
          send_can_message(CAN_ID_REAR_LIGHT_RIGHT, data); 
        } 
      }
    }
  }

  if ((millis() - lastDebounce2Time) > debounceDelay) {
    if (btn2reading != button2State) {
      button2State = btn2reading;
      
      if (button2State == LOW) {
        Serial.println("Button 2 pressed");
          is_blinking = true; 
          digitalWrite(LED2, HIGH);
          Serial.println("Turning on blinker");
          data[2] = 1;
          send_can_message(CAN_ID_REAR_LIGHT_RIGHT, data);
      } else {
        Serial.println("Button 2 released");
        data[2] = 0;
        send_can_message(CAN_ID_REAR_LIGHT_RIGHT, data);
      }
    }
  }

  lastButton1State = btn1reading;
  lastButton2State = btn2reading;
}
