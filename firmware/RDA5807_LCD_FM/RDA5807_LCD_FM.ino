/*
  RDA5807 FM Radio with Arduino Nano and 16x2 I2C LCD
  Tuning range: 86.0 to 108.0 MHz

  Author: Anderson Cardoso da Silva
  Project page: https://create.arduino.cc/projecthub/acardosodasilva/
  Original environment: Arduino IDE 1.8.19
  Original date: January 13, 2022

  Connections:

  | Device       | Device pin       | Arduino Nano |
  |--------------|------------------|--------------|
  | I2C LCD      | GND              | GND          |
  |              | VCC              | 5V           |
  |              | SDA              | A4           |
  |              | SCL              | A5           |
  | RDA5807      | SDA              | A4           |
  |              | SCL              | A5           |
  |              | VCC              | 3.3V         |
  |              | GND              | GND          |
  | Rotary       | A (data)         | D2           |
  | encoder      | B (clock)        | D3           |
  |              | SW (not used)    | D4           |
  |              | VCC              | 3.3V         |
  |              | GND              | GND          |

  Radio library documentation:
  http://mathertel.github.io/Radio/html/index.html
*/

#include <LiquidCrystal_I2C.h>
#include "Rotary.h"
#include <RDA5807M.h>

#include "Bigfonte.h"

// Hardware configuration.
constexpr uint8_t ENCODER_PIN_A = 2;
constexpr uint8_t ENCODER_PIN_B = 3;
constexpr uint8_t STEREO_LED_PIN = A1;
constexpr uint8_t LCD_ADDRESS = 0x3F;
constexpr uint8_t LCD_COLUMNS = 16;
constexpr uint8_t LCD_ROWS = 2;

// The frequency is stored in tenths of MHz (999 means 99.9 MHz).
constexpr uint16_t MIN_FREQUENCY = 860;
constexpr uint16_t MAX_FREQUENCY = 1080;
constexpr uint16_t INITIAL_FREQUENCY = 999;
constexpr uint8_t INITIAL_VOLUME = 15;
constexpr uint8_t SIGNAL_BAR_WIDTH = 7;

uint16_t currentFrequency = INITIAL_FREQUENCY;
volatile int8_t encoderDirection = 0;

LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);
Rotary encoder(ENCODER_PIN_A, ENCODER_PIN_B);
LCDBigNumbers lcdNumber(&lcd, 0, 0);
RDA5807M radio;
RADIO_INFO radioInfo;

void showFrequency();
void showStatus();

// Handles changes on rotary encoder pins D2 and D3.
ISR(PCINT2_vect)
{
  const uint8_t result = encoder.process();

  if (result != 0) {
    encoderDirection = (result == DIR_CW) ? 1 : -1;
  }
}

void setup()
{
  lcd.init();
  lcd.backlight();
  lcd.print("Radio...");
  delay(1000);
  lcd.clear();

  lcdNumber.createChars();

  lcd.setCursor(1, 1);
  lcd.print("FM");
  lcd.setCursor(9, 0);
  lcd.print("MHz");

  pinMode(STEREO_LED_PIN, OUTPUT);

  radio.init();
  radio.setBandFrequency(RADIO_BAND_FM, currentFrequency * 10);
  radio.setVolume(INITIAL_VOLUME);
  radio.setMono(false);
  radio.setMute(false);

  // Enable pin-change interrupts for Arduino Nano pins D2 and D3.
  PCICR |= _BV(PCIE2);
  PCMSK2 |= _BV(PCINT18) | _BV(PCINT19);
  sei();

  showFrequency();
}

void loop()
{
  // Copy and clear the ISR value atomically.
  noInterrupts();
  const int8_t direction = encoderDirection;
  encoderDirection = 0;
  interrupts();

  if (direction != 0) {
    int16_t nextFrequency = static_cast<int16_t>(currentFrequency) + direction;
    nextFrequency = constrain(nextFrequency, MIN_FREQUENCY, MAX_FREQUENCY);

    if (nextFrequency != currentFrequency) {
      currentFrequency = static_cast<uint16_t>(nextFrequency);
      radio.setFrequency(currentFrequency * 10);
      showFrequency();
    }
  }

  showStatus();
}

// Displays the tuned frequency using the custom large-number characters.
void showFrequency()
{
  lcdNumber.setCol((currentFrequency > 999) ? 4 : 5);
  lcdNumber.setValue(currentFrequency / 10);

  lcd.setCursor(7, 0);
  lcd.print(currentFrequency % 10);
  lcd.setCursor(7, 1);
  lcd.print(' ');
}

// Updates the stereo indicator, LED, and received-signal bar.
void showStatus()
{
  radio.getRadioInfo(&radioInfo);

  lcd.setCursor(14, 0);
  lcd.write(radioInfo.stereo ? 219 : 254);
  digitalWrite(STEREO_LED_PIN, radioInfo.stereo ? HIGH : LOW);

  // Scale RSSI approximately to the seven available LCD columns.
  const uint8_t signalLevel = min(
    static_cast<uint8_t>(radioInfo.rssi * 0.3f),
    SIGNAL_BAR_WIDTH
  );

  lcd.setCursor(9, 1);
  for (uint8_t i = 0; i < SIGNAL_BAR_WIDTH; ++i) {
    lcd.print((i < signalLevel) ? '_' : ' ');
  }
}
