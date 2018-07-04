/** Menus
 *  The Radio menus are accessed by tapping on the function button. 
 *  - The main loop() constantly looks for a button press and calls doMenu() when it detects
 *  a function button press. 
 *  - As the encoder is rotated, at every 10th pulse, the next or the previous menu
 *  item is displayed. Each menu item is controlled by it's own function.
 *  - Eache menu function may be called to display itself
 *  - Each of these menu routines is called with a button parameter. 
 *  - The btn flag denotes if the menu itme was clicked on or not.
 *  - If the menu item is clicked on, then it is selected,
 *  - If the menu item is NOT clicked on, then the menu's prompt is to be displayed
 */
 

/*
* A generic control to read variable values
*/
int getValueByKnob(int btn, int minimum, int maximum, int step_size,  int initial, char* prefix, char *postfix)
{
    int knob = 0;
    int knob_value;

    while (btnDown(btn))
      active_delay(100);

    active_delay(200);
    knob_value = initial;
     
    strcpy(b, prefix);
    itoa(knob_value, c, 10);
    strcat(b, c);
    strcat(b, postfix);
    printLine2(b);
    active_delay(300);

    while(!btnDown(btn) && digitalRead(PTT) == HIGH){

      knob = enc_read();
      if (knob != 0){
        if (knob_value > minimum && knob < 0)
          knob_value -= step_size;
        if (knob_value < maximum && knob > 0)
          knob_value += step_size;

        printLine2(prefix);
        itoa(knob_value, c, 10);
        strcpy(b, c);
        strcat(b, postfix);
        printLine1(b);
      }
      checkCAT();
    }

   return knob_value;
}

//# Menu: 1

int menuBand(int btn){
  int knob = 0;
  int band;
  unsigned long offset;

 // band = frequency/1000000l;
 // offset = frequency % 1000000l;
    
  if (!btn){
   printLine2((char*)F("Band Select    \x7E"));
   return;
  }

  printLine2((char*)F("Band Select:"));
  //wait for the button menu select button to be lifted)
  while (btnDown(btn))
    active_delay(50);
  active_delay(50);    
  ritDisable();
 menuOn = 1;
  while(!btnDown(btn)){
    knob = enc_read();
    if (knob != 0){
      /*
      if (band > 3 && knob < 0)
        band--;
      if (band < 30 && knob > 0)
        band++; 
      if (band > 10)
        isUSB = true;
      else
        isUSB = false;
      setFrequency(((unsigned long)band * 1000000L) + offset); */
      if (knob < 0 && frequency > LOWEST_FREQ)
        setFrequency(frequency - 200000L);
      if (knob > 0 && frequency < HIGHEST_FREQ)
        setFrequency(frequency + 200000L);
      if (frequency > 10000000L)
        isUSB = true;
      else
        isUSB = false;
      updateDisplay();
    }
    checkCAT();
    active_delay(20);
  }

  while(btnDown(btn))
    active_delay(50);
  active_delay(50);
  
  printLine2((char*)F(""));
  updateDisplay();
  menuOn = 0;
}

// Menu #2
void menuRitToggle(int btn){
  if (!btn){
    if (ritOn == 1)
      printLine2((char*)F("RIT On \x7E Off"));
    else
      printLine2((char*)F("RIT Off \x7E On"));
  }
  else {
    if (ritOn == 0){
      //enable RIT so the current frequency is used at transmit
      ritEnable(frequency);
      printLine2((char*)F("RIT is On"));
 
    }
    else{
      ritDisable();
      printLine2((char*)F("RIT is Off"));
    }
    menuOn = 0;
    active_delay(500);
    printLine2((char*)F(""));
    updateDisplay();
  }
}


