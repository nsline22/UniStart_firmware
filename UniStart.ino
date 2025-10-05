#include <BluetoothSerial.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <RTClib.h>
#include <microDS18B20.h>
#include <SPI.h>
#include <MFRC522.h>
#include <EEPROM.h>

MicroDS18B20<18> sensor;
MicroDS18B20<19> sensor1;
BluetoothSerial SerialBT;
RTC_DS3231 rtc;

#define RELAY1_PIN 4
#define RELAY2_PIN 5
#define LED_PIN 15 // Лед лента
#define analog_pin 34 // Вольтметер
#define BUZZ_PIN 17
#define RST_PIN 26 // rc522 spi
#define SS_PIN 25 // rc522 spi

// OLED Display Settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
MFRC522 rfid(SS_PIN, RST_PIN);

// Настройки EEPROM
#define EEPROM_SIZE 512
#define CARD_START_ADDR 0
#define MAX_ALLOWED_CARDS 10
#define CARD_SIZE 4

byte allowedUIDs[MAX_ALLOWED_CARDS][CARD_SIZE];
int numAllowedCards = 0;

// Иконки и картинки
static const unsigned char PROGMEM bt_icon[] = {
  0x00, 0x00, 0x01, 0x80, 0x01, 0xe0, 0x19, 0xf0, 0x1d, 0xf8, 0x0f, 0xb8, 0x07, 0xf0, 0x03, 0xe0, 
  0x01, 0xe0, 0x07, 0xf0, 0x0f, 0xf8, 0x1d, 0xb8, 0x19, 0xf0, 0x01, 0xe0, 0x01, 0xc0, 0x00, 0x00
};
static const unsigned char PROGMEM ignition_icon[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x7f, 0x80, 0xf3, 0x80, 0xff, 0xff, 0xff, 0xff, 
  0xff, 0xf7, 0xff, 0xf7, 0xf3, 0xbe, 0x7f, 0xbe, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
static const unsigned char PROGMEM starter_icon[] = {
  0x07, 0xf0, 0x07, 0xf0, 0x06, 0x30, 0x0c, 0x70, 0x0c, 0x60, 0x0c, 0xe0, 0x18, 0xf8, 0x1e, 0x38, 
  0x1e, 0x70, 0x06, 0xe0, 0x07, 0xc0, 0x0f, 0x80, 0x0f, 0x00, 0x0e, 0x00, 0x0c, 0x00, 0x08, 0x00
};
static const unsigned char PROGMEM no_icon[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
static const unsigned char PROGMEM led_icon[] = {
  0x07, 0xe0, 0x0e, 0x70, 0x0c, 0x30, 0x08, 0x10, 0x08, 0x10, 0x08, 0x10, 0x3f, 0xfc, 0x3f, 0xfc, 
  0x3f, 0xfc, 0x3f, 0xfc, 0x06, 0x60, 0x06, 0x60, 0x06, 0x60, 0x06, 0x60, 0x06, 0x60, 0x06, 0x60
};
static const unsigned char PROGMEM engine_icon[] = {
  0x00, 0x00, 0x01, 0xf0, 0x00, 0x40, 0x00, 0x00, 0x0f, 0xf8, 0x1e, 0x1a, 0x3c, 0x8a, 0xa1, 0x8a, 
  0xa1, 0xca, 0xa1, 0xca, 0xa0, 0xca, 0x3c, 0x98, 0x1e, 0x38, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x00
};
static const unsigned char PROGMEM sun_icon[] = {
  0x00, 0x00, 0x01, 0x80, 0x01, 0x80, 0x19, 0x98, 0x1d, 0xb8, 0x0f, 0xf0, 0x06, 0x60, 0x7c, 0x3e, 
  0x7c, 0x3e, 0x06, 0x60, 0x0f, 0xf0, 0x1d, 0xb8, 0x19, 0x98, 0x01, 0x80, 0x01, 0x80, 0x00, 0x00
};
static const unsigned char degreeSymbol[] = {
  0b00111000,
  0b01000100,
  0b01000100,
  0b00111000,
  0b00000000,
  0b00000000,
  0b00000000,
  0b00000000
};
static const unsigned char PROGMEM start_logo [] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 
	0x0f, 0x80, 0xe0, 0x00, 0x0f, 0xc0, 0xf0, 0x00, 0x0f, 0xc0, 0xf0, 0x00, 0x1f, 0xc0, 0xf0, 0x80, 
	0x1f, 0x81, 0xf1, 0xc0, 0x1f, 0x81, 0xf3, 0xe0, 0x1f, 0x83, 0xf1, 0xe0, 0x1f, 0x07, 0xf1, 0xe0, 
	0x1f, 0x07, 0xe0, 0x00, 0x1f, 0x0f, 0xff, 0xe0, 0x1f, 0xbf, 0xff, 0xf0, 0x1f, 0xff, 0xff, 0xf8, 
	0x1f, 0xff, 0xff, 0xfc, 0x0f, 0xff, 0xff, 0xfc, 0x07, 0xfd, 0xfe, 0x7c, 0x03, 0xf1, 0xff, 0x7c, 
	0x00, 0x00, 0xff, 0xbc, 0x00, 0x00, 0x7f, 0xc0, 0x00, 0x00, 0x3f, 0xe0, 0x00, 0x38, 0x0f, 0xf0, 
	0x00, 0x3c, 0x03, 0xf0, 0x00, 0x3f, 0xff, 0xf0, 0x00, 0x3f, 0xff, 0xf0, 0x00, 0x3f, 0xff, 0xf0, 
	0x00, 0x1f, 0xff, 0xe0, 0x00, 0x07, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
static const unsigned char PROGMEM nfc_logo [] = {
  0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0x80, 0x0f, 0xff, 0xff, 0xf0, 0x1f, 0xff, 0xff, 0xf8, 
	0x3c, 0x00, 0x00, 0x3c, 0x38, 0xe0, 0x00, 0x1c, 0x30, 0xf0, 0x03, 0x0c, 0x70, 0xf8, 0x03, 0x0e, 
	0x70, 0xfc, 0x03, 0x0e, 0x70, 0xde, 0x03, 0x0e, 0x70, 0xcf, 0x03, 0x0e, 0x70, 0xc7, 0x83, 0x0e, 
	0x70, 0xc3, 0xc3, 0x0e, 0x70, 0xc1, 0xe3, 0x0e, 0x70, 0xd8, 0xf3, 0x0e, 0x70, 0xdc, 0x7b, 0x0e, 
	0x70, 0xde, 0x3b, 0x0e, 0x70, 0xcf, 0x1b, 0x0e, 0x70, 0xc7, 0x83, 0x0e, 0x70, 0xc3, 0xc3, 0x0e, 
	0x70, 0xc1, 0xe3, 0x0e, 0x70, 0xc0, 0xf3, 0x0e, 0x70, 0xc0, 0x7b, 0x0e, 0x70, 0xc0, 0x3f, 0x0e, 
	0x70, 0xc0, 0x1f, 0x0e, 0x30, 0xc0, 0x0f, 0x0c, 0x38, 0x00, 0x07, 0x1c, 0x3c, 0x00, 0x00, 0x3c, 
	0x1f, 0xff, 0xff, 0xf8, 0x0f, 0xff, 0xff, 0xf0, 0x01, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00
};
static const unsigned char PROGMEM invalid_logo [] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x80, 0x01, 0xe0, 
	0x0f, 0xc0, 0x03, 0xf0, 0x1f, 0xe0, 0x07, 0xf8, 0x1c, 0xf0, 0x0f, 0x38, 0x1c, 0x78, 0x1e, 0x38, 
	0x1e, 0x3c, 0x3c, 0x78, 0x0f, 0x1e, 0x78, 0xf0, 0x07, 0x8f, 0xf1, 0xe0, 0x03, 0xc7, 0xe3, 0xc0, 
	0x01, 0xe3, 0xc7, 0x80, 0x00, 0xf0, 0x0f, 0x00, 0x00, 0x78, 0x1e, 0x00, 0x00, 0x38, 0x1c, 0x00, 
	0x00, 0x38, 0x1c, 0x00, 0x00, 0x78, 0x1e, 0x00, 0x00, 0xf0, 0x0f, 0x00, 0x01, 0xe3, 0xc7, 0x80, 
	0x03, 0xc7, 0xe3, 0xc0, 0x07, 0x8f, 0xf1, 0xe0, 0x0f, 0x1e, 0x78, 0xf0, 0x1e, 0x3c, 0x3c, 0x78, 
	0x1c, 0x78, 0x1e, 0x38, 0x1c, 0xf0, 0x0f, 0x38, 0x1f, 0xe0, 0x07, 0xf8, 0x0f, 0xc0, 0x03, 0xf0, 
	0x07, 0x80, 0x01, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

bool bt_connected = false;
bool previous_bt_connected = false;
bool unlocked_by_bt = false; // Флаг разблокировки по Bluetooth
bool adding_new_card = false; // Флаг режима добавления новой карты
unsigned long last_bt_check = 0;
char inputBuffer[40];
byte inputPos = 0;

const char* daysOfWeek[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

float adc_voltage = 0.0;
float in_voltage = 0.0;
float R1 = 30000.0;
float R2 = 7500.0;
float ref_voltage = 3.3;
int adc_value = 0;

float eng_temp = 0;
float sun_temp = 0;

unsigned long starterStartTime = 0;
bool starterActive = false;
int buzzCount = 0;
unsigned long lastBuzzTime = 0;

void buzz() {
  digitalWrite(BUZZ_PIN, HIGH);
  delay(50);
  digitalWrite(BUZZ_PIN, LOW);
}

// Функции для работы с EEPROM
void saveCardsToEEPROM() {
  int addr = CARD_START_ADDR;
  EEPROM.write(addr++, numAllowedCards);
  for (int i = 0; i < numAllowedCards; i++) {
    for (int j = 0; j < CARD_SIZE; j++) {
      EEPROM.write(addr++, allowedUIDs[i][j]);
    }
  }
  EEPROM.commit();
}

void loadCardsFromEEPROM() {
  int addr = CARD_START_ADDR;
  numAllowedCards = EEPROM.read(addr++);
  if (numAllowedCards > MAX_ALLOWED_CARDS) numAllowedCards = 0;
  
  for (int i = 0; i < numAllowedCards; i++) {
    for (int j = 0; j < CARD_SIZE; j++) {
      allowedUIDs[i][j] = EEPROM.read(addr++);
    }
  }
  
  // Если нет карт, добавляем дефолтную
  if (numAllowedCards == 0) {
    byte defaultCard[] = {0x9E, 0x93, 0xB3, 0x2};
    addCard(defaultCard);
  }
}

void addCard(byte* uid) {
  if (numAllowedCards >= MAX_ALLOWED_CARDS) return;
  
  for (int i = 0; i < CARD_SIZE; i++) {
    allowedUIDs[numAllowedCards][i] = uid[i];
  }
  numAllowedCards++;
  saveCardsToEEPROM();
}

void removeCard(int index) {
  if (index < 0 || index >= numAllowedCards) return;
  
  for (int i = index; i < numAllowedCards - 1; i++) {
    for (int j = 0; j < CARD_SIZE; j++) {
      allowedUIDs[i][j] = allowedUIDs[i+1][j];
    }
  }
  numAllowedCards--;
  saveCardsToEEPROM();
}

void listCards() {
  SerialBT.print("Stored cards: ");
  SerialBT.println(numAllowedCards);
  for (int i = 0; i < numAllowedCards; i++) {
    SerialBT.print("Card ");
    SerialBT.print(i);
    SerialBT.print(": ");
    for (int j = 0; j < CARD_SIZE; j++) {
      SerialBT.print(allowedUIDs[i][j] < 0x10 ? "0" : "");
      SerialBT.print(allowedUIDs[i][j], HEX);
      if (j < CARD_SIZE - 1) SerialBT.print(":");
    }
    SerialBT.println();
  }
}

void setup() {
  Serial.begin(115200);
  SerialBT.begin("UniStart");

  // Инициализация EEPROM
  EEPROM.begin(EEPROM_SIZE);
  loadCardsFromEEPROM();

  Wire.begin();
  rtc.begin();
  SPI.begin(14, 12, 13); // SCK, MISO, MOSI
  rfid.PCD_Init();
  
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("OLED display not found");
    while (1);
  }

  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZ_PIN, OUTPUT);
  digitalWrite(RELAY1_PIN, HIGH); // Инверт
  digitalWrite(RELAY2_PIN, HIGH); // Инверт
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZ_PIN, LOW);
  
  display.clearDisplay();
  display.drawBitmap(48, 30, nfc_logo, 32, 32, SSD1306_WHITE);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 19);
  display.println("Waiting for card...");
  display.display();
  buzz();

  while (!unlocked_by_bt) {
    // Проверяем команды Bluetooth
    while (SerialBT.available()) {
      char c = SerialBT.read();
      if (c == '\n' || c == '\r') {
        inputBuffer[inputPos] = '\0';
        if (inputBuffer[0] == '9' && inputBuffer[1] == '3' && inputBuffer[2] == '7' && inputBuffer[3] == '4') {
          unlocked_by_bt = true;
          buzz();
          display.clearDisplay();
          display.drawBitmap(48, 14, start_logo, 32, 32, SSD1306_WHITE);
          display.setTextSize(1);
          display.setTextColor(SSD1306_WHITE);
          display.setCursor(40, 51);
          display.println("UniStart");
          display.display();
          digitalWrite(RELAY1_PIN, LOW);
          delay(2000);
          break;
        }
        inputPos = 0;
      } else if (inputPos < sizeof(inputBuffer) - 1) {
        inputBuffer[inputPos++] = c;
      }
    }
    
    if (unlocked_by_bt) break;

    // Проверяем карту RFID
    if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
      delay(100);
      continue;
    }

    bool match = false;
    for (int card = 0; card < numAllowedCards; card++) {
      match = true;
      for (byte i = 0; i < 4; i++) {
        if (rfid.uid.uidByte[i] != allowedUIDs[card][i]) {
          match = false;
          break;
        }
      }
      if (match) break;
    }

    if (match) {
      buzz();
      display.clearDisplay();
      display.drawBitmap(48, 14, start_logo, 32, 32, SSD1306_WHITE);
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(40, 51);
      display.println("UniStart");
      display.display();
      digitalWrite(RELAY1_PIN, LOW);
      delay(2000);
      unlocked_by_bt = true;
      break;
    } else {
      buzz();
      delay(50);
      buzz();
      display.clearDisplay();
      display.drawBitmap(48, 30, invalid_logo, 32, 32, SSD1306_WHITE);
      display.setCursor(26, 19);
      display.println("Invalid card");
      display.display();
      delay(1000);
      display.clearDisplay();
      display.drawBitmap(48, 30, nfc_logo, 32, 32, SSD1306_WHITE);
      display.setCursor(10, 19);
      display.println("Waiting for card...");
      display.display();
    }

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }

  display.clearDisplay();
  display.display();
  buzz();
}

