/*
  AnalogReadSerial
  Reads an analog input on pin 0, prints the result to the serial monitor.
  Attach the center pin of a potentiometer to pin A0, and the outside pins to +5V and ground.
http://www.forward.com.au/pfod/ArduinoProgramming/DebouncedSwitch/DebouncedSwitch.html
 This example code is in the public domain.
 */
//#include <DebouncedSwitch.h>

//int BTN_CALIBRATE = 7; 
//DebouncedSwitch swCalibrate(BTN_CALIBRATE); // monitor a switch on input D4
 
const int ledPin =  13;      // the number of the LED pin
int outPin = 13;       // the number of the output pin
int LEDstate = HIGH;      // the current state of the output pin
int pin_test = 9;

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
boolean cfgRcvd = false;
 
int inPin = 6; 
int reading;           // the current reading from the input pin
int previous = LOW;    // the previous reading from the input pin
int switchprevious = LOW;


volatile boolean state = LOW;
  
// the follow variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long time = 0;         // the last time the output pin was toggled
long debounce = 50;   // the debounce time, increase if the output flickers

#define encoder0PinA  2
#define encoder0PinB  4

#define encoder1PinA  3
#define encoder1PinB  5

// *** CALIBRATE RELATED
int BTN_CALIBRATE = 7; 
boolean  btnClb_StateCur = HIGH;             // the current reading from the input pin
boolean  btnClb_StateLast = HIGH;   // the previous reading from the input pin
boolean  btnClb_StateDebounce = HIGH;   // the previous reading from the input pin
unsigned long btnClb_lastDebounceTime = 0;  // the last time the output pin was toggled   // the following variables are long's because the time, measured in miliseconds,
unsigned long btnClb_debounceDelay = 50;    // the debounce time; increase if the output flickers    // will quickly become a bigger number than can be stored in an int.
// *** CALIBRATE RELATED



volatile int encoder0Pos = 0;
volatile int encoder1Pos = 0;

volatile int minPan = 0;
volatile int maxPan = 0;
volatile int minTilt = 0;
volatile int maxTilt = 0;


#define STEP_SML 5
#define STEP_MED 20
#define STEP_LRG 200

volatile int xFactor = STEP_MED;


int lastState_Calibrate = HIGH; 

// Variables will change:
int ledState = LOW;    
long interval = 1000;  
///int LastVal;
volatile int LastValX;
volatile int LastValY;
///int sensorValueX;
///int sensorValueY;
unsigned long previousMillis = 0;
unsigned long currentMillis=0;
int aprintf(char *, ...);
char *j = "jik";

unsigned long previousMillisSer = 0;
unsigned long currentMillisSer=0;
long intervalSer = 3000;  // 3 sec to say hello
// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
      cfgRcvd = false;
      pinMode(inPin, INPUT);
      digitalWrite(inPin, HIGH);   // turn on the built in pull-up resistor

      pinMode(BTN_CALIBRATE, INPUT);
      digitalWrite(BTN_CALIBRATE, HIGH);   // turn on the built in pull-up resistor
    
      lastState_Calibrate = HIGH;
    
      pinMode(pin_test, OUTPUT);
      
      pinMode(outPin, OUTPUT);

      xFactor = STEP_MED;

      
      pinMode(encoder0PinA, INPUT);
      digitalWrite(encoder0PinA, HIGH);       // turn on pullup resistor
      pinMode(encoder0PinB, INPUT);
      digitalWrite(encoder0PinB, HIGH);       // turn on pullup resistor

      pinMode(encoder1PinA, INPUT);
      digitalWrite(encoder1PinA, HIGH);       // turn on pullup resistor
      pinMode(encoder1PinB, INPUT);
      digitalWrite(encoder1PinB, HIGH);       // turn on pullup resistor      
      
      attachInterrupt(0, doEncoder, CHANGE);  // encoder pin on interrupt 0 - pin 2
      attachInterrupt(1, doEncoder2, CHANGE);  // encoder pin on interrupt 0 - pin 3
 
      previousMillis = millis();
      previousMillisSer = millis();  
      interval=250;
      inputString = "";
       
      stringComplete = false;  
      Serial.begin(9600);
}

