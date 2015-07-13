#include "DHT.h"
#define DHTPIN 2     // what pin we're connected to
#define DHTTYPE DHT21   // DHT 21 (AM2301)




const int ledPin =  13;      // the number of the LED pin
int ledState = LOW;    
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
long interval = 1000;  
unsigned long previousMillis = 0;
unsigned long currentMillis=0;
unsigned long previousMillisT = 0;
DHT dht(DHTPIN, DHTTYPE);
void setup() {
  // initialize serial:
  previousMillis = millis();
  previousMillisT = millis();
  interval=250;  
  Serial.begin(9600);
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);
}

void loop() {
  
  currentMillis = millis();
  serialEvent(); //call the function
  // print the string when a newline arrives:
  if (stringComplete) {
       if (inputString.startsWith("<A1_"))
          {
            int p1 = inputString.indexOf('_', 4);
            int p2 = inputString.indexOf('_', p1+1);
            int x = inputString.substring(4,p1).toInt();
            interval=x;
            Serial.println(String(x));
          }
    //Serial.println(inputString);
    // clear the string:
    inputString = "";
    stringComplete = false;

  }
    tick();
    readtemp();
}


void readtemp()
{
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
Serial.println("_>");

/*
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");
  Serial.print("Heat index: ");
  Serial.print(hic);
  Serial.print(" *C ");
  Serial.print(hif);
  Serial.println(" *F");
*/
  }
}


/*
  SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
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
  

