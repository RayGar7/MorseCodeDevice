
#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h>
#include "rgb_lcd.h"

/**************** WEB SERVER SYSTEM ****************************/
// MAC address from Ethernet shield sticker on board
byte mac[] = { 0x98, 0x4F, 0xEE, 0x00, 0x2C, 0xDF };
IPAddress ip(169,254,8,55); // IP address of the board
EthernetServer server(80);  // create a server at port 80, it's the default

String HTTP_req;          // stores the HTTP request
boolean LED_status = 0;   // state of LED, off by default


/**************** MORSE CODE SYSTEM ***************************/
//hardware constants
rgb_lcd lcd;
const int colorR = 66;
const int colorG = 241;
const int colorB = 244;
const int buttonPin = 3;

// Set the timer system for the button presses
long buttonPressTime = 0;
long longPressTime = 1000;                                            //a long button press is one second, in this morse code implementation
unsigned long previousMillis = 0;                                     // will store last time LED was updated
const long interval = 10000;                                          // interval at which to decode a new morse code character (milliseconds)
long syncInterval = 0;                                                // interval for the synchronization of the server

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



void setup()
{
    Ethernet.begin(mac, ip);  // initialize Ethernet device
    server.begin();           // start to listen for clients
    Serial.begin(9600);       // for diagnostics
    
    pinMode(LED_BUILTIN, OUTPUT);       // On board LED 

    // set up the LCD
    lcd.begin(16, 2);
    lcd.setRGB(colorR, colorG, colorB);
    lcd.cursor();                                                                                       //makes the cursor where the next character will be printed at, but only useful for debugging as it looks too much like a dah
    lcd.setCursor(0, 0);
}

void loop()
{
    EthernetClient client = server.available();  // try to get client

    if (client) {  // got client?
        boolean currentLineIsBlank = true;
        while (client.connected()) {
            if (client.available()) {   // client data available to read                                                TODO: CHECK THE SYNCHING INTERVAL HERE
                char c = client.read(); // read 1 byte (character) from client
                HTTP_req += c;  // save the HTTP request 1 char at a time
                // last line of client request is blank and ends with \n
                // respond to client only after last line received
                if (c == '\n' && currentLineIsBlank) {
                    // send a standard http response header
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-Type: text/html");
                    client.println("Connection: close");
                    client.println();
                    // send web page
                    client.println("<!DOCTYPE html>");
                    client.println("<html>");
                    client.println("<head>");
                    client.println("<title>Arduino Morse Code Control</title>");
                    client.println("<meta name='viewport' content='user-scalable=no'/>");
                    client.println("</head>");
                    client.println("<body>");
                    client.println("<h1>Morse Code Decoding</h1>");
                    client.println("<p>Click to send Morse Code</p>");
                    client.println("<form action='' method='get'>");
                    client.println("<input style='height:20%;width:40%;font-size:70px' name='_' type='submit' value='_' />");
                    client.println("<input style='height:20%;width:40%;font-size:70px' name='.' type='submit' value='.' />");
                    client.println("<h1>Encode English to Morse characters</h1>");
                    client.println("<input name='A' type='submit' value='A'/>");
                    client.println("<input name='B' type='submit' value='B'/>");
                    client.println("<input name='C' type='submit' value='C'/>");
                    client.println("<input name='D' type='submit' value='D'/>");
                    client.println("<input name='E' type='submit' value='E'/>");
                    client.println("<input name='F' type='submit' value='F'/>");
                    client.println("<input name='G' type='submit' value='G'/>");
                    client.println("<input name='H' type='submit' value='H'/>");
                    client.println("<input name='I' type='submit' value='I'/>");
                    client.println("<input name='J' type='submit' value='J'/>");
                    client.println("<input name='K' type='submit' value='K'/>");
                    client.println("<input name='L' type='submit' value='L'/>");
                    client.println("<input name='M' type='submit' value='M'/>");
                    client.println("<input name='N' type='submit' value='N'/>");
                    client.println("<input name='O' type='submit' value='O'/>");
                    client.println("<input name='P' type='submit' value='P'/>");
                    client.println("<input name='Q' type='submit' value='Q'/>");
                    client.println("<input name='R' type='submit' value='R'/>");
                    client.println("<input name='S' type='submit' value='S'/>");
                    client.println("<input name='T' type='submit' value='T'/>");
                    client.println("<input name='U' type='submit' value='U'/>");
                    client.println("<input name='V' type='submit' value='V'/>");
                    client.println("<input name='W' type='submit' value='W'/>");
                    client.println("<input name='X' type='submit' value='X'/>");
                    client.println("<input name='Y' type='submit' value='Y'/>");
                    client.println("<input name='Z' type='submit' value='Z'/>");
                    client.println("<input name='1' type='submit' value='1'/>");
                    client.println("<input name='2' type='submit' value='2'/>");
                    client.println("<input name='3' type='submit' value='3'/>");
                    client.println("<input name='4' type='submit' value='4'/>");
                    client.println("<input name='5' type='submit' value='5'/>");
                    client.println("<input name='6' type='submit' value='6'/>");
                    client.println("<input name='7' type='submit' value='7'/>");
                    client.println("<input name='8' type='submit' value='8'/>");
                    client.println("<input name='9' type='submit' value='9'/>");
                    client.println("<input name='0' type='submit' value='0'/>");
                    webMorseHandler(client);
                    client.println("</form>");
                    client.println("</body>");
                    client.println("</html>");
                    Serial.print(HTTP_req);
                    HTTP_req = "";    // finished with request, empty string
                    break;
                }
                // every line of text received from the client ends with \r\n
                if (c == '\n') {
                    // last character on line of received text
                    // starting new line with next character read
                    currentLineIsBlank = true;
                } 
                else if (c != '\r') {
                    // a text character was received from client
                    currentLineIsBlank = false;
                }
            } // end if (client.available())
        } // end while (client.connected())
        delay(1);      // give the web browser time to receive the data
        client.stop(); // close the connection
    } // end if (client)

        
  //enable sending dits and dahs with the pushbuttons, separate from the web page also
  morseDecoder();

}


