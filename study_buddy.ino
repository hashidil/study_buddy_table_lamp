/*
 * Name: Hashini Dilhara
 */
#include "Countimer.h" // includes the timer Library 
#include <Keypad.h> // includes the Keypad Library 
#include <LiquidCrystal.h> // includes the LiquidCrystal Library 

LiquidCrystal lcd(14, 15, 16, 17, 18, 20); // Creates an LCD object. Parameters: (rs, enable, d4, d5, d6, d7) 
Countimer timer;
Countimer timer1ForStudyMode;
Countimer timer2ForStudyMode;
#define buzzerPin 21 //change to any output pin (not analog inputs) 

static int motionArray=0;

static int hours=0;
static int minutes=0;

const byte ROWS = 4; 
const byte COLS = 4; 

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'E'},
  {'7', '8', '9', 'M'},
  {'*', '0', '#', 'S'}
};

byte rowPins[ROWS] = {9, 8, 7, 6}; 
byte colPins[COLS] = {5, 4, 3, 2}; 

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 
 
int ldr_pin=0;//analog pin
int pir_pin=10;//pir pin
int ledpin=12;//light pin

static char Incoming_value='0';

int val;//ldr value
int pir_state = LOW;             // by default, no motion detected
int pir_val = 0;                 // variable to store the sensor status (value)

static bool isLedOn=false;
static bool isItAutomatic=false;

static bool isItExamMode=false;
static bool isTimeSet=false;
static bool isHourSet=false;
static bool isMinutesSe=false;

static bool studyMode=false;
static bool timerBreak=false;
static int  studyModeMin;
static int  studyModeHours;
static int  studyModeInterval;

void checkTheAvaliableLight();
void readSerialValues();
void ledOnOffFunction(bool ledOnOffState);
void checkTheMotion();
void refreshClock();
void keypadInputForTimerSet(bool selectVariable);
void readKeypadInputsForModes();
void turnOnOffStudyMode();
void turnOnOffAutomaticLightMode();
void turnOnOffManualLightMode();
void turnOnOffExamMode();
void activateBuzzer();
void timerTerminator();
void alertTheInterval();
void setTheTimerForStudyMode(int  studyModeHours,int studyModeMin,int studyModeSec,int intervalInMin);
void alertTheEndOfTheStudyMode();
void displayTheCurrentTimerDetails();
void StudyModeTimerTerminator();

void StudyModeTimerTerminator(){
         studyMode=false;
         timer1ForStudyMode.stop();
         timer2ForStudyMode.stop();
         Serial.println("Study Mode Off");
         lcd.clear();
         lcd.setCursor(0,0);
         lcd.print("Study Mode Off");
         delay(3000); 
         lcd.clear();  
}

void turnOnOffStudyMode(){
     if(!studyMode){//set the new timer
          //turn on the study mode
           lcd.setCursor(0,0);
           lcd.print("Study Mode On");
           Serial.println("Study Mode On"); 
           delay(4000);
           lcd.clear();
           Serial.println("Set The Hours"); 
           lcd.setCursor(0,0);
           lcd.print("Set The Hours");
           keypadInputForStudyModeTimerSet(1);
           lcd.clear();
           Serial.println("Hours Entered");
           lcd.setCursor(0,0);
           lcd.print("Hours Entered");
           delay(1000);
           lcd.clear();
           Serial.print("Enter Minutes");
           lcd.setCursor(0,0);
           lcd.print("Enter Minutes");
           keypadInputForStudyModeTimerSet(2);
           lcd.clear();
           lcd.print("Minutes Entereds");
           Serial.println("Minutes Entered");
           delay(1000);
           lcd.clear();
           lcd.setCursor(0,0);
           lcd.print("Enter Interval Time(Min)");
           Serial.println("Enter Interval Time(Min)");
           keypadInputForStudyModeTimerSet(3);
           lcd.clear();
           lcd.print("Interval Entereds");
           Serial.println("Interval Entered");
           delay(1000);
           lcd.clear();
           setTheTimerForStudyMode(studyModeHours,studyModeMin,0,studyModeInterval);
           studyMode=true;
      }else if(studyMode){
           StudyModeTimerTerminator();
      }
}

