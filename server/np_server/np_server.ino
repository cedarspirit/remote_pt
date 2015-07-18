#include "DHT.h"
#include <AccelStepper.h>
#include <Bounce2.h>
#define DHTPIN 12     // what pin we're connected to
#define DHTTYPE DHT21   // DHT 21 (AM2301)

#define STOP_PIN 2
#define LED_PIN 13

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
String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
long interval = 1000;  
unsigned long previousMillis = 0;
unsigned long currentMillis=0;
unsigned long previousMillisT = 0;
DHT dht(DHTPIN, DHTTYPE);
void setup() {
  // initialize serial:
  stepper1.setMaxSpeed(1000);    // 1000 coz....greater than 1000 is unreliable .....as told on the info page
  stepper1.setAcceleration(1000);
  stepper1.setSpeed(1000);
  stepper1.moveTo(0); // 4096 steps for 1 rotation in 28BYJ-48 – 5V Stepper Motor

  stepper2.setMaxSpeed(1000);    // 1000 coz....greater than 1000 is unreliable .....as told on the info page
  stepper2.setAcceleration(1000);
  stepper2.setSpeed(1000);
  stepper2.moveTo(0); // 4096 steps for 1 rotation in 28BYJ-48 – 5V Stepper Motor  

  previousMillis = millis();
  previousMillisT = millis();
  interval=250;  
  Serial.begin(9600);
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);

  // Setup the button with an internal pull-up :
  pinMode(STOP_PIN,INPUT_PULLUP);

  // After setting up the button, setup the Bounce instance :
  debouncer.attach(STOP_PIN);
  debouncer.interval(5); // interval in ms

  //Setup the LED :
  pinMode(LED_PIN,OUTPUT);


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
            //interval=x;
            stepper1.moveTo(x);

            x = inputString.substring(p1+1,p2).toInt();
            stepper2.moveTo(x);
             
          }
    //Serial.println(inputString);
    // clear the string:
    inputString = "";
    stringComplete = false;

  }
    tick();
//    readtemp();


  // Update the Bounce instance :
  debouncer.update();

  // Get the updated value :
  int value = debouncer.read();

  // Turn on or off the LED as determined by the state :
  if ( value == LOW ) {
    //digitalWrite(LED_PIN, HIGH );
    stepper1.setCurrentPosition(0);
    stepper1.move(50);
    stepper2.setCurrentPosition(0);
    stepper2.move(50);
    readtemp();
   // stepper1.disableOutputs();
   // stepper1.stop();
  } 
  else {
    //digitalWrite(LED_PIN, LOW );
  }



 if (stepper1.distanceToGo() == 0)
 {
   stepper1.disableOutputs();
   readtemp(); // read temp id no motor movement
 }  

if (stepper2.distanceToGo() == 0)
 {
   stepper2.disableOutputs();
 }  


      stepper1.run();        // run the stepper to the destined position
      stepper2.run();        // run the stepper to the destined position
    
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
      {
      ledState = HIGH;
      }
    else
      ledState = LOW;

    // set the LED with the ledState of the variable:
    digitalWrite(ledPin, ledState);
  }
}
  

