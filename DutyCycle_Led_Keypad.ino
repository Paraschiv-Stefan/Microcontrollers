#include <Keypad.h>
#include <Wire.h> 
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define OLED_RESET 16 
#define SCREEN_ADDRESS 0x3C 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int ledpin1= 21;
int ledpin2= 13;
int pushbutton= 34;

int max_duty_value; //maximum value of the duty cycle that is dependent of resolution!
int selectedR=0; // variable used for selecting the LEDS
int firstRead;  //variable used for displaying guiding message on what to do after changing the LEDs

int pwmChannel1 = 0; 
int pwmChannel2 = 2;
int flagDutyRead=3;  //variable used for tying the duty cycle or resolution / it is 3 to not put the res to 0 in the beginning
int duty_int;     
int frequence = 1000;    
int reso=8; // res=8 default resolution | between 1-16 bits
int maxResolution;
int resolution;

String duty;
String res;  
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

Keypad keypad = Keypad(makeKeymap(keys),rowPins,colPins,ROWS,COLS);
 
char message[50];

void oledDisplay(unsigned short colour, int textSize, int row, int col, char message[])
{
   display.setTextColor(colour);
   display.setTextSize(textSize);
   display.setCursor(col,row);
   display.print(message);
   display.display();
}
void handleKey(KeypadEvent key);
void displayMaxResolutionExceeded();
void displayDutyCycleExceeded();
void displayDefaultRes();
void writeFirstMessages();
void UpdatePwmSettings();
void DutyValueSet();
//had to declare the functions before the setup otherwise it ketp giving me errors;

