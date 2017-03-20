// comparator test - window comparator example
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

// Connect an analog voltage to COMP +input pin (COMP1 only)
// Example for butterfly uses pin 13 (LED PIN)
// when the voltage goes outside window of 0.6V to 1.2V, green LED will light
// if you select inverted polarity, green LED will light when voltage is inside window of 0.6V to 1.2V

// Window comparator has only ONE +ve input, TWO -ve inputs, and TWO outputs.

// Green LED shows voltage detected

#include "Arduino.h"
#include "COMP.h"

COMPClass Comparator(COMP1, 13, COMP_VINTREF, COMP_INVERT);  //COMP1, +input pin 13 (LED PIN), -input 1.2V, output inverted 
//or
//COMPClass Comparator(COMP1);  //create COMP1 object - but then have to configure options in begin or elsewhere

COMPClass Comparator2(COMP2, A3, COMP_VINTREF_1_2, COMP_INVERT | COMP_WINMODE);  //COMP2, +input A3, -input 0.6V, output inverted, window comparator mode enabled
                                                                                //NOTE: +input pin must be defined as valid input pin (A3 in this case), but will be ignored
                                                                                //      as in window comparator mode COMP2 +ve input is connected to COMP1 +ve input. 
//or
//COMPClass Comparator2(COMP2);  //create COMP2 object - but then have to configure options in begin or elsewhere

void setup() {
  Serial.begin(115200);
  while(!Serial) if (millis() > 30000) break; //only for native USB, not USART

  pinMode(PIN_LED2, OUTPUT);
  digitalWrite(PIN_LED2, HIGH); //green LED off

  if (!Comparator.begin())  //start comparator 1
    Serial.println("COMP1 not enabled");
  else
    Serial.println("COMP1 initialised");

  if (!Comparator2.begin())  //start comparator 2
    Serial.println("COMP2 not enabled");
  else
    Serial.println("COMP2 initialised");

  Comparator.hysteresis_mode(LOW_MODE); //enable low hysteresis (prevent jitter) - NONE_MODE is default
  Comparator2.hysteresis_mode(LOW_MODE); //enable low hysteresis (prevent jitter) - NONE_MODE is default
  Comparator.enable();  //enable comparator
  Comparator2.enable();  //enable comparator
}

unsigned long previous_millis = millis();

void loop() {
  bool comp_value = Comparator.read();
  bool comp_value2 = Comparator2.read();

  if (comp_value ^ comp_value2) //XOR COMP1 and COMP2 (ie COMP1 or COMP2, but not both)
    digitalWrite(PIN_LED2, LOW); //green LED on
  
  if (millis() - previous_millis > 100) {  //every 1/10 second
    if (!(comp_value ^ comp_value2))
      digitalWrite(PIN_LED2, HIGH); //green LED off
    Serial.print(millis()/1000);Serial.print(" COMP: ");Serial.print(Comparator.this_comp);Serial.print(" VALUE: ");Serial.print(comp_value);
    Serial.print(" COMP: ");Serial.print(Comparator2.this_comp);Serial.print(" VALUE: ");Serial.println(comp_value2);
    previous_millis = millis();
  }
}