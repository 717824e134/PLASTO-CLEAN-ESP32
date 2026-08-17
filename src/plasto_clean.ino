#include <ESP32Servo.h>

// ---------- PIN DEFINITIONS ----------
#define BRUSH_SERVO 12
#define POT_PIN 34
#define RELAY_PIN 26

#define GREEN_LED 25
#define RED_LED 32

#define BUZZER 14
#define BUTTON 27

#define TRIG_PIN 5
#define ECHO_PIN 18

// ---------- SETTINGS ----------
#define OVERLOAD_LIMIT 3500
#define OBSTACLE_DISTANCE 10   // cm

Servo brushServo;


// ---------- ULTRASONIC FUNCTION ----------
long getDistance()
{
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0)
  {
    return 999;   // No object detected
  }

  long distance = duration * 0.034 / 2;

  return distance;
}


// ---------- SETUP ----------
void setup()
{
  Serial.begin(115200);

  // Outputs
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  // Inputs
  pinMode(BUTTON, INPUT_PULLUP);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Servo
  brushServo.attach(BRUSH_SERVO);

  // Safe startup
  brushServo.write(0);

  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);

  noTone(BUZZER);

  Serial.println("================================");
  Serial.println("       PLASTO-CLEAN SYSTEM");
  Serial.println("================================");
  Serial.println("System Ready");
  Serial.println("Motor OFF - Safe Startup");
}


// ---------- MAIN LOOP ----------
void loop()
{
  // ---------- READ SENSORS ----------

  int loadValue = analogRead(POT_PIN);

  long distance = getDistance();

  bool emergency = (digitalRead(BUTTON) == LOW);


  // ---------- DISPLAY READINGS ----------

  Serial.print("Load = ");
  Serial.print(loadValue);

  Serial.print(" | Distance = ");
  Serial.print(distance);
  Serial.println(" cm");


  // ---------- FAULT DETECTION ----------

  bool overload = (loadValue > OVERLOAD_LIMIT);

  bool obstacle = (distance <= OBSTACLE_DISTANCE);

  bool fault = overload || obstacle || emergency;


  // =================================================
  //                  FAULT CONDITION
  // =================================================

  if (fault)
  {
    // STOP BRUSH
    brushServo.write(0);

    // CUT MOTOR POWER
    digitalWrite(RELAY_PIN, LOW);

    // WARNING INDICATORS
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, HIGH);

    // BUZZER ON
    tone(BUZZER, 2000);


    // ---------- FAULT MESSAGE ----------

    if (emergency)
    {
      Serial.println("!!! EMERGENCY STOP !!!");
    }
    else if (obstacle)
    {
      Serial.println("!!! HARD OBJECT DETECTED !!!");
      Serial.println("!!! BRUSH STOPPED !!!");
    }
    else if (overload)
    {
      Serial.println("!!! MOTOR OVERLOAD !!!");
      Serial.println("!!! BRUSH STOPPED !!!");
    }
  }


  // =================================================
  //                  NORMAL CONDITION
  // =================================================

  else
  {
    // RUN BRUSH
    brushServo.write(90);

    // MOTOR POWER ON
    digitalWrite(RELAY_PIN, HIGH);

    // NORMAL INDICATORS
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);

    // BUZZER OFF
    noTone(BUZZER);

    Serial.println("NORMAL - BRUSH RUNNING");
  }


  delay(300);
}
