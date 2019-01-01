/**
   The user interface of the ubitx consists of the encoder, the push-button on top of it
   and the 16x2 LCD display.
*/

byte life_indicator_1[8] = { B11100, B11000, B10000, B00000, B00000, B00000, B00000 };
byte life_indicator_2[8] = { B00111, B00011, B00001, B00000, B00000, B00000, B00000 };
byte life_indicator_3[8] = { B00000, B00000, B00000, B00000, B00001, B00011, B00111 };
byte life_indicator_4[8] = { B00000, B00000, B00000, B00000, B10000, B11000, B11100 };

// initializes the custom characters
// we start from char 1 as char 0 terminates the string!
void initLcd() {
  lcd.createChar(1, life_indicator_1);
  lcd.createChar(2, life_indicator_2);
  lcd.createChar(3, life_indicator_3);
  lcd.createChar(4, life_indicator_4);
  // lcd.createChar(5, life_indicator_1);
  // lcd.createChar(6, life_indicator_1);
  // lcd.createChar(7, life_indicator_1);
}

// The generic routine to display one line on the LCD
void printLine(char linenmbr, char *c) {
  if (strcmp(c, printBuff[linenmbr])) {     // only refresh the display when there was a change
    lcd.setCursor(0, linenmbr);             // place the cursor at the beginning of the selected line
    lcd.print(c);
    strcpy(printBuff[linenmbr], c);

    for (byte i = strlen(c); i < 16; i++) { // add white spaces until the end of the 16 characters line is reached
      lcd.print(' ');
    }
  }
}

void printLineF(char linenmbr, const __FlashStringHelper *c) {
  int i;
  char tmpBuff[17];
  PGM_P p = reinterpret_cast<PGM_P>(c);

  for (i = 0; i < 17; i++) {
    unsigned char fChar = pgm_read_byte(p++);
    tmpBuff[i] = fChar;
    if (fChar == 0)
      break;
  }

  printLine(linenmbr, tmpBuff);
}

void printIntValue(char linenmbr, char* prefix, char *postfix, int value) {
  strcpy(b, prefix);
  itoa(value, c, 10);
  strcat(b, c);
  strcat(b, postfix);
  printLine(linenmbr, b);
}

void printLongValue(char linenmbr, char* prefix, char *postfix, long value) {
  strcpy(b, prefix);
  ltoa(value, c, 10);
  strcat(b, c);
  strcat(b, postfix);
  printLine(linenmbr, b);
}

void printInfoF(const __FlashStringHelper *c1, const __FlashStringHelper *c2) {
  printLineF0(c1);
  printLineF1(c2);
  active_delay(2000);
}

void printStateChangeF(const __FlashStringHelper *c) {
  printLine1("");
  printLineF0(c);
  active_delay(500);
  printLine0("");
  updateDisplay();
}

// this builds up the line of the display with frequency and mode
void updateDisplayInLine(char linenmbr) {
  memset(c, 0, sizeof(c));
  memset(b, 0, sizeof(b));

  ultoa(frequency, b, DEC);

  if (inTx) {
    if (cwTimeout > 0)
      strcpy(c, "   CW:");
    else
      strcpy(c, "   TX:");
  }
  else {
    if (ritOn)
      strcpy(c, "RIT ");
    else {
      if (isUSB)
        strcpy(c, "USB ");
      else
        strcpy(c, "LSB ");
    }
    if (vfoActive == VFO_A) // VFO A is active
      strcat(c, "A:");
    else
      strcat(c, "B:");
  }

  //one mhz digit if less than 10 M, two digits if more
  if (frequency < 10000000l) {
    c[6] = ' ';
    c[7]  = b[0];
    strcat(c, ".");
    strncat(c, &b[1], 3);
    strcat(c, ".");
    strncat(c, &b[4], 3);
  }
  else {
    strncat(c, b, 2);
    strcat(c, ".");
    strncat(c, &b[2], 3);
    strcat(c, ".");
    strncat(c, &b[5], 3);
  }

  if (inTx)
    strcat(c, " TX");
  printLine(linenmbr, c);
}

