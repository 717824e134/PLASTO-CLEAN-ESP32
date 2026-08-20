#include <ESP32Servo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// =====================================================
// PLASTO-CLEAN
// ESP32 SAFETY CONTROL SYSTEM
// =====================================================

// ---------------- PIN DEFINITIONS ----------------

#define BRUSH_SERVO 12

// Motor load simulation
#define LOAD_PIN 34

// Relay controlling motor power
#define RELAY_PIN 26

// Status LEDs
#define GREEN_LED 25
#define RED_LED 32

// Fault buzzer
#define BUZZER 14

// Emergency stop
#define EMERGENCY_STOP 27

// Reset button
#define RESET_BUTTON 33

// Ultrasonic sensor
#define TRIG_PIN 5
#define ECHO_PIN 18


// ---------------- SAFETY THRESHOLDS ----------------

#define OVERLOAD_LIMIT 3500
#define OBSTACLE_DISTANCE 10   // cm

//-------------------OLED----------------

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// ---------------- SERVO ----------------

Servo brushServo;


// ---------------- FAULT MEMORY ----------------

// Once a fault occurs, the system remains OFF
// until the operator presses RESET.
bool faultLatched = false;


// =====================================================
// HC-SR04 DISTANCE MEASUREMENT
// =====================================================

long getDistance()
{
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  // No echo received
  if (duration == 0)
  {
    return 999;
  }

  long distance = duration * 0.034 / 2;

  return distance;
}


// =====================================================
// STOP BRUSH / MOTOR
// =====================================================

void stopBrush()
{
  // Stop brush
  brushServo.write(0);

  // Cut motor power
  digitalWrite(RELAY_PIN, LOW);

  // Fault indication
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, HIGH);

  // Audible alarm
  tone(BUZZER, 2000);
}


// =====================================================
// RUN BRUSH / MOTOR
// =====================================================

void runBrush()
{
  // Run brush
  brushServo.write(90);

  // Allow motor power
  digitalWrite(RELAY_PIN, HIGH);

  // Normal indication
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, LOW);

  // Stop buzzer
  noTone(BUZZER);
}


// =====================================================
// SETUP
// =====================================================

void showOLED(const char* line1, const char* line2)
{
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 15);
  display.println(line1);

  display.setCursor(0, 35);
  display.println(line2);

  display.display();
}

void setup()
{
  Serial.begin(115200);
  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println("OLED initialization failed!");
    while (1);
  }

showOLED("PLASTO-CLEAN", "INITIALIZING...");
  // Outputs
  pinMode(RELAY_PIN, OUTPUT);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  pinMode(BUZZER, OUTPUT);

  // Buttons
  pinMode(EMERGENCY_STOP, INPUT_PULLUP);
  pinMode(RESET_BUTTON, INPUT_PULLUP);

  // Ultrasonic
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Servo
  brushServo.attach(BRUSH_SERVO);

  // -------------------------------------------------
  // SAFE STARTUP
  // -------------------------------------------------

  brushServo.write(0);

  digitalWrite(RELAY_PIN, LOW);

  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);

  noTone(BUZZER);
  showOLED("SYSTEM READY", "Motor: OFF");
  // -------------------------------------------------
  // SERIAL MESSAGE
  // -------------------------------------------------

  Serial.println();
  Serial.println("========================================");
  Serial.println("          PLASTO-CLEAN");
  Serial.println("      SAFETY CONTROL SYSTEM");
  Serial.println("========================================");

  Serial.println("System Ready");
  Serial.println("Motor Load Simulation: GPIO 34");
  Serial.println("Obstacle Sensor: HC-SR04");
  Serial.println("Emergency Stop: GPIO 27");
  Serial.println("Reset Button: GPIO 33");
  Serial.println();
}


// =====================================================
// MAIN LOOP
// =====================================================