//Menu #3
void menuVfoToggle(int btn){
  
  if (!btn){
    if (vfoActive == VFO_A)
      printLine2((char*)F("VFO A \x7E B"));
    else
      printLine2((char*)F("VFO B \x7E A"));
  }
  else {
    if (vfoActive == VFO_B){
      vfoB = frequency;
      isUsbVfoB = isUSB;
      currentBandIndexVfoB = currentBandIndex;
      currentFreqStepIndexVfoB = currentFreqStepIndex;

      // write storage
      EEPROM.put(VFO_B, frequency);
      if (isUsbVfoB)
        EEPROM.put(VFO_B_MODE, VFO_MODE_USB);
      else
        EEPROM.put(VFO_B_MODE, VFO_MODE_LSB);

      // new by PH2LB
      EEPROM.put(VFO_B_BAND, currentBandIndex);
      EEPROM.put(VFO_B_STEP, currentFreqStepIndex);

      vfoActive = VFO_A;
      frequency = vfoA;
      isUSB = isUsbVfoA;

      // new by PH2LB
      currentBandIndex = currentBandIndexVfoA;
      currentFreqStepIndex = currentFreqStepIndexVfoA;
    }
    else {
      vfoA = frequency;
      isUsbVfoA = isUSB;
      currentBandIndexVfoA = currentBandIndex;
      currentFreqStepIndexVfoA = currentFreqStepIndex;
      EEPROM.put(VFO_A, frequency);
      if (isUsbVfoA)
        EEPROM.put(VFO_A_MODE, VFO_MODE_USB);
      else
        EEPROM.put(VFO_A_MODE, VFO_MODE_LSB);

      // new by PH2LB
      EEPROM.put(VFO_A_BAND, currentBandIndex);
      EEPROM.put(VFO_A_STEP, currentFreqStepIndex);

      vfoActive = VFO_B;
      frequency = vfoB;
      isUSB = isUsbVfoB;

       // new by PH2LB
      currentBandIndex = currentBandIndexVfoB;
      currentFreqStepIndex = currentFreqStepIndexVfoB;
    }
      
    ritDisable();
    setFrequency(frequency);
    updateDisplay();
    //printLine2((char*)F(""));
    //exit the menu
    menuOn = 0;
  }
}

// Menu #4
void menuSidebandToggle(int btn){
  if (!btn){
    if (isUSB == true)
      printLine2((char*)F("USB \x7E LSB"));
    else
      printLine2((char*)F("LSB \x7E USB"));
  }
  else {
    if (isUSB == true){
      isUSB = false;
//      printLine2((char*)F("LSB Selected"));
//      active_delay(500);
//      printLine2((char*)F("");
    }
    else {
      isUSB = true;
//      printLine2((char*)F("USB Selected"));
//      active_delay(500);
//      printLine2((char*)F(""));
    }
    //Added by KD8CEC
    if (vfoActive == VFO_B){
      isUsbVfoB = isUSB;
    }
    else {
      isUsbVfoB = isUSB;
    }
    updateDisplay();
    menuOn = 0;
  }
}

//Split communication using VFOA and VFOB by KD8CEC
//Menu #5
void menuSplitToggle(int btn){
  if (!btn){
    if (splitOn == 0)
      printLine2((char*)F("Split Off \x7E On"));
    else
      printLine2((char*)F("Split On \x7E Off"));
  }
  else {
    if (splitOn == 1){
      splitOn = 0;
      printLine2((char*)F("Split ON"));
    }
    else {
      splitOn = 1;
      if (ritOn == 1)
        ritOn = 0;
      printLine2((char*)F("Split Off"));
    }
    active_delay(500);
    printLine2((char*)F(""));
    updateDisplay();
    menuOn = 0;
  }
}


int menuCWSpeed(int btn){
    int knob = 0;
    int wpm;

    wpm = 1200/cwSpeed;
     
    if (!btn){
     strcpy(b, (char *)F("CW: "));
     itoa(wpm,c, 10);
     strcat(b, c);
     strcat(b, (char *)F(" WPM     \x7E"));
     printLine2(b);
     return;
    }

/*
    printLine1("Press FN to Set");
    strcpy(b, "5:CW>");
    itoa(wpm,c, 10);
    strcat(b, c);
    strcat(b, " WPM");
    printLine2(b);
    active_delay(300);

    while(!btnDown(btn) && digitalRead(PTT) == HIGH){

      knob = enc_read();
      if (knob != 0){
        if (wpm > 3 && knob < 0)
          wpm--;
        if (wpm < 50 && knob > 0)
          wpm++;

        strcpy(b, "5:CW>");
        itoa(wpm,c, 10);
        strcat(b, c);
        strcat(b, " WPM");
        printLine2(b);
      }
      //abort if this button is down
      if (btnDown(btn))
        //re-enable the clock1 and clock 2
        break;
      checkCAT();
    }
  */
    wpm = getValueByKnob(btn, 1, 100, 1,  wpm, "CW: ", " WPM>");
  
    printLine2((char*)F("CW Speed set!"));
    cwSpeed = 1200/wpm;
    EEPROM.put(CW_SPEED, cwSpeed);
    active_delay(500);
     
    printLine2((char*)F(""));
    updateDisplay();
    menuOn = 0;
}

