#include <Adafruit_NeoPixel.h>
#include <EEPROM.h> // Для хранения данных между перезагрузками

#define PIN         4           // Пин подключения ленты
#define NUMPIXELS   18          // Кол-во светодиодов
#define BUTTON_PIN  A1          // Кнопка (аналоговая)

// Индексы EEPROM
#define EEPROM_MODE_ADDR       0
#define EEPROM_BRIGHTNESS_ADDR 1

Adafruit_NeoPixel strip(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// Яркости, доступные пользователю
uint8_t brightnessLevels[] = {50, 100, 180, 255};
uint8_t brightnessIndex = 3;

uint8_t mode = 0;
const uint8_t modeCount = 8; // количество режимов

unsigned long lastButtonPress = 0;
bool buttonHeld = false;

// ============================ SETUP =============================
void setup() {
  pinMode(BUTTON_PIN, INPUT);
  strip.begin();

  // Загрузка режима и яркости из EEPROM
  mode = EEPROM.read(EEPROM_MODE_ADDR);
  brightnessIndex = EEPROM.read(EEPROM_BRIGHTNESS_ADDR);
  if (mode >= modeCount) mode = 0;
  if (brightnessIndex >= sizeof(brightnessLevels)) brightnessIndex = sizeof(brightnessLevels) - 1;

  strip.setBrightness(brightnessLevels[brightnessIndex]);
  strip.show();

  Serial.begin(9600);
}

// ============================ LOOP ==============================
void loop() {
  handleButton();
  runEffect(mode);
}

// ===================== ОБРАБОТКА КНОПКИ =========================
void handleButton() {
  static bool lastState = false;
  bool currentState = analogRead(BUTTON_PIN) > 50;

  // Кнопка нажата
  if (currentState && !lastState) {
    lastButtonPress = millis();
    buttonHeld = false;
  }

  // Кнопка отпущена
  if (!currentState && lastState) {
    // Короткое нажатие — смена эффекта
    if (!buttonHeld && millis() - lastButtonPress < 500) {
      mode = (mode + 1) % modeCount;
      EEPROM.write(EEPROM_MODE_ADDR, mode); // Сохраняем в EEPROM
    }
    // Длинное нажатие — смена яркости
    else if (buttonHeld) {
      brightnessIndex = (brightnessIndex + 1) % sizeof(brightnessLevels);
      strip.setBrightness(brightnessLevels[brightnessIndex]);
      EEPROM.write(EEPROM_BRIGHTNESS_ADDR, brightnessIndex); // Сохраняем в EEPROM
    }
  }

  // Держим кнопку дольше 1 сек — засчитывается как долгое нажатие
  if (currentState && (millis() - lastButtonPress > 1000)) {
    buttonHeld = true;
  }

  lastState = currentState;
}

// ======================= ЗАПУСК ЭФФЕКТА ========================
void runEffect(uint8_t m) {
  switch (m) {
    case 0: effectBackToFuture(); break; // 🔥 Эффект из фильма "Назад в будущее" — красно-оранжевый огонь
    case 1: effectElectricBlue(); break; // 🔷 Электрический синий + белое ядро
    case 2: effectPulsatingNeon(); break; // 🌈 Пульсирующий неон — цвета переливаются в дыхании
    case 3: effectGhostTrail(); break; // 👻 Эффект: "Призрачное движение"
    case 4: effectClassic1(); break;
    case 5: effectClassic2(); break;
    case 6: effectClassic3(); break;
    case 7: effectOff(); break;
  }
}

// 🔥 Эффект из фильма "Назад в будущее" — красно-оранжевый огонь
void effectBackToFuture() {
  for (int i = 0; i < NUMPIXELS; i++) {
    if (i < 5 || i >= 11) {
      strip.setPixelColor(i, strip.Color(255, 0, 0));  // Красный
    } else {
      strip.setPixelColor(i, strip.Color(255, 140, 0)); // Оранжевый
    }
  }
  strip.show();
}

// 🔷 Электрический синий + белое ядро
void effectElectricBlue() {
  for (int i = 0; i < NUMPIXELS; i++) {
    if (i < 5 || i >= 11) {
      strip.setPixelColor(i, strip.Color(0, 0, 255));  // Синий
    } else {
      strip.setPixelColor(i, strip.Color(200, 250, 255)); // Бело-голубой
    }
  }
  strip.show();
}

// 🌈 Пульсирующий неон — цвета переливаются в дыхании
void effectPulsatingNeon() {
  static uint8_t pulse = 0;
  static int8_t dir = 5;
  pulse += dir;
  if (pulse > 200 || pulse < 50) dir = -dir;

  for (int i = 0; i < NUMPIXELS; i++) {
    uint32_t color = strip.Color(pulse, 0, 255 - pulse); // Переход от розово-синего к синему
    strip.setPixelColor(i, color);
  }
  strip.show();
  delay(30);
}

// 👻 Эффект: "Призрачное движение"
void effectGhostTrail() {
  static int head = 0;
  static uint8_t fade = 40; // Чем больше — быстрее затухает

  // Затухание всех пикселей
  for (int i = 0; i < NUMPIXELS; i++) {
    uint32_t c = strip.getPixelColor(i);

    uint8_t r = (c >> 16) & 0xFF;
    uint8_t g = (c >> 8)  & 0xFF;
    uint8_t b = c & 0xFF;

    r = (r > fade) ? r - fade : 0;
    g = (g > fade) ? g - fade : 0;
    b = (b > fade) ? b - fade : 0;

    strip.setPixelColor(i, r, g, b);
  }

  // Яркий "след" движется
  strip.setPixelColor(head, 255, 80, 10); // Яркий оранжевый
  head = (head + 1) % NUMPIXELS;

  strip.show();
  delay(40);
}

// 🔴🟠🔴 Красный - Оранжевый - Красный (как в оригинале case 0)
void effectClassic1() {
  strip.clear();
  for (int i = 0; i < strip.numPixels(); i++) {
    if (i < 5 || i >= 11)
      strip.setPixelColor(i, strip.Color(255, 0, 0));      // Красный
    else
      strip.setPixelColor(i, strip.Color(180, 140, 0));    // Оранжевый
  }
  strip.show();
}

// 🔵⚪️🔵 Синий - Голубой - Синий (как в оригинале case 1)
void effectClassic2() {
  strip.clear();
  for (int i = 0; i < strip.numPixels(); i++) {
    if (i < 5 || i >= 11)
      strip.setPixelColor(i, strip.Color(0, 0, 255));       // Синий
    else
      strip.setPixelColor(i, strip.Color(200, 250, 255));   // Голубой
  }
  strip.show();
}

// 🔴🔵🔴 Красно-синий неон (как в оригинале case 2)
void effectClassic3() {
  strip.clear();
  for (int i = 0; i < strip.numPixels(); i++) {
    if (i < 5 || i >= 11)
      strip.setPixelColor(i, strip.Color(255, 0, 10));      // Розово-красный
    else
      strip.setPixelColor(i, strip.Color(0, 10, 255));      // Синий
  }
  strip.show();
}

// 💡 Выключено
void effectOff() {
  strip.clear();
  strip.show();
}