void setTheTimerForStudyMode(int  studyModeHours,int studyModeMin,int studyModeSec,int intervalInMin){
    timer1ForStudyMode.setCounter(studyModeHours, studyModeMin,studyModeSec,timer1ForStudyMode.COUNT_UP, alertTheEndOfTheStudyMode);
    timer2ForStudyMode.setCounter(studyModeHours, studyModeMin,studyModeSec,timer2ForStudyMode.COUNT_UP, alertTheEndOfTheStudyMode);
    //when every defined interval this alertTheInterval start
    timer1ForStudyMode.setInterval(alertTheInterval,intervalInMin*1000*60);
    //display the time in every seconds
    timer2ForStudyMode.setInterval(displayTheCurrentTimerDetails,1000);
    timer1ForStudyMode.start();
    timer2ForStudyMode.start();
}

void displayTheCurrentTimerDetails(){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(timer2ForStudyMode.getCurrentTime());
    Serial.println(timer2ForStudyMode.getCurrentTime());
}

void alertTheInterval(){
     
     //pause the timer2 for interval
     
     timer1ForStudyMode.pause();
     timer2ForStudyMode.pause();

  activateBuzzer();
  activateBuzzer();
  activateBuzzer();
  activateBuzzer();
   
     Serial.println("Interval Started");
     lcd.setCursor(0,1);
     lcd.print("Interval Started");
     delay(3000);
     lcd.clear();
     int interval=5000;
     while(interval!=0){
            lcd.setCursor(0,1);
            lcd.print("Wating");
            Serial.println("Wating");
            delay(1000);
            lcd.clear();
            interval=interval-1000;
     }
     timer1ForStudyMode.start();
     timer2ForStudyMode.start();

}

void alertTheEndOfTheStudyMode(){
  Serial.println("Timer Completed");
  lcd.clear();
  lcd.print("Timer Completed");
  activateBuzzer();
  activateBuzzer();
  activateBuzzer();
  activateBuzzer();
  delay(3000);
  lcd.clear();
}

void timerTerminator(){
         isTimeSet=false;
         isItExamMode=false; 
         timer.stop();
         Serial.println("Exam Mode Off");
         lcd.clear();
         lcd.setCursor(0,0);
         lcd.print("Exam Mode Off");
         delay(3000); 
         lcd.clear();  
         
}

void activateBuzzer(){
  for(int i=0; i<255; i++) { //do this 255 times
    analogWrite(buzzerPin, i); //raise the voltage sent out of the pin by 1
    delay(10); //wait 10 milliseconds to repeat 
  }
  for(int i=125; i>0; i--) { // do this 255 times
    analogWrite(buzzerPin, i); //lower the voltage sent out of the pin by 1
    delay(10); //wait 10 milliseconds to repeat
  }
}

void turnOnOffExamMode(){
      if(!isItExamMode){//set the new timer
          //turn on the exam mode
           lcd.setCursor(0,0);
           lcd.print("Exam Mode On");
           Serial.println("Exam Mode On"); 
           delay(4000);
           lcd.clear();
           lcd.setCursor(0,0); 
           Serial.println("Set The Hours"); 
           lcd.setCursor(0,0);
           lcd.print("Set The Hours");
           keypadInputForTimerSet(true);
           lcd.clear();
           Serial.println("Hours Entered");
           lcd.setCursor(0,0);
           lcd.print("Hours Entered");
           delay(1000);
           lcd.clear();
           Serial.print("Enter Minutes");
           lcd.setCursor(0,0);
           lcd.print("Enter Minutes");
           keypadInputForTimerSet(false);
           lcd.clear();
           lcd.print("Minutes Entereds");
           Serial.println("Minutes Entered");
           delay(1000);
           lcd.clear();
           timer.setCounter(hours,minutes,0, timer.COUNT_DOWN, onComplete);
           isItExamMode=true;
           // Print current time every 1s on serial port by calling method refreshClock().
           timer.setInterval(refreshClock, 1000);
           timer.start();  
      }else if(isItExamMode){
           timerTerminator();
      }

}

