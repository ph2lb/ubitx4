/*
 * Mods by PH2LB 
 */


#define PCF8574_I2C_ADDR 0x38


int btnDown(byte btn){
  byte p = getButton();
  if ( p == btn)
    return 1;
  return 0;
}

byte getButton() {
  if (hwBtnDown())
     return BUTTON_HW;

#if PCF8574_I2C_ADDR
  Wire.requestFrom(PCF8574_I2C_ADDR, 1);     
  byte c = Wire.read();  // receive a byte as character
  byte x = ~c;  // 255 = no buttons pressed
  
  //Serial.println((int)x);         // print the character

  if (x & 0x01)
    return BUTTON_1;
  else if (x & 0x02)
    return BUTTON_2;
   else if (x & 0x04)
    return BUTTON_3;
   else if (x & 0x08)
    return BUTTON_4;
  else if (x & 0x10)
    return BUTTON_5;
  else if (x & 0x20)
    return BUTTON_6;
  else if (x & 0x40)
    return BUTTON_7;
  else if (x & 0x80)
    return BUTTON_8; 
#else
 int button = analogRead(ANALOG_SPARE);
  
  if (button > 800)         // above 4v is up
    return BUTTON_NONE;
    
  if (button > 600)         
    return BUTTON_1;
  else if (button > 400)   
    return BUTTON_2;
  else if (button > 300)
    return BUTTON_3;         
  else if (button > 200)
    return BUTTON_4;         
  else 
    return BUTTON_5;    
#endif
    
  return BUTTON_NONE;
}
 
void doButtonMenu(int btn){
  while(getButton() == btn)
    delay(50);
  delay(50);

  if (btn == BUTTON_1)
    menuBandDown(BUTTON_1);
  else if (btn == BUTTON_2)
    menuBandUp(BUTTON_2);
  else if (btn == BUTTON_3)
    menuStepDown(BUTTON_3);
  else if (btn == BUTTON_4)
    menuStepUp(BUTTON_4);
  else if (btn == BUTTON_5)
    menuSidebandToggle(BUTTON_5);
  else if (btn == BUTTON_6)
    menuSidebandToggle(BUTTON_6);
  else if (btn == BUTTON_7)
    menuVfoToggle(BUTTON_7);
  else if (btn == BUTTON_8)
    menuVfoToggle(BUTTON_8);
  
  while(getButton() == btn)
    delay(50);
  delay(50);
}
 
 
 /*
 * by PH2LB : Simple step menu 
 * allows selection of 10..10Khz steps by menu 
 */
  
 
typedef struct {
  char * Text;
  uint32_t Step;
} 
StepStruct;

// define the stepstruct array
StepStruct  Steps [] = {
  { (char *)"10Hz", 10 }, 
  { (char *)"50Hz", 50 }, 
  { (char *)"100Hz", 100 }, 
  { (char *)"500Hz", 500 }, 
  { (char *)"1KHz", 1000 }, 
  { (char *)"5KHz", 5000 }, 
  { (char *)"10KHz", 10000 }
};

int getFrequencyStep() {
   return Steps[currentFreqStepIndex].Step;
}

int menuStep(int btn){
  int knob = 0;
  
  if (!btn){
   printLine2((char *)F("Step Select?"));
   return;
  }
  printLine2((char *)F("Press to confirm"));
  //wait for the button menu select button to be lifted)
  while (btnDown(btn))
    delay(50);
  delay(50);     
  printLine1(Steps[currentFreqStepIndex].Text);
  while(!btnDown(btn)){
    knob = enc_read();
    if (knob != 0){
      bool updateStep = false;

      if (knob > 0) {
         if (currentFreqStepIndex < STEPMAX) {
            currentFreqStepIndex++;
            updateStep = true;            
         }
      }
      if (knob < 0) {
         if (currentFreqStepIndex > STEPMIN) {
            currentFreqStepIndex--;
            updateStep = true;            
         }
      }
      if (updateStep) {
        printLine1(Steps[currentFreqStepIndex].Text);
      }
       
    }
    delay(100);
  }
 
  updateDisplay();
  menuOn = 0;
}

int menuStepUp(int btn){
   
  while (btnDown(btn))
    delay(50);
  delay(50);     
  
   if (currentFreqStepIndex < STEPMAX) {
      currentFreqStepIndex++;             
   }
     
  updateDisplay();
  menuOn = 0;
}


int menuStepDown(int btn){
   
  while (btnDown(btn))
    delay(50);
  delay(50);     
  
  if (currentFreqStepIndex > STEPMIN) {
    currentFreqStepIndex--;        
  }
     
  updateDisplay();
  menuOn = 0;
}

 /*
 * by PH2LB : Simple band menu 
 * allows selection of 80..10m bands by menu 
 */

 
// define the bandstruct
typedef struct 
{
  char * Text;
  uint32_t FreqLowerLimit; // lower limit acording to band plan
  uint32_t FreqUpperLimit; // upper limit according to bandplan 
  uint32_t Freq; // the current frequency on that band (set with default)
} 
BandStruct;


// define the bandstruct array (PA country full-license HF Band plan)
BandStruct  Bands [] =
{
  // (uint32_t) to prevent > warning: narrowing conversion of '1.86e+6' from 'double' to 'uint32_t {aka long unsigned int}' inside { } [-Wnarrowing]
  // (char *) to prevent > warning: deprecated conversion from string constant to 'char*' [-Wwrite-strings]
  { (char *)"80M", (uint32_t)3500e3,  (uint32_t)3800e3,  (uint32_t)3650e3  }, 
  { (char *)"60M", (uint32_t)53515e2, (uint32_t)53665e2, (uint32_t)53515e2 }, 
  { (char *)"40M", (uint32_t)7000e3,  (uint32_t)7200e3,  (uint32_t)7100e3  },
  { (char *)"30M", (uint32_t)10100e3, (uint32_t)10150e3, (uint32_t)10100e3 },  
  { (char *)"20M", (uint32_t)14000e3, (uint32_t)14350e3, (uint32_t)14175e3 },
  { (char *)"17M", (uint32_t)18068e3, (uint32_t)18168e3, (uint32_t)18068e3 },  
  { (char *)"15M", (uint32_t)21000e3, (uint32_t)21450e3, (uint32_t)21225e3 },
  { (char *)"12M", (uint32_t)24890e3, (uint32_t)24990e3, (uint32_t)24890e3 },  
  { (char *)"10M", (uint32_t)28000e3, (uint32_t)29700e3, (uint32_t)28225e3 },
  { (char *)"FUL", (uint32_t)3500e3,  (uint32_t)29700e3, (uint32_t)14200e3 },
};

long getFrequencyUpperLimit() {
   return Bands[currentBandIndex].FreqUpperLimit;
}

long getFrequencyLowerLimit() {
   return Bands[currentBandIndex].FreqLowerLimit;
}

void setCurrentBandFrequency(long frequency) {
  Bands[currentBandIndex].Freq = (uint32_t)frequency;
}

int menuBandUp(int btn){
   
  while (btnDown(btn))
    delay(50);
  delay(50);     
  
  if (currentBandIndex < BANDMAX) {
    currentBandIndex++;     
    setFrequency(Bands[currentBandIndex].Freq);        
  }
 
  updateDisplay();
  menuOn = 0;
}


int menuBandDown(int btn){
   
  while (btnDown(btn))
    delay(50);
  delay(50);     
  
  if (currentBandIndex > BANDMIN) {
    currentBandIndex--;        
    setFrequency(Bands[currentBandIndex].Freq);
  }
     
  updateDisplay();
  menuOn = 0;
}