void menuExit(int btn){

  if (!btn){
      printLine2((char*)F("Exit Menu      \x7E"));
  }
  else{
      printLine2((char*)F("Exiting..."));
      active_delay(500);
      printLine2((char*)F(""));
      updateDisplay();
      menuOn = 0;
  }
}

/*
 * The calibration routines are not normally shown in the menu as they are rarely used
 * They can be enabled by choosing this menu option
 */
int menuSetup(int btn){
  if (!btn){
    if (!modeCalibrate)
      printLine2((char*)F("Settings       \x7E"));
    else
      printLine2((char*)F("Settings \x7E Off"));
  }else {
    if (!modeCalibrate){
      modeCalibrate = true;
      printLine2((char*)F("Settings On"));
    }
    else {
      modeCalibrate = false;
      printLine2((char*)F("Settings Off"));      
    }
   while (btnDown(btn))
    delay(50);
   active_delay(500);
   printLine2((char*)F(""));
   return 10;
   menuOn = 0;
  }
  return 0;
}


/*
 * Take a deep breath, math(ematics) ahead
 * The 25 mhz oscillator is multiplied by 35 to run the vco at 875 mhz
 * This is divided by a number to generate different frequencies.
 * If we divide it by 875, we will get 1 mhz signal
 * So, if the vco is shifted up by 875 hz, the generated frequency of 1 mhz is shifted by 1 hz (875/875)
 * At 12 Mhz, the carrier will needed to be shifted down by 12 hz for every 875 hz of shift up of the vco
 * 
 */

 //this is used by the si5351 routines in the ubitx_5351 file
extern int32_t calibration;
extern uint32_t si5351bx_vcoa;

int calibrateClock(int btn){
  int knob = 0;
  int32_t prev_calibration;
 
  //keep clear of any previous button press
  while (btnDown(btn))
    delay(100);
  delay(100);
   
  if (!btn){
    printLine2((char*)F("Set Calibration?"));
    return 0;
  }


  digitalWrite(TX_LPF_A, 0);
  digitalWrite(TX_LPF_B, 0);
  digitalWrite(TX_LPF_C, 0);

  prev_calibration = calibration;
  calibration = 0;

  isUSB = true;

  //turn off the second local oscillator and the bfo
  si5351_set_calibration(calibration);
  startTx(TX_CW);
  si5351bx_setfreq(2, 10000000L); 
  
  strcpy(b, (char *)F("#1 10 MHz cal:"));
  ltoa(calibration/8750, c, 10);
  strcat(b, c);
  printLine2(b);     

  while (!btnDown(btn))
  {

    if (digitalRead(PTT) == LOW && !keyDown)
      cwKeydown();
    if (digitalRead(PTT)  == HIGH && keyDown)
      cwKeyUp();
      
    knob = enc_read();

    if (knob > 0)
      calibration += 875;
    else if (knob < 0)
      calibration -= 875;
    else 
      continue; //don't update the frequency or the display
      
    si5351_set_calibration(calibration);
    si5351bx_setfreq(2, 10000000L);
    strcpy(b, (char*)F("#1 10 MHz cal:"));
    ltoa(calibration/8750, c, 10);
    strcat(b, c);
    printLine2(b);     
  }

  cwTimeout = 0;
  keyDown = 0;
  stopTx();

  printLine2((char*)F("Calibration set!"));
  EEPROM.put(MASTER_CAL, calibration);
  initOscillators();
  setFrequency(frequency);    
  updateDisplay();

  while(btnDown(btn))
    delay(50);
  delay(100);
}


 

