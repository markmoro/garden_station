#include <SPI.h>
#include <LiquidCrystal2.h>
#include <Servo.h> 

const int DATA_PIN = 11;
const int LATCH_PIN =13;
const int CLOCK_PIN = 2;

const int VALVE_OPEN = 0;
const int VALVE_CLOSED = 180;
const int WATER_DIV = 10;
const int WATER_FLOW_TIME = 2;
const int WATER_COOL_TIME = -2;

const int SENSOR_OFF = 1024;
const int SLEEP_COUNT = 150;

const int buttonPins[] = {8,7,0};
const int servoPins[] = {9,10,6,5,3};

int buttonState[3];         // current state of the button
int lastButtonState[3];   

int sensor_number = 0;
int editing = 99;

int thresholds[6];
int pins =0;
int alerts[6];

int water_divider;
int sleepCounter;

int servoTime[5];

int setupScreen;
int setupServo;

int  uiMode;
Servo servo;


LiquidCrystal2 lcd(CLOCK_PIN);
 
void setup() {
  
  uiMode =0;
  pinMode(DATA_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(buttonPins[0], INPUT);   
  pinMode(buttonPins[1], INPUT);   
  pinMode(buttonPins[2], INPUT); 

  sleepCounter=SLEEP_COUNT;
  //  Setup servos to off position
  for(int i=0;i<5;i++) {
    servoTime[i]=0; 
    servo.attach(servoPins[i]);
    servo.write(VALVE_CLOSED); 
    servo.detach();
  }
  
  //  Clear button states
  for(int i=0;i<3;i++) {
    buttonState[i]=digitalRead(i);
    lastButtonState[i]=buttonState[i]; 
  }
  
  //  Setup water divider
  water_divider = WATER_DIV;

  // Tresholds
  thresholds[0] = 500;
  thresholds[1] = SENSOR_OFF;
  thresholds[2] = SENSOR_OFF;
  thresholds[3] = SENSOR_OFF;
  thresholds[4] = SENSOR_OFF;
  setupScreen = 0;
  
  setupServo =0;
  //Serial.begin(9600);   
  lcd.begin(20, 2);
  lcd.print("Sensor 0");
}
 
void wake() {
  if(sleepCounter <= 1) {
    lcd.backlightOn();
  }
    sleepCounter=SLEEP_COUNT;

    
}

boolean checkButtonPress(int button_no) {
  boolean ret = false;
  buttonState[button_no] = digitalRead(buttonPins[button_no]);
  // compare the buttonState to its previous state
  if (buttonState[button_no] != lastButtonState[button_no]) {
    // if the state has changed, increment the counter
    if (buttonState[button_no] == HIGH) {
      // if the current state is HIGH then the button
      // wend from off to on:
      wake();
      ret = true;
     } 
  }
  lastButtonState[button_no] = buttonState[button_no];
  return ret;
}


void setupLoop() {
  if( checkButtonPress(2) && setupScreen == 0) {
      uiMode = 0;
      return; 
  }
  if( checkButtonPress(1)) {
    setupServo = VALVE_CLOSED;
    for(int i=0;i<5;i++) { 
      servo.attach(servoPins[i]);
      servo.write(VALVE_CLOSED); 
      delay(400);
      servo.detach();
    }
  }
  if( checkButtonPress(2)) {
    setupServo = VALVE_OPEN;
    for(int i=0;i<5;i++) { 
      servo.attach(servoPins[i]);
      servo.write(VALVE_OPEN);
      delay(400); 
      servo.detach();
    }
  }
  lcd.setCursor(0, 0);
  lcd.print("Valve Setup ");

  lcd.setCursor(0, 1);
  if(setupServo = VALVE_OPEN) {
    lcd.print("Open   ");  
  } else {
    lcd.print("Close   "); 
  }
 
}



void editLoop() {
   if( checkButtonPress(2) ) {
      uiMode = 0;
      return; 
  }
  
  if( checkButtonPress(0) && thresholds[sensor_number] > 0) {
      thresholds[sensor_number]=thresholds[sensor_number]-100;
  }
  
   if( checkButtonPress(1) && thresholds[sensor_number] < SENSOR_OFF) {
      thresholds[sensor_number]=thresholds[sensor_number]+100;
      if(thresholds[sensor_number] > SENSOR_OFF)
        thresholds[sensor_number] = SENSOR_OFF;
  }
  
  
  lcd.setCursor(0, 0);
  lcd.print("E Sensor ");
  lcd.print(sensor_number);
  lcd.print(" - ");
  
  if(thresholds[sensor_number] < SENSOR_OFF) {
    lcd.print(thresholds[sensor_number]);
    lcd.print("   ");  
  } else {
    lcd.print("Off   "); 
  }
  
  lcd.setCursor(0, 1);
  lcd.print("Editing...");
}



void normalLoop() {
  if( checkButtonPress(1) ) {
      sensor_number++;
       if(sensor_number > 4)
         sensor_number= 0;
    
  }
    if( checkButtonPress(0) ) {
      if(sensor_number == 0)
         sensor_number= 5;
      sensor_number--;
  }
  if( checkButtonPress(2) ) {
      uiMode = 1;
      return; 
  }
  
  
  lcd.setCursor(0, 0);
  lcd.print("Sensor ");
  lcd.print(sensor_number);
  lcd.print(" - ");
  
  if(thresholds[sensor_number] < SENSOR_OFF) {
    int moisture_val = analogRead(sensor_number);
    lcd.print(moisture_val);
    lcd.print("   ");  
  } else {
    lcd.print("Off   "); 
  }
  
  lcd.setCursor(0, 1);
  lcd.print("Temp - ");
  if(thresholds[5] <1024 && thresholds[5] >30) {
    int moisture_val = analogRead(5);
    lcd.print(moisture_val);
    lcd.print((char)223);
    lcd.print("C  ");  
  } else {
    lcd.print("Off   "); 
  }
}

void servoUpdate() {
  if(water_divider == 0 ) {
    water_divider = WATER_DIV;
    for(int i=0;i<5;i++)  {
      if(servoTime[i] == 1) {
        servo.attach(servoPins[i]);
        servo.write(VALVE_CLOSED);  
        servo.detach(); 
      }
      if(servoTime[i] > 0) {
        servoTime[i]--;
        if(servoTime[i] == 0) {
          servoTime[i] = WATER_COOL_TIME;
        }
      }
      if(servoTime[i] < 0) {
        servoTime[i]++;
      }
    }
  }
  water_divider--;
}

void sensorUpdate() {
  pins = 0;
  for(int i=0;i<5;i++)  {
    int moisture_val = analogRead(i);  
    if(thresholds[i] < SENSOR_OFF && moisture_val < thresholds[i]) {
       alerts[i]++;
    } else {
      alerts[i] = 0;
    }
    if(alerts[i] >10) {
      alerts[i] =0;
    }
    if(alerts[i]>5) {
       pins= pins | 32;
       if(servoTime[i] == 0) {
         servoTime[i] = WATER_FLOW_TIME;
         servo.attach(servoPins[i]);
         servo.write(VALVE_OPEN);  
         servo.detach();
       }
    }
    pins= pins >> 1;
  }
}


void loop() {
  sensorUpdate();
  servoUpdate();
  if(uiMode ==0)
    normalLoop();
  if(uiMode ==1)
    editLoop();

  if(sleepCounter == 1)
    lcd.backlightOff();
    
  if(sleepCounter > 0)
    sleepCounter--;
 // Serial.println(pins);
  lcd.setSecondPins(pins);
  delay(100);
}
