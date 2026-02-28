#define BLYNK_TEMPLATE_ID "TMPL32zg9_mo3"
#define BLYNK_TEMPLATE_NAME "gas sensor"
#define BLYNK_AUTH_TOKEN "YOUR_NEW_AUTH_TOKEN"

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>

// ---------------- PIN DEFINITIONS ----------------
#define MQ135_PIN     32
#define BUZZER_PIN    2
#define RED_LED_PIN   22
#define SERVO_PIN     5
#define SDA_PIN       25
#define SCL_PIN       33

// ---------------- OBJECTS ----------------
LiquidCrystal_I2C lcd(0x27, 16, 2);
BlynkTimer timer;
Servo servoMotor;
WidgetLED led(V2);

// ---------------- WIFI CREDENTIALS ----------------
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "YOUR_WIFI_NAME";
char pass[] = "YOUR_WIFI_PASSWORD";

// ---------------- GAS THRESHOLD ----------------
#define GAS_THRESHOLD 50

// ---------------- SERVO CONTROL ----------------
BLYNK_WRITE(V1) {
  int switchState = param.asInt();

  if (switchState == 1) {
    servoMotor.write(180);
    Serial.println("Servo Opened (180°)");
  } else {
    servoMotor.write(0);
    Serial.println("Servo Closed (0°)");
  }
}

void setup() {
  Serial.begin(115200);

  // Initialize I2C
  Wire.begin(SDA_PIN, SCL_PIN);

  lcd.init();
  lcd.backlight();

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);

  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);

  servoMotor.attach(SERVO_PIN);
  servoMotor.write(0);  // Start closed

  Blynk.begin(auth, ssid, pass);

  lcd.setCursor(0, 0);
  lcd.print("Gas Monitor");
  lcd.setCursor(0, 1);
  lcd.print("Connecting...");
  delay(2000);
  lcd.clear();

  timer.setInterval(1000L, checkGasLevel);
}

void checkGasLevel() {

  int rawValue = analogRead(MQ135_PIN);

  // Smooth reading
  int gasLevel = map(rawValue, 0, 4095, 0, 100);

  lcd.setCursor(0, 0);
  lcd.print("Gas: ");
  lcd.print(gasLevel);
  lcd.print("%    ");

  static bool alertSent = false;

  if (gasLevel >= GAS_THRESHOLD) {

    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(RED_LED_PIN, HIGH);
    led.on();

    lcd.setCursor(0, 1);
    lcd.print("!! HIGH GAS !! ");

    if (!alertSent) {
      Blynk.logEvent("gas_alert", "High Gas Detected!");
      alertSent = true;
    }

  } else {

    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(RED_LED_PIN, LOW);
    led.off();

    lcd.setCursor(0, 1);
    lcd.print("Gas Normal     ");

    alertSent = false;
  }

  Blynk.virtualWrite(V0, gasLevel);
  Serial.println("Gas Level: " + String(gasLevel));
}

void loop() {
  Blynk.run();
  timer.run();
}
