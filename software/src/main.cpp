
#define NEW__

#ifdef NEW__



// Libraries
#include <U8g2lib.h>             // https://github.com/olikraus/u8glib
#include <PID_v1.h>             // https://github.com/mblythe86/C-PID-Library/tree/master/PID_v1
#include <EEPROM.h>             // for storing user settings into EEPROM
// #include <avr/sleep.h>          // for sleeping during ADC sampling
#include "draw_api.h"
#include "../lib/Oled/img/ui_img.h"
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif


// Firmware version
#define VERSION       "v1.0"

// Type of rotary encoder
#define ROTARY_TYPE   1         // 0: 2 increments/step; 1: 4 increments/step (default)


// Pins
#define NoUSE 0
#define SENSOR_PIN    32        // tip temperature sense //温度传感器的ad输入
#define BUTTON_PIN     12        // rotary encoder switch
#define ROTARY_1_PIN   13        // rotary encoder 1
#define ROTARY_2_PIN   14        // rotary encoder 2
#define CONTROL_PIN    18        // heater MOSFET PWM control
#define SWITCH_PIN    NoUSE        // handle vibration switch
#define PWM_CHANNEL 8 //0-7通道是高速通道 8-15是慢速
#define PWM_FREQ_HZ 1000


// Default temperature control values (recommended soldering temperature: 300-380°C)
#define TEMP_MIN      200       // min selectable temperature 最低温度
#define TEMP_MAX      400       // max selectable temperature 最高温度
#define TEMP_DEFAULT  320       // default start setpoint 默认启动后升温
#define TEMP_SLEEP    150       // temperature in sleep mode 休眠状态下的温度
#define TEMP_BOOST     50       // temperature increase in boost mode 快速升温阶段的步进
#define TEMP_STEP      10       // rotary encoder temp change steps 正常状态的步进

// Default tip temperature calibration values
#define TEMP200       216       // temperature at ADC = 200
#define TEMP280       308       // temperature at ADC = 280
#define TEMP360       390       // temperature at ADC = 360
#define TEMPCHP       30        // chip temperature while calibration
#define TIPMAX        8         // max number of tips
#define TIPNAMELENGTH 6         // max length of tip names (including termination)
#define TIPNAME       "BC1.5"   // default tip name

// Default timer values (0 = disabled)
#define TIME2SLEEP     5        // time to enter sleep mode in minutes
#define TIME2OFF      15        // time to shut off heater in minutes
#define TIMEOFBOOST   40        // time to stay in boost mode in seconds

// Control values
#define TIME2SETTLE   950       // time in microseconds to allow OpAmp output to settle
#define SMOOTHIE      0.05      // OpAmp output smooth factor (1=no smoothing; 0.05 default)
#define PID_ENABLE    true     // enable PID control
// #define MAINSCREEN    1         // type of main screen (0: big numbers; 1: more infos)
#define SCREEN_FPS    30

// Define the aggressive and conservative PID tuning parameters
double aggKp=11, aggKi=0.5, aggKd=1;
double consKp=11, consKi=3, consKd=5;

// Default values that can be changed by the user and stored in the EEPROM
uint16_t  DefaultTemp = TEMP_DEFAULT;
uint16_t  SleepTemp   = TEMP_SLEEP;
uint8_t   BoostTemp   = TEMP_BOOST;
uint8_t   time2sleep  = TIME2SLEEP;
uint8_t   time2off    = TIME2OFF;
uint8_t   timeOfBoost = TIMEOFBOOST;
// uint8_t   MainScrType = MAINSCREEN;
bool      PIDenable   = PID_ENABLE;

// Default values for tips
uint16_t  CalTemp[TIPMAX][4] = {TEMP200, TEMP280, TEMP360, TEMPCHP};
char      TipName[TIPMAX][TIPNAMELENGTH] = {TIPNAME};
uint8_t   CurrentTip   = 0;
uint8_t   NumberOfTips = 1;


// Variables for pin change interrupt
volatile uint8_t  a0, b0, c0, d0;
volatile bool     ab0;
volatile int      count, countMin, countMax, countStep;
volatile bool     handleMoved;

// Variables for temperature control
uint16_t  SetTemp, ShowTemp, gap, Step;
double    Input, Output, Setpoint, RawTemp, CurrentTemp, ChipTemp;


// State variables
bool      inSleepMode = false;
bool      inOffMode   = false;
bool      inBoostMode = false;
bool      inCalibMode = false;
bool      isWorky     = true;
bool      TipIsPresent= true;