int menuSetupCalibration(int btn){
  int knob = 0;
  int32_t prev_calibration;
   
  if (!btn){
    printLine2((char*)F("Setup:Calibrate\x7E"));
    return 0;
  }

  printLine1((char*)F("Press PTT & tune"));
  printLine2((char*)F("to exactly 10 MHz"));
  active_delay(2000);
  calibrateClock(btn);
}

void printCarrierFreq(unsigned long freq){

  memset(c, 0, sizeof(c));
  memset(b, 0, sizeof(b));

  ultoa(freq, b, DEC);
  
  strncat(c, b, 2);
  strcat(c, ".");
  strncat(c, &b[2], 3);
  strcat(c, ".");
  strncat(c, &b[5], 1);
  printLine2(c);    
}

void menuSetupCarrier(int btn){
  int knob = 0;
  unsigned long prevCarrier;
   
  if (!btn){
      printLine2((char*)F("Setup:BFO      \x7E"));
    return;
  }

  prevCarrier = usbCarrier;
  printLine1((char*)F("Tune to best Signal"));  
  printLine2((char*)F("Press to confirm. "));
  active_delay(1000);

  usbCarrier = 11995000l;
  si5351bx_setfreq(0, usbCarrier);
  printCarrierFreq(usbCarrier);

  //disable all clock 1 and clock 2 
  while (digitalRead(PTT) == HIGH &&  !btnDown(btn))
  {
    knob = enc_read();

    if (knob > 0)
      usbCarrier -= 50;
    else if (knob < 0)
      usbCarrier += 50;
    else
      continue; //don't update the frequency or the display
      
    si5351bx_setfreq(0, usbCarrier);
    printCarrierFreq(usbCarrier);
    
    delay(100);
  }

  //save the setting
  /*
  if (digitalRead(PTT) == LOW){
    printLine2("Carrier set!    ");
    EEPROM.put(USB_CAL, usbCarrier);
    delay(1000);
  }
  else 
    usbCarrier = prevCarrier;
  */
  printLine2((char*)F("Carrier set!    "));
  EEPROM.put(USB_CAL, usbCarrier);
  active_delay(1000);
  
  si5351bx_setfreq(0, usbCarrier);          
  setFrequency(frequency);    
  updateDisplay();
  printLine2((char*)F(""));
  menuOn = 0; 
}

void menuSetupCwTone(int btn){
  int knob = 0;
  int prev_sideTone;
     
  if (!btn){
    printLine2((char*)F("Setup:CW Tone  \x7E"));
    return;
  }

  prev_sideTone = sideTone;
  printLine1((char*)F("Tune CW tone"));  
  printLine2((char*)F("PTT to confirm. "));
  active_delay(1000);
  tone(CW_TONE, sideTone);

  //disable all clock 1 and clock 2 
  while (digitalRead(PTT) == HIGH && !btnDown(btn))
  {
    knob = enc_read();

    if (knob > 0 && sideTone < 2000)
      sideTone += 10;
    else if (knob < 0 && sideTone > 100 )
      sideTone -= 10;
    else
      continue; //don't update the frequency or the display
        
    tone(CW_TONE, sideTone);
    itoa(sideTone, b, 10);
    printLine2(b);

    checkCAT();
    active_delay(20);
  }
  noTone(CW_TONE);
  //save the setting
  if (digitalRead(PTT) == LOW){
    printLine2((char*)F("Sidetone set!    "));
    EEPROM.put(CW_SIDETONE, sideTone);
    active_delay(2000);
  }
  else
    sideTone = prev_sideTone;
    
  printLine2((char*)F(""));  
  updateDisplay(); 
  menuOn = 0;
}

void menuSetupCwDelay(int btn){
  int knob = 0;
  int prev_cw_delay;

  if (!btn){
    printLine2((char*)F("Setup:CW Delay \x7E"));
    return;
  }

  active_delay(500);
  prev_cw_delay = cwDelayTime;
  cwDelayTime = getValueByKnob(btn, 10, 1000, 50,  cwDelayTime, "CW Delay>", " msec");

  printLine1((char*)F("CW Delay Set!"));  
  printLine2((char*)F(""));
  active_delay(500);
  updateDisplay();
  menuOn = 0;
}

