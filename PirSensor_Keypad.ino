#include <Keypad.h>
#include <Wire.h> 
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define OLED_RESET 16 
#define SCREEN_ADDRESS 0x3C 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
const int pirpin = 34;
int pirvalue;
int ledpin1= 21; //white led to determine when a button is pressed
int ledpin2= 0; //green led to determine when the pir sensor is activated
int ledpin3= 13; // red led to activate if motion is detected when system is locked
enum SYSTEM_STATUS
{
  LOCKED, //0
  UNLOCKED //1
 };
static SYSTEM_STATUS currentStatus = LOCKED;
const String password = "2006";
String input;
const byte ledPin = 21;
const byte ROWS = 4; 
const byte COLS = 4; 
char keys[ROWS][COLS] = {
 {'1','2','3','A'},
 {'4','5','6','B'},
 {'7','8','9','C'},
 {'*','0','#','D'}
 };
byte rowPins[ROWS] ={22, 23, 2, 17}; 
byte colPins[COLS] = {32, 33, 25, 12}; 

Keypad keypad = Keypad(makeKeymap(keys),rowPins,colPins,ROWS,COLS); // Instantiates a . 
char message[50];
  
void setup() 
{  
    pinMode(pirpin, INPUT);
    pinMode(ledpin1, OUTPUT);
    pinMode(ledpin2, OUTPUT);
    pinMode(ledpin3, OUTPUT);
    digitalWrite(ledpin1, LOW);
    digitalWrite(ledpin2, LOW);
    digitalWrite(ledpin3, LOW);
    Wire.begin(4,15);
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) 
    {
         Serial.println("SSD1306 allocation failed...");
         for(;;);
    }   
    Serial.begin(115200);
    keypad.addEventListener(handleKey); 
    display.clearDisplay();
    sprintf(message,"   Initializing..."); 
    oledDisplay(WHITE, 1, 0, 0, message);
    sprintf(message," type the password"); 
    oledDisplay(WHITE, 1, 20, 0, message);
    
    
}
void loop() 
{
  pirvalue = digitalRead(pirpin);
  if(pirvalue == 1)
  {
    digitalWrite(ledpin2, pirvalue);
  }
  if(pirvalue == 0)
  {
    digitalWrite(ledpin2, pirvalue);
  }
  if(pirvalue == 1 && currentStatus == LOCKED)
  {
    digitalWrite(ledpin3, pirvalue);
    display.clearDisplay();
    sprintf(message,"      INTRUDER"); 
    oledDisplay(WHITE, 1, 20, 0, message);
    sprintf(message," calling the owner..."); 
    oledDisplay(WHITE, 1, 40, 0, message);

  }
  if(pirvalue == 0  && currentStatus == UNLOCKED)
  {
    digitalWrite(ledpin3, pirvalue);
  }
  char key = keypad.getKey(); // reading the keyboard
  if (key) 
  {
    if ((key != '#') && (key != '*'))
  {
      input += key;
  }
    Serial.print("key:");
    Serial.println(key);
 }
}

void updateLEDStatus() 
{
 if (currentStatus == LOCKED)
 {
    currentStatus = UNLOCKED;
    Serial.println("SYSTEM UNLOCKED");
    sprintf(message,"\t SYSTEM UNLOCKED"); 
    oledDisplay(WHITE, 1, 0, 0, message);
    digitalWrite(ledPin, LOW); 
 }
 else
 {
    currentStatus = LOCKED;
    Serial.println("SYSTEM LOCKED");
    sprintf(message,"\t SYSTEM LOCKED"); 
    oledDisplay(WHITE, 1, 0, 0, message);
  
    digitalWrite(ledPin, HIGH); 
}
}
void handleKey(KeypadEvent key)
{
   switch (keypad.getState())
   {
     case PRESSED:  digitalWrite(ledPin, !digitalRead(ledPin));
                    delay(500);
                    digitalWrite(ledPin, !digitalRead(ledPin));
                     if (key == '#') // this is our ENTER
                     {
                        Serial.println(input);
                        if (input == password)
                        {
                            display.clearDisplay();
                            updateLEDStatus();
                        }
                        input = "";
                     }
                     break;
     case RELEASED:  if (key == '*') 
                     {
                        input = ""; // this is our CLEAR
                     }
   }
}
void oledDisplay(unsigned short colour, int textSize, int row, int col, char message[])
{
   display.setTextColor(colour);
   display.setTextSize(textSize);
   display.setCursor(col,row);
   display.print(message);
   display.display();
}
