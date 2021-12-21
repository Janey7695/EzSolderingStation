#include <Arduino.h>
// Libraries
#include <PID_v1.h>             // https://github.com/mblythe86/C-PID-Library/tree/master/PID_v1
#include <EEPROM.h>             // for storing user settings into EEPROM
// #include <avr/sleep.h>          // for sleeping during ADC sampling
#include "draw_api.h"
#include "../lib/Oled/img/ui_img.h"


// Firmware version
#define VERSION       "v1.0"

// Type of rotary encoder
#define ROTARY_TYPE   1         // 0: 2 increments/step; 1: 4 increments/step (default)


// Pins
#define SENSOR_PIN    32        // tip temperature sense //温度传感器的ad输入
#define BUTTON_PIN     12        // rotary encoder switch
#define ROTARY_1_PIN   13        // rotary encoder 1
#define ROTARY_2_PIN   14        // rotary encoder 2
#define CONTROL_PIN    18        // heater MOSFET PWM control
#define SWITCH_PIN      21        // handle vibration switch
#define PWM_CHANNEL 8 //0-7通道是高速通道 8-15是慢速
#define PWM_FREQ_HZ 1000


// Default temperature control values (recommended soldering temperature: 300-380°C)
#define TEMP_MIN      200       // min selectable temperature 最低温度
#define TEMP_MAX      400       // max selectable temperature 最高温度
#define TEMP_DEFAULT  320       // default start setpoint 默认启动后升温
#define TEMP_SLEEP    150       // temperature in sleep mode 休眠状态下的温度
#define TEMP_BOOST     50       // temperature increase in boost mode
// #define TEMP_STEP      10       // rotary encoder temp change steps 正常状态的步进

// Default tip temperature calibration values
#define TEMP200       216       // temperature at ADC = 200
#define TEMP280       308       // temperature at ADC = 280
#define TEMP360       390       // temperature at ADC = 360
#define TEMPCHP       30        // chip temperature while calibration
#define TIPMAX        8         // max number of tips
#define TIPNAMELENGTH 6         // max length of tip names (including termination)
#define TIPNAME       "BC1.5"   // default tip name

// Default timer values (0 = disabled)
#define TIME2SLEEP     1        // time to enter sleep mode in minutes
#define TIME2OFF        5        // time to shut off heater in minutes
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
double    Input, Output, Setpoint, RawTemp, CurrentTemp;


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
  pinMode(SENSOR_PIN,   INPUT); //温度传感器ad输入
  pinMode(CONTROL_PIN,  OUTPUT);  //pwm输出控制

  //震动传感器
  pinMode(SWITCH_PIN,   INPUT_PULLUP); //温度传感器ad输入

  //oled初始化
  Oled_DrawApi_Init();

  //旋转编码器引脚初始化
  pinMode(ROTARY_1_PIN, INPUT_PULLUP);
  pinMode(ROTARY_2_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PIN,   INPUT_PULLUP);

  //pwm初始化
  ledcSetup(PWM_CHANNEL,PWM_FREQ_HZ,8);
  ledcAttachPin(CONTROL_PIN,PWM_CHANNEL);
  ledcWrite(PWM_CHANNEL,255);

  //adc初始化
  pinMode(SENSOR_PIN,INPUT_PULLDOWN);
  adcAttachPin(SENSOR_PIN);
  analogSetAttenuation(ADC_11db);
  analogReadResolution(10);

  //为旋转编码器绑定中断
  attachInterrupt(ROTARY_1_PIN,rotary_irq,FALLING);
  attachInterrupt(ROTARY_2_PIN,rotary_irq,FALLING);

  // read and set current iron temperature
  SetTemp  = DefaultTemp;
  RawTemp  = denoiseAnalog(SENSOR_PIN);
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
  setRotary(TEMP_MIN, TEMP_MAX, (TEMP_MAX-TEMP_MIN)/20, DefaultTemp);

  // reset sleep timer
  sleepmillis = millis();
}


void loop() {
  ROTARYCheck();      // check rotary encoder (temp/boost setting, enter setup menu)
  SLEEPCheck();       // check and activate/deactivate sleep modes
  SENSORCheck();      // reads temperature and vibration switch of the iron
  Thermostat();       // heater control
  MainScreen();       // updates the main page on the OLED
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

    //TODO:插入新的烙铁时进入烙铁选择页面
    // ChangeTipScreen();                        // show tip selection screen
    // updateEEPROM();                           // update setting in EEPROM

    handleMoved = true;                       // reset all timers
    RawTemp  = denoiseAnalog(SENSOR_PIN);     // restart temp smooth algorithm
    c0 = LOW;                                 // switch must be released
    setRotary(TEMP_MIN, TEMP_MAX,  (TEMP_MAX-TEMP_MIN)/20, SetTemp);  // reset rotary encoder
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


// average several ADC readings in sleep mode to denoise
uint16_t  denoiseAnalog (byte port) {
  uint16_t result = 0;
  for (uint8_t i=0; i<32; i++) {        // get 32 readings
    result += analogRead(SENSOR_PIN);                   // add them up
  }
  result = result*(3.3/5);
  return (result>>5);                 // devide by 32 and return value
}


// TODO:get internal temperature by reading ADC channel 8 against 1.1V reference
double getChipTemp() {
  uint16_t result = 0;
  return result;
}


// TODO:get input voltage in mV by reading 1.1V reference against AVcc
uint16_t getVCC() {
  uint16_t result = 0;

  return result;
}


//TODO: get supply voltage in mV
uint16_t getVIN() {
  long result;

  return 0;
}

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
