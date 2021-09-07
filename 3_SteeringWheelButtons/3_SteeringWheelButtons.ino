
#define btnOk A0
#define btnMode A1
#define btnFlipper A2
#define rotaryInputRight A3
#define rotaryInputLeft A4
#define leds 6

unsigned long lastDebounceTimeBtnOk = 0; 
unsigned long lastDebounceTimeBtnMode = 0; 
unsigned long lastDebounceTimeBtnFlipper = 0;

bool btnOkPressed = false;
bool btnModePressed = false;
bool btnFlipperPressed = false;
bool ledsOn = false;

bool btnOkPressTriggered = false;
bool btnOkHoldTriggered = false;
bool btnModePressTriggered = false;
bool btnModeHoldTriggered = false;
bool btnFlipperPressTriggered = false;
bool btnFlipperHoldTriggered = false;

unsigned long debouncePressDelay = 50;
unsigned long debounceHoldDelay = 750;

int rotaryInputRightValue;
int rotaryInputLeftValue;
int rotaryInputRightValuePrevious;
int rotaryInputLeftValuePrevious;

bool assumeUp = false;
bool assumeDown = false;
bool rotaryInHighRange = false;
int rotaryTimeOut = 0;

void setup() {
  // declare pin 9 to be an output:
  pinMode(leds, OUTPUT);
  pinMode(btnOk, INPUT_PULLUP);
  pinMode(btnMode, INPUT_PULLUP);
  pinMode(btnFlipper, INPUT_PULLUP);
  pinMode(rotaryInputLeft, INPUT);

  Serial.begin(115200);

  analogWrite(leds, 0);

  if(analogRead(rotaryInputLeft) > 10){
    rotaryInHighRange = true;;
  }
}

void loop() {
  int btnOkReading = analogRead(btnOk);
  int btnModeReading = analogRead(btnMode);
  int btnFlipperReading = analogRead(btnFlipper);

  // =================== BUTTON Ok ===================
  if (btnOkReading < 125) { 
    if(btnOkPressed == false){
      lastDebounceTimeBtnOk = millis(); 
      btnOkPressed = true;
    }
  } else {
    btnOkPressed = false;
    btnOkPressTriggered = false;
    btnOkHoldTriggered = false;
  }  
  if ((millis() - lastDebounceTimeBtnOk) > debouncePressDelay) {
    if (btnOkReading < 125 && !btnOkPressTriggered) {
      Serial.println("Button OK Pressed");
      btnOkPressTriggered = true;
      
    }
  }

   if ((millis() - lastDebounceTimeBtnOk) > debounceHoldDelay) {
    if (btnOkReading < 125 && !btnOkHoldTriggered) {
      Serial.println("Button OK Hold");
      btnOkHoldTriggered = true;
    }
  }

  // =================== BUTTON Mode ===================
  if (btnModeReading < 125) { 
    if(btnModePressed == false){
      lastDebounceTimeBtnMode = millis(); 
      btnModePressed = true;
    }
  } else {
    btnModePressed = false;
    btnModePressTriggered = false;
    btnModeHoldTriggered = false;
  }  
  if ((millis() - lastDebounceTimeBtnMode) > debouncePressDelay) {
    if (btnModeReading < 125 && !btnModePressTriggered) {
      Serial.println("Button Mode Pressed");
      btnModePressTriggered = true;
      
    }
  }

   if ((millis() - lastDebounceTimeBtnMode) > debounceHoldDelay) {
    if (btnModeReading < 125 && !btnModeHoldTriggered) {
      Serial.println("Button Mode Hold");
      btnModeHoldTriggered = true;
    }
  }

  // =================== BUTTON Flipper ===================
  if (btnFlipperReading < 125) { 
    if(btnFlipperPressed == false){
      lastDebounceTimeBtnFlipper = millis(); 
      btnFlipperPressed = true;
    }
  } else {
    btnFlipperPressed = false;
    btnFlipperPressTriggered = false;
    btnFlipperHoldTriggered = false;
  }  
  if ((millis() - lastDebounceTimeBtnFlipper) > debouncePressDelay) {
    if (btnFlipperReading < 125 && !btnFlipperPressTriggered) {
      Serial.println("Button Flipper Pressed");
      btnFlipperPressTriggered = true;
      
    }
  }

   if ((millis() - lastDebounceTimeBtnFlipper) > debounceHoldDelay) {
    if (btnFlipperReading < 125 && !btnFlipperHoldTriggered) {
      Serial.println("Button Flipper Hold");
      btnFlipperHoldTriggered = true;

      //For Testing purposes
      if(!ledsOn){
        analogWrite(leds, 255);
        ledsOn = true;
      } else {
        analogWrite(leds, 0);
        ledsOn = false;
      }
      
    }
  }

  // =================== ROTARY ENCODER ===================
  rotaryInputLeftValue = analogRead(rotaryInputLeft);
  rotaryInputRightValue = analogRead(rotaryInputRight);

  if(rotaryTimeOut > 0){
    rotaryTimeOut = rotaryTimeOut - 1;
  } else {
    if(rotaryInHighRange){
      if(rotaryInputRightValue < 150){
      
        if(!assumeDown){
          assumeUp = true;
        } else {
          Serial.println("!! Encoder Left - Rotation Up");
          assumeDown = false;
          rotaryInHighRange = false;
          rotaryTimeOut = 100;
        }
      }
      // if first, assume scrollwhell is going down
      if(rotaryInputLeftValue < 150){
      
        if(!assumeUp){
          assumeDown = true;
        } else {
          Serial.println("! Encoder Left - Rotation Down");
          assumeUp = false;
          rotaryInHighRange = false;
          rotaryTimeOut = 100;
        }
      }
      // if first, assume scrollwhell is going up
      
    } else {
      if(rotaryInputRightValue > 75){
        if(!assumeDown){
          assumeUp = true;
        } else {
          Serial.println("!!!! Encoder Left - Rotation Up");
          assumeDown = false;
          rotaryInHighRange = true;
          rotaryTimeOut = 100;
        }
      }
      if(rotaryInputLeftValue > 75){
        if(!assumeUp){
          assumeDown = true;
        } else {
          Serial.println("!!! Encoder Left - Rotation Down");
          assumeUp = false;
          rotaryInHighRange = true;
          rotaryTimeOut = 100;
        }
      }
    

    }
  }
}