void loop()
{
  // -------------------------------------------------
  // READ SENSORS
  // -------------------------------------------------

  int motorLoad = analogRead(LOAD_PIN);

  long distance = getDistance();

  bool emergencyPressed =
      (digitalRead(EMERGENCY_STOP) == LOW);

  bool resetPressed =
      (digitalRead(RESET_BUTTON) == LOW);

  bool overload =
      (motorLoad > OVERLOAD_LIMIT);

  bool obstacle =
      (distance <= OBSTACLE_DISTANCE);


  // =================================================
  // 1. DETECT NEW FAULT
  // =================================================

  if (!faultLatched)
  {
    // -----------------------------------------------
    // EMERGENCY STOP
    // -----------------------------------------------

    if (emergencyPressed)
    {
      faultLatched = true;

      showOLED("EMERGENCY STOP", "Motor: OFF");

      Serial.println();
      Serial.println("!!! EMERGENCY STOP !!!");
      Serial.println("FAULT LATCHED");
    }

    // -----------------------------------------------
    // MOTOR OVERLOAD
    // -----------------------------------------------

    else if (overload)
    {
      faultLatched = true;

      showOLED("OVERLOAD FAULT", "Motor: OFF");

      Serial.println();
      Serial.println("!!! MOTOR OVERLOAD !!!");
      Serial.println("FAULT LATCHED");
    }

    // -----------------------------------------------
    // OBSTACLE DETECTED
    // -----------------------------------------------

    else if (obstacle)
    {
      faultLatched = true;

      showOLED("OBSTACLE DETECTED", "Motor: OFF");

      Serial.println();
      Serial.println("!!! HARD OBJECT / OBSTACLE !!!");
      Serial.println("FAULT LATCHED");
    }
  }


  // =================================================
  // 2. FAULT LATCHED
  // =================================================

  if (faultLatched)
  {
    // Keep motor OFF continuously
    stopBrush();

    Serial.print("LOAD = ");
    Serial.print(motorLoad);

    Serial.print(" | DISTANCE = ");
    Serial.print(distance);

    Serial.println(" cm | MOTOR OFF");


    // -----------------------------------------------
    // RESET BUTTON
    // -----------------------------------------------

    if (resetPressed)
    {
      Serial.println();
      Serial.println("RESET PRESSED");
      Serial.println("Checking safety conditions...");

      // Small debounce delay
      delay(300);


      // ---------------------------------------------
      // READ SAFETY CONDITIONS AGAIN
      // ---------------------------------------------

      motorLoad = analogRead(LOAD_PIN);

      distance = getDistance();

      emergencyPressed =
          (digitalRead(EMERGENCY_STOP) == LOW);

      overload =
          (motorLoad > OVERLOAD_LIMIT);

      obstacle =
          (distance <= OBSTACLE_DISTANCE);


      // ---------------------------------------------
      // ALL CONDITIONS SAFE?
      // ---------------------------------------------

      if (!emergencyPressed &&
          !overload &&
          !obstacle)
      {
        // Clear fault
        faultLatched = false;

        showOLED("SYSTEM READY", "Motor: ON");

        // Turn off fault indicators
        digitalWrite(RED_LED, LOW);

        noTone(BUZZER);

        Serial.println("SAFETY CONDITIONS OK");
        Serial.println("FAULT CLEARED");
        Serial.println("MOTOR READY");
        Serial.println();
      }

      // ---------------------------------------------
      // SAFETY CONDITION STILL ACTIVE
      // ---------------------------------------------

      else
      {
        Serial.println("RESET REJECTED");
        Serial.println("SAFETY CONDITION STILL ACTIVE");

        if (emergencyPressed)
        {
          Serial.println("Reason: EMERGENCY STOP ACTIVE");
        }

        if (overload)
        {
          Serial.println("Reason: MOTOR OVERLOAD");
        }

        if (obstacle)
        {
          Serial.println("Reason: OBSTACLE DETECTED");
        }

        Serial.println();
      }
    }
  }


  // =================================================
  // 3. NORMAL OPERATION
  // =================================================

  else
  {
    runBrush();

    showOLED("SYSTEM READY", "Motor: ON");

    Serial.print("LOAD = ");
    Serial.print(motorLoad);

    Serial.print(" | DISTANCE = ");
    Serial.print(distance);

    Serial.println(" cm | NORMAL | MOTOR ON");
  }


  // Small loop delay
  delay(500);
}
