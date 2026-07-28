#ifndef BIGFONTE_H
#define BIGFONTE_H

/*
  Large digits for a 16x2 character LCD.

  Original implementation:
  Fabiano A. Arndt, 2015
  www.youtube.com/user/fabianoallex
  www.facebook.com/dicasarduino
  fabianoallex@gmail.com

  The class uses all eight user-defined character slots available on an
  HD44780-compatible LCD to draw digits across two display rows.
*/

struct LCDNumber {
  byte top;
  byte bottom;
};

class LCDBigNumbers {
 public:
  LCDBigNumbers(LiquidCrystal_I2C *lcd, uint8_t row, uint8_t column)
      : lcd_(lcd), row_(row), column_(column), value_(0) {}

  // Loads the eight custom character patterns into LCD CGRAM.
  void createChars()
  {
    for (uint8_t i = 0; i < 8; ++i) {
      lcd_->createChar(i, customChars_[i]);
    }
  }

  void setRow(uint8_t row)
  {
    clearValue();
    row_ = row;
    drawValue();
  }

  void setCol(uint8_t column)
  {
    clearValue();
    column_ = column;
    drawValue();
  }

  void setValue(unsigned long value)
  {
    clearValue();
    value_ = value;
    drawValue();
  }

 private:
  LiquidCrystal_I2C *lcd_;
  uint8_t row_;
  uint8_t column_;
  unsigned long value_;

  static byte customChars_[8][8];
  static const LCDNumber digitParts_[10];

  static uint8_t digitCount(unsigned long value)
  {
    uint8_t count = 1;
    while (value >= 10) {
      value /= 10;
      ++count;
    }
    return count;
  }

  void clearValue()
  {
    const uint8_t count = digitCount(value_);

    for (uint8_t i = 0; i < count; ++i) {
      lcd_->setCursor(column_ + i, row_);
      lcd_->print(' ');
      lcd_->setCursor(column_ + i, row_ + 1);
      lcd_->print(' ');
    }
  }

  void drawValue()
  {
    const uint8_t count = digitCount(value_);
    unsigned long divisor = 1;

    for (uint8_t i = 1; i < count; ++i) {
      divisor *= 10;
    }

    unsigned long remainder = value_;
    for (uint8_t i = 0; i < count; ++i) {
      const uint8_t digit = remainder / divisor;
      remainder %= divisor;

      lcd_->setCursor(column_ + i, row_);
      lcd_->write(digitParts_[digit].top);
      lcd_->setCursor(column_ + i, row_ + 1);
      lcd_->write(digitParts_[digit].bottom);

      if (divisor > 1) {
        divisor /= 10;
      }
    }
  }
};

byte LCDBigNumbers::customChars_[8][8] = {
  {B11111, B10001, B10001, B10001, B10001, B10001, B10001, B10001},
  {B10001, B10001, B10001, B10001, B10001, B10001, B10001, B11111},
  {B00001, B00001, B00001, B00001, B00001, B00001, B00001, B00001},
  {B11111, B00001, B00001, B00001, B00001, B00001, B00001, B11111},
  {B11111, B10000, B10000, B10000, B10000, B10000, B10000, B11111},
  {B11111, B00001, B00001, B00001, B00001, B00001, B00001, B00001},
  {B11111, B10001, B10001, B10001, B10001, B10001, B10001, B11111},
  {B00001, B00001, B00001, B00001, B00001, B00001, B00001, B11111}
};

const LCDNumber LCDBigNumbers::digitParts_[10] = {
  {0, 1},  // 0
  {2, 2},  // 1
  {5, 4},  // 2
  {3, 7},  // 3
  {1, 2},  // 4
  {4, 7},  // 5
  {4, 1},  // 6
  {5, 2},  // 7
  {6, 1},  // 8
  {6, 7}   // 9
};

#endif  // BIGFONTE_H
