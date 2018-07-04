/**
 CW Keyer
 CW Key logic change with ron's code (ubitx_keyer.cpp)
 Ron's logic has been modified to work with the original uBITX by KD8CEC

 Original Comment ----------------------------------------------------------------------------
 * The CW keyer handles either a straight key or an iambic / paddle key.
 * They all use just one analog input line. This is how it works.
 * The analog line has the internal pull-up resistor enabled. 
 * When a straight key is connected, it shorts the pull-up resistor, analog input is 0 volts
 * When a paddle is connected, the dot and the dash are connected to the analog pin through
 * a 10K and a 2.2K resistors. These produce a 4v and a 2v input to the analog pins.
 * So, the readings are as follows :
 * 0v - straight key
 * 1-2.5 v - paddle dot
 * 2.5 to 4.5 v - paddle dash
 * 2.0 to 0.5 v - dot and dash pressed
 * 
 * The keyer is written to transparently handle all these cases
 * 
 * Generating CW
 * The CW is cleanly generated by unbalancing the front-end mixer
 * and putting the local oscillator directly at the CW transmit frequency.
 * The sidetone, generated by the Arduino is injected into the volume control
 */

 //CW ADC Range
int cwAdcSTFrom = 0;
int cwAdcSTTo = 50;
int cwAdcBothFrom = 51;
int cwAdcBothTo = 300;
int cwAdcDotFrom = 301;
int cwAdcDotTo = 600;
int cwAdcDashFrom = 601;
int cwAdcDashTo = 800;
//byte cwKeyType = 0; //0: straight, 1 : iambica, 2: iambicb

byte delayBeforeCWStartTime = 50;




// in milliseconds, this is the parameter that determines how long the tx will hold between cw key downs
//#define CW_TIMEOUT (600L)   //Change to CW Delaytime for value save to eeprom
#define PADDLE_DOT 1
#define PADDLE_DASH 2
#define PADDLE_BOTH 3
#define PADDLE_STRAIGHT 4

//we store the last padde's character 
//to alternatively send dots and dashes 
//when both are simultaneously pressed
char lastPaddle = 0;

//reads the analog keyer pin and reports the paddle
byte getPaddle(){
  int paddle = analogRead(ANALOG_KEYER);

  if (paddle > 800)         // above 4v is up
    return 0;
    
  if (paddle > 600)    // 4-3v is dot
    return PADDLE_DASH;
  else if (paddle > 300)    //1-2v is dash
    return PADDLE_DOT;
  else if (paddle > 50)
    return PADDLE_BOTH;     //both are between 1 and 2v
  else
    return PADDLE_STRAIGHT; //less than 1v is the straight key
}

/**
 * Starts transmitting the carrier with the sidetone
 * It assumes that we have called cwTxStart and not called cwTxStop
 * each time it is called, the cwTimeOut is pushed further into the future
 */
void cwKeydown(){
  keyDown = 1;                  //tracks the CW_KEY
  tone(CW_TONE, (int)sideTone); 
  digitalWrite(CW_KEY, 1);     

  //Modified by KD8CEC, for CW Delay Time save to eeprom
  //cwTimeout = millis() + CW_TIMEOUT;
  cwTimeout = millis() + cwDelayTime * 10;  
}

/**
 * Stops the cw carrier transmission along with the sidetone
 * Pushes the cwTimeout further into the future
 */
void cwKeyUp(){
  keyDown = 0;    //tracks the CW_KEY
  noTone(CW_TONE);
  digitalWrite(CW_KEY, 0);    
  
  //Modified by KD8CEC, for CW Delay Time save to eeprom
  //cwTimeout = millis() + CW_TIMEOUT;
  cwTimeout = millis() + cwDelayTime * 10;
}

//Variables for Ron's new logic
#define DIT_L 0x01 // DIT latch
#define DAH_L 0x02 // DAH latch
#define DIT_PROC 0x04 // DIT is being processed
#define PDLSWAP 0x08 // 0 for normal, 1 for swap
#define IAMBICB 0x10 // 0 for Iambic A, 1 for Iambic B
enum KSTYPE {IDLE, CHK_DIT, CHK_DAH, KEYED_PREP, KEYED, INTER_ELEMENT };
static unsigned long ktimer;
unsigned char keyerState = IDLE;