// the loop routine runs over and over again forever:
//http://arduino.stackexchange.com/questions/176/how-do-i-print-multiple-variables-in-a-string
void loop() {
  // read the input on analog pin 0:
///////sw.update(); // call this every loop to update switch state

   currentMillis = millis();
   currentMillisSer = millis();
  /// sensorValueX = analogRead(A0);
 ///  sensorValueY = analogRead(A1);
  
  tick();
  tickSer();
  tickCalibrateCheck2();
  detectMultiplier();
  
  
  boolean potChange = false;
  if (abs(encoder0Pos-LastValX) > 0)
    {
      LastValX = encoder0Pos;
      potChange = true;
    }
  if (abs(encoder1Pos-LastValY) > 0)
    {
      LastValY = encoder1Pos;
      potChange = true;
    }
    
  if (potChange)
    {
      aprintf("<X1_%d_%d_>", LastValX  , LastValY );
      Serial.println();
    }
    
    
  
    if (stringComplete) {
       if (inputString.startsWith("<Y"))
          {
              if (cfgRcvd == false)
                {
                  Serial.println(String("<ZZ_>")) ;  // <ZZ_>
                }
              else
                {
                  char c = inputString.charAt(2);
                  switch (c) 
                  {
                     case '1':
                          interval=1000;
                          break;
                     default:
                          interval=250;     
                          break;
                  }
                  
                }
          }
         else if (inputString.startsWith("<Z5_"))  
          {
            int p1 = inputString.indexOf('_', 4);
            int p2 = inputString.indexOf('_', p1+1);
            encoder0Pos = inputString.substring(4,p1).toInt();
            encoder1Pos = inputString.substring(p1+1,p2).toInt();    

            p1 = inputString.indexOf('_', p2+1);
            minPan = inputString.substring(p2+1,p1).toInt(); // minx
            
            p2 = inputString.indexOf('_', p1+1);
            maxPan = inputString.substring(p1+1,p2).toInt(); // maxx


            p1 = inputString.indexOf('_', p2+1);
            minTilt = inputString.substring(p2+1,p1).toInt(); // miny
            
            p2 = inputString.indexOf('_', p1+1);
            maxTilt = inputString.substring(p1+1,p2).toInt(); // maxy

            cfgRcvd = true;
                                
          }
         else if (inputString.startsWith("<Z3_"))  
          {
            int p1 = inputString.indexOf('_', 4);
            int p2 = inputString.indexOf('_', p1+1);
            encoder0Pos = inputString.substring(4,p1).toInt();
            encoder1Pos = inputString.substring(p1+1,p2).toInt();    
            cfgRcvd = true;

          }
         
        //Serial.println(inputString);
        // process input string here
    // clear the string:
    
    inputString = "";
    stringComplete = false;
    }
  delay(1);        // delay in between reads for stability
}