// Timing variables
uint32_t  sleepmillis;
uint32_t  boostmillis;
uint32_t  buttonmillis;
uint8_t   goneMinutes;
uint8_t   goneSeconds;
uint8_t   SensorCounter = 255;
unsigned long  lastRendermillis=0;

// Specify variable pointers and initial PID tuning parameters
PID ctrl(&Input, &Output, &Setpoint, aggKp, aggKi, aggKd, REVERSE);

// Setup u82g object: uncomment according to the OLED used
// U8G2_SSD1306_128X64_NONAME_1_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 4, /* cs=*/ 19, /* dc=*/ 5, /* reset=*/ 2);

void ROTARYCheck();
void SLEEPCheck();
void SENSORCheck();
void calculateTemp();
void Thermostat();
void setRotary(int rmin, int rmax, int rstep, int rvalue);
int getRotary();
void getEEPROM();
void updateEEPROM();
void MainScreen();
void SetupScreen();
void TipScreen();
void TempScreen();
void TimerScreen();
uint8_t MenuScreen(const char *Items[], uint8_t numberOfItems, uint8_t selected);
void MessageScreen(const char *Items[], uint8_t numberOfItems);
uint16_t InputScreen(const char *Items[]);
void InfoScreen();
void ChangeTipScreen();
void CalibrationScreen();
uint8_t InputNameScreen();
void DeleteTipScreen();
void AddTipScreen();
uint16_t denoiseAnalog (byte port);
double getChipTemp();
uint16_t getVCC();
uint16_t getVIN();
void rotary_irq();


void setup() { 
  // set the pin modes
  pinMode(SENSOR_PIN,   INPUT);
  pinMode(CONTROL_PIN,  OUTPUT);
  Oled_DrawApi_Init();
  
  pinMode(ROTARY_1_PIN, INPUT_PULLUP);
  pinMode(ROTARY_2_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PIN,   INPUT_PULLUP);

  ledcSetup(PWM_CHANNEL,PWM_FREQ_HZ,8);
  ledcAttachPin(CONTROL_PIN,PWM_CHANNEL);
  ledcWrite(PWM_CHANNEL,255);
  
  pinMode(SENSOR_PIN,INPUT_PULLDOWN);
  adcAttachPin(SENSOR_PIN);
  analogSetAttenuation(ADC_11db);
  analogReadResolution(10);

  attachInterrupt(ROTARY_1_PIN,rotary_irq,FALLING);
  attachInterrupt(ROTARY_2_PIN,rotary_irq,FALLING);

  // read and set current iron temperature
  SetTemp  = DefaultTemp;
  RawTemp  = denoiseAnalog(SENSOR_PIN);
  // ChipTemp = getChipTemp();
  calculateTemp();

  // turn on heater if iron temperature is well below setpoint
  if ((CurrentTemp + 20) < DefaultTemp) ledcWrite(PWM_CHANNEL, 0);

  // set PID output range and start the PID
  ctrl.SetOutputLimits(0, 255);
  ctrl.SetMode(AUTOMATIC);

  // set initial rotary encoder values
  // a0 = PINB & 1; b0 = PIND>>7 & 1; ab0 = (a0 == b0);
  a0 = digitalRead(ROTARY_1_PIN);
  b0 = digitalRead(ROTARY_2_PIN);
  ab0 = (a0 == b0);
  setRotary(TEMP_MIN, TEMP_MAX, TEMP_STEP, DefaultTemp);
  
  // reset sleep timer
  sleepmillis = millis();
}


void loop() {
  ROTARYCheck();      // check rotary encoder (temp/boost setting, enter setup menu)
  SLEEPCheck();       // check and activate/deactivate sleep modes
  SENSORCheck();      // reads temperature and vibration switch of the iron
  Thermostat();       // heater control
  MainScreen();       // updates the main page on the OLED


  // OledPaint.Canvas.Clear();
  // OledPaint.Draw.Picture(0,0,128,64,main_menu_null);
  // // u8g2.drawBitmap(0,0,128,64,menu_pic);
  // OledPaint.Canvas.Display();
  // delay(1000);

  // OledPaint.Canvas.Clear();
  // OledPaint.Draw.Picture(0,0,16,35,tip_type[0]);
  // // u8g2.drawBitmap(0,0,128,64,menu_pic);
  // OledPaint.Canvas.Display();
  // delay(1000);

  // for(int i =0;i<20;i++){
  //   OledPaint.Canvas.Clear();
  //   OledPaint.Draw.Picture(0,0,95,45,temperature_selection_bar[i]);
  // // u8g2.drawBitmap(0,0,128,64,menu_pic);
  //   OledPaint.Canvas.Display();
  //   delay(1000);
  // }
  // for(int i =0;i<10;i++){
  //   OledPaint.Canvas.Clear();
  //   OledPaint.Draw.Picture(0,0,20,32,temperatur_number[i]);
  // // u8g2.drawBitmap(0,0,128,64,menu_pic);
  //   OledPaint.Canvas.Display();
  //   delay(1000);
  // }
  
}