//Below is a test to reduce the keying error. do not delete lines
//create by KD8CEC for compatible with new CW Logic
char update_PaddleLatch(byte isUpdateKeyState) {
  unsigned char tmpKeyerControl = 0;
  
  int paddle = analogRead(ANALOG_KEYER);
  //diagnostic, VU2ESE
  //itoa(paddle, b, 10);
  //printLine2(b);

  if (paddle >= cwAdcDashFrom && paddle <= cwAdcDashTo)
    tmpKeyerControl |= DAH_L;
  else if (paddle >= cwAdcDotFrom && paddle <= cwAdcDotTo)
    tmpKeyerControl |= DIT_L;
  else if (paddle >= cwAdcBothFrom && paddle <= cwAdcBothTo)
    tmpKeyerControl |= (DAH_L | DIT_L) ;     
  else 
  {
    if (Iambic_Key)
      tmpKeyerControl = 0 ;
    else if (paddle >= cwAdcSTFrom && paddle <= cwAdcSTTo)
      tmpKeyerControl = DIT_L ;
     else
       tmpKeyerControl = 0 ; 
  }
  
  if (isUpdateKeyState == 1)
    keyerControl |= tmpKeyerControl;

  return tmpKeyerControl;
}

/*****************************************************************************
// New logic, by RON
// modified by KD8CEC
******************************************************************************/
void cwKeyer(void){
  lastPaddle = 0;
  bool continue_loop = true;
  unsigned tmpKeyControl = 0;
  
  if( Iambic_Key ) {
    while(continue_loop) {
      switch (keyerState) {
        case IDLE:
          tmpKeyControl = update_PaddleLatch(0);
          if ( tmpKeyControl == DAH_L || tmpKeyControl == DIT_L || 
            tmpKeyControl == (DAH_L | DIT_L) || (keyerControl & 0x03)) {
             update_PaddleLatch(1);
             keyerState = CHK_DIT;
          }else{
            if (0 < cwTimeout && cwTimeout < millis()){
              cwTimeout = 0;
              stopTx();
            }
            continue_loop = false;
          }
          break;
    
        case CHK_DIT:
          if (keyerControl & DIT_L) {
            keyerControl |= DIT_PROC;
            ktimer = cwSpeed;
            keyerState = KEYED_PREP;
          }else{
            keyerState = CHK_DAH;
          }
          break;
    
        case CHK_DAH:
          if (keyerControl & DAH_L) {
            ktimer = cwSpeed*3;
            keyerState = KEYED_PREP;
          }else{
            keyerState = IDLE;
          }
          break;
    
        case KEYED_PREP:
          //modified KD8CEC
          if (!inTx){
            //DelayTime Option
            active_delay(delayBeforeCWStartTime * 2);
            
            keyDown = 0;
            cwTimeout = millis() + cwDelayTime * 10;  //+ CW_TIMEOUT;
            startTx(TX_CW);
          }
          ktimer += millis(); // set ktimer to interval end time
          keyerControl &= ~(DIT_L + DAH_L); // clear both paddle latch bits
          keyerState = KEYED; // next state
          
          cwKeydown();
          break;
    
        case KEYED:
          if (millis() > ktimer) { // are we at end of key down ?
           cwKeyUp();
           ktimer = millis() + cwSpeed; // inter-element time
            keyerState = INTER_ELEMENT; // next state
          }else if (keyerControl & IAMBICB) {
            update_PaddleLatch(1); // early paddle latch in Iambic B mode
          }
          break;
    
        case INTER_ELEMENT:
          // Insert time between dits/dahs
          update_PaddleLatch(1); // latch paddle state
          if (millis() > ktimer) { // are we at end of inter-space ?
            if (keyerControl & DIT_PROC) { // was it a dit or dah ?
              keyerControl &= ~(DIT_L + DIT_PROC); // clear two bits
              keyerState = CHK_DAH; // dit done, check for dah
            }else{
              keyerControl &= ~(DAH_L); // clear dah latch
              keyerState = IDLE; // go idle
            }
          }
          break;
      }
  
      checkCAT();
    } //end of while
  }
  else{
    while(1){
      if (update_PaddleLatch(0) == DIT_L) {
        // if we are here, it is only because the key is pressed
        if (!inTx){
          //DelayTime Option
          active_delay(delayBeforeCWStartTime * 2);
          
          keyDown = 0;
          cwTimeout = millis() + cwDelayTime * 10;  //+ CW_TIMEOUT; 
          startTx(TX_CW);
        }
        cwKeydown();
        
        while ( update_PaddleLatch(0) == DIT_L ) 
          active_delay(1);
          
        cwKeyUp();
      }
      else{
        if (0 < cwTimeout && cwTimeout < millis()){
          cwTimeout = 0;
          keyDown = 0;
          stopTx();
        }
        //if (!cwTimeout) //removed by KD8CEC
        //   return;
        // got back to the beginning of the loop, if no further activity happens on straight key
        // we will time out, and return out of this routine 
        //delay(5);
        //delay_background(5, 3); //removed by KD8CEC
        //continue;               //removed by KD8CEC
        return;                   //Tx stop control by Main Loop
      }

      checkCAT();
    } //end of while
  }   //end of elese
}


