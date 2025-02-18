#include <Keypad.h>
#include<ESP32Servo.h>
int led =21;
int ledPin =0; // determines when a button on the keypad is pressed
int button=34; // indicates if the servo motor can be set up with the keypad
int serv=13;
int aprins=0;
Servo myservo; 
#define MIN 600
#define MAX 2300
String input;
const byte ROWS = 4; 
const byte COLS = 4; 
char keys[ROWS][COLS] = {
 {'1','2','3','A'},
 {'4','5','6','B'},
 {'7','8','9','C'},
 {'*','0','#','D'}
 };
byte rowPins[ROWS] ={22, 23, 2, 17}; 

byte colPins[ROWS] = {32, 33, 25, 12};  
 
Keypad keypad = Keypad(makeKeymap(keys),rowPins,colPins,ROWS,COLS); 
 
void setup() 
{
  myservo.attach(serv, MIN, MAX); 
  myservo.write (90);
  pinMode(button, INPUT);
  pinMode(led, OUTPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
 Serial.begin(115200);
 keypad.addEventListener(handleKey); 
}
void loop() 
{
  
  int value=digitalRead(button);
  if(value == 1)
  {
    delay(250);
     aprins=!aprins;
     Serial.println(aprins);
     digitalWrite(led, aprins);
  }
  if(aprins==1)
  {
    
     char key = keypad.getKey(); 
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
}


void handleKey(KeypadEvent key)
{
 switch (keypad.getState())
 {
 case PRESSED: digitalWrite(ledPin, !digitalRead(ledPin));
 delay(250);
 digitalWrite(ledPin, !digitalRead(ledPin));
 if (key == '#') // this is our ENTER
 {
    Serial.println(input);
    int servo_int=input.toInt();
    myservo.write(servo_int);
    input = ""; //clearing after writing to servo motor
 }
 break;
 case RELEASED: if (key == '*') 
 {
 input = ""; // this is our CLEAR
 }
 break;
 }
}