void turnOnOffManualLightMode(){
  isItAutomatic=false;
  //turn off automatic led mode
          if(!isLedOn){
              //turn on the led if its not on
              Serial.println("manually light on");
              ledOnOffFunction(true);
              lcd.clear();
              lcd.print("Light ON");
              delay(4000);
              lcd.clear();
          }else{
              //turn off the led if its on
              Serial.println("manually light off");
              ledOnOffFunction(false);
              lcd.clear();
              lcd.print("Light OFF");
              delay(4000);
              lcd.clear();
          }
}

void turnOnOffAutomaticLightMode(){

//automatic light on
if(isItAutomatic==false){
           
            isItAutomatic=true;
          
            Serial.println("automatic light mode actiavted");

            lcd.clear();
            lcd.print("A-light ON");
            delay(4000);
            lcd.clear();
}else{
            //automatic light off

            isItAutomatic=false;
            if(isLedOn){
                ledOnOffFunction(false);
            }
            Serial.println("automatic light mode deactiavted");
            
            lcd.clear();
            lcd.print("A-light OFF");
            delay(4000);
            lcd.clear();
}
        
     
}

void refreshClock() {
    Serial.print("Current count time is: ");
    Serial.println(timer.getCurrentTime());
    lcd.clear();
    lcd.print(timer.getCurrentTime());
}

void keypadInputForTimerSet(bool selectVariable){
  String inputChar;
  int digit=0;
  while(1){
    
    char customKey = customKeypad.getKey();
  
    if (customKey and customKey!='#'){
          inputChar+=customKey;
          Serial.print(customKey);
          if(selectVariable==true){//display the user input for hours
                    lcd.setCursor(digit,1);
                    
          }
          if(selectVariable==false){//display the user input for minutes
                    lcd.setCursor(digit+4,1);
          }
          
          lcd.print(customKey);
          digit=digit+1;
          
    }  

    
    
    if(customKey=='#' || digit==2){
      lcd.clear();
      if(selectVariable==true){
              hours=inputChar.toInt();
              isHourSet=true;
      }

      if(selectVariable==false){
              minutes=inputChar.toInt();
              isMinutesSe=true;
      }
      digit=0;
      break;
      
      
    }
    
  }
}

void keypadInputForStudyModeTimerSet(int selectVariable){
  String inputChar;
  int digit=0;
  while(1){
    
    char customKey = customKeypad.getKey();
  
    if (customKey and customKey!='#'){
          inputChar+=customKey;
          Serial.print(customKey);
          if(selectVariable==1){//display the user input for hours
                    lcd.setCursor(digit,1);
                    
          }
          if(selectVariable==2){//display the user input for minutes
                    lcd.setCursor(digit+3,1);
          }

          if(selectVariable==3){//display the user input for interval time in minutes
                    lcd.setCursor(digit+6,1);
          }
          
          lcd.print(customKey);
          digit=digit+1;
          
    }  

    
    
    if(customKey=='#' || digit==2){
      lcd.clear();
      if(selectVariable==1){
              studyModeHours=inputChar.toInt();
              isHourSet=true;
      }

      if(selectVariable==2){
              studyModeMin=inputChar.toInt();
              isMinutesSe=true;
      }
      if(selectVariable==3){
              studyModeInterval=inputChar.toInt();
              isMinutesSe=true;
      }
      digit=0;
      break;
      
      
    }
    
  }
}