// check rotary encoder; set temperature, toggle boost mode, enter setup menu accordingly
void ROTARYCheck() {
  // set working temperature according to rotary encoder value
  SetTemp = getRotary();
  
  // TODO:check rotary encoder switch 编码器按键功能 -> 进入设置菜单
  // uint8_t c = digitalRead(BUTTON_PIN);
  // if ( !c && c0 ) {
  //   buttonmillis = millis();
  //   while( (!digitalRead(BUTTON_PIN)) && ((millis() - buttonmillis) < 500) );
  //   if ((millis() - buttonmillis) >= 500) SetupScreen();
  //   else {
  //     inBoostMode = !inBoostMode;
  //     if (inBoostMode) boostmillis = millis();
  //     handleMoved = true;
  //   }
  // }
  // c0 = c;

  // check timer when in boost mode
  if (inBoostMode && timeOfBoost) {
    goneSeconds = (millis() - boostmillis) / 1000;
    if (goneSeconds >= timeOfBoost) {
      inBoostMode = false;              // stop boost mode
    }
  }
}


// check and activate/deactivate sleep modes
void SLEEPCheck() {
  if (handleMoved) {                    // if handle was moved
    if (inSleepMode) {                  // in sleep or off mode?
      if ((CurrentTemp + 20) < SetTemp) // if temp is well below setpoint
        ledcWrite(PWM_CHANNEL, 0);    // then start the heater right now
    }
    handleMoved = false;                // reset handleMoved flag
    inSleepMode = false;                // reset sleep flag
    inOffMode   = false;                // reset off flag
    sleepmillis = millis();             // reset sleep timer
  }

  // check time passed since the handle was moved
  goneMinutes = (millis() - sleepmillis) / 60000;
  if ( (!inSleepMode) && (time2sleep > 0) && (goneMinutes >= time2sleep) ) {inSleepMode = true;}
  if ( (!inOffMode)   && (time2off   > 0) && (goneMinutes >= time2off  ) ) {inOffMode   = true;}
}


// reads temperature, vibration switch and supply voltages
void SENSORCheck() {
  ledcWrite(PWM_CHANNEL, 255);              // shut off heater in order to measure temperature
  delayMicroseconds(TIME2SETTLE);             // wait for voltage to settle
  
  double temp = denoiseAnalog(SENSOR_PIN);    // read ADC value for temperature

  uint8_t d = digitalRead(SWITCH_PIN);        // check handle vibration switch
  if (d != d0) {handleMoved = true; d0 = d;}  // set flag if handle was moved
  
  ledcWrite(PWM_CHANNEL, Output);           // turn on again heater
  
  RawTemp += (temp - RawTemp) * SMOOTHIE;     // stabilize ADC temperature reading
  calculateTemp();                            // calculate real temperature value

  // stabilize displayed temperature when around setpoint
  if ((ShowTemp != Setpoint) || (abs(ShowTemp - CurrentTemp) > 5)) ShowTemp = CurrentTemp;
  if (abs(ShowTemp - Setpoint) <= 1) ShowTemp = Setpoint;

  // set state variable if temperature is in working range; beep if working temperature was just reached
  gap = abs(SetTemp - CurrentTemp);
  if (gap < 5) {
    if (!isWorky);
    isWorky = true;
  }
  else isWorky = false;

  // checks if tip is present or currently inserted
  if (ShowTemp > 500) TipIsPresent = false;   // tip removed ?
  if (!TipIsPresent && (ShowTemp < 500)) {    // new tip inserted ?
    ledcWrite(PWM_CHANNEL, 255);            // shut off heater
    TipIsPresent = true;                      // tip is present now
    // ChangeTipScreen();                        // show tip selection screen
    // updateEEPROM();                           // update setting in EEPROM
    handleMoved = true;                       // reset all timers
    RawTemp  = denoiseAnalog(SENSOR_PIN);     // restart temp smooth algorithm
    c0 = LOW;                                 // switch must be released
    setRotary(TEMP_MIN, TEMP_MAX, TEMP_STEP, SetTemp);  // reset rotary encoder
  }
}