void loop() {
  if (millis() - last_bt_check > 1000) {
    previous_bt_connected = bt_connected;
    bt_connected = SerialBT.hasClient();

    if (bt_connected != previous_bt_connected) {
      buzz();
    }

    last_bt_check = millis();
    updateDisplay();
  }

  while (SerialBT.available()) {
    char c = SerialBT.read();
    if (c == '\n' || c == '\r') {
      inputBuffer[inputPos] = '\0';
      processCommand(inputBuffer);
      inputPos = 0;
    } else if (inputPos < sizeof(inputBuffer) - 1) {
      inputBuffer[inputPos++] = c;
    }
  }

  // Обработка RFID карты после разблокировки
  if (unlocked_by_bt && rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    bool match = false;
    for (int card = 0; card < numAllowedCards; card++) {
      match = true;
      for (byte i = 0; i < 4; i++) {
        if (rfid.uid.uidByte[i] != allowedUIDs[card][i]) {
          match = false;
          break;
        }
      }
      if (match) break;
    }

    if (match) {
      buzz();
      // Выполняем команды '2' (стартер) и 'L' (светодиод)
      if (!starterActive) { // Запускаем процесс только если стартер не активен
        starterActive = true;
        buzzCount = 0;
        lastBuzzTime = millis();
        buzz(); // Первый сигнал
        buzzCount++;
      }
      digitalWrite(LED_PIN, HIGH); // Переключаем светодиод
      SerialBT.println("Card detected");
    } else if (adding_new_card) {
      // Добавляем новую карту
      addCard(rfid.uid.uidByte);
      SerialBT.print("New card added: ");
      for (byte i = 0; i < 4; i++) {
        SerialBT.print(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
        SerialBT.print(rfid.uid.uidByte[i], HEX);
        if (i < 3) SerialBT.print(":");
      }
      SerialBT.println();
      adding_new_card = false;
      buzz();
      delay(50);
      buzz();
    }

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }

  // Обработка команды стартера
  if (starterActive) {
    unsigned long currentTime = millis();
    
    // Обработка звуковых сигналов
    if (buzzCount < 3) {
      if (currentTime - lastBuzzTime >= 500) {
        buzz();
        lastBuzzTime = currentTime;
        buzzCount++;
      }
    }
    // Включение стартера после 3 сигналов
    else if (buzzCount == 3) {
      digitalWrite(RELAY2_PIN, LOW);
      starterStartTime = currentTime;
      buzzCount++; // Переходим к следующему этапу
      SerialBT.println("Starter: 3s");
    }
    // Выключение стартера через 3 секунды
    else if (currentTime - starterStartTime >= 3000) {
      digitalWrite(RELAY2_PIN, HIGH);
      starterActive = false;
      SerialBT.println("Done");
      buzz();
    }
  }

  static uint32_t tmr;
  if (millis() - tmr >= 1000) {
    tmr = millis();
    sensor.requestTemp();
    sensor1.requestTemp();
  }

  adc_value = analogRead(analog_pin);
  adc_voltage = (adc_value * ref_voltage) / 4095.0;
  in_voltage = adc_voltage / (R2/(R1+R2));

  if (sensor.readTemp()) eng_temp = sensor.getTemp();
  if (sensor1.readTemp()) sun_temp = sensor1.getTemp();
}

void processCommand(const char* cmd) {
  if (cmd[0] == '1') {
    digitalWrite(RELAY1_PIN, !digitalRead(RELAY1_PIN));
    SerialBT.print("Ignition: ");
    SerialBT.println(digitalRead(RELAY1_PIN) ? "OFF" : "ON");
    buzz();
  } else if (cmd[0] == '2') {
    if (!starterActive) { // Запускаем процесс только если стартер не активен
      starterActive = true;
      buzzCount = 0;
      lastBuzzTime = millis();
      buzz(); // Первый сигнал
      buzzCount++;
    }
  } else if (cmd[0] == 'L') {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    SerialBT.print("LED: ");
    SerialBT.println(digitalRead(LED_PIN) ? "ON" : "OFF");
    buzz();
  } else if (cmd[0] == 'S') {
    sendStatus();
  } else if (cmd[0] == 'D' && cmd[1] == ':') {
    setRTCFromString(cmd + 2);
    buzz();
  } else if (cmd[0] == 'A') {
    // Команда добавления новой карты
    adding_new_card = true;
    SerialBT.println("Ready to add new card. Please scan the card now.");
    buzz();
  } else if (cmd[0] == 'R' && cmd[1] == ':') {
    // Удаление карты по индексу (R:0, R:1 и т.д.)
    int index = atoi(cmd + 2);
    removeCard(index);
    SerialBT.print("Removed card ");
    SerialBT.println(index);
    buzz();
  } else if (cmd[0] == 'F') {
    listCards();
    buzz();
  }

  updateDisplay();
}

void updateDisplay() {
  display.clearDisplay();

  // Icons (top)
  display.drawBitmap(0, 1, bt_connected ? bt_icon : no_icon, 16, 16, SSD1306_WHITE);
  display.drawBitmap(23, 1, digitalRead(RELAY1_PIN) ? no_icon : ignition_icon, 16, 16, SSD1306_WHITE);
  display.drawBitmap(46, 1, digitalRead(RELAY2_PIN) ? no_icon : starter_icon, 16, 16, SSD1306_WHITE);
  display.drawBitmap(66, 1, digitalRead(LED_PIN) ? led_icon : no_icon, 16, 16, SSD1306_WHITE);

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(92, 5);
  display.print(in_voltage, 1);
  display.print(" V");

  DateTime now = rtc.now();

  // Время
  char timeStr[9];
  snprintf(timeStr, sizeof(timeStr), "%02d:%02d", now.hour(), now.minute(), now.second());
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 25);
  display.print(timeStr);

  // День недели и дата
  char dateStr[20];
  snprintf(dateStr, sizeof(dateStr), "%s %02d/%02d", daysOfWeek[now.dayOfTheWeek()], now.day(), now.month(), now.year());
  display.setTextSize(1);
  display.setCursor(0, 45);
  display.print(dateStr);

  display.drawBitmap(65, 23, engine_icon, 16, 16, SSD1306_WHITE);
  display.drawBitmap(65, 41, sun_icon, 16, 16, SSD1306_WHITE);

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(86, 28);
  display.print(eng_temp, 1);
  display.drawBitmap(109, 28, degreeSymbol, 8, 8, SSD1306_WHITE);
  display.print(" C");

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(86, 46);
  display.print(sun_temp, 1);
  display.drawBitmap(109, 46, degreeSymbol, 8, 8, SSD1306_WHITE);
  display.print(" C");

  display.drawLine(7, 20, 121, 20, SSD1306_WHITE);
  display.drawLine(7, 60, 121, 60, SSD1306_WHITE);

  display.display();
}

void sendStatus() {
  SerialBT.print("I-");
  SerialBT.println(digitalRead(RELAY1_PIN) ? "0" : "1");
  SerialBT.print("S-");
  SerialBT.println(digitalRead(RELAY2_PIN) ? "0" : "1");
  SerialBT.print("L-");
  SerialBT.println(digitalRead(LED_PIN) ? "1" : "0");

  DateTime now = rtc.now();
  SerialBT.print("T-");
  SerialBT.print(now.year()); SerialBT.print("-");
  if (now.month() < 10) SerialBT.print("0"); // Добавляем ноль для месяцев 1-9
  SerialBT.print(now.month()); SerialBT.print("-");
  if (now.day() < 10) SerialBT.print("0"); // Добавляем ноль для дней 1-9
  SerialBT.print(now.day()); SerialBT.print(" ");
  if (now.hour() < 10) SerialBT.print("0"); // Добавляем ноль для часов 0-9
  SerialBT.print(now.hour()); SerialBT.print(":");
  if (now.minute() < 10) SerialBT.print("0"); // Добавляем ноль для минут 0-9
  SerialBT.print(now.minute()); SerialBT.print(":");
  if (now.second() < 10) SerialBT.print("0"); // Добавляем ноль для секунд 0-9
  SerialBT.println(now.second());

  SerialBT.print("V-");
  SerialBT.println(in_voltage, 2);

  SerialBT.print("ETEMP-");
  SerialBT.println(sensor.getTemp());

  SerialBT.print("STEMP-");
  SerialBT.println(sensor1.getTemp());
}

void setRTCFromString(const char* timeStr) {
  int y, M, d, h, m, s;
  if (sscanf(timeStr, "%d-%d-%d %d:%d:%d", &y, &M, &d, &h, &m, &s) == 6) {
    rtc.adjust(DateTime(y, M, d, h, m, s));
    SerialBT.println("RTC updated");
  } else {
    SerialBT.println("Invalid time format. Use D:YYYY-MM-DD HH:MM:SS");
  }
}