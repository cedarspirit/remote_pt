int pulsePin = 7;
int beatMode = 0; //0 idle  1:start berat 2:in beat
unsigned long lastHeartbeat = 0;
unsigned long lastUptimeReport = 0;
unsigned long beatStarted=0;

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
        Serial.println("BeatStarted");
        break;
        
      case 2:
        if ( ( millis()-beatStarted < 0) or (millis()-beatStarted) > 300)  // millisRolledOver
          {

            // Return to high-Z
            pinMode(pulsePin, INPUT);
            lastHeartbeat = millis();            
            beatMode= 0;
            Serial.println("BeatEnded");
            break;
          }
    }
      


}

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:

  beatMode = 1;

  
  Serial.begin(9600);  
  Serial.println("Arduino reset");
  
  // Send an initial heartbeat.
  heartbeat();
}

// the loop routine runs over and over again forever:
void loop() {
  // Check for serial inputs.  If found, send heartbeat.
  if (Serial.available()) {
    // Clear input buffer
    while (Serial.available()) {
      Serial.read();
    }
    beatMode = 1; //trigger a beat
    
  }
  unsigned long uptime = millis();
  if ((uptime - lastUptimeReport) >= 5000) {
    // It has been at least 5 seconds since our last uptime report.  
    Serial.println("Uptime: " + String((uptime - (uptime % 5000)) / 1000) + " seconds (" + String((uptime - lastHeartbeat) / 1000) + " seconds since last heartbeat)");
    // Pretend we did it exactly on the 5 second mark so we don't start slipping.
    lastUptimeReport = (uptime - (uptime % 5000));
  }
  // delay in between loops
  heartbeat();
  delay(100);
}