// calculates real temperature value according to ADC reading and calibration values
void calculateTemp() {
  if      (RawTemp < 200) CurrentTemp = map (RawTemp,   0, 200,                     21, CalTemp[CurrentTip][0]);
  else if (RawTemp < 280) CurrentTemp = map (RawTemp, 200, 280, CalTemp[CurrentTip][0], CalTemp[CurrentTip][1]);
  else                    CurrentTemp = map (RawTemp, 280, 360, CalTemp[CurrentTip][1], CalTemp[CurrentTip][2]);
}


// controls the heater
void Thermostat() {
  // define Setpoint acoording to current working mode
  if      (inOffMode)   Setpoint = 0;
  else if (inSleepMode) Setpoint = SleepTemp;
  else if (inBoostMode) Setpoint = SetTemp + BoostTemp;
  else                  Setpoint = SetTemp; 

  // control the heater (PID or direct)
  gap = abs(Setpoint - CurrentTemp);
  if (PIDenable) {
    Input = CurrentTemp;
    if (gap < 30) ctrl.SetTunings(consKp, consKi, consKd);
    else ctrl.SetTunings(aggKp, aggKi, aggKd); 
    ctrl.Compute();
  } else {
    // turn on heater if current temperature is below setpoint
    if ((CurrentTemp + 0.5) < Setpoint) Output = 0; else Output = 255;
  }
  ledcWrite(PWM_CHANNEL, Output);     // set heater PWM
}


// sets start values for rotary encoder
void setRotary(int rmin, int rmax, int rstep, int rvalue) {
  countMin  = rmin << ROTARY_TYPE;
  countMax  = rmax << ROTARY_TYPE;
  countStep = rstep;
  count     = rvalue << ROTARY_TYPE;  
}


// reads current rotary encoder value
int getRotary() {
  return (count >> ROTARY_TYPE);
}




// draws the main screen
void MainScreen() {

  unsigned long currentmillis = millis();
  if(currentmillis - lastRendermillis > 1000/SCREEN_FPS){
    lastRendermillis = currentmillis;
  
  OledPaint.Canvas.Clear();
  //绘制空的主页面
  OledPaint.Draw.Picture(0,0,128,64,main_menu_null);

  //绘制pwm占空比条
  uint8_t output_value;
  output_value = Output;
  float perc = (256-output_value)/256.0;
  uint8_t finalLen = 87*perc;
  OledPaint.Draw.Rect(34,5,34+finalLen,6,0);
  
  //绘制烙铁头类型
  OledPaint.Draw.Picture(3,26,16,35,tip_type[CurrentTip]);

  //绘制当前选定的温度条
  OledPaint.Draw.Picture(26,16,95,45,temperature_selection_bar[(SetTemp-TEMP_MIN)/10]);

  //绘制当前温度
  OledPaint.Draw.Picture(48,30,20,32,temperatur_number[ShowTemp/100]);
  OledPaint.Draw.Picture(48+24,30,20,32,temperatur_number[ShowTemp%100/10]);
  OledPaint.Draw.Picture(48+24+24,30,20,32,temperatur_number[ShowTemp%100%10]);

  //绘制状态标志
  if (ShowTemp > 500)    OledPaint.Draw.Picture(4,4,14,14,status_flag[0]);
  // else if (inOffMode)    OledPaint.Draw.Picture(4,4,14,14,status_flag[0]);
  else if (inSleepMode)  OledPaint.Draw.Picture(4,4,14,14,status_flag[2]);
  // else if (inBoostMode)  OledPaint.Draw.Picture(4,4,14,14,status_flag[0]);
  else if (isWorky)      OledPaint.Draw.Picture(4,4,14,14,status_flag[3]);
  else                   OledPaint.Draw.Picture(4,4,14,14,status_flag[1]);

  OledPaint.Canvas.Display();
  }
}


// // setup screen
// void SetupScreen() {
//   ledcWrite(PWM_CHANNEL, 255);      // shut off heater
//   beep();
//   uint16_t SaveSetTemp = SetTemp;
//   uint8_t selection = 0;
//   bool repeat = true;
  
//   while (repeat) {
//     selection = MenuScreen(SetupItems, sizeof(SetupItems), selection);
//     switch (selection) {
//       case 0:   TipScreen(); repeat = false; break;
//       case 1:   TempScreen(); break;
//       case 2:   TimerScreen(); break;
//       case 3:   PIDenable = MenuScreen(ControlTypeItems, sizeof(ControlTypeItems), PIDenable); break;
//       case 4:   MainScrType = MenuScreen(MainScreenItems, sizeof(MainScreenItems), MainScrType); break;
//       case 5:   beepEnable = MenuScreen(BuzzerItems, sizeof(BuzzerItems), beepEnable); break;
//       case 6:   InfoScreen(); break;
//       default:  repeat = false; break;
//     }
//   }  
//   updateEEPROM();
//   handleMoved = true;
//   SetTemp = SaveSetTemp;
//   setRotary(TEMP_MIN, TEMP_MAX, TEMP_STEP, SetTemp);
// }