void morseDecoder() {
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

  if ((currentMillis - previousMillis >= interval) || (morseVals.length() >= 5) ) {                   //TODO: this if statement works as a delay right now, but I should I should actually reset the timer whenever the user presses the button
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


// MORSE CODE SYSTEM INDEPENDENT:helper function for reseting the first row
void resetMorseRow() {
  for(int i = 0; i < 5; i++){
    lcd.setCursor(i, MORSE_ROW);
    lcd.print(" ");
  }
  morseColumn = 0;
  lcd.setCursor(morseColumn, MORSE_ROW);
}

void resetOutputRow() {
  for(int i = 0; i < englishColumn; i++){
    lcd.setCursor(i, ENGLISH_ROW);
    lcd.print(" ");
  }
  englishColumn = 0;
  //lcd.setCursor(englishColumn, ENLGISH_ROW);
}

//MORSE CODE SYSTEM AND WEB SERVER dependent: decode and encode morse characters from the page into the web server
void webMorseHandler(EthernetClient cl) {
  
   //first handle morse decoding

   
   if (HTTP_req.indexOf("_=_") > -1){
      lcd.setCursor(morseColumn++,MORSE_ROW);                                                                             // specify the next column in the "morse row"
      lcd.print('_');                                                                                 // print a dah
      morseVals = morseVals + "_";
      //webpage.send(morseVals);
    } 
  else if(HTTP_req.indexOf(".=.") > -1) {    
    lcd.setCursor(morseColumn++,MORSE_ROW);
    lcd.print('.');                                                                                 // print a dit
     morseVals = morseVals + ".";
     //webpage.send(morseVals);
  }
  

  unsigned long currentMillis = millis();

  if ((currentMillis - previousMillis >= interval) || (morseVals.length() >= 5) ) {                   //TODO: this if statement works as a delay right now, but I should I should actually reset the timer whenever the user presses the button
    // save the last time you printed an english character
    previousMillis = currentMillis;
 

    lcd.setCursor(morseColumn, MORSE_ROW);  
    //TODO: display the morse code string into the web page here
    //cl.println(morseVals);                                                                                     
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
    //TODO: display the english character onto the web page here.

    
    //Reset the decoding algorithm dependencies and print the English character
    morseVals = "";
    lcd.setCursor(englishColumn++,ENGLISH_ROW);
    lcd.print(englishChar);
    cl.print("<h1>");
    cl.print(englishChar);
    cl.print("</h1>");
    resetMorseRow();
  }    




      //now handle morse encoding

    if (HTTP_req.indexOf("A=A") > -1){
      lcd.setCursor(morseColumn++,MORSE_ROW);                                                                             // specify the next column in the "morse row"
      englishChar = 'A';
      //webpage.send(morseVals);
    } 
    if (HTTP_req.indexOf("B=B") > -1){
      lcd.setCursor(morseColumn++,MORSE_ROW);                                                                             // specify the next column in the "morse row"
      englishChar='B';
      //webpage.send(morseVals);
    }   
    if (HTTP_req.indexOf("C=C") > -1){
      lcd.setCursor(morseColumn++,MORSE_ROW);                                                                             // specify the next column in the "morse row"
      englishChar='C';
      //webpage.send(morseVals);
    }    
    if (HTTP_req.indexOf("D=D") > -1){
      lcd.setCursor(morseColumn++,MORSE_ROW);                                                                             // specify the next column in the "morse row"
      englishChar='D';
      //webpage.send(morseVals);
    }    
    if (HTTP_req.indexOf("E=E") > -1){
      lcd.setCursor(morseColumn++,MORSE_ROW);                                                                             // specify the next column in the "morse row"
      englishChar='E';
      //webpage.send(morseVals);
    }    
    if (HTTP_req.indexOf("F=F") > -1){
      lcd.setCursor(morseColumn++,MORSE_ROW);                                                                             // specify the next column in the "morse row"
      englishChar='F';
      //webpage.send(morseVals);
    }    
    if (HTTP_req.indexOf("G=G") > -1){
      lcd.setCursor(morseColumn++,MORSE_ROW);                                                                             // specify the next column in the "morse row"
      englishChar='G';
      //webpage.send(morseVals);
    }    
    if (HTTP_req.indexOf("H=H") > -1){
      lcd.setCursor(morseColumn++,MORSE_ROW);                                                                             // specify the next column in the "morse row"
      englishChar='H';
      //webpage.send(morseVals);
    }    
    if (HTTP_req.indexOf("I=I") > -1){
      lcd.setCursor(morseColumn++,MORSE_ROW);                                                                             // specify the next column in the "morse row"
      englishChar='I';
      //webpage.send(morseVals);
    }    
    if (HTTP_req.indexOf("J=J") > -1){
      lcd.setCursor(morseColumn++,MORSE_ROW);                                                                             // specify the next column in the "morse row"
      englishChar='J';
      //webpage.send(morseVals);
    }    
    if (HTTP_req.indexOf("K=K") > -1){
      lcd.setCursor(morseColumn++,MORSE_ROW);                                                                             // specify the next column in the "morse row"
      englishChar='K';
      //webpage.send(morseVals);
    }    
    if (HTTP_req.indexOf("L=L") > -1){
      lcd.setCursor(morseColumn++,MORSE_ROW);                                                                             // specify the next column in the "morse row"
      englishChar='L';
      //webpage.send(morseVals);
    }    
    if (HTTP_req.indexOf("M=M") > -1){
      lcd.setCursor(morseColumn++,MORSE_ROW);                                                                             // specify the next column in the "morse row"
      englishChar='M';
      //webpage.send(morseVals);
    }    
    if (HTTP_req.indexOf("N=N") > -1){
      lcd.setCursor(morseColumn++,MORSE_ROW);                                                                             // specify the next column in the "morse row"
      englishChar='N';
      //webpage.send(morseVals);
    }    
    if (HTTP_req.indexOf("O=O") > -1){
      lcd.setCursor(morseColumn++,MORSE_ROW);                                                                             // specify the next column in the "morse row"
      englishChar='O';
      //webpage.send(morseVals);
    }   
    if (HTTP_req.indexOf("P=P") > -1){
      lcd.setCursor(morseColumn++,MORSE_ROW);                                                                             // specify the next column in the "morse row"
      englishChar='P';
      //webpage.send(morseVals);
    }    
    if (HTTP_req.indexOf("Q=Q") > -1){
      lcd.setCursor(morseColumn++,MORSE_ROW);                                                                             // specify the next column in the "morse row"
      englishChar='Q';
      //webpage.send(morseVals);
    }    
    if (HTTP_req.indexOf("R=R") > -1){
      lcd.setCursor(morseColumn++,MORSE_ROW);                                                                             // specify the next column in the "morse row"
      englishChar='R';
      //webpage.send(morseVals);
    }   
    if (HTTP_req.indexOf("S=S") > -1){
      lcd.setCursor(morseColumn++,MORSE_ROW);                                                                             // specify the next column in the "morse row"
      englishChar='S';
      //webpage.send(morseVals);
    }    
    if (HTTP_req.indexOf("T=T") > -1){
      lcd.setCursor(morseColumn++,MORSE_ROW);                                                                             // specify the next column in the "morse row"
      englishChar='T';
      //webpage.send(morseVals);
    }    
    if (HTTP_req.indexOf("U=U") > -1){
      lcd.setCursor(morseColumn++,MORSE_ROW);                                                                             // specify the next column in the "morse row"
      englishChar='U';
      //webpage.send(morseVals);
    }    
    if (HTTP_req.indexOf("V=V") > -1){
      lcd.setCursor(morseColumn++,MORSE_ROW);                                                                             // specify the next column in the "morse row"
      englishChar='V';
      //webpage.send(morseVals);
    }    
    if (HTTP_req.indexOf("W=W") > -1){
      lcd.setCursor(morseColumn++,MORSE_ROW);                                                                             // specify the next column in the "morse row"
      englishChar='W';
      //webpage.send(morseVals);
    }    
    if (HTTP_req.indexOf("X=X") > -1){
      lcd.setCursor(morseColumn++,MORSE_ROW);                                                                             // specify the next column in the "morse row"
      englishChar='X';
      //webpage.send(morseVals);
    }    
    if (HTTP_req.indexOf("Y=Y") > -1){
      lcd.setCursor(morseColumn++,MORSE_ROW);                                                                             // specify the next column in the "morse row"
      englishChar='Y';
      //webpage.send(morseVals);
    }    
    if (HTTP_req.indexOf("Z=Z") > -1){
      lcd.setCursor(morseColumn++,MORSE_ROW);                                                                             // specify the next column in the "morse row"
      englishChar='Z';
      //webpage.send(morseVals);
    }    
    if (HTTP_req.indexOf("1=1") > -1){
      lcd.setCursor(morseColumn++,MORSE_ROW);                                                                             // specify the next column in the "morse row"
      englishChar='1';
      //webpage.send(morseVals);
    }    
    if (HTTP_req.indexOf("2=2") > -1){
      lcd.setCursor(morseColumn++,MORSE_ROW);                                                                             // specify the next column in the "morse row"
      englishChar='2';
      //webpage.send(morseVals);
    }    
    if (HTTP_req.indexOf("3=3") > -1){
      lcd.setCursor(morseColumn++,MORSE_ROW);                                                                             // specify the next column in the "morse row"
      englishChar='3';
      //webpage.send(morseVals);
    }    
    if (HTTP_req.indexOf("4=4") > -1){
      lcd.setCursor(morseColumn++,MORSE_ROW);                                                                             // specify the next column in the "morse row"
      englishChar='4';
      //webpage.send(morseVals);
    }    
    if (HTTP_req.indexOf("5=5") > -1){
      lcd.setCursor(morseColumn++,MORSE_ROW);                                                                             // specify the next column in the "morse row"
      englishChar='5';
      //webpage.send(morseVals);
    } 
    if (HTTP_req.indexOf("6=6") > -1){
      lcd.setCursor(morseColumn++,MORSE_ROW);                                                                             // specify the next column in the "morse row"
      englishChar='6';
      //webpage.send(morseVals);
    }    
    if (HTTP_req.indexOf("7=7") > -1){
      //lcd.setCursor(morseColumn++,MORSE_ROW);                                                                             // specify the next column in the "morse row"
      englishChar='7';
      //webpage.send(morseVals);
    }    
    if (HTTP_req.indexOf("8=8") > -1){
      //lcd.setCursor(morseColumn++,MORSE_ROW);                                                                             // specify the next column in the "morse row"
      englishChar='8';
      //webpage.send(morseVals);
    }    
    if (HTTP_req.indexOf("9=9") > -1){
      //lcd.setCursor(morseColumn++,MORSE_ROW);                                                                             // specify the next column in the "morse row"
      englishChar='9';
      //webpage.send(morseVals);
    } 
    if (HTTP_req.indexOf("0=0") > -1){
      //lcd.setCursor(morseColumn++,MORSE_ROW);                                                                             // specify the next column in the "morse row"
      englishChar='0';
      //webpage.send(morseVals);
    } 
 
  //resetMorseRow();
  lcd.setCursor(0, MORSE_ROW);  
    //cl.println(morseVals);                                                                                     
    //declare the big switch case here
    if(englishChar == 'A'){
      morseVals = "._";
    }
     else if(englishChar == 'B') {
      morseVals = "_...";
     }
     else if(englishChar == 'C') {
      morseVals = "_._.";
     }
     else if(englishChar == 'D') {
      morseVals = "_..";
     }
     else if(englishChar == 'E') {
      morseVals = ".";
     }
     else if(englishChar == 'F') {
      morseVals = ".._.";
     }
     else if(englishChar == 'G') {
      morseVals = "__.";
     }
     else if(englishChar == 'H') {
      morseVals = "....";
     }
     else if(englishChar == 'I') {
      morseVals = "..";
     }
     else if(englishChar == 'J') {
      morseVals = ".___";
     }
     else if(englishChar == 'K') {
      morseVals = "_._";
     }
     else if(englishChar == 'L') {
      morseVals = "._..";
     }
     else if(englishChar == 'M') {
      morseVals = "__";
     }
     else if(englishChar == 'N') {
      morseVals = "_.";
     }
     else if(englishChar == 'O') {
      morseVals = "___";
     }
     else if(englishChar == 'P') {
      morseVals = ".__.";
     }
     else if(englishChar == 'Q') {
      morseVals = "__._";
     }
     else if(englishChar == 'R') {
      morseVals = "._.";
     }
     else if(englishChar == 'S') {
      morseVals = "...";
     }
     else if(englishChar == 'T') {
      morseVals = "_";
     }
     else if(englishChar == 'U') {
      morseVals = ".._";
     }
     else if(englishChar == 'V') {
      morseVals = "..._";
     }
     else if(englishChar == 'W') {
      morseVals = ".__";
     }
     else if(englishChar == 'X') {
      morseVals = "_.._";
     }
     else if(englishChar == 'Y') {
      morseVals = "_.__";
     }
     else if(englishChar == 'Z') {
      morseVals = "__..";
     }
     else if(englishChar == '1') {
      morseVals = ".____";
     }
     else if(englishChar == '2') {
      morseVals = "..___";
     }
     else if(englishChar == '3') {
      morseVals = "...__";
     }
     else if(englishChar == '4') {
      morseVals = "...._";
     }
     else if(englishChar == '5') {
      morseVals = ".....";
     }
     else if(englishChar == '6') {
      morseVals = "_....";
     }
     else if(englishChar == '7') {
      morseVals = "__...";
     }
     else if(englishChar == '8') {
      morseVals = "____..";
     }
     else if(englishChar == '9') {
      morseVals = "____.";
     }
     else if(englishChar == '0') {
      morseVals = "_____";
     }    
    else {
      // in this implementation of a morse code device, no valid input within the 10 second counter means that we must print a space character. 
      morseVals = "";
    }
    //TODO: display the english character onto the web page here.


    //TODO: first check if we actually got sent a new english character because this code runs regardless, try doing it first by checking if morseVals is null
 
    //Reset the decoding algorithm dependencies and print the English character
    englishColumn += morseVals.length();
    if(englishColumn >= 14) {
        for(int i = 0; i < morseVals.length(); i++){
          lcd.setCursor(i, ENGLISH_ROW);
          lcd.print(" ");
        }
       englishColumn = 0;
    }
    
    lcd.setCursor(englishColumn,ENGLISH_ROW);
    lcd.print(morseVals);
    cl.print("<h1>");
    cl.print(morseVals);
    cl.print("</h1>");
    morseVals = "";
}