void setup() 
{   
    maxResolution = maxReso(); 
    pinMode(pushbutton, INPUT);
    pinMode(ledpin1, OUTPUT);
    pinMode(ledpin2, OUTPUT);
    Wire.begin(4,15);
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) 
    {
         Serial.println("SSD1306 allocation failed...");
         for(;;);
    }   
    Serial.begin(115200);
    keypad.addEventListener(handleKey); 
    display.clearDisplay();
    sprintf(message,"Loading..."); 
    oledDisplay(WHITE, 1, 10, 0, message);
    sprintf(message,"PRESS THE BUTTON"); 
    oledDisplay(WHITE, 1, 20, 0, message);
    sprintf(message,"TO SELECT A LED"); 
    oledDisplay(WHITE, 1, 30, 0, message);
     display.clearDisplay();
     ledcAttachChannel(ledpin1, frequence, reso, pwmChannel1); 
     ledcAttachChannel(ledpin2, frequence, reso, pwmChannel2);
}
void loop() 
{
  
  int val= digitalRead(pushbutton);
  if(val==1 && firstRead==0){ writeFirstMessages(); firstRead+=1;} //to display the guiding message  on what to do after selecting the led
  if(val!=0 && duty_int == 0) 
    { 
      switch(selectedR)      // for selecting the wanted led
      {
        case 0:   for(int i=1;i<4;i++)
                  {
                    ledcWrite(ledpin1,255); // becuase of the default resolution , i used ledcWrite.
                    delay(200);
                    ledcWrite(ledpin1,0);
                    delay(200);
                  } 
                  selectedR = 1;
                  break;
         case 1:  for(int i=1;i<4;i++)
                  {
                    ledcWrite(ledpin2,255);
                    delay(200);
                    ledcWrite(ledpin2,0);
                    delay(200);
                  } 
                  selectedR = 0;
      }
    }
  char key = keypad.getKey(); 
  if (key)
  {
    if ((key != '#') && (key != '*')&& (key != 'A') && (key!='B'))
    {
      if(flagDutyRead==1)
        duty += key;
      else
        res += key;
      Serial.print("key:");
      Serial.println(key);
    }
  
      if(key == 'A')
      {
        duty = "";  /// reseting the duty, maybe the user only wants to change the resolution and keep the same duty cycle
        if(flagDutyRead == 0) reso =res.toInt(); // if the resolution was selected, changing the value of reso now to display the range correctly
        flagDutyRead=1;
        display.clearDisplay();
        sprintf(message,"SELECT DUTY CYCLE"); 
        oledDisplay(WHITE, 1, 10, 0, message);
        sprintf(message,"RANGE:0 - %d",maxDuty()); 
        oledDisplay(WHITE, 1, 20, 0, message);
      }
      if(key == 'B')
      {
        res = "";  //same case for resolution as for duty in the above if
        flagDutyRead=0;
        duty_int=duty.toInt();
        max_duty_value=maxDuty();
        if(duty_int != 0 && duty_int>max_duty_value) //if the user selected a bigger duty cycle than specified in the RANGE
        {
          for(resolution=1;resolution<=16;resolution++) //calculating the resolutions that can be selected with the higher duty cycle
          {
            int duty_max= maxDuty2();
            if(duty_int<duty_max) break;
          }
          DutyValueSet(); // a duty cycle was already selected, attentionating user to select the resolution acordingly
        }
        else // otherwise displaying the full range of possible resolutions
        {
          display.clearDisplay();
          sprintf(message,"SELECT RESOLUTION"); 
          oledDisplay(WHITE, 1, 10, 0, message);
          sprintf(message,"BETWEEN 1 AND %d",maxResolution); 
          oledDisplay(WHITE, 1, 20, 0, message);
        }
      }
  }
}
void handleKey(KeypadEvent key)
{
   switch (keypad.getState())
   {
     case PRESSED: if (key == '#')
                     {  
                        Serial.println(duty);
                        Serial.println(res); 
                        if(res=="") // if there is no resolution selected by the user.
                        { 
                          displayDefaultRes(); //displaying the message that no resolution was selected
                          reso=8; //setting the resolution to the default value
                          delay(1500);
                        }
                        else reso=res.toInt();
                        max_duty_value=maxDuty();
                        duty_int=duty.toInt();
                        if(max_duty_value<duty_int) // testing to see if duty cycle is not more than the resolution selected
                        {
                          displayDutyCycleExceeded();
                          duty="";
                          duty_int = 0;
                          delay(2000);
                        }  
                        if(selectedR==0) //firs led
                        {
                          
                          ledcAttachChannel(ledpin1, frequence, reso, pwmChannel1); 
                          
                        }   
                        else  //second led
                        {
                          
                          ledcAttachChannel(ledpin2, frequence, reso, pwmChannel2);
                        } 
                                 
                        if(reso>maxResolution) // testing the max resolution, if it is above the maximum, it resets to the default value 8
                        {
                          reso=8;
                          displayMaxResolutionExceeded(); // message to display the resolution is exceeded
                          delay(2000);
                        }         
                        if(selectedR==1) //tunring the first led
                          {
                              Serial.println("Turning on LED 1");
                              ledcWrite(ledpin1, duty_int);
                              ledcWrite(ledpin2, 0);
                          }
                          else //turning the second led
                          {
                            Serial.println("Turning on LED 2");
                              ledcWrite(ledpin1, 0);
                              ledcWrite(ledpin2, duty_int);
                          }
                          
                        display.clearDisplay();
                        UpdatePwmSettings(); //displaying the selected resolution and duty cycle selected by the user              
                     }
                     break;
                    
     case RELEASED:  if (key == '*') 
                     {
                        display.clearDisplay();
                        sprintf(message,"  SETTINGS RESET"); 
                        oledDisplay(WHITE, 1, 10, 0, message);
                        sprintf(message,"  Press A and B"); 
                        oledDisplay(WHITE, 1, 20, 1, message);
                        sprintf(message,"         OR"); 
                        oledDisplay(WHITE, 1, 30, 0, message);
                        sprintf(message,"  change the led"); 
                        oledDisplay(WHITE, 1, 40, 0, message);
                        duty = ""; 
                        res = ""; 
                        ledcWrite(ledpin1,0);
                        ledcWrite(ledpin2,0);
                        selectedR = 0;
                        duty_int =0;
                        reso =8; // res=8 for the default value
                        firstRead=0; // to display the guiding message after selecting the leds again.
                        flagDutyRead=3; // to not display the RANGE 0-0 when wanting to change the duty for the first time after reset
                     }
                     break;
   }
}
void DutyValueSet() 
{
  display.clearDisplay();
  sprintf(message, "DUTY RANGE EXCEEDED");
  oledDisplay(WHITE, 1,0,0, message);
  sprintf(message,"SELECT RESOLUTION"); 
  oledDisplay(WHITE, 1, 10, 0, message);
  sprintf(message,"BETWEEN %d AND %d",resolution,maxResolution); 
  oledDisplay(WHITE, 1, 20, 0, message);
  
}
void displayMaxResolutionExceeded()
{
  display.clearDisplay();
  sprintf(message, "  DUTY TOO HIGH"); 
  oledDisplay(WHITE, 1, 0, 0, message);
  sprintf(message, " RESET TO DEFAULT RES"); 
  oledDisplay(WHITE, 1, 10, 0, message);  

}
void displayDutyCycleExceeded()
{
    display.clearDisplay();
    sprintf(message, "   RES TOO LOW "); 
    oledDisplay(WHITE, 1, 10, 0, message);
    sprintf(message, "   DUTY RESET TO 0"); 
    oledDisplay(WHITE, 1, 20, 0, message);
    sprintf(message, "   res set to 8"); 
    oledDisplay(WHITE, 1, 30, 0, message);

}
void writeFirstMessages()
{
    display.clearDisplay();
    sprintf(message," [once you selected]"); 
    oledDisplay(WHITE, 1, 0, 0, message);
    sprintf(message,"  PRESS A TO CHANGE"); 
    oledDisplay(WHITE, 1, 10, 0, message);
    sprintf(message,"--duty cycle"); 
    oledDisplay(WHITE, 1, 20, 0, message);
    sprintf(message,"  PRESS B TO CHANGE"); 
    oledDisplay(WHITE, 1, 30, 0, message);
    sprintf(message,"--the resolution"); 
    oledDisplay(WHITE, 1, 40, 0, message);
}
void displayDefaultRes()
{
    display.clearDisplay();
    sprintf(message, "  NO RES SELECTED"); 
    oledDisplay(WHITE, 1, 10, 0, message);
    sprintf(message, "  USING DEFAULT RES"); 
    oledDisplay(WHITE, 1, 20, 0, message);

}
void UpdatePwmSettings() 
{
    
    sprintf(message,"DUTY CYCLE:%s%%",duty); 
    oledDisplay(WHITE, 1, 0, 0, message);
    sprintf(message,"RESOLUTION:%d",reso);
    oledDisplay(WHITE, 1, 10, 0, message);
    sprintf(message,"if you want to "); 
    oledDisplay(WHITE, 1, 20, 0, message);
    sprintf(message,"change the values "); 
    oledDisplay(WHITE, 1, 30, 0, message);
    sprintf(message,"PRESS A or B "); 
    oledDisplay(WHITE, 1, 40, 0, message);
    sprintf(message, "PRESS * TO RESET"); 
    oledDisplay(WHITE, 1, 50, 0, message);
}
int maxDuty()
{
  int maxValue=1;
  for(int i=1;i<=reso;i++)
      maxValue*=2;
    return maxValue-1;
}
int maxDuty2() // function which was used in the void loop when pressing B to select a resolution
{
  int maxValue=1;
  for(int i=1;i<=resolution;i++)
      maxValue*=2;
    return maxValue-1;
}
int maxReso(void)
{
  int maxDuty= 80000000/frequence;
  int i=0;   
  while(maxDuty>1)
  {
    maxDuty/=2;
    i++;
  }
  return i;
}
