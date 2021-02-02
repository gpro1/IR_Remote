#include <Sodaq_PcInt.h>
#include <LowPower.h>
#include <IRremote.h>

#define IR_POWER        0x20DF10EF
#define IR_INPUT        0x20DFD02F
#define IR_VOLUME_UP    0x20DF40BF
#define IR_VOLUME_DOWN  0x20DFC03F

#define LED 2
#define BUTTON_MAIN 8
#define BUTTON_VOL_UP 10
#define BUTTON_VOL_DOWN 9

#define BUTTON_ACTIVE_LEVEL 0 //Active high(1) or low(0) button?
#define LONG_PRESS_PERIOD 2000 //The period of a "long" button press in milliseconds

enum state {DEBOUNCE, COUNT_DURATION_MAIN, COUNT_DURATION_VOL};

IRsend irSender;
enum state main_state;
enum state vol_up_state;
enum state vol_down_state;
int button_main_1;
int button_vol_up_1;
int button_vol_down_1;
int button_main;
int button_main_duration;
int button_vol_up_duration;
int button_vol_down_duration;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(BUTTON_MAIN, INPUT_PULLUP);
  pinMode(BUTTON_VOL_DOWN, INPUT_PULLUP);
  pinMode(BUTTON_VOL_UP, INPUT_PULLUP);
  PcInt::attachInterrupt(BUTTON_MAIN, interrupt);
  PcInt::attachInterrupt(BUTTON_VOL_DOWN, interrupt);
  PcInt::attachInterrupt(BUTTON_VOL_UP, interrupt);
  main_state = DEBOUNCE;
  vol_up_state = DEBOUNCE;
  vol_down_state = DEBOUNCE;
  button_main_1 = -1;
  button_vol_up_1 = -1;
  button_vol_down_1 = -1;
  button_main_duration = 0;
  button_vol_up_duration = 0;
  button_vol_down_duration = 0;
}

void loop() {
  digitalWrite(LED_BUILTIN, 1);
  for(int i = 0; i < 1000; i++)
  {
    checkButton(&main_state, BUTTON_MAIN, &button_main_1, &button_main_duration);
    checkButton(&vol_down_state, BUTTON_VOL_DOWN, &button_vol_down_1, &button_vol_down_duration);
    checkButton(&vol_up_state, BUTTON_VOL_UP, &button_vol_up_1, &button_vol_up_duration);

    LowPower.powerDown(SLEEP_60MS, ADC_OFF, BOD_OFF);
  }
  PcInt::enableInterrupt(BUTTON_MAIN);
  PcInt::enableInterrupt(BUTTON_VOL_UP);
  PcInt::enableInterrupt(BUTTON_VOL_DOWN);
  digitalWrite(LED_BUILTIN, 0);
  
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
}

void checkButton(enum state *state, int pin, int* prev_val, int *counter)
{
  int val;
  val = digitalRead(pin);
  switch (*state)
  {
    case DEBOUNCE:
      //Debounce the main button
      if (*prev_val == val && val == BUTTON_ACTIVE_LEVEL)
      {
        if (pin == BUTTON_MAIN)
        {
          *state = COUNT_DURATION_MAIN;
        }
        else if (pin == BUTTON_VOL_DOWN || pin == BUTTON_VOL_UP)
        {
          *state = COUNT_DURATION_VOL;
        }
        *counter = 0;
      }
      *prev_val = val;
      break;

    case COUNT_DURATION_MAIN:
      if (*prev_val != val) {
        if (*counter > 25)
        {
          //long press
          irSender.sendNEC(IR_POWER, 32);
          digitalWrite(LED, LOW);
        }
        else
        {
          //short press
          irSender.sendNEC(IR_INPUT, 32);
        }
        *state = DEBOUNCE;
        *prev_val = -1;
      }
      else if (*counter > 25) 
      {
        digitalWrite(LED, HIGH);
      }
      *counter = *counter + 1;

      break;

    case COUNT_DURATION_VOL:
      if (*prev_val != val)
      {
        if (pin == BUTTON_VOL_DOWN)
        {
          irSender.sendNEC(IR_VOLUME_DOWN, 32);
        }
        else if (pin == BUTTON_VOL_UP)
        {
          irSender.sendNEC(IR_VOLUME_UP, 32);
        }
        *state = DEBOUNCE;
        *prev_val = -1;
      }
      else if (*counter > 2)
      {
        if (pin == BUTTON_VOL_DOWN)
        {
          irSender.sendNEC(IR_VOLUME_DOWN, 32);
        }
        else if (pin == BUTTON_VOL_UP)
        {
          irSender.sendNEC(IR_VOLUME_UP, 32);
        }
      }
      *counter = *counter + 1;
      break;

    default:
      break;
  }
}

void interrupt()
{
  PcInt::disableInterrupt(BUTTON_MAIN);
  PcInt::disableInterrupt(BUTTON_VOL_UP);
  PcInt::disableInterrupt(BUTTON_VOL_DOWN);
}
