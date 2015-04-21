 
//#include "global_def.h"  // These don't seem to work.  They should.
//#include "ms_remote_map.h" // These don't seem to work.  They should.
#include <IRremote.h>

//#define ms_remote_map.h

//below mappings are for Microsoft Media Center Remote
#define OK_POS1 0x800F0422
#define OK_POS2	0x800F8422
#define VOL_DWN_POS1 0x800F041F
#define VOL_DWN_POS2 0x800F841F
#define VOL_UP_POS1 0x800F041E
#define VOL_UP_POS2 0x800F841E
#define MAX_BRIGHT 255
#define SL_BD_RT 9600


#define RECV_PIN 11
#define LED_PIN 5 // choose the pin for the LED

byte ledState;
IRrecv irrecv(RECV_PIN);
decode_results results;
boolean power_state = LOW;

void setup(){
  Serial.begin(SL_BD_RT);
  irrecv.enableIRIn(); // Start the receiver
  pinMode(LED_PIN, OUTPUT);  // declare LED as output
}

void loop() {
  if (irrecv.decode(&results)) {  //If IR receive results are detected
   // Serial.println(results.value, HEX);
    switch (results.value) {

     //Power
    case OK_POS1:
      //   Serial.println("Power");
         ledState = 125;
         digitalWrite(LED_PIN, LOW);  // turn LED ON
    break;

    case OK_POS2:
         digitalWrite(LED_PIN, HIGH);  // turn LED ON
    break;

    //Dim
    case VOL_DWN_POS1:
          ledState -= 10;
          if (ledState > 0){
          analogWrite(LED_PIN, ledState);  // Dim LED
          }
    break;

    case VOL_DWN_POS2:
         ledState -= 10;
         if (ledState > 0){
         analogWrite(LED_PIN, ledState);  // Dim LED
         }
    break;

    //Brighten
    case VOL_UP_POS1:
         if (ledState < MAX_BRIGHT){
            ledState += 10;
          }
         analogWrite(LED_PIN, ledState);  // Brighten LED 
    break;

   case VOL_UP_POS2:
          if (ledState < MAX_BRIGHT){
            ledState += 10;
          }
         analogWrite(LED_PIN, ledState);  // Brighten LED
    break; 
    //default:
     //  Serial.println("");
  }

    delay(200);  // 1/5 second delay for arbitrary clicks.  Will implement debounce later.
    irrecv.resume(); // Receive the next value
  }
}
