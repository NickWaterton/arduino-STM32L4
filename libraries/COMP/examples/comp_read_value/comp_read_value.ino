// comparator test - read value example
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
// when the voltage goes above 1.2V, green LED will light
// if you select inverted polarity, green LED will light when voltage drops below 1.2V

// Green LED shows voltage detected

#include "Arduino.h"
#include "COMP.h"

//COMPClass Comparator(COMP2, A3, COMP_VINTREF, true);  //COMP2, +input A3, -input x, output inverted 
//or
COMPClass Comparator(COMP2);  //enter COMP1 or COMP2 depending on which you can connect pins to

void setup() {
  Serial.begin(15200);
  while(!Serial) if (millis() > 30000) break; //only for native USB, not USART

  pinMode(PIN_LED2, OUTPUT);
  digitalWrite(PIN_LED2, HIGH); //green LED off

  if (!Comparator.init(A3, COMP_VINTREF, true))  //enable pin A3, VINTREF (1.2V), Inverted polarity
    Serial.println("COMP not enabled");
  else
    Serial.println("COMP initialised");

  Comparator.hysteresis_mode(LOW_MODE); //enable low hysteresis (prevent jitter) - NONE_MODE is default
  Comparator.enable();  //enable comparator
}

unsigned long previous_millis = millis();

void loop() {
  bool comp_value = Comparator.read();

  if (comp_value)
    digitalWrite(PIN_LED2, LOW); //green LED on
  
  if (millis() - previous_millis > 100) {  //every 1/10 second
    if (!comp_value)
      digitalWrite(PIN_LED2, HIGH); //green LED off
    Serial.print(millis()/1000);Serial.print(" COMP: ");Serial.print(Comparator.this_comp);Serial.print(" VALUE: ");Serial.println(comp_value);
    previous_millis = millis();
  }
}