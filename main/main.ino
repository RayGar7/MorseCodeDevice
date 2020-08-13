//Note: this code was written using the header file library and the github repository's code samples. Both credits to this belong to Seeed and their employees.
// I also used this guide for the algorithm on long presses/short presses: http://www.instructables.com/id/Arduino-Dual-Function-Button-Long-PressShort-Press/

#include <Wire.h>
#include "rgb_lcd.h"

//hardware constants
rgb_lcd lcd;
const int colorR = 66;
const int colorG = 241;
const int colorB = 244;
const int buttonPin = 3;

// Set the timer system for the button presses
long buttonPressTime = 0;
long longPressTime = 1000;                                            //a long button press is one second, in this morse code
unsigned long previousMillis = 0;                                     // will store last time LED was updated
const long interval = 10000;                                          // interval at which to decode a new morse code character (milliseconds)

//variables for the decoding algorithm
String morseVals = String("");                                          //where I will store each morse character that the user enters, will be passed to the loop statement
char englishChar = ' ';                                                 

//Text state of the LCD
int morseColumn = 0;
int englishColumn = 0;
const int MORSE_ROW = 0;
const int ENGLISH_ROW = 1;

// Set the state system of the button presses
boolean isButtonActive = false;                                                                   // for telling if the button is being pressed so that we only have to use digitalRead() once
boolean isLongPressActive = false;


//helper function for reseting the first row
void resetMorseRow() {
  for(int i = 0; i < 5; i++){
    lcd.setCursor(i, MORSE_ROW);
    lcd.print(" ");
  }
  morseColumn = 0;
  lcd.setCursor(morseColumn, MORSE_ROW);
}

void setup()
{
   // set up the LCD
  lcd.begin(16, 2);
  lcd.setRGB(colorR, colorG, colorB);
  //lcd.cursor();                                                                                       //makes the cursor where the next character will be printed at, but only useful for debugging as it looks too much like a dah
  lcd.setCursor(0, 0);
 
}

void loop()
{
  
  if (digitalRead(buttonPin) == HIGH) {                                                               // if the button is being pressed ...
    if (isButtonActive == false) {                                                                    // if the last saved state of the button was set to false i.e it's the first run of the program ...
      isButtonActive = true;                                                                          // set it to true
      buttonPressTime = millis();                                                                     // save the time when the button was pressed  
    }
    if ((millis() - buttonPressTime > longPressTime) && (isLongPressActive == false)) {               // if the current time - the time the button was pressed is the long button press time
                                                                                                      // AND the last saved state of the Long button press is set to false (to avoid recording two long presses)
      isLongPressActive = true;                                                                       // set the state of isLongPressActive to true
      lcd.setCursor(morseColumn++,MORSE_ROW);                                                                             // specify the next column in the "morse row"
      lcd.print('_');                                                                                 // print a dah
      morseVals = morseVals + "_";
    }
  } 
  
  else {                                                                                                // happens if the long press case failed
    if (isButtonActive == true) {                                                                       // if the button is being pressed
      if (isLongPressActive == true) {                                                                  // if the long press state is set to true ...
        isLongPressActive = false;                                                                      // set it to false
      } 
      else {
        lcd.setCursor(morseColumn++,MORSE_ROW);
        lcd.print('.');                                                                                 // print a dit
        morseVals = morseVals + ".";
      }
      isButtonActive = false;                                                                           // set button active to false (we don't check how short a short press is)
    }
  }


  //it is my understanding that the character timer does not depend on whether or not a button was presses, just time so save the time for each interval
  unsigned long currentMillis = millis();

  if ((currentMillis - previousMillis >= interval) || (morseVals.length() >= 5) ) {                   //TODO: this if statement works as a delay right now, but I should I should actually reset the timer whenever the user
    // save the last time you printed an english character
    previousMillis = currentMillis;

    // print the character now and move up one row once you finish
    lcd.setCursor(morseColumn, MORSE_ROW);                                                                                  
    //declare the big switch case here
    if(morseVals.equals("._")){
      englishChar = 'A';
    }
     else if(morseVals.equals("_...")) {
      englishChar = 'B';
     }
     else if(morseVals.equals("_._.")) {
      englishChar = 'C';
     }
     else if(morseVals.equals("_..")) {
      englishChar = 'D';
     }
     else if(morseVals.equals(".")) {
      englishChar = 'E';
     }
     else if(morseVals.equals(".._.")) {
      englishChar = 'F';
     }
     else if(morseVals.equals("__.")) {
      englishChar = 'G';
     }
     else if(morseVals.equals("....")) {
      englishChar = 'H';
     }
     else if(morseVals.equals("..")) {
      englishChar = 'I';
     }
     else if(morseVals.equals(".___")) {
      englishChar = 'J';
     }
     else if(morseVals.equals("_._")) {
      englishChar = 'K';
     }
     else if(morseVals.equals("._..")) {
      englishChar = 'L';
     }
     else if(morseVals.equals("__")) {
      englishChar = 'M';
     }
     else if(morseVals.equals("_.")) {
      englishChar = 'N';
     }
     else if(morseVals.equals("___")) {
      englishChar = 'O';
     }
     else if(morseVals.equals(".__.")) {
      englishChar = 'P';
     }
     else if(morseVals.equals("__._")) {
      englishChar = 'Q';
     }
     else if(morseVals.equals("._.")) {
      englishChar = 'R';
     }
     else if(morseVals.equals("...")) {
      englishChar = 'S';
     }
     else if(morseVals.equals("_")) {
      englishChar = 'T';
     }
     else if(morseVals.equals(".._")) {
      englishChar = 'U';
     }
     else if(morseVals.equals("..._")) {
      englishChar = 'V';
     }
     else if(morseVals.equals(".__")) {
      englishChar = 'W';
     }
     else if(morseVals.equals("_.._")) {
      englishChar = 'X';
     }
     else if(morseVals.equals("_.__")) {
      englishChar = 'Y';
     }
     else if(morseVals.equals("__..")) {
      englishChar = 'Z';
     }
     else if(morseVals.equals(".____")) {
      englishChar = '1';
     }
     else if(morseVals.equals("..___")) {
      englishChar = '2';
     }
     else if(morseVals.equals("...__")) {
      englishChar = '3';
     }
     else if(morseVals.equals("...._")) {
      englishChar = '4';
     }
     else if(morseVals.equals(".....")) {
      englishChar = '5';
     }
     else if(morseVals.equals("_....")) {
      englishChar = '6';
     }
     else if(morseVals.equals("__...")) {
      englishChar = '7';
     }
     else if(morseVals.equals("____..")) {
      englishChar = '8';
     }
     else if(morseVals.equals("____.")) {
      englishChar = '9';
     }
     else if(morseVals.equals("_____")) {
      englishChar = '0';
     }    
    else {
      // in this implementation of a morse code device, no valid input within the 10 second counter means that we must print a space character. 
      englishChar = ' ';
    }
    //Reset the decoding algorithm dependencies and print the English character
    morseVals = "";
    lcd.setCursor(englishColumn++,ENGLISH_ROW);
    lcd.print(englishChar);
    resetMorseRow();    
  }

  
}