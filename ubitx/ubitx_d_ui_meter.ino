/**
   Meter (not used in this build for anything)
   the meter is drawn using special characters. Each character is composed of 5 x 8 matrix.
   The  s_meter array holds the definition of the these characters.
   each line of the array is is one character such that 5 bits of every byte
   makes up one line of pixels of the that character (only 5 bits are used)
   The current reading of the meter is assembled in the string called meter
*/

char meter[17];

const byte PROGMEM s_meter_bitmap[] = {
  B00000, B00000, B00000, B00000, B00000, B00100, B00100, B11011,
  B10000, B10000, B10000, B10000, B10100, B10100, B10100, B11011,
  B01000, B01000, B01000, B01000, B01100, B01100, B01100, B11011,
  B00100, B00100, B00100, B00100, B00100, B00100, B00100, B11011,
  B00010, B00010, B00010, B00010, B00110, B00110, B00110, B11011,
  B00001, B00001, B00001, B00001, B00101, B00101, B00101, B11011,
  B10000, B11000, B11100, B11110, B11100, B11000, B10000, B00000,
  B00001, B00011, B00111, B01111, B00111, B00011, B00001, B00000
};

// initializes the custom characters
// we start from char 1 as char 0 terminates the string!
void initMeter() {
  lcd.createChar(1, s_meter_bitmap);
  lcd.createChar(2, s_meter_bitmap + 8);
  lcd.createChar(3, s_meter_bitmap + 16);
  lcd.createChar(4, s_meter_bitmap + 24);
  lcd.createChar(5, s_meter_bitmap + 32);
  lcd.createChar(6, s_meter_bitmap + 40);
  lcd.createChar(0, s_meter_bitmap + 48);
  lcd.createChar(7, s_meter_bitmap + 56);
}

/**
   The meter is drawn with special characters.
   character 1 is used to simple draw the blocks of the scale of the meter
   characters 2 to 6 are used to draw the needle in positions 1 to within the block
   This displays a meter from 0 to 100, -1 displays nothing
*/
void drawMeter(int8_t needle) {
  int16_t best, i, s;

  if (needle < 0)
    return;

  s = (needle * 4) / 10;
  for (i = 0; i < 8; i++) {
    if (s >= 5)
      meter[i] = 1;
    else if (s >= 0)
      meter[i] = 2 + s;
    else
      meter[i] = 1;
    s = s - 5;
  }
  if (needle >= 40)
    meter[i - 1] = 6;
  meter[i] = 0;
}

