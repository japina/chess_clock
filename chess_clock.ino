#include <Wire.h>
#include <LiquidCrystal.h>
#define BUTTON_ADC_PIN           A0  // A0 is the button ADC input
#define LCD_BACKLIGHT_PIN         3  // D3 controls LCD backlight
// ADC readings expected for the 5 buttons on the ADC input
#define RIGHT_10BIT_ADC           0  // right
#define UP_10BIT_ADC            97  // up
#define DOWN_10BIT_ADC          251  // down
#define LEFT_10BIT_ADC          402  // left
#define SELECT_10BIT_ADC        636  // right
#define BUTTONHYSTERESIS         10  // hysteresis for valid button sensing window
//return values for ReadButtons()
#define BUTTON_NONE               0  // 
#define BUTTON_RIGHT              1  // 
#define BUTTON_UP                 2  // 
#define BUTTON_DOWN               3  // 
#define BUTTON_LEFT               4  // 
#define BUTTON_SELECT             5  // 
//some example macros with friendly labels for LCD backlight/pin control, tested and can be swapped into the example code as you like
#define LCD_BACKLIGHT_OFF()     digitalWrite( LCD_BACKLIGHT_PIN, LOW )
#define LCD_BACKLIGHT_ON()      digitalWrite( LCD_BACKLIGHT_PIN, HIGH )
#define LCD_BACKLIGHT(state)    { if( state ){digitalWrite( LCD_BACKLIGHT_PIN, HIGH );}else{digitalWrite( LCD_BACKLIGHT_PIN, LOW );} }
#define FISCHER 0
#define BRONSTEIN_DELAY 1
#define SIMPLE_DELAY 2
#define OVERTIME_PENALTY 3
#define HOUR_GLASS 4
#define ARRAY_SIZE 5

int timer1_counter;
int leftClockInSec;
int rightClockInSec;
boolean leftButtonPressed;
boolean rightButtonPressed;
boolean downButtonPressed;
boolean upButtonPressed;
boolean selectButtonPressed;
boolean showMenuFlag;
boolean refresh;
boolean doIncrement;
int pos=0;


LiquidCrystal lcd( 8, 9, 4, 5, 6, 7 );

String showSelection(){
  String retText;
  switch(pos){
    case FISCHER:
      retText="Fischer";
      break;
    case BRONSTEIN_DELAY:
      retText="Bronstein delay";
      break;
    case SIMPLE_DELAY:
      retText="Simple delay";
      break;
    case OVERTIME_PENALTY:
      retText="Overtime penalty";
      break;
    case HOUR_GLASS:
      retText="Hour glass";
      break;
  }
  return retText;
}

int showMenu(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Select clock:");
  lcd.setCursor(0,1);
  lcd.print(showSelection());   
}

void setup()
{
   //button adc input
   pinMode( BUTTON_ADC_PIN, INPUT );         //ensure A0 is an input
   digitalWrite( BUTTON_ADC_PIN, LOW );      //ensure pullup is off on A0
   lcd.begin(16,2); // 16 chars in 2 lines
  showMenuFlag=true;
  refresh=true;
  leftButtonPressed = false;
  rightButtonPressed = false;
  doIncrement = true;
  startClock();
   
}

void startClock(){
  // initialize timer1 
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;

  // Set timer1_counter to the correct value for our interrupt interval
  //timer1_counter = 64886;   // preload timer 65536-16MHz/256/100Hz
  //timer1_counter = 64286;   // preload timer 65536-16MHz/256/50Hz
  //timer1_counter = 34286;   // preload timer 65536-16MHz/256/2Hz
  timer1_counter = 17143;
  
  TCNT1 = timer1_counter;   // preload timer
  TCCR1B |= (1 << CS12);    // 256 prescaler 
  TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
  interrupts();             // enable all interrupts
}

String showTime(int timeInSeconds) {
  int minutes = int(timeInSeconds/60);
  int seconds = timeInSeconds-60*minutes;
  String minutes_text = String(minutes);
  String seconds_text = String(seconds);
  if(minutes<10) {
    minutes_text = '0' + minutes_text;
  }
  if(seconds<10) {
    seconds_text = '0' + seconds_text;
  }
  return String(minutes_text + ":" + seconds_text);
}