/*
  SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
void serialEvent() {
  
  previousMillisSer = millis(); 
  if (stringComplete)
    {
        // don't read until the main loop process the previous data
        
        return;
    }
  while (Serial.available()) {
     
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      //Serial.print(inputString);
      stringComplete = true;
    }
  }
}

int tick()
{


 
  if(currentMillis - previousMillis > interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;  

    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW)
      ledState = HIGH;
    else
      ledState = LOW;

    // set the LED with the ledState of the variable:
    digitalWrite(ledPin, ledState);
  }
}
  
int tickSer()
{

  if(currentMillisSer - previousMillisSer > intervalSer) {
    // save the last time you blinked the LED
    previousMillisSer = currentMillisSer;  
    //Serial.println(String("XAB") + String("sd")); 
    Serial.println(String("<X0>")) ;
  }
}

int aprintf(char *str, ...) {
	int i, j, count = 0;
 
	va_list argv;
	va_start(argv, str);
	for(i = 0, j = 0; str[i] != '\0'; i++) {
		if (str[i] == '%') {
			count++;
 
			Serial.write(reinterpret_cast<const uint8_t*>(str+j), i-j);
 
			switch (str[++i]) {
				case 'd': Serial.print(va_arg(argv, int));
					break;
				case 'l': Serial.print(va_arg(argv, long));
					break;
				case 'f': Serial.print(va_arg(argv, double));
					break;
				case 'c': Serial.print((char) va_arg(argv, int));
					break;
				case 's': Serial.print(va_arg(argv, char *));
					break;
				case '%': Serial.print("%");
					break;
				default:;
			};
 
			j = i+1;
		}
	};
	va_end(argv);
 
	if(i > j) {
		Serial.write(reinterpret_cast<const uint8_t*>(str+j), i-j);
	}
        
	return count;
}

    void doEncoder(){
      if (digitalRead(encoder0PinA) == HIGH) {   // found a low-to-high on channel A
        if (digitalRead(encoder0PinB) == LOW) {  // check channel B to see which way
                                                 // encoder is turning
          encoder0Pos = encoder0Pos - xFactor;         // CCW
        }
        else {
          encoder0Pos = encoder0Pos + xFactor;         // CW
        }
      }
      else                                        // found a high-to-low on channel A
      {
        if (digitalRead(encoder0PinB) == LOW) {   // check channel B to see which way
                                                  // encoder is turning 
          encoder0Pos = encoder0Pos + xFactor;          // CW
          //delay(10);
        }
        else {
          encoder0Pos = encoder0Pos - xFactor;          // CCW
          //delay(10);
        }

      }
    
      if (encoder0Pos < minPan) {encoder0Pos = minPan;} 
      if (encoder0Pos > maxPan) {encoder0Pos = maxPan;} 
      
     // Serial.println (encoder0Pos, DEC);          // debug - remember to comment out
                                                  // before final program run
      // you don't want serial slowing down your program if not needed
    }

    void doEncoder2(){
      blink();  //debug
      if (digitalRead(encoder1PinA) == HIGH) {   // found a low-to-high on channel A
        if (digitalRead(encoder1PinB) == LOW) {  // check channel B to see which way
                                                 // encoder is turning
          encoder1Pos = encoder1Pos - xFactor;         // CCW
        }
        else {
          encoder1Pos = encoder1Pos + xFactor;         // CW
        }
      }
      else                                        // found a high-to-low on channel A
      {
        if (digitalRead(encoder1PinB) == LOW) {   // check channel B to see which way
                                                  // encoder is turning 
          encoder1Pos = encoder1Pos + xFactor;          // CW
          //delay(10);
        }
        else {
          encoder1Pos = encoder1Pos - xFactor;          // CCW
          //delay(10);
        }

      }

    
      if (encoder1Pos < minTilt) {encoder1Pos = minTilt;} 
      if (encoder1Pos > maxTilt) {encoder1Pos = maxTilt;} 
      
      
    //  Serial.println (encoder1Pos, DEC);          // debug - remember to comment out
                                                  // before final program run
      // you don't want serial slowing down your program if not needed
    }

void detectMultiplier() {
  /* http://streylab.com/blog/2012/9/21/arduino-debounced-toggle.html */
 
  
 int switchstate;
 
  reading = digitalRead(inPin);
  
  // If the switch changed, due to bounce or pressing...
  if (reading != previous) {
    // reset the debouncing timer
    time = millis();
  } 
  
  if ((millis() - time) > debounce) {
     // whatever the switch is at, its been there for a long time
     // so lets settle on it!
     switchstate = reading;
  
     
    if (switchstate == HIGH && switchprevious == LOW) {
       switchprevious = HIGH;
       toggleFactor();       
       }
    if (switchstate == LOW && switchprevious == HIGH) {
       switchprevious = LOW;
       }
 
  }

  
 /// digitalWrite(outPin, LEDstate);
  
  // Save the last reading so we keep a running tally
  previous = reading;
  
}

void toggleFactor()
{
    if (xFactor == STEP_MED)
      {
        xFactor = STEP_LRG;
      }
    else  if (xFactor == STEP_LRG)
      {
        xFactor = STEP_SML;
      }
    else 
      {
        xFactor = STEP_MED;
      }

    
}

void blink()
{
  if (state ==LOW)
  { 
    state=HIGH;
  } 
  else
  {
    state=LOW;
  } 

}



void tickCalibrateCheck2()
{
  
  btnClb_StateCur = digitalRead(BTN_CALIBRATE);
  unsigned long currentTime = millis();

  if (btnClb_StateCur != btnClb_StateLast){
    btnClb_lastDebounceTime = currentTime;
  }
  
  if (currentTime - btnClb_lastDebounceTime > btnClb_debounceDelay){//if enough time has passed
    if (btnClb_StateCur != btnClb_StateDebounce){//if the current state is still different than our last stored debounced state
      btnClb_StateDebounce = btnClb_StateCur;//update the debounced state
      
      //trigger an event
      if (btnClb_StateDebounce == HIGH){
        //Serial.println("released");
      } else {
        //Serial.println("pressed");
        Serial.println(String("<ZZ>"));
      }
    }
  }
  
  btnClb_StateLast = btnClb_StateCur;
}



