// comparator test - wake from stop example
//butterfly (433):
// NOTE: COMP1 +input can only be attached to PC5, PB2, PA1
//       COMP2 +input can only be attached to PB4, PB6, PA3
//       -input can be one of comp_neginput_t or a pin number
//       COMP1 PC4, PA0, PA4, PA5
//       COMP2 PB7, PA2, PA4, PA5
//ladybug (432)
// NOTE: COMP1 +input can only be attached to PC5, PB2, PA1
//       COMP2 +input can only be attached to PB4, PB6, PA3
//       -input can be one of comp_neginput_t or a pin number
//       COMP1 PB1, PC4, PA0, PA4, PA5
//       COMP2 PB3, PB7, PA2, PA4, PA5
//dragonfly (476)
// NOTE: COMP1 +input can only be attached to PC5, PB2
//       COMP2 +input can only be attached to PB4, PB6
//       -input can be one of comp_neginput_t or a pin number
//       COMP1 PB1, PC4
//       COMP2 PB3, PB7
// Not all pins are broken out/available on all boards - check your board pin out!
// you must enter the pin number as the names are not defined, except for some specialist pins (A3, PIN_LED etc.)

// Connect an analog voltage to COMP +input pin (COMP1 or 2 depending on what pins you have accassible)
// Example for butterfly uses COMP2 and pin A3 (Pin PA3)
// when the voltage goes above 1.2V, MCU will come out of stop mode.
// if you select inverted polarity, MCU will come out of sleep when voltage drops below 1.2V

// Green LED shows voltage detected
// Blue LED shows Stop mode.

#include "Arduino.h"
#include "COMP.h"

#define Serial Serial1  //switch to USART1 as USB does not survive stop mode (if you want to see messages)

//COMPClass Comparator(COMP2, A3, COMP_VINTREF, COMP_INVERT | COMP_EVENT_ENABLE);  //COMP2, +input A3, -input x, output inverted, events enabled, interrupts disabled
//or
COMPClass Comparator(COMP2);  //enter COMP1 or COMP2 depending on which you can connect pins to

void setup() {
  Serial.begin(115200);
  while(!Serial) if (millis() > 30000) break; //only for native USB, not USART

  pinMode(PIN_LED2, OUTPUT);
  digitalWrite(PIN_LED2, HIGH); //green LED off
  pinMode(PIN_LED3, OUTPUT);
  digitalWrite(PIN_LED3, HIGH); //blue LED off
  
  delay(10000); //wait to open serial monitor on USART1
  
  if (!Comparator.begin(A3, COMP_VINTREF, COMP_INVERT | COMP_EVENT_ENABLE))  //enable pin A3, VINTREF (1.2V), Inverted polarity, Events enabled (rising edge is default)
    Serial.println("COMP not enabled");
  else
    Serial.println("COMP initialised");

  Comparator.power_mode(LOW_MODE); //select low power mode (high is default)
  Comparator.hysteresis_mode(HIGH_MODE); //enable high hysteresis (prevent jitter) - NONE_MODE is default
  Comparator.enable_event(RISING);  //already enabled, but you can select RISING, FALING or CHANGE (both) (RISING is default)
  Comparator.enable();  //enable comparator
}

unsigned long previous_millis = millis(), led_millis = millis();

void loop() {
  bool comp_value = Comparator.read();

  if (comp_value && digitalRead(PIN_LED2)) {
    digitalWrite(PIN_LED2, LOW); //green LED on
    Serial.print(millis()/1000);Serial.print("s - TRIGGERED: ");Serial.print("COMP: ");Serial.print(Comparator.this_comp);Serial.print(" VALUE: ");Serial.println(comp_value);
    previous_millis = millis();
    led_millis = millis();
  }
  
  if (millis() - led_millis > 1000) { //every 1 second (after triggering)
    if (!comp_value) {
      digitalWrite(PIN_LED2, HIGH); //green LED off
      Serial.print(millis()/1000);Serial.print("s - not triggered, ");Serial.print(" VALUE: ");Serial.println(comp_value);
    }
    led_millis = millis();
  }
  
  if (millis() - previous_millis > 10000) { //every 10 seconds  (after triggering)
    if (!comp_value) {
      Serial.println("Stop mode - adjust comparator input voltage to trigger");
      digitalWrite(PIN_LED3, LOW); //blue LED on
      delay(1000);  //just allow chars to print before sleeping - not needed in reality
      STM32.stop(0);  //enter stop mode
      digitalWrite(PIN_LED3, HIGH); //blue LED off
      previous_millis = millis();
      led_millis = millis();
    }
  }

  //STM32.sleep();  //optional sleep
}