#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

volatile uint8_t  g_status  = 0;
volatile uint16_t g_speed10 = 0;

#define SPEED_LIMIT_10  50
#define MIN_TIME_MS     1
#define USE_SENSOR      0  

static void asm_setup() {
  asm volatile (
    "in   r24, 0x0A   \n\t"
    "andi r24, 0xF3   \n\t"
    "out  0x0A, r24   \n\t"
    "in   r24, 0x0B   \n\t"
    "ori  r24, 0x0C   \n\t"
    "out  0x0B, r24   \n\t"
    "in   r24, 0x04   \n\t"
    "ori  r24, 0x0F   \n\t"
    "out  0x04, r24   \n\t"
    "in   r24, 0x05   \n\t"
    "andi r24, 0xF0   \n\t"
    "out  0x05, r24   \n\t"
    ::: "r24"
  );
}

static void asm_yellow_on() {
  asm volatile (
    "in   r24, 0x05 \n\t"
    "andi r24, 0xF0 \n\t"
    "ori  r24, 0x04 \n\t"
    "out  0x05, r24 \n\t"
    ::: "r24"
  );
}

static void asm_green_on() {
  asm volatile (
    "in   r24, 0x05 \n\t"
    "andi r24, 0xF0 \n\t"
    "ori  r24, 0x02 \n\t"
    "out  0x05, r24 \n\t"
    ::: "r24"
  );
}

static void asm_red_on() {
  asm volatile (
    "in   r24, 0x05 \n\t"
    "andi r24, 0xF0 \n\t"
    "ori  r24, 0x08 \n\t"
    "out  0x05, r24 \n\t"
    ::: "r24"
  );
}

static void asm_buzzer_on() {
  asm volatile (
    "in   r24, 0x05 \n\t"
    "ori  r24, 0x01 \n\t"
    "out  0x05, r24 \n\t"
    ::: "r24"
  );
}

static void asm_buzzer_off() {
  asm volatile (
    "in   r24, 0x05 \n\t"
    "andi r24, 0xFE \n\t"
    "out  0x05, r24 \n\t"
    ::: "r24"
  );
}

static uint16_t asm_div16(uint16_t dividend, uint16_t divisor) {
  if (divisor == 0) return 0;
  return dividend / divisor;
}

static void process_time(uint32_t timeTaken) {
  if (timeTaken < MIN_TIME_MS || timeTaken > 65535UL) {
    g_status  = 0;
    g_speed10 = 0;
    return;
  }
  uint16_t speed10 = asm_div16(7200, (uint16_t)timeTaken);
  g_speed10 = speed10;
  if (speed10 >= SPEED_LIMIT_10) {
    g_status = 2;
    asm_red_on();
    asm_buzzer_on();
  } else {
    g_status = 1;
    asm_green_on();
    asm_buzzer_off();
  }
}

#if USE_SENSOR == 1
static void detect_sensor_mode() {
  while (digitalRead(3) == LOW);
  while (digitalRead(2) == HIGH);
  uint32_t t1 = millis();
  while (digitalRead(3) == HIGH);
  uint32_t t2 = millis();
  process_time(t2 - t1);
}
#endif

static void detect_serial_mode() {
  String inputStr = "";

  unsigned long t = millis();
  while (millis() - t < 1000) {
    if (Serial.available()) {
      char c = Serial.read();
      if (c == '\n' || c == '\r') break;
      if (c >= '0' && c <= '9') inputStr += c;
      t = millis();
    }
  }

  while (Serial.available()) Serial.read();

  if (inputStr.length() == 0) {
    g_status = 0;
    return;
  }

  long input = inputStr.toInt();
  if (input <= 0) {
    g_status = 0;
    return;
  }

  Serial.print("Time input: ");
  Serial.print(input);
  Serial.println(" ms");

  process_time((uint32_t)input);
}

void setup() {
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();

  asm_setup();
  asm_yellow_on();

  lcd.setCursor(0, 0);
  lcd.print("ETLE Detector");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");

  delay(1000);
  while (Serial.available()) Serial.read();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Waiting...");

  Serial.println("=== ETLE Speed Detector ===");
  Serial.println("Type time (ms) + Enter");
  Serial.println("Speed limit: 5.0 km/h");
  Serial.println("===========================");
}

void loop() {
  asm_yellow_on();

  lcd.setCursor(0, 0);
  lcd.print("Waiting...      ");
  lcd.setCursor(0, 1);
  lcd.print("                ");

  g_status  = 0;
  g_speed10 = 0;

  while (true) {
    if (Serial.available()) {
      detect_serial_mode();
      if (g_status != 0) break;
      while (Serial.available()) Serial.read();
    }
#if USE_SENSOR == 1
    else if (digitalRead(2) == LOW) {
      detect_sensor_mode();
      if (g_status != 0) break;
    }
#endif
  }

  uint16_t speed10  = g_speed10;
  uint8_t  status   = g_status;
  uint16_t intPart  = speed10 / 10;
  uint16_t fracPart = speed10 % 10;

  lcd.clear();

  if (status == 2) {
    lcd.setCursor(0, 0);
    lcd.print("OVER SPEED!");
    lcd.setCursor(0, 1);
    lcd.print(intPart);
    lcd.print(".");
    lcd.print(fracPart);
    lcd.print(" km/h");

    Serial.print("OVER SPEED: ");
    Serial.print(intPart);
    Serial.print(".");
    Serial.print(fracPart);
    Serial.println(" km/h");

    while (Serial.available()) Serial.read();
    delay(3000);
    asm_buzzer_off();

  } else {
    lcd.setCursor(0, 0);
    lcd.print("SAFE SPEED");
    lcd.setCursor(0, 1);
    lcd.print(intPart);
    lcd.print(".");
    lcd.print(fracPart);
    lcd.print(" km/h");

    Serial.print("SAFE SPEED: ");
    Serial.print(intPart);
    Serial.print(".");
    Serial.print(fracPart);
    Serial.println(" km/h");

    while (Serial.available()) Serial.read();
    delay(3000);
  }

  asm_yellow_on();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Waiting...");

  while (Serial.available()) Serial.read();
}