#include "DHT.h"
#include <AccelStepper.h>
#include <Bounce2.h>
#define DHTPIN 12     // what pin we're connected to
#define DHTTYPE DHT21   // DHT 21 (AM2301)

#define STOP_PIN 2
#define LED_PIN 13

#define PAN_MIN 100
#define PAN_MAX 4000
#define PAN_DEFAULT 2000

#define TILT_MIN 99
#define TILT_MAX 3999
#define TILT_DEFAULT 1500


// WORKING SAMPLE FOR TJ
Bounce debouncer = Bounce(); 
#define HALFSTEP 8       // To know about HalfStep Stepper Motor plzz check the above mentioned link
#define motorPin1  8     // IN1 on the ULN2003 driver 1
#define motorPin2  9     // IN2 on the ULN2003 driver 1
#define motorPin3  10    // IN3 on the ULN2003 driver 1
#define motorPin4  11    // IN4 on the ULN2003 driver 1
AccelStepper stepper1(HALFSTEP, motorPin1, motorPin3, motorPin2, motorPin4);

#define motor2Pin1  3     // IN1 on the ULN2003 driver 1
#define motor2Pin2  4     // IN2 on the ULN2003 driver 1
#define motor2Pin3  5    // IN3 on the ULN2003 driver 1
#define motor2Pin4  6    // IN4 on the ULN2003 driver 1
AccelStepper stepper2(HALFSTEP, motor2Pin1, motor2Pin3, motor2Pin2, motor2Pin4);
const int ledPin =  13;      // the number of the LED pin
int ledState = LOW;    

int opMode = 0;
int stopMode = 0;

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
long interval = 1000;  
unsigned long previousMillis = 0;
unsigned long currentMillis=0;
unsigned long previousMillisT = 0;

/* HEART BEAT */
int pulsePin = 7;
int beatMode = 0; //0 idle  1:start berat 2:in beat
unsigned long lastHeartbeat = 0;
unsigned long lastUptimeReport = 0;
unsigned long beatStarted=0;
/* HEART BEAT */

int targetX=PAN_DEFAULT;
int targetY=TILT_DEFAULT;



DHT dht(DHTPIN, DHTTYPE);

//---------------------------------------------------------------------------
void setup() {
  opMode = 0;  // 0:Setup 1:Active 2:Calobration

  
  stepper1.setMaxSpeed(1000);    // 1000 coz....greater than 1000 is unreliable .....as told on the info page
  stepper1.setAcceleration(1000);
  stepper1.setSpeed(1000);
  goPan(PAN_DEFAULT);
  

  stepper2.setMaxSpeed(1000);    // 1000 coz....greater than 1000 is unreliable .....as told on the info page
  stepper2.setAcceleration(1000);
  stepper2.setSpeed(1000);
  goTilt(TILT_DEFAULT); // 4096 steps for 1 rotation in 28BYJ-48 – 5V Stepper Motor  

  previousMillis = millis();
  previousMillisT = millis();
  interval=250;  

  // reserve 200 bytes for the inputString:
  inputString.reserve(600);

  // Setup the button with an internal pull-up :
  pinMode(STOP_PIN,INPUT_PULLUP);

  // After setting up the button, setup the Bounce instance :
  debouncer.attach(STOP_PIN);
  debouncer.interval(5); // interval in ms

  //Setup the LED :
  pinMode(LED_PIN,OUTPUT);

  Serial.begin(9600);
  beatMode = 1;
  heartbeat();
  
}

//---------------------------------------------------------------------------
void goPan(int pos)
  {
    int y;
    y=pos;

    if (opMode != 2)
      {
        if (y<PAN_MIN) {y=PAN_MIN;}
        if (y>PAN_MAX) {y=PAN_MAX;}
      }
    stepper1.moveTo(y); // 4096 steps for 1 rotation in 28BYJ-48 – 5V Stepper Motor
    targetX = y;
  }

//---------------------------------------------------------------------------
void goTilt(int pos)
  {
    int y;
    y=pos;
    if (opMode != 2)
      {
        if (y<TILT_MIN) {y=TILT_MIN;}
        if (y>TILT_MAX) {y=TILT_MAX;}      
      }
    stepper2.moveTo(y); // 4096 steps for 1 rotation in 28BYJ-48 – 5V Stepper Motor
    targetY = y;
  }


//---------------------------------------------------------------------------
void triggerBeat()
{
  if (beatMode ==0 )
  {
    beatMode = 1;
    heartbeat();
  }
}

//---------------------------------------------------------------------------
void heartbeat() {
  // Sink current to drain charge from watchdog circuit
 
  switch (beatMode)
    
    {
      case 0:
        return;
        break;

      case 1:
        pinMode(pulsePin, OUTPUT);
        digitalWrite(pulsePin, LOW);      
        beatStarted = millis();
        beatMode=2;
       // Serial.println("BeatStarted");
        break;
        
      case 2:
        if ( ( millis()-beatStarted < 0) or (millis()-beatStarted) > 300)  // millisRolledOver
          {

            // Return to high-Z
            pinMode(pulsePin, INPUT);
            lastHeartbeat = millis();            
            beatMode= 0;
          //  Serial.println("BeatEnded");
            break;
          }
    }
      


}