// // tip settings screen
// void TipScreen() {
//   uint8_t selection = 0;
//   bool repeat = true;  
//   while (repeat) {
//     selection = MenuScreen(TipItems, sizeof(TipItems), selection);
//     switch (selection) {
//       case 0:   ChangeTipScreen();   break;
//       case 1:   CalibrationScreen(); break;
//       case 2:   InputNameScreen();   break;
//       case 3:   DeleteTipScreen();   break;
//       case 4:   AddTipScreen();      break;
//       default:  repeat = false;      break;
//     }
//   }
// }


// // temperature settings screen
// void TempScreen() {
//   uint8_t selection = 0;
//   bool repeat = true;  
//   while (repeat) {
//     selection = MenuScreen(TempItems, sizeof(TempItems), selection);
//     switch (selection) {
//       case 0:   setRotary(TEMP_MIN, TEMP_MAX, TEMP_STEP, DefaultTemp);
//                 DefaultTemp = InputScreen(DefaultTempItems); break;
//       case 1:   setRotary(20, 200, TEMP_STEP, SleepTemp);
//                 SleepTemp = InputScreen(SleepTempItems); break;
//       case 2:   setRotary(10, 100, TEMP_STEP, BoostTemp);
//                 BoostTemp = InputScreen(BoostTempItems); break;
//       default:  repeat = false; break;
//     }
//   }
// }


// // timer settings screen
// void TimerScreen() {
//   uint8_t selection = 0;
//   bool repeat = true;  
//   while (repeat) {
//     selection = MenuScreen(TimerItems, sizeof(TimerItems), selection);
//     switch (selection) {
//       case 0:   setRotary(0, 30, 1, time2sleep);
//                 time2sleep = InputScreen(SleepTimerItems); break;
//       case 1:   setRotary(0, 60, 5, time2off);
//                 time2off = InputScreen(OffTimerItems); break;
//       case 2:   setRotary(0, 180, 10, timeOfBoost);
//                 timeOfBoost = InputScreen(BoostTimerItems); break;
//       default:  repeat = false; break;
//     }
//   }
// }


// // menu screen
// uint8_t MenuScreen(const char *Items[], uint8_t numberOfItems, uint8_t selected) {
//   bool isTipScreen = (Items[0] == "Tip:");
//   uint8_t lastselected = selected;
//   int8_t  arrow = 0;
//   if (selected) arrow = 1;
//   numberOfItems >>= 1;
//   setRotary(0, numberOfItems - 2, 1, selected);
//   bool    lastbutton = (!digitalRead(BUTTON_PIN));

//   do {
//     selected = getRotary();
//     arrow = constrain(arrow + selected - lastselected, 0, 2);
//     lastselected = selected;
//     u8g2.firstPage();
//       do {
//         u8g2.setFont(u8g_font_9x15);
//         u8g2.setFontPosTop();
//         u8g2.drawStr( 0, 0,  Items[0]);
//         if (isTipScreen) u8g2.drawStr( 54, 0,  TipName[CurrentTip]);
//         u8g2.drawStr( 0, 16 * (arrow + 1), ">");
//         for (uint8_t i=0; i<3; i++) {
//           uint8_t drawnumber = selected + i + 1 - arrow;
//           if (drawnumber < numberOfItems)
//             u8g2.drawStr( 12, 16 * (i + 1), Items[selected + i + 1 - arrow]);
//         }
//       } while(u8g2.nextPage());
//     if (lastbutton && digitalRead(BUTTON_PIN)) {delay(10); lastbutton = false;}
//   } while (digitalRead(BUTTON_PIN) || lastbutton);

//   beep();
//   return selected;
// }


// void MessageScreen(const char *Items[], uint8_t numberOfItems) {
//   bool lastbutton = (!digitalRead(BUTTON_PIN));
//   u8g2.firstPage();
//   do {
//     u8g2.setFont(u8g_font_9x15);
//     u8g2.setFontPosTop();
//     for (uint8_t i = 0; i < numberOfItems; i++) u8g2.drawStr( 0, i * 16,  Items[i]);
//   } while(u8g2.nextPage());
//   do {
//     if (lastbutton && digitalRead(BUTTON_PIN)) {delay(10); lastbutton = false;}
//   } while (digitalRead(BUTTON_PIN) || lastbutton);
//   beep();
// }


