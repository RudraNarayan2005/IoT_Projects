#define BLYNK_TEMPLATE_ID "TMPL32zg9_mo3"
#define BLYNK_TEMPLATE_NAME "gas sensor"
#define BLYNK_AUTH_TOKEN "PVZnIrd5D1hriT-a_brCb-oyGSWK1lRH"

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>

// --- Pin Definitions ---
#define MQ135_PIN 32      // MQ135 analog output to GPIO32
#define BUZZER_PIN 2      // Buzzer to GPIO2
#define RED_LED_PIN 22    // Red LED to GPIO22
#define SERVO_PIN 5       // Servo signal to GPIO5
#define SDA_PIN 25        // I2C SDA
#define SCL_PIN 33        // I2C SCL

// --- Objects ---
LiquidCrystal_I2C lcd(0x27, 16, 2);
BlynkTimer timer;
Servo servoMotor;
WidgetLED led(V2);  // Virtual LED on Blynk

// --- WiFi and Blynk Credentials ---
char auth[] = "PVZnIrd5D1hriT-a_brCb-oyGSWK1lRH";
char ssid[] = "Merlin";
char pass[] = "244466666";

// --- Blynk Button for Servo Control ---
BLYNK_WRITE(V1) {
  int switchState = param.asInt();
  if (switchState == 1) {
    servoMotor.write(180);  // Changed from 90 to 180 degrees
    Serial.println("Servo moved to 180 degrees");
  } else {
    servoMotor.write(0);   // Closed
    Serial.println("Servo moved to 0 degrees");
  }
}

void setup() {
  Serial.begin(115200);

  // Initialize I2C and LCD
  Wire.begin(SDA_PIN, SCL_PIN);
  lcd.begin(16, 2);
  lcd.backlight();

  // Pin modes
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);

  // Attach servo
  servoMotor.attach(SERVO_PIN);

  // Connect to Blynk
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);

  // LCD boot animation
  lcd.setCursor(1, 0);
  lcd.print("System Loading");
  for (int i = 0; i <= 15; i++) {
    lcd.setCursor(i, 1);
    lcd.print(".");
    delay(100);
  }
  lcd.clear();

  // Set up timer to check gas level every second
  timer.setInterval(1000L, checkGasLevel);
}

void checkGasLevel() {
  int raw = analogRead(MQ135_PIN);
  int gasLevel = map(raw, 0, 4095, 0, 100);

  // Display gas level on LCD
  lcd.setCursor(0, 0);
  lcd.print("GAS Level : ");
  lcd.print(gasLevel);
  lcd.print("   ");  // To clear previous digits

  // Condition check
  static bool alertSent = false;

  if (gasLevel >= 50) {
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(RED_LED_PIN, HIGH);
    led.on();
    lcd.setCursor(0, 1);
    lcd.print("Warning! High Gas");

    if (!alertSent) {
      Blynk.logEvent("gas_alert", String("High gas level detected: ") + gasLevel + "%");
      alertSent = true;
    }
  } else {
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(RED_LED_PIN, LOW);
    led.off();
    lcd.setCursor(0, 1);
    lcd.print("Gas Level Normal ");

    alertSent = false;
  }

  // Send gas level to Blynk
  Blynk.virtualWrite(V0, gasLevel);
  Serial.println("Gas Level: " + String(gasLevel));
}

void loop() {
  Blynk.run();
  timer.run();
}