// ------------------------------------------------------------------
void loop() {
  
  currentMillis = millis();
  tickSer();
  tick();
  heartbeat();
  StopHandler();
  



     if (stepper1.distanceToGo() == 0)
     {
       stepper1.disableOutputs();
     }  
    
    if (stepper2.distanceToGo() == 0)
     {
       stepper2.disableOutputs();
     }  

     if ((stepper2.distanceToGo() == 0) && (stepper1.distanceToGo() == 0) )
     {
        if (opMode == 3)
          {
            sendStatus();
            opMode = 1;
          }
        else if (opMode == 4)
          {
            sendStatus();
            opMode = 1;
          }
        else if (opMode == 1)
          {
              readtemp();
          }
     }
     

      stepper1.run();        // run the stepper to the destined position
      stepper2.run();        // run the stepper to the destined position
    
}

//---------------------------------------------------------------------------------
void StopHandler()
{
  // Update the Bounce instance :
  debouncer.update();
  // Get the updated value :
  int value = debouncer.read();

  // Turn on or off the LED as determined by the state :
  if ( value == LOW ) 
    {
      if (stopMode == 0)
        {
          stopMode = 1;
          stepper1.setCurrentPosition(0);
          stepper2.setCurrentPosition(0);    
          if (opMode == 2)
            {
              goTilt(TILT_DEFAULT);
              goPan(PAN_DEFAULT);               
              opMode = 3;
            }   
          else if (opMode ==1)
            {
              stepper1.move(PAN_MIN);
              stepper2.move(TILT_MIN);  
              opMode = 4;      
            }
        }
     else if (stopMode == 1)
        {
          
        }
    }
  else
    {
      if (stopMode == 1)
        {
          stopMode = 0;
        }
    }
    
  
}

//---------------------------------------------------------------------------------
void tickSer()
{
    if(Serial.available() > 0)  //because serialEvent does not workk with Leonardo
    {
      while (Serial.available()) {
        // get the new byte:
        char inChar = (char)Serial.read();
        // add it to the inputString:
        inputString += inChar;
        // if the incoming character is a newline, set a flag
        // so the main loop can do something about it:
        if (inChar == '\n') {
           processIncomingMsg();
        }
      }      
    }
}

void processIncomingMsg()  // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
{
       if (inputString.startsWith("<A1_"))      
          {
            if (opMode == 0) 
              {
                  opMode=1;  
              }
              
            triggerBeat();
            int p1 = inputString.indexOf('_', 4);
            int p2 = inputString.indexOf('_', p1+1);
            int x = inputString.substring(4,p1).toInt();
            goPan(x);

            x = inputString.substring(p1+1,p2).toInt();          
            goTilt(x);
            
          }
      else if (inputString.startsWith("<HS_"))  //hello message 
          {
            if (opMode == 0) 
              {
                  opMode=1;  
              }
            triggerBeat();
            Serial.println("<HX_1234_XH>");  
          }
      else if (inputString.startsWith("<ZZ_"))  //calibrate mode
          {
           
            triggerBeat();
            calibrate(); 
          }
      else if (inputString.startsWith("<ST_"))  //request status
            {
                sendStatus();  //reply with <SZ_
            }
    
    inputString = "";  // clear the string:
}
//---------------------------------------------------------------------------
void sendStatus() {
  // as a response to hello message = <HS_
  if ( (stepper1.distanceToGo() == 0) && (stepper2.distanceToGo() == 0))
      {
          Serial.print("<SZ_");
          Serial.print(stepper1.currentPosition());
          Serial.print("_");
          Serial.print(stepper2.currentPosition());
          Serial.print("_");
          Serial.print(PAN_MIN);
          Serial.print("_");
          Serial.print(PAN_MAX);
          Serial.print("_");
          Serial.print(TILT_MIN);
          Serial.print("_");
          Serial.print(TILT_MAX);
          Serial.println("_ZS>");
      }
   else
       {
        Serial.println("<HX_1234_XH>");  
       } 
  
}

//---------------------------------------------------------------------------
void readtemp()
{

  if (opMode != 1)
    return;
  
  if(currentMillis - previousMillisT > 2000) {
    // save the last time you blinked the LED
    previousMillisT = currentMillis;  
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    //Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print("<T1_");
  Serial.print(t);
  Serial.print("_");
  Serial.print(h);
  Serial.println("_T1>");


  }
}



//---------------------------------------------------------------------------
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}


//---------------------------------------------------------------------------
int tick()
{
 
  if(currentMillis - previousMillis > interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;  

    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW)
      {
      ledState = HIGH;
      }
    else
      ledState = LOW;

    // set the LED with the ledState of the variable:
    digitalWrite(ledPin, ledState);
  }
}

void calibrate() {
 opMode=2;  //calibrate mode
 goPan(-1000);  
 goTilt(-1000);
 
}