ISR(TIMER1_OVF_vect)        // interrupt service routine 
{  
  TCNT1 = timer1_counter;   // preload timer  
  if((rightButtonPressed)&&(leftClockInSec>0)) { 
    leftClockInSec--;
  }
  if((leftButtonPressed)&&(rightClockInSec>0)) {
     rightClockInSec--;
  }   
  if(!showMenuFlag) {
     lcd.clear();
     lcd.print (showTime(leftClockInSec) + "    "+ showTime(rightClockInSec));
  }
}

void loop()
{
   byte button;
   
   if(showMenuFlag){
    if(refresh){
      showMenu();  
      refresh=false;
    }
   }
   
   button = ReadButtons();
   // check if button is pressed and released 
  if((leftButtonPressed) && (button == BUTTON_NONE)) {
    if(doIncrement){
      rightClockInSec += 5; // for Fischer
      doIncrement=false;
    }
  }
  
  if((rightButtonPressed) && (button == BUTTON_NONE)) {
    if(doIncrement){
      leftClockInSec += 5; // for Fischer
      doIncrement=false;
    }

  }
  
  if((downButtonPressed) && (button == BUTTON_NONE)) { //pressed down and released
    refresh=true;
    pos--;    
    downButtonPressed=false;
  }
  if((upButtonPressed) && (button == BUTTON_NONE)) { // pressed up and released
    refresh=true;
    pos++;
    upButtonPressed=false;    
  }
  if((selectButtonPressed) && (button == BUTTON_NONE)) {
    // set up for the chess clock depending on the pos !!!!
    showMenuFlag=false;
   // setup timer
   leftClockInSec = 300; // 5 minutes Fischer
   rightClockInSec = 300; // 5 minutes Fischer  
   selectButtonPressed=false;  
    
  }
  
 if(pos<0){
    pos=ARRAY_SIZE-1;
  }
  if(pos>=ARRAY_SIZE){
    pos=0;
   }
}

/*--------------------------------------------------------------------------------------
  ReadButtons()
  Detect the button pressed and return the value
--------------------------------------------------------------------------------------*/
byte ReadButtons()
{
   unsigned int buttonVoltage;
   byte button = BUTTON_NONE;   // return no button pressed if the below checks don't write to btn
   
   //read the button ADC pin voltage
   buttonVoltage = analogRead( BUTTON_ADC_PIN );
   //sense if the voltage falls within valid voltage windows
   // right
   if( buttonVoltage < ( RIGHT_10BIT_ADC + BUTTONHYSTERESIS )
      && buttonVoltage <= ( RIGHT_10BIT_ADC + BUTTONHYSTERESIS ) )
   {
     leftButtonPressed=false;
     rightButtonPressed=true;
     doIncrement=true;
      button = BUTTON_RIGHT;
   }
   // left
   if(   buttonVoltage >= ( LEFT_10BIT_ADC - BUTTONHYSTERESIS )
           && buttonVoltage <= ( LEFT_10BIT_ADC + BUTTONHYSTERESIS ) )
   {
     leftButtonPressed=true;
     rightButtonPressed=false;
     doIncrement=true;
      button = BUTTON_LEFT;
   }
   // up
   if(   buttonVoltage >= ( UP_10BIT_ADC - BUTTONHYSTERESIS )
           && buttonVoltage <= ( UP_10BIT_ADC + BUTTONHYSTERESIS ) )
   {
      button = BUTTON_UP;
      upButtonPressed=true;
      downButtonPressed=false;      
   }
   // down
   if(   buttonVoltage >= ( DOWN_10BIT_ADC - BUTTONHYSTERESIS )
           && buttonVoltage <= ( DOWN_10BIT_ADC + BUTTONHYSTERESIS ) )
   {
      button = BUTTON_DOWN;
      upButtonPressed=false;
      downButtonPressed=true;      
   } 
   // select
   if(   buttonVoltage >= ( SELECT_10BIT_ADC - BUTTONHYSTERESIS )
           && buttonVoltage <= ( SELECT_10BIT_ADC + BUTTONHYSTERESIS ) )
   {
      button = BUTTON_SELECT;
      selectButtonPressed=true;
   } 
   return( button );
}
