/*
This code was written by Frederick Thomas Ford, 2015 
Its purpose is to control the Arduslider (camera slider project)
More information can be found on Instructables.com and Solidgnd.co.uk; my personal site.

Edited by Justin Porteous zs6jgp@gmail.com
 * Added Serial debugging
 * Refactored main loop - TODO: rename modeTrue and modeFalse procedures
 * Implemented serial read to determine if must run
*/

//********************************************************

#define SW1 2
#define SW2 3

volatile byte flag0 = LOW; // declare IRQ flag
volatile byte flag1 = LOW; // declare IRQ flag

int DIR = 8;
int STEP = 4;
int SLP = 12;
int MS3 = 7;
int MS2 = 6;
int MS1 = 5;
int ENABLE = 11;
int SWDIR = 9;
int sensorPin = A0;
int sensorValue = 0;
boolean mode = false;
int state = 1;
int calibration = 0;
int SW1_state = digitalRead(SW1);
int SW2_state = digitalRead(SW2);
int time_del = 0;
volatile boolean mustRun = false;
int builtInLedValue = HIGH;

// *******************************************************
void setup()
{
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  pinMode(A4, OUTPUT);
  pinMode(DIR, OUTPUT);
  pinMode(STEP, OUTPUT);
  pinMode(ENABLE, OUTPUT);
  pinMode(SLP, OUTPUT);
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);
  pinMode(MS3, OUTPUT);
  pinMode(SW1, INPUT);
  pinMode(SW2, INPUT);
  pinMode(SWDIR, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, builtInLedValue);

  if (digitalRead(SWDIR) == HIGH) {
    mode = true;
  }

  if ((SW1_state == HIGH) && (SW2_state == LOW)) {
    state = 0;
  }

  if ((SW1_state == LOW) && (SW2_state == HIGH)) {
    state = 1;
  }

  digitalWrite(SLP, HIGH);
  digitalWrite(ENABLE, LOW);
  digitalWrite(MS1, HIGH);
  digitalWrite(MS2, HIGH);
  digitalWrite(MS3, HIGH);
  attachInterrupt(0, ISR0, FALLING);
  attachInterrupt(1, ISR1, FALLING);

  Serial.begin(115200);
  Serial.println("Starup!");
  delay(100);
}

//********************************************************
void loop()
{
  mustRun = false;
  mustRun = serialSaysRun();

  if (mustRun) {
    //Serial.println("Must Run");
    mustRun == false;
    if (mode == true)
    { // if switch is up
      modeTrue();
    } else { 
      //if switch is down
      modeFalse();
    }
  }
  
}

void changeBuiltinLed() {
  if (builtInLedValue == HIGH)
   {
    builtInLedValue = LOW;
   } else {
    builtInLedValue = HIGH;
   }
   digitalWrite(LED_BUILTIN, builtInLedValue);
}

//********************************************************
void modeTrue() {
  //Serial.println("modeTrue");
  int currentState = state;

  while (currentState == state) {
  switch (state)
    {
      case 0:   // HOME
        sensorValue = analogRead(sensorPin);
        digitalWrite(DIR, HIGH);
        digitalWrite(STEP, HIGH);
        delayMicroseconds((sensorValue * 10) + 5);
        digitalWrite(STEP, LOW);
        delayMicroseconds((sensorValue * 10) + 5);
        break;
      case 1:   // AWAY
        sensorValue = analogRead(sensorPin);
        digitalWrite(DIR, LOW);
        digitalWrite(STEP, HIGH);
        delayMicroseconds((sensorValue * 10) + 5);
        digitalWrite(STEP, LOW);
        delayMicroseconds((sensorValue * 10) + 5);
        break;
    }
  }
    mustRun = false;
}

boolean serialSaysRun() {
  String incoming = "";
  mustRun = false;
  boolean run = false;
  while (Serial.available() > 0) {
    incoming += char(Serial.read());
    delay(20);
    run = true;
  }

  return run;

  if (incoming.length() != 0) {  
    incoming.trim( );
    if (incoming == "run") {
      return true;
    } else {
    return false;
    }
  } else {
    return false;
  }
}

//********************************************************
void modeFalse() {
  calibration = 0;
  noInterrupts();
  SW1_state = digitalRead(SW1);
  if (SW1_state == HIGH)
  {
    while (SW1_state == HIGH)
    {
      digitalWrite(DIR, HIGH); // return home
      digitalWrite(STEP, HIGH);
      delayMicroseconds(80);
      digitalWrite(STEP, LOW);
      delayMicroseconds(80);
      SW1_state = digitalRead(SW1);
    }
  }
  interrupts();
  
  //trigger camera here first
  for (int frame_count = 1; frame_count < 360; frame_count++) 
  { // the slider is split into 360 posistion which equates to 15 seconds of footage played at 24 fps
    for (int steps = 0; steps < 40; steps++)
    {
      if (calibration == 0)
      {
        digitalWrite(DIR, LOW); //move away from home
        digitalWrite(STEP, HIGH);
        delayMicroseconds(80);
        digitalWrite(STEP, LOW);
        delayMicroseconds(80);
      }
      else
      {
        return;
      }
    }
    digitalWrite(A4, HIGH);
    delay(180);
    digitalWrite(A4, LOW);
    time_del = analogRead(sensorPin);
    onesec(time_del);
    //trigger camera for all other shots here, to reduce camera shake
  }
}

void onesec(int val)
{
  for(int cycle = 0 ;cycle <val;cycle++)
  {
  delay(100);
  }
}  

//********************************************************
void ISR0()  //HOME
{
  state = 1;
  calibration = 1;
  mustRun = false;
}

//********************************************************
void ISR1()  //AWAY
{
  state = 0;
  calibration = 1;
}
//********************************************************
