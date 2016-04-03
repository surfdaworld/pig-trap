// This #include statement was automatically added by the Spark IDE.
#include "TinyGPS.h"
//===========================================================
int armed_led = D6;
int triggered_led = D5;

int switch_trigger = D0; // Trap trigger switch connected to D0
int switch_arm = D1; // Trap arming switch connected to D1

int battery_monitor = A0; // Analog input pin with potentiometer to simulate battery level reading

int armed = 1; // variable to store whether the trap is armed

int battery_alert_sent = 0; //variable to store whether a battery level alert has already been sent
int arming_alert_sent = 0; //variable to store whether the arming alert has already been sent
int triggered_alert_sent = 0; //variable to store whether the trap trigger alert has already been sent

int triggered_queue = 0; //variable to queue up trap-triggered alert
int arming_queue = 0; //variable to queue up trap-triggered alert
int low_bat_queue = 0; //variable to queue up low battery alert
int ok_bat_queue = 0; // variable to queue up ok bat alert
int ready_to_sleep = 0; // are we ready to go to sleep?

int battery_low_limit = 500; // sets low battery alert level
int battery_ok_limit = 2000; // sets battery 'ok' level

String LastLocation;
//http://www.google.com/search?q=
//============================================================

TinyGPS gps;
char szInfo[64];
// Every 10 seconds
int sleep = 20000;

void setup(){
    Serial1.begin(9600);
    //Serial.begin(9600);

    pinMode(armed_led, OUTPUT);
    pinMode(triggered_led, OUTPUT);
    pinMode(switch_trigger, INPUT_PULLUP);
    pinMode(switch_arm, INPUT_PULLUP);
    pinMode(battery_monitor, INPUT);
    digitalWrite(armed_led, LOW);
    digitalWrite(triggered_led, LOW);
    //Particle.variable("LastLocation", LastLocation);
}

void loop(){
    bool isValidGPS = false;

    for (unsigned long start = millis(); millis() - start < 1000;){
        // Check GPS data is available
        while (Serial1.available()){
            char c = Serial1.read();

            // parse GPS data
            if (gps.encode(c))
                isValidGPS = true;
        }
    }

    // If we have a valid GPS location then publish it
    if (isValidGPS){
        float lat, lon;
        unsigned long age;

        gps.f_get_position(&lat, &lon, &age);

        sprintf(szInfo, "%.6f,%.6f", (lat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : lat), (lon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : lon));
    }
    else{
        sprintf(szInfo, "0.0,0.0");
    }

    String LocationTemp(szInfo);
    //Spark.publish("gpsloc", szInfo);
    if (LocationTemp != "0.0,0.0") {
        String str = "http://www.google.com/search?q=";
        LastLocation = str + LocationTemp;
    }

    //Particle.publish("gpsloc", LastLocation);
    //Spark.publish("gpsloc", szInfo);
    // Sleep for some time
    //delay(sleep);
    //=============================================================
    check_trap();
    send_alerts();
    //=============================================================
}

//=================================================================

void check_trap() {

  if ((armed == 0) && (digitalRead(switch_trigger) == HIGH)) {
    armed = 1;
    digitalWrite(armed_led, HIGH);
    digitalWrite(triggered_led, LOW);
    arming_queue = 1;
    ready_to_sleep = 0;
    }

  if ((armed == 1) && (digitalRead(switch_trigger) == LOW)) {
    armed = 0;
    digitalWrite(armed_led, LOW);
    digitalWrite(triggered_led, HIGH);
    triggered_queue = 1;
    ready_to_sleep = 0;
    }

  if ((analogRead(battery_monitor) < battery_low_limit) && (battery_alert_sent == 0)) {
    low_bat_queue = 1;
    battery_alert_sent = 1;
    ready_to_sleep = 0;
    }

  if ((analogRead(battery_monitor) > battery_ok_limit) && (battery_alert_sent == 1)) {
    ok_bat_queue = 1;
    battery_alert_sent = 0;
    ready_to_sleep = 0;
    }

}

void send_alerts() {

//    if ((arming_queue == 1) || (triggered_queue == 1)) {
//        if(Particle.connected() == FALSE) {
//            Particle.connect();
//        }
//    }

//    if (Particle.connected() == TRUE) {
//        delay(4000);
//    }

//    if ((arming_queue == 1) && (Particle.connected() == TRUE)) {
    if (arming_queue == 1) {
        bool success;
        success = Particle.publish("TrapArmed", LastLocation);
        //bool success = TRUE;
        //String str1 = "TrapArmed, " + LastLocation;
        //Serial.println(str1);
        if (success) {
            arming_queue = 0;
            ready_to_sleep = 1;
            delay(2000);
        }
    }

//    if ((triggered_queue == 1) && (Particle.connected() == TRUE)) {
    if (triggered_queue == 1) {
        bool success;
        success = Particle.publish("TrapTriggered", LastLocation);
        //bool success = TRUE;
        //String str1 = "TrapTriggered, " + LastLocation;
        //Serial.println(str1);
        if (success) {
            triggered_queue = 0;
            ready_to_sleep = 1;
            delay(2000);
        }
    }

//    if ((low_bat_queue == 1) && (Particle.connected() == TRUE)) {
    if (low_bat_queue == 1) {
        bool success;
        success = Particle.publish("BatteryLow", LastLocation);
        //bool success = TRUE;
        //String str1 = "BatteryLow, " + LastLocation;
        //Serial.println(str1);
        if (success) {
            low_bat_queue = 0;
            ready_to_sleep = 1;
            delay(2000);
        }
    }

//    if ((ok_bat_queue == 1) && (Particle.connected() == TRUE)) {
    if (ok_bat_queue == 1) {
        bool success;
        success = Particle.publish("BatteryOk", LastLocation);
        //bool success = TRUE;
        //String str1 = "BatteryOk, " + LastLocation;
        //Serial.println(str1);
        if (success) {
            ok_bat_queue = 0;
            ready_to_sleep = 1;
            delay(2000);
        }
    }

}