void setup() {
  Serial.begin(9600);//beign the serial communication
  lcd.begin(16,2); // Initializes the interface to the LCD screen, and specifies the dimensions (width and height) of the display 
  //loading screen start
  lcd.setCursor(4,0);
  lcd.print("Study");
  lcd.setCursor(4,1);
  lcd.print("Buddy");
  delay(4000);
  lcd.clear();
 //loading screen end
  pinMode(buzzerPin, OUTPUT); //tell arduino the buzzer is an output device
  pinMode(ledpin,OUTPUT);
  pinMode(pir_pin, INPUT);    // initialize sensor as an input
  delay(1000);
}

//check the ldr light sensitivity
void checkTheAvaliableLight(){
    if(isItAutomatic){
      val=analogRead(ldr_pin);
      if(val>=750){
        ledOnOffFunction(true);
      }else{
        ledOnOffFunction(false);
      }
      delay(1000);
    }
}

//timer on complete
void onComplete() {
  isTimeSet=false;
  activateBuzzer();
  Serial.println("Complete!!!");
  
}

void setTimer(int hours,int minutes){
      if(!isTimeSet){
          timer.setCounter(hours,minutes,0, timer.COUNT_DOWN, onComplete);  
      }else{
          Serial.println("Already Time Set");
      }
    
}

void readKeypadInputsForModes(){
  
  char customKey = customKeypad.getKey();

  if (customKey){
    //turn on or off study mode
    if(customKey=='S'){
      turnOnOffStudyMode();
    }
    //turn on or off exam mode
    if(customKey=='E'){
       turnOnOffExamMode();
    }
    //turn on or off manual mode
    if(customKey=='M'){
        turnOnOffManualLightMode();
    }
    //turn on or off automatic mode
    if(customKey=='A'){
        turnOnOffAutomaticLightMode();
    }

    //turn on or off PIR
    if(customKey=='*'){
        checkTheMotion();
    }
    
  }
  
  
}

void readSerialValues(){
  String userInput;
    
    if(Serial.available()>0){

      Incoming_value = Serial.read();      //Read the incoming data and store it into variable Incoming_value
      
      Serial.println(Incoming_value);

        //automatic light on off via ble or serial comunication
        if(Incoming_value=='A'){
            turnOnOffAutomaticLightMode();
        }
        
        //manual light on off via ble or serial comunication
        if(Incoming_value=='M'){
            turnOnOffManualLightMode();
        }

        if(Incoming_value=='S'){
            turnOnOffStudyMode();                   
        }

        if(Incoming_value=='E'){
            turnOnOffExamMode();  
        }
    
}
}


void ledOnOffFunction(bool ledOnOffState){
    if(ledOnOffState){
        digitalWrite(ledpin,HIGH);
        isLedOn=true;
    }else{
        digitalWrite(ledpin,LOW);
        isLedOn=false;
    }
}

void checkTheMotion(){

       pir_val = digitalRead(pir_pin);   // read sensor value
  motionArray=0;
  if (pir_val == HIGH) {           // check if the sensor is HIGH
    motionArray+=1;
    Serial.println("Motion detected!");
    
    
    delay(1000);                // delay 100 milliseconds 
    
    if (pir_state == LOW) {
      Serial.println("Motion detected!"); 
      motionArray=0;
      pir_state = HIGH;       // update variable state to HIGH
    }
  } 
  else {
      delay(500);             // delay 200 milliseconds 
        Serial.println("Motion stopped!");
      if(motionArray<50){
          
        activateBuzzer();
        motionArray=0;
      }
      
        pir_state = LOW;       // update variable state to LOW
    }

 
}

void loop() {
  readSerialValues();
  readKeypadInputsForModes();
  checkTheAvaliableLight();
  
  // Run timer
  timer.run();
  timer1ForStudyMode.run();
  timer2ForStudyMode.run();
  
}
