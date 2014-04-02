#include <Wire.h>
#include <LiquidCrystal.h>
#define BUTTON_ADC_PIN           A0  // A0 is the button ADC input
#define LCD_BACKLIGHT_PIN         3  // D3 controls LCD backlight
// ADC readings expected for the 5 buttons on the ADC input
#define RIGHT_10BIT_ADC           0  // right
#define UP_10BIT_ADC            145  // up
#define DOWN_10BIT_ADC          329  // down
#define LEFT_10BIT_ADC          402  // left
#define SELECT_10BIT_ADC        741  // right
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

int timer1_counter;
int leftClockInSec;
int rightClockInSec;
boolean leftButtonPressed;
boolean rightButtonPressed;

LiquidCrystal lcd( 8, 9, 4, 5, 6, 7 );

void setup()
{
  lcd.begin(16, 2);
  lcd.print("hello, buttons!");
   //button adc input
   pinMode( BUTTON_ADC_PIN, INPUT );         //ensure A0 is an input
   digitalWrite( BUTTON_ADC_PIN, LOW );      //ensure pullup is off on A0
   // setup timer
   leftClockInSec = 0;
   rightClockInSec = 0;
   leftButtonPressed = true;
   rightButtonPressed = false;
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

ISR(TIMER1_OVF_vect)        // interrupt service routine 
{
  String leftSec = "0";
  String rightSec = "0";
  String leftMin = "0";
  String rightMin = "0";  
  int minutes = 0;
  TCNT1 = timer1_counter;   // preload timer  
  if(leftButtonPressed){
     leftClockInSec++;
     if(leftClockInSec==60) {
       minutes++;
       leftClockInSec = 0;
     }
  leftSec = String(leftClockInSec);
  leftMin = String(minutes);  
  } else {
     rightClockInSec++;
     if(rightClockInSec==60) {
       minutes++;
       rightClockInSec = 0;
     }
  rightSec = String(rightClockInSec);
  rightMin = String(minutes);
  }
  
  lcd.clear();
  lcd.print (leftMin + ":" + leftSec + "         "+ rightMin + ":" + rightSec);
}

void loop()
{
   byte button;
   button = ReadButtons();
  if(button == BUTTON_LEFT) {
    leftButtonPressed = true;
    rightButtonPressed = false;
    leftClockInSec = 0;
  }
  if ( button == BUTTON_RIGHT) {
    leftButtonPressed = false;
    rightButtonPressed = true;
    rightClockInSec = 0;
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
   if( buttonVoltage < ( RIGHT_10BIT_ADC + BUTTONHYSTERESIS ) )
   {
      button = BUTTON_RIGHT;
   }
   else if(   buttonVoltage >= ( LEFT_10BIT_ADC - BUTTONHYSTERESIS )
           && buttonVoltage <= ( LEFT_10BIT_ADC + BUTTONHYSTERESIS ) )
   {
      button = BUTTON_LEFT;
   }
   return( button );
}
