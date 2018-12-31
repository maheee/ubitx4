/**
   The user interface of the ubitx consists of the encoder, the push-button on top of it
   and the 16x2 LCD display.
   The upper line of the display is constantly used to display frequency and status
   of the radio. Occasionally, it is used to provide a two-line information that is
   quickly cleared up.
*/

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

// this builds up the top line of the display with frequency and mode
void updateDisplay() {
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
  printLine(1, c);
}

