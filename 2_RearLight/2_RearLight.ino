
#include <SPI.h>   
#include <mcp2515.h>
#include "FastLED.h"

#define DATA_PIN 3
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS 5
#define BRIGHTNESS 96

int ledsDimLight[] = {0, 1};
int ledsBrakeLight[] = {0, 1};
int ledsBlinker[] = {2, 3, 4};

bool dimlight = false;
bool is_braking = false;
bool is_blinking = false;

bool state_dimlight = false;
bool state_brakelight = false;
bool state_blinklight = false;

bool blink_cycle_started = false;
unsigned long currentMillis = 0;
unsigned long cycle_millis;
unsigned long timeDifference;

CRGB leds[NUM_LEDS];

struct can_frame canMsg; 
MCP2515 mcp2515(10);   

void showProgramCleanUp() {
  for (int i = 0; i < NUM_LEDS; ++i) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
}

// switches off all LEDs
void showLightDim() {
  for(int i = 0; i < 2; i++) {
    if(dimlight && !is_braking){
      leds[ledsDimLight[i]] = CRGB::Red;
      leds[ledsDimLight[i]].maximizeBrightness(75);
      state_dimlight = true;
    }
    else if (!dimlight && !is_braking){
      leds[ledsDimLight[i]] = CRGB::Black;
      leds[ledsDimLight[i]].maximizeBrightness(75);
      state_dimlight = false;
    }
    
  }
  FastLED.show();
}

void showLightBrake() {
  for(int i = 0; i < 2; i++) {
    if(is_braking){
      leds[ledsDimLight[i]] = CRGB::Red;
      leds[ledsDimLight[i]].maximizeBrightness(255);
      state_brakelight = true;
    }
    else if (dimlight && !is_braking){
      leds[ledsDimLight[i]] = CRGB::Red;
      leds[ledsDimLight[i]].maximizeBrightness(75);
      state_brakelight = false;
    }
    else if (!dimlight && !is_braking){
      leds[ledsDimLight[i]] = CRGB::Black;
      leds[ledsDimLight[i]].maximizeBrightness(75);
      state_brakelight = false;
    }
    
  }
  FastLED.show();
}

void showLightBlink() {

  if(!blink_cycle_started){
    blink_cycle_started = true;
    cycle_millis = currentMillis;
  } else {
    timeDifference = currentMillis - cycle_millis;
    
    if(timeDifference > 0 && timeDifference < 100){
      leds[ledsBlinker[0]] = CRGB::Orange;
      FastLED.show();
    }
    if(timeDifference > 250 && timeDifference < 350){
      leds[ledsBlinker[1]] = CRGB::Orange;
      FastLED.show();
    }
    if(timeDifference > 500 && timeDifference < 600){
      leds[ledsBlinker[2]] = CRGB::Orange;
      FastLED.show();
    }
    if(timeDifference > 900){
      for(int i = 0; i < 3; i++) {
        leds[ledsBlinker[i]] = CRGB::Black;
      }
      FastLED.show();
    }
    if(timeDifference > 1500){
      blink_cycle_started = false;
    }
  }
}

void checkSerialInput() {
  if(Serial.available() > 0){
    char serialInput = Serial.read();

    if(serialInput == 'B'){
      if(is_blinking){
        Serial.println("Blinking OFF");
        is_blinking = false;
      } else {
        Serial.println("Blinking ON");
        is_blinking = true;
      }
    } 
    else if(serialInput == 'R'){
      if(is_braking){
        Serial.println("Braking OFF");
        is_braking = false;
      } else {
        Serial.println("Braking ON");
        is_braking = true;
      }
    } 
    else if(serialInput == 'D'){
      if(dimlight){
        Serial.println("Dimlight OFF");
        dimlight = false;
      } else {
        Serial.println("Dimlight ON");
        dimlight = true;
      }
    } 
  }
}

void setup() {
  SPI.begin();  
  Serial.begin(9600);
  
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip); // initializes LED strip
  FastLED.setBrightness(BRIGHTNESS);// global brightness

  mcp2515.reset();                          
  mcp2515.setBitrate(CAN_500KBPS,MCP_8MHZ); //Sets CAN at speed 500KBPS and Clock 8MHz 
  mcp2515.setNormalMode(); 

  showProgramCleanUp();
}

// main program
void loop() {
  currentMillis = millis();
  checkSerialInput();    

  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) // To receive data (Poll Read)
  {
     dimlight = canMsg.data[0];         
     is_blinking = canMsg.data[2]; 

     Serial.println("Received data");
  }      

  if(dimlight != state_dimlight){ showLightDim(); }

  if(is_braking != state_brakelight){ showLightBrake(); }

  if(is_blinking || blink_cycle_started){ showLightBlink(); }

}
