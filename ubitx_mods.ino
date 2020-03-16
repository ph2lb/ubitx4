/*
 * Mods by PH2LB 
 *  
 * Revision 4.3.3
 * - Used A7 for s-meter readout 
 *   adding AGC to my uBiX and used A7 for S meter reading
 *   hardware mod : www.ph2lb.nl/blog/index.php?page=ubitx-mods2#ubitx-mod9
 * - Used button5 for 'FAST' step (step *= 5)
 *  
 * Revision 4.3.2
 * - Fixed F() bug added EPROM storage and recovery.
 * 
 * Revision 4.3.1 
 * - Added PCF8574 support for more  buttons.
 * - Added Step select by menu and direct selection
 * - Added Band select by direct selection
 * - Used (char*)F("SOMESTRING") to reduce variablesize (F macro puts string in to flash see : http://playground.arduino.cc/Learning/Memory )
 * - changed usage of lower case l to upper case L for longs (better reading because lowercase l looks like a 1 in the arduino studio)
 * 
 * Revision 4.3.1 
 * - inital merge 2.0 with 4.3
 */


void printMeter()
{ 
   int meter_reading = analogRead(ANALOG_SPARE);
   int meter_char = 0;

  // Serial.println(meter_reading);

  if (meter_reading >= 200) // S9+
    meter_char = 10;
  else if (meter_reading >= 140) // S9 
    meter_char = 9;
  else if (meter_reading >= 100) // S8 
    meter_char = 8;
  else if (meter_reading >= 64) // S7 
    meter_char = 7;
 else if (meter_reading >= 38) // S6
    meter_char = 6;
 else if (meter_reading >= 32) // S5
    meter_char = 5;
 else if (meter_reading >= 16) // S4
    meter_char = 4;
 else if (meter_reading >= 8) // S3
    meter_char = 3;
 else if (meter_reading >= 4) // S2
    meter_char = 2;
  else if (meter_reading >= 2) // S1
    meter_char = 1;
   
  // 16 x 2
  // last 3 digits are for smeter (S0...S9, S9+ 0..9,10+
   lcd.setCursor(13, 0);
   lcd.print('S');
   lcd.setCursor(14, 0);
   if (meter_char <= 9)
   {
      char output = '0' + (char)meter_char; 
      lcd.print(output);
   }
   else
     lcd.print('9'); 
   lcd.setCursor(15, 0);
   if (meter_char >= 10) 
     lcd.print('+');
   else
     lcd.print(' '); 
}


#define PCF8574_BUTTONS_I2C_ADDR 0x38 
#define PCF8574_OUTPUT_I2C_ADDR 0x39

int btnDown(byte btn){
  byte p = getButton();
  if ( p == btn)
    return 1;
  return 0;
}

byte getButton() {
  if (hwBtnDown())
     return BUTTON_HW;

#if PCF8574_BUTTONS_I2C_ADDR
  Wire.requestFrom(PCF8574_BUTTONS_I2C_ADDR, 1);     
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
    menuSidebandUpDown(BUTTON_5);
  else if (btn == BUTTON_6)
    menuSidebandUpDown(BUTTON_6);
  else if (btn == BUTTON_7)
    menuVfoToggle(BUTTON_7);
  else if (btn == BUTTON_8)
    menuFastStep(BUTTON_8);
  
  while(getButton() == btn)
    delay(50);
  delay(50);
}

byte setPin(byte current, uint8_t pin, uint8_t value)
{
  if (value == LOW) 
  {
    current &= ~(1<<pin);
  }
  else 
  {
    current |= (1<<pin);
  }
  return current;
}

void initMods()
{
#if PCF8574_OUTPUT_I2C_ADDR 
  Wire.beginTransmission(PCF8574_OUTPUT_I2C_ADDR);
  Wire.write(0);
  Wire.endTransmission();   
#endif
}

void setMods()
{
  // do fancy stuff like setting cwFilter active 
 
#if PCF8574_OUTPUT_I2C_ADDR
  Wire.requestFrom(PCF8574_OUTPUT_I2C_ADDR, 1);     
  byte current = Wire.read();  // receive a byte as character

  // pin D0 = 1 > CW filer active
  byte data = setPin(current, 7, (uint8_t)isCw);

  Wire.beginTransmission(PCF8574_OUTPUT_I2C_ADDR);
  Wire.write(data);
  Wire.endTransmission();   
#endif
      
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
  { (char *)" 10Hz", 10 }, 
  { (char *)" 50Hz", 50 }, 
  { (char *)"100Hz", 100 }, 
  { (char *)"500Hz", 500 }, 
  { (char *)" 1KHz", 1000 }, 
  { (char *)" 5KHz", 5000 }, 
  { (char *)"10KHz", 10000 }
};

