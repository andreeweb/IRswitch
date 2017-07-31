/*
 * IRswitch
 * Version 1.0 April, 2017
 * https://github.com/andreeweb
 * https://twitter.com/andreeweb
 */

#include <IRremote.h>
#include <EEPROM.h>

const int LED_PIN = 0;    // LED status, always ON, indicate programming mode
const int BUTTON_PIN = 1; // Used for programming the board and turn on/off
const int RECV_PIN = 2;   // Used for receive the ir signal
const int RELAY_PIN = 4;  // Relay pin

IRrecv irrecv(RECV_PIN);
decode_results results;

unsigned long last = millis();  // Time is important
unsigned long pressed = 0;      // Used as flag
int relayStatus = 0;            // Relay state
int buttonState = 0;            // Push button state, HIGH = pressed
int buttonCounter = 0;          // For "Long press" function

/* Blink led */
void ledBlink(){

  digitalWrite(LED_PIN, LOW);
  delay(250);
  digitalWrite(LED_PIN, HIGH);
  delay(250);
  digitalWrite(LED_PIN, LOW);
  delay(250);
  digitalWrite(LED_PIN, HIGH);
  delay(250);
  digitalWrite(LED_PIN, LOW);
}

void setup(){

  pinMode(LED_PIN, OUTPUT);   
  pinMode(BUTTON_PIN, INPUT); 
  pinMode(RECV_PIN, INPUT);   
  pinMode(RELAY_PIN, OUTPUT);
  
  Serial.begin(9600);

  // Start the receiver
  irrecv.enableIRIn(); 
}

void loop() {

  // Read the state of the push button:
  buttonState = digitalRead(BUTTON_PIN);

  // check if the pushbutton is pressed, if it is, the buttonState is HIGH
  if (buttonState == HIGH) {

    // if is pressed for the first time set pressed variabile to "now"
    if(pressed == 0){

      // Start to count for how long the push button is pressed
      pressed = millis();
    
    }else{

      // When release the push button, if user keep pressed the button for > 2000 start "Programming mode"
      if(millis() - pressed > 2000){

        // Now user can chose a button on his remote for control the board
        
        // led ON to report the rec mode
        digitalWrite(LED_PIN, HIGH);

        // loop until user press a button on the remote control
        while(true){
          
          if (irrecv.decode(&results)) {
            
            // value to save
            unsigned long irvalue = results.value;
            Serial.print("Rec: ");
            Serial.println(irvalue, HEX);

            // save on EEPROM
            Serial.println("Write to EEPROM");
            EEPROM.put(0, irvalue);

            // Blink indicate the end of programming mode
            // from now, the relay can be controlled by this button
            ledBlink();

            // reset
            irrecv.resume();
            pressed = 0;
            break;
          }
          
        }
      }
    }

  // the buttonState is LOW = not pressed
  }else{

    // if the button was pressed and released
    if(pressed != 0){

      // the user release the button until 2000 milliseconds, this will act as a switch to turn the relay on and off
      if(millis() - pressed < 2000){
        relayStatus = !relayStatus;
        digitalWrite(RELAY_PIN, relayStatus ? HIGH : LOW);
      }
       
    }

    // reset
    pressed = 0;
  }

  // Receive IR signal
  if (irrecv.decode(&results)) {

    Serial.print("From IR ");
    Serial.println(results.value, HEX);

    // Get IR code recorded from EEPROM
    unsigned long ircode;
    EEPROM.get(0, ircode);
    Serial.print("From EEPROM ");
    Serial.println(ircode, HEX);

    // compare
    if (results.value == ircode){

      // Use this simple counter for realize a simple "Long press button"
      if(millis() - last < 500){
        buttonCounter++;
      }else{
        buttonCounter = 0;
      }
      
      // IR received, toggle the relay
      if (buttonCounter > 5) {
        relayStatus = !relayStatus;
        digitalWrite(RELAY_PIN, relayStatus ? HIGH : LOW);
        buttonCounter = 0;
      }
      
      last = millis();      
    }

    // Receive the next value
    irrecv.resume(); 
  }
}
