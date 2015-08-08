/*
  AnalogReadSerial
  Reads an analog input on pin 0, prints the result to the serial monitor.
  Attach the center pin of a potentiometer to pin A0, and the outside pins to +5V and ground.
http://www.forward.com.au/pfod/ArduinoProgramming/DebouncedSwitch/DebouncedSwitch.html
 This example code is in the public domain.
 */

#include <DebouncedSwitch.h>
int D4 = 4; // give the pin a name

DebouncedSwitch sw(D4); // monitor a switch on input D4
 
const int ledPin =  13;      // the number of the LED pin
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

// Variables will change:
int ledState = LOW;    
long interval = 1000;  
int LastVal;
int LastValX;
int LastValY;
int sensorValueX;
int sensorValueY;
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
sw.update(); // call this every loop to update switch state

   currentMillis = millis();
   currentMillisSer = millis();
   sensorValueX = analogRead(A0);
   sensorValueY = analogRead(A1);
  
  tick();
  tickSer();

  boolean potChange = false;
  if (abs(sensorValueX-LastValX) > 3)
    {
      LastValX = sensorValueX;
      potChange = true;
    }
  if (abs(sensorValueY-LastValY) > 3)
    {
      LastValY = sensorValueY;
      potChange = true;
    }
    
  if (potChange)
    {
      aprintf("<X1_%d_%d_>", LastValX, LastValY);
      Serial.println();
    }
    
    
  
    if (stringComplete) {
       if (inputString.startsWith("<Y"))
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