// // input value screen
// uint16_t InputScreen(const char *Items[]) {
//   uint16_t  value;
//   bool      lastbutton = (!digitalRead(BUTTON_PIN));

//   do {
//     value = getRotary();
//     u8g2.firstPage();
//       do {
//         u8g2.setFont(u8g_font_9x15);
//         u8g2.setFontPosTop();
//         u8g2.drawStr( 0, 0,  Items[0]);
//         u8g2.setCursor(0, 32); u8g2.print(">"); u8g2.setCursor(10, 32);        
//         if (value == 0)  u8g2.print(F("Deactivated"));
//         else            {u8g2.print(value);u8g2.print(" ");u8g2.print(Items[1]);}
//       } while(u8g2.nextPage());
//     if (lastbutton && digitalRead(BUTTON_PIN)) {delay(10); lastbutton = false;}
//   } while (digitalRead(BUTTON_PIN) || lastbutton);

//   beep();
//   return value;
// }


// // information display screen
// void InfoScreen() {
//   bool lastbutton = (!digitalRead(BUTTON_PIN));

//   do {
//     Vcc = getVCC();                     // read input voltage
//     float fVcc = (float)Vcc / 1000;     // convert mV in V
//     Vin = getVIN();                     // read supply voltage
//     float fVin = (float)Vin / 1000;     // convert mv in V
//     float fTmp = getChipTemp();         // read cold junction temperature
//     u8g2.firstPage();
//       do {
//         u8g2.setFont(u8g_font_9x15);
//         u8g2.setFontPosTop();
//         u8g2.setCursor(0,  0); u8g2.print(F("Firmware: ")); u8g2.print(VERSION);
//         u8g2.setCursor(0, 16); u8g2.print(F("Tmp: "));  u8g2.print(fTmp, 1); u8g2.print(F(" C"));
//         u8g2.setCursor(0, 32); u8g2.print(F("Vin: "));  u8g2.print(fVin, 1); u8g2.print(F(" V"));
//         u8g2.setCursor(0, 48); u8g2.print(F("Vcc:  ")); u8g2.print(fVcc, 1); u8g2.print(F(" V"));
//       } while(u8g2.nextPage());
//     if (lastbutton && digitalRead(BUTTON_PIN)) {delay(10); lastbutton = false;}
//   } while (digitalRead(BUTTON_PIN) || lastbutton);

//   beep();
// }


// // change tip screen
// void ChangeTipScreen() {
//   uint8_t selected = CurrentTip;
//   uint8_t lastselected = selected;
//   int8_t  arrow = 0;
//   if (selected) arrow = 1;
//   setRotary(0, NumberOfTips - 1, 1, selected);
//   bool    lastbutton = (!digitalRead(BUTTON_PIN));

//   do {
//     selected = getRotary();
//     arrow = constrain(arrow + selected - lastselected, 0, 2);
//     lastselected = selected;
//     u8g2.firstPage();
//       do {
//         u8g2.setFont(u8g_font_9x15);
//         u8g2.setFontPosTop();
//         u8g2.drawStr( 0, 0,  "Select Tip");
//         u8g2.drawStr( 0, 16 * (arrow + 1), ">");
//         for (uint8_t i=0; i<3; i++) {
//           uint8_t drawnumber = selected + i - arrow;
//           if (drawnumber < NumberOfTips)
//             u8g2.drawStr( 12, 16 * (i + 1), TipName[selected + i - arrow]);
//         }
//       } while(u8g2.nextPage());
//     if (lastbutton && digitalRead(BUTTON_PIN)) {delay(10); lastbutton = false;}
//   } while (digitalRead(BUTTON_PIN) || lastbutton);

//   beep();
//   CurrentTip = selected;
// }


// // temperature calibration screen
// void CalibrationScreen() {
//   uint16_t CalTempNew[4]; 
//   for (uint8_t CalStep = 0; CalStep < 3; CalStep++) {
//     SetTemp = CalTemp[CurrentTip][CalStep];
//     setRotary(100, 500, 1, SetTemp);
//     beepIfWorky = true;
//     bool    lastbutton = (!digitalRead(BUTTON_PIN));

//     do {
//       SENSORCheck();      // reads temperature and vibration switch of the iron
//       Thermostat();       // heater control
      