void menuSetupKeyer(int btn){
  int tmp_key, knob;
  
  if (!btn){
    if (!Iambic_Key)
      printLine2((char*)F("Setup:CW(Hand)\x7E"));
    else if (keyerControl & IAMBICB)
      printLine2((char*)F("Setup:CW(IambA)\x7E"));
    else 
      printLine2((char*)F("Setup:CW(IambB)\x7E"));    
    return;
  }
  
  active_delay(500);

  if (!Iambic_Key)
    tmp_key = 0; //hand key
  else if (keyerControl & IAMBICB)
    tmp_key = 2; //Iambic B
  else 
    tmp_key = 1;
 
  while (!btnDown(btn))
  {
    knob = enc_read();
    if (knob < 0 && tmp_key > 0)
      tmp_key--;
    if (knob > 0)
      tmp_key++;

    if (tmp_key > 2)
      tmp_key = 0;
      
    if (tmp_key == 0)
      printLine1((char*)F("Hand Key?"));
    else if (tmp_key == 1)
      printLine1((char*)F("Iambic A?"));
    else if (tmp_key == 2)  
      printLine1((char*)F("Iambic B?"));  
  }

  active_delay(500);
  if (tmp_key == 0)
    Iambic_Key = false;
  else if (tmp_key == 1){
    Iambic_Key = true;
    keyerControl &= ~IAMBICB;
  }
  else if (tmp_key == 2){
    Iambic_Key = true;
    keyerControl |= IAMBICB;
  }
  
  EEPROM.put(CW_KEY_TYPE, tmp_key);
  
  printLine1((char*)F("Keyer Set!"));
  active_delay(600);
  printLine1((char*)F(""));

  //Added KD8CEC
  printLine2((char*)F(""));
  updateDisplay(); 
  menuOn = 0;  
}

void menuReadADC(int btn){
  int adc;
  
  if (!btn){
    printLine2((char*)F("Setup:Read ADC\x7E"));
    return;
  }
  delay(500);

  while (!btnDown(btn)){
    adc = analogRead(ANALOG_KEYER);
    itoa(adc, b, 10);
    printLine1(b);
  }

  printLine1((char*)F(""));
  updateDisplay();
}


void doMenu(byte btn){
  int select=0, i,btnState;

  //wait for the button to be raised up
  while(btnDown(btn)) 
    delay(50);
  delay(50);  //debounce
  
  menuOn = 2;
  
  while (menuOn){
    i = enc_read();
    btnState = btnDown(btn);

    if (btnState != BUTTON_NONE)
      btnState = btn;

    if (i > 0){
      if (modeCalibrate && select + i < 160)
        select += i;
      if (!modeCalibrate && select + i < 90)
        select += i;
    }
    if (i < 0 && select - i >= 0)
      select += i;      //caught ya, i is already -ve here, so you add it

    if (select < 10)
      menuStep(btnState);
    else if (select < 20)
      menuBand(btnState);
    else if (select < 30)
      menuBand(btnState);
    else if (select < 40)
      menuRitToggle(btnState);
    else if (select < 50)
      menuVfoToggle(btnState);
    else if (select < 60)
      menuSidebandToggle(btnState);
    else if (select < 70)
      menuSplitToggle(btnState);
    else if (select < 80)
      menuCWSpeed(btnState);
    else if (select < 90)
      select += menuSetup(btnState);
    else if (select < 100 && !modeCalibrate)
      menuExit(btnState);
    else if (select < 110 && modeCalibrate)
      menuSetupCalibration(btnState);   //crystal
    else if (select < 120 && modeCalibrate)
      menuSetupCarrier(btnState);       //lsb
    else if (select < 130 && modeCalibrate)
      menuSetupCwTone(btnState);
    else if (select < 140 && modeCalibrate)
      menuSetupCwDelay(btnState);
    else if (select < 150 && modeCalibrate)
      menuReadADC(btnState);
    else if (select < 160 && modeCalibrate)
        menuSetupKeyer(btnState);
    else
      menuExit(btnState);  
  }

  //debounce the button
  while(btnDown(btn))
    delay(50);
  delay(50);
  checkCAT();
}