int getFrequencyStep() {
  if (fastStep)
    return Steps[currentFreqStepIndex].Step*5;
  else
    return Steps[currentFreqStepIndex].Step;
}

int menuStep(int btn){
  int knob = 0;
  
  if (!btn){
   printLine2(F("Step Select?"));
   return;
  }
  printLine2(F("Press to confirm"));
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
 
  menuOn = 0;
  updateDisplay(); 
}

void menuSidebandUpDown(int btn){

  if (btn == BUTTON_6)
  {
    if (isUSB == true && isCw == false) // USB > LSB
    {
       isUSB = false;
       isCw = false;
    }
    else if (isUSB == false && isCw == false) // LSB > CW
     {
       isUSB = false;
       isCw = true;
    }
    else if (isUSB == false && isCw == true) // CW > CWR
    {
       isUSB = true;
       isCw = true;
    }
    else if (isUSB == true && isCw == true) // CWR > USB
    {
       isUSB = true;
       isCw = false;
    }
  }
  else
  {
     if (isUSB == true && isCw == false) // USB > CWR
    {
       isUSB = true;
       isCw = true;
    }
    else if (isUSB == true && isCw == true) // CWR > CW
    {
       isUSB = false;
       isCw = true;
    }
    else if (isUSB == false && isCw == true) // CW > LSB
    {
       isUSB = false;
       isCw = false;
    }
    else if (isUSB == false && isCw == false) // LSB > USB
    {
       isUSB = true;
       isCw = false;
    }
  }
  
  //Added by KD8CEC
  if (vfoActive == VFO_B){
    isUsbVfoB = isUSB;
    isCwVfoB = isCw;
  }
  else {
    isUsbVfoA = isUSB; 
    isCwVfoA = isCw;
  }
  setMods();
  setFrequency(frequency);
  setCurrentBandFrequency(frequency);
  updateDisplay();  
} 
 
int menuStepUp(int btn){
   
  while (btnDown(btn))
    delay(50);
  delay(50);     
  
   if (currentFreqStepIndex < STEPMAX) {
      currentFreqStepIndex++;             
   } 
     
  menuOn = 0;
  updateDisplay(); 
}


int menuStepDown(int btn){
   
  while (btnDown(btn))
    delay(50);
  delay(50);     
  
  if (currentFreqStepIndex > STEPMIN) {
    currentFreqStepIndex--;        
  }
 
  menuOn = 0;
  updateDisplay(); 
}


int menuFastStep(int btn){
   
  while (btnDown(btn))
    delay(50);
  delay(50);     

  if (fastStep)
    fastStep = false;
  else 
    fastStep = true;
 
  menuOn = 0;
  updateDisplay(); 
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
  { (char *)"FUL", (uint32_t)1000e3,  (uint32_t)29700e3, (uint32_t)14200e3 },
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

void setBandForFrequency(long frequency) {
  for(int idx=0; idx<=BANDMAX; idx++) {
    // check if frequency is between lowerlimit and upperlimit of the band
    if (frequency >= Bands[idx].FreqLowerLimit && frequency <= Bands[idx].FreqUpperLimit) {
      // found it
      currentBandIndex = idx;
      break;
    }
  }
  setCurrentBandFrequency(frequency);
}

int menuBandUp(int btn){
   
  while (btnDown(btn))
    delay(50);
  delay(50);     
  
  if (currentBandIndex < BANDMAX) {
    currentBandIndex++;   
    setFrequency(Bands[currentBandIndex].Freq);   
    if (Bands[currentBandIndex].Freq > 10000000L)
      isUSB = true;
    else
      isUSB = false;     
  }
 
  menuOn = 0;
  updateDisplay(); 
}


int menuBandDown(int btn){
   
  while (btnDown(btn))
    delay(50);
  delay(50);     
  
  if (currentBandIndex > BANDMIN) {
    currentBandIndex--;         
    setFrequency(Bands[currentBandIndex].Freq);
    if (Bands[currentBandIndex].Freq > 10000000L)
      isUSB = true;
    else
      isUSB = false; 
  }
     
  menuOn = 0;
  updateDisplay();
}