//       u8g2.firstPage();
//       do {
//         u8g2.setFont(u8g_font_9x15);
//         u8g2.setFontPosTop();
//         u8g2.drawStr( 0, 0,  "Calibration");
//         u8g2.setCursor(0, 16); u8g2.print(F("Step: ")); u8g2.print(CalStep + 1); u8g2.print(" of 3");
//         if (isWorky) {
//           u8g2.setCursor(0, 32); u8g2.print("Set measured");
//           u8g2.setCursor(0, 48); u8g2.print("temp: "); u8g2.print(getRotary());
//         } else {
//           u8g2.setCursor(0, 32); u8g2.print("ADC:  "); u8g2.print(uint16_t(RawTemp));
//           u8g2.setCursor(0, 48); u8g2.print("Please wait...");
//         }
//       } while(u8g2.nextPage());
//     if (lastbutton && digitalRead(BUTTON_PIN)) {delay(10); lastbutton = false;}
//     } while (digitalRead(BUTTON_PIN) || lastbutton);

//   CalTempNew[CalStep] = getRotary();
//   beep(); delay (10);
//   }

//   ledcWrite(PWM_CHANNEL, 255);              // shut off heater
//   delayMicroseconds(TIME2SETTLE);             // wait for voltage to settle
//   CalTempNew[3] = getChipTemp();              // read chip temperature
//   if ((CalTempNew[0] + 10 < CalTempNew[1]) && (CalTempNew[1] + 10 < CalTempNew[2])) {
//     if (MenuScreen(StoreItems, sizeof(StoreItems), 0)) {
//       for (uint8_t i = 0; i < 4; i++) CalTemp[CurrentTip][i] = CalTempNew[i];
//     }
//   }
// }


// // input tip name screen
// uint8_t InputNameScreen() {
//   uint8_t  value;

//   for (uint8_t digit = 0; digit < (TIPNAMELENGTH - 1); digit++) {
//     bool      lastbutton = (!digitalRead(BUTTON_PIN));
//     setRotary(31, 96, 1, 65);
//     do {
//       value = getRotary();
//       if (value == 31) {value = 95; setRotary(31, 96, 1, 95);}
//       if (value == 96) {value = 32; setRotary(31, 96, 1, 32);}
//       u8g2.firstPage();
//         do {
//           u8g2.setFont(u8g_font_9x15);
//           u8g2.setFontPosTop();
//           u8g2.drawStr( 0, 0,  "Enter Tip Name");
//           u8g2.setCursor(9 * digit, 48); u8g2.print(char(94));
//           u8g2.setCursor(0, 32);
//           for (uint8_t i = 0; i < digit; i++) u8g2.print(TipName[CurrentTip][i]);
//           u8g2.setCursor(9 * digit, 32); u8g2.print(char(value));
//         } while(u8g2.nextPage());
//       if (lastbutton && digitalRead(BUTTON_PIN)) {delay(10); lastbutton = false;}
//     } while (digitalRead(BUTTON_PIN) || lastbutton);
//     TipName[CurrentTip][digit] = value;
//     beep(); delay (10);
//   }
//   TipName[CurrentTip][TIPNAMELENGTH - 1] = 0;


  

//   return value;
// }


// // delete tip screen
// void DeleteTipScreen() {
//   if (NumberOfTips == 1) {MessageScreen(DeleteMessage, sizeof(DeleteMessage));}
//   else if (MenuScreen(SureItems, sizeof(SureItems), 0)) {
//     if (CurrentTip == (NumberOfTips - 1)) {CurrentTip--;}
//     else {
//       for (uint8_t i = CurrentTip; i < (NumberOfTips - 1); i++) {
//         for (uint8_t j = 0; j < TIPNAMELENGTH; j++) TipName[i][j] = TipName[i+1][j];
//         for (uint8_t j = 0; j < 4; j++)             CalTemp[i][j] = CalTemp[i+1][j];
//       }
//     }
//     NumberOfTips--;
//   }
// }


// // add new tip screen
// void AddTipScreen() {
//   if (NumberOfTips < TIPMAX) {
//     CurrentTip = NumberOfTips++; InputNameScreen();
//     CalTemp[CurrentTip][0] = TEMP200; CalTemp[CurrentTip][1] = TEMP280;
//     CalTemp[CurrentTip][2] = TEMP360; CalTemp[CurrentTip][3] = TEMPCHP;
//   } else MessageScreen(MaxTipMessage, sizeof(MaxTipMessage));
// }


