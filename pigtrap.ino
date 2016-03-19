int armed_led = D6;
int triggered_led = D5;

int switch_trigger = D0; // Trap trigger switch connected to D0
int switch_arm = D1; // Trap arming switch connected to D1

int battery_monitor = A0; // Analog input pin with potentiometer to simulate battery level reading

int armed = 1; // variable to store whether the trap is armed

int battery_alert_sent = 0; //variable to store whether a battery level alert has already been sent
int arming_alert_sent = 0; //variable to store whether the arming alert has already been sent
int triggered_alert_sent = 0; //variable to store whether the trap trigger alert has already been sent

int trigger_queue = 0; //variable to store whether we need to send a trap-triggered alert
int arming_queue = 0; //variable to store whether we need to send a trap-triggered alert

int battery_low_limit = 500; // sets low battery alert level
int battery_ok_limit = 2000; // sets battery 'ok' level


void setup() 
{
  pinMode(armed_led, OUTPUT);
  pinMode(triggered_led, OUTPUT);
  pinMode(switch_trigger, INPUT_PULLUP); 
  pinMode(switch_arm, INPUT_PULLUP); 
  pinMode(battery_monitor, INPUT);
  digitalWrite(armed_led, LOW);
  digitalWrite(triggered_led, LOW);
}

void loop() {

System.sleep(20);

check_trap();

send_alerts();

}

void check_trap() {
    
  if ((armed == 0) && (digitalRead(switch_trigger) == HIGH)) {
    armed = 1;
    WiFi.connect();
    Particle.connect();
    delay(10000);
    digitalWrite(armed_led, HIGH);
    digitalWrite(triggered_led, LOW);
    arming_queue = 1;
    
    //bool success;
    //success = Particle.publish("TrapStatus", "armed");
    //if (!success) {
    // get here if event publish did not work
    //}
    //Particle.publish("TrapStatus", "armed");
    }

  if ((armed == 1) && (digitalRead(switch_trigger) == LOW)) {
    armed = 0;
    WiFi.connect();
    Particle.connect();
    delay(10000);
    digitalWrite(armed_led, LOW);
    digitalWrite(triggered_led, HIGH);
    triggered_queue = 1;
    
    //Particle.publish("TrapStatus", "triggered");
    }


  if ((analogRead(battery_monitor) < battery_low_limit) && (battery_alert_sent == 0)) {
    WiFi.connect();
    Particle.connect();
    delay(10000);
    Particle.publish("BatteryLevel", "low");
    battery_alert_sent = 1;
    }

  if ((analogRead(battery_monitor) > battery_ok_limit) && (battery_alert_sent == 1)) {
    WiFi.connect();
    Particle.connect();
    delay(10000);
    Particle.publish("BatteryLevel", "ok");
    battery_alert_sent = 0;
    }  
    
}

void send_alerts() {
    
    if (arming_queue == 1) {
        bool success;
        success = Particle.publish("TrapStatus", "armed");
        if (success) {
            arming_queue = 0;
        }
    }
    
    if (triggered_queue == 1) {
        bool success;
        success = Particle.publish("TrapStatus", "triggered");
        if (success) {
            triggered_queue = 0;
        }
    }
    
}