// average several ADC readings in sleep mode to denoise
uint16_t  denoiseAnalog (byte port) {
  uint16_t result = 0;
  // ADCSRA |= bit (ADEN) | bit (ADIF);    // enable ADC, turn off any pending interrupt
  // if (port >= A0) port -= A0;           // set port and
  // ADMUX = (0x0F & port) | bit(REFS0);   // reference to AVcc 
  // set_sleep_mode (SLEEP_MODE_ADC);      // sleep during sample for noise reduction
  for (uint8_t i=0; i<32; i++) {        // get 32 readings
    // sleep_mode();                       // go to sleep while taking ADC sample
    // while (bitRead(ADCSRA, ADSC));      // make sure sampling is completed
    // ADC=analogRead(SENSOR_PIN);
    result += analogRead(SENSOR_PIN);                   // add them up
  }
  result = result*(3.3/5);
  // bitClear (ADCSRA, ADEN);              // disable ADC
  return (result>>5);                 // devide by 32 and return value
}


// get internal temperature by reading ADC channel 8 against 1.1V reference
double getChipTemp() {
  uint16_t result = 0;
  // ADCSRA |= bit (ADEN) | bit (ADIF);    // enable ADC, turn off any pending interrupt
  // ADMUX = bit (REFS1) | bit (REFS0) | bit (MUX3); // set reference and mux
  // delay(20);                            // wait for voltages to settle
  // set_sleep_mode (SLEEP_MODE_ADC);      // sleep during sample for noise reduction
  // for (uint8_t i=0; i<32; i++) {        // get 32 readings
  //   sleep_mode();                       // go to sleep while taking ADC sample
  //   while (bitRead(ADCSRA, ADSC));      // make sure sampling is completed
  //   result += ADC;                      // add them up
  // }
  // bitClear (ADCSRA, ADEN);              // disable ADC  
  // result >>= 2;                         // devide by 4
  // return ((result - 2594) / 9.76);      // calculate internal temperature in degrees C
  return result;
}


// get input voltage in mV by reading 1.1V reference against AVcc
uint16_t getVCC() {
  uint16_t result = 0;
  // ADCSRA |= bit (ADEN) | bit (ADIF);    // enable ADC, turn off any pending interrupt
  // // set Vcc measurement against 1.1V reference
  // ADMUX = bit (REFS0) | bit (MUX3) | bit (MUX2) | bit (MUX1);
  // delay(1);                             // wait for voltages to settle
  // set_sleep_mode (SLEEP_MODE_ADC);      // sleep during sample for noise reduction
  // for (uint8_t i=0; i<16; i++) {        // get 16 readings
  //   sleep_mode();                       // go to sleep while taking ADC sample
  //   while (bitRead(ADCSRA, ADSC));      // make sure sampling is completed
  //   result += ADC;                      // add them up
  // }
  // bitClear (ADCSRA, ADEN);              // disable ADC  
  // result >>= 4;                         // devide by 16
  // return (1125300L / result);           // 1125300 = 1.1 * 1023 * 1000 
  return result;
}


// get supply voltage in mV
uint16_t getVIN() {
  long result;
  // result = denoiseAnalog (VIN_PIN);     // read supply voltage via voltage divider
  // return (result * Vcc / 179.474);      // 179.474 = 1023 * R13 / (R12 + R13)
  return 0;
}


// ADC interrupt service routine
// EMPTY_INTERRUPT (ADC_vect);             // nothing to be done here


// Pin change interrupt service routine for rotary encoder
// ISR (PCINT0_vect) {
//   uint8_t a = PINB & 1;
//   uint8_t b = PIND>>7 & 1;

//   if (a != a0) {              // A changed
//     a0 = a;
//     if (b != b0) {            // B changed
//       b0 = b;
//       count = constrain(count + ((a == b) ? countStep : -countStep), countMin, countMax);
//       if (ROTARY_TYPE && ((a == b) != ab0)) {
//         count = constrain(count + ((a == b) ? countStep : -countStep), countMin, countMax);;
//       }
//       ab0 = (a == b);
//       handleMoved = true;
//     }
//   }
// }

void rotary_irq() {
  uint8_t a = digitalRead(ROTARY_1_PIN);
  uint8_t b = digitalRead(ROTARY_2_PIN);

  if (a != a0) {              // A changed
    a0 = a;
    if (b != b0) {            // B changed
      b0 = b;
      count = constrain(count + ((a == b) ? countStep : -countStep), countMin, countMax);
      if (ROTARY_TYPE && ((a == b) != ab0)) {
        count = constrain(count + ((a == b) ? countStep : -countStep), countMin, countMax);;
      }
      ab0 = (a == b);
      handleMoved = true;
    }
  }
}


#endif // DEBUG