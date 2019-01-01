#define printCond(C, A, B) printLineF2(C ? F(A) : F(B));

int menuBand(int btn) {
  int knob = 0;
  int band;
  unsigned long offset;

  // band = frequency/1000000l;
  // offset = frequency % 1000000l;

  if (!btn) {
    printLineF2(F("Band Select    \x7E"));
    return;
  }

  printLineF2(F("Band Select:"));
  //wait for the button menu select button to be lifted)
  while (btnDown())
    active_delay(50);
  active_delay(50);
  ritDisable();

  while (!btnDown()) {

    knob = enc_read();
    if (knob != 0) {
      /*
        if (band > 3 && knob < 0)
        band--;
        if (band < 30 && knob > 0)
        band++;
        if (band > 10)
        isUSB = true;
        else
        isUSB = false;
        setFrequency(((unsigned long)band * 1000000l) + offset); */
      if (knob < 0 && frequency > 3000000l)
        setFrequency(frequency - 200000l);
      if (knob > 0 && frequency < 30000000l)
        setFrequency(frequency + 200000l);
      if (frequency > 10000000l)
        isUSB = true;
      else
        isUSB = false;
      updateDisplay();
    }
    checkCAT();
    active_delay(20);
  }

  printStateChange(F("Band set!"));
}

// Menu #2
void menuRit_print() {
  printCond(ritOn == 1, "RIT On \x7E Off", "RIT Off \x7E On");
}

void menuRit_toggle() {
  if (ritOn == 0) {
    ritEnable(frequency);
    printStateChange(F("RIT is On!"));
  } else {
    ritDisable();
    printStateChange(F("RIT is Off!"));
  }
}


//Menu #3
void menuVfo_print() {
  printCond(vfoActive == VFO_A, "VFO A \x7E B", "VFO B \x7E A");
}

void menuVfo_toggle() {
  if (vfoActive == VFO_B) {
    vfoB = frequency;
    isUsbVfoB = isUSB;
    EEPROM.put(VFO_B, frequency);
    if (isUsbVfoB)
      EEPROM.put(VFO_B_MODE, VFO_MODE_USB);
    else
      EEPROM.put(VFO_B_MODE, VFO_MODE_LSB);

    vfoActive = VFO_A;
    frequency = vfoA;
    isUSB = isUsbVfoA;
    printStateChange(F("Selected VFO A!"));
  } else {
    vfoA = frequency;
    isUsbVfoA = isUSB;
    EEPROM.put(VFO_A, frequency);
    if (isUsbVfoA)
      EEPROM.put(VFO_A_MODE, VFO_MODE_USB);
    else
      EEPROM.put(VFO_A_MODE, VFO_MODE_LSB);

    vfoActive = VFO_B;
    frequency = vfoB;
    isUSB = isUsbVfoB;
    printStateChange(F("Selected VFO B!"));
  }

  ritDisable();
  setFrequency(frequency);
}

// Menu #4
void menuSideband_print() {
  printCond(isUSB == true, "USB \x7E LSB", "LSB \x7E USB");
}

void menuSideband_toggle() {
  if (isUSB == true) {
    isUSB = false;
    printStateChange(F("LSB Selected!"));
  }
  else {
    isUSB = true;
    printStateChange(F("USB Selected!"));
  }

  if (vfoActive == VFO_B) {
    isUsbVfoB = isUSB;
  } else {
    isUsbVfoB = isUSB;
  }
}

//Menu #5
void menuSplit_print() {
  printCond(splitOn == 0, "Split Off \x7E On", "Split On \x7E Off");
}

void menuSplit_toggle() {
  if (splitOn == 1) {
    splitOn = 0;
    printLineF2(F("Split ON!"));
  }
  else {
    splitOn = 1;
    if (ritOn == 1) {
      ritOn = 0;
    }
    printLineF2(F("Split OFF!"));
  }
  active_delay(500);
  printLine2("");
  updateDisplay();
}

//Menu #6
int menuCWSpeed(int btn) {
  int knob = 0;
  int wpm;

  wpm = 1200 / cwSpeed;

  if (!btn) {
    strcpy(b, "CW Speed> ");
    itoa(wpm, c, 10);
    strcat(b, c);
    strcat(b, " WPM     \x7E");
    printLine2(b);
    return;
  }

  wpm = getValueByKnob(1, 100, 1,  wpm, "CW Speed> ", " WPM");

  cwSpeed = 1200 / wpm;
  EEPROM.put(CW_SPEED, cwSpeed);
  
  printStateChange(F("CW Speed set!"));
}

void menuExit(int btn) {

  if (!btn) {
    printLineF2(F("Exit Menu      \x7E"));
  }
  else {
    printStateChange(F("Exiting ..."));
  }
}

int calibrateClock() {
  int knob = 0;
  int32_t prev_calibration;


  //keep clear of any previous button press
  while (btnDown())
    active_delay(100);
  active_delay(100);

  digitalWrite(PIN_TX_LPF_A, 0);
  digitalWrite(PIN_TX_LPF_B, 0);
  digitalWrite(PIN_TX_LPF_C, 0);

  prev_calibration = calibration;
  calibration = 0;

  isUSB = true;

  //turn off the second local oscillator and the bfo
  si5351_set_calibration(calibration);
  startTx(TX_CW);
  si5351bx_setfreq(2, 10000000l);

  strcpy(b, "#1 10 MHz cal:");
  ltoa(calibration / 8750, c, 10);
  strcat(b, c);
  printLine2(b);

  while (!btnDown())
  {

    if (digitalRead(PIN_PTT) == LOW && !keyDown)
      cwKeydown();
    if (digitalRead(PIN_PTT)  == HIGH && keyDown)
      cwKeyUp();

    knob = enc_read();

    if (knob > 0)
      calibration += 875;
    else if (knob < 0)
      calibration -= 875;
    else
      continue; //don't update the frequency or the display

    si5351_set_calibration(calibration);
    si5351bx_setfreq(2, 10000000l);
    strcpy(b, "#1 10 MHz cal:");
    ltoa(calibration / 8750, c, 10);
    strcat(b, c);
    printLine2(b);
  }

  cwTimeout = 0;
  keyDown = 0;
  stopTx();

  EEPROM.put(MASTER_CAL, calibration);
  initOscillators();
  setFrequency(frequency);

  printStateChange(F("Calibration set!"));
}

int menuSetupCalibration(int btn) {
  int knob = 0;
  int32_t prev_calibration;

  if (!btn) {
    printLineF2(F("Setup:Calibrate\x7E"));
    return 0;
  }

  printLineF2(F("Press PTT & tune"));
  printLineF1(F("to exactly 10 MHz"));
  active_delay(2000);
  calibrateClock();
}

void printCarrierFreq(unsigned long freq) {

  memset(c, 0, sizeof(c));
  memset(b, 0, sizeof(b));

  ultoa(freq, b, DEC);

  strncat(c, b, 2);
  strcat(c, ".");
  strncat(c, &b[2], 3);
  strcat(c, ".");
  strncat(c, &b[5], 1);
  printLine2(c);
}

void menuSetupCarrier(int btn) {
  int knob = 0;
  unsigned long prevCarrier;

  if (!btn) {
    printLineF2(F("Setup:BFO      \x7E"));
    return;
  }

  prevCarrier = usbCarrier;
  printLineF2(F("Tune to best Signal"));
  printLineF1(F("Press to confirm. "));
  active_delay(1000);

  usbCarrier = 11995000l;
  si5351bx_setfreq(0, usbCarrier);
  printCarrierFreq(usbCarrier);

  //disable all clock 1 and clock 2
  while (!btnDown()) {
    knob = enc_read();

    if (knob > 0)
      usbCarrier -= 50;
    else if (knob < 0)
      usbCarrier += 50;
    else
      continue; //don't update the frequency or the display

    si5351bx_setfreq(0, usbCarrier);
    printCarrierFreq(usbCarrier);

    active_delay(100);
  }

  EEPROM.put(USB_CAL, usbCarrier);
  si5351bx_setfreq(0, usbCarrier);
  setFrequency(frequency);

  printStateChange(F("Carrier set!"));
}

void menuSetupCwTone(int btn) {
  int knob = 0;
  int prev_sideTone;

  if (!btn) {
    printLineF2(F("Setup:CW Tone  \x7E"));
    return;
  }

  prev_sideTone = sideTone;
  printLineF2(F("Tune CW tone"));
  printLineF1(F("PTT to confirm. "));
  active_delay(1000);
  tone(PIN_CW_TONE, sideTone);

  //disable all clock 1 and clock 2
  while (digitalRead(PIN_PTT) == HIGH && !btnDown())
  {
    knob = enc_read();

    if (knob > 0 && sideTone < 2000)
      sideTone += 10;
    else if (knob < 0 && sideTone > 100 )
      sideTone -= 10;
    else
      continue; //don't update the frequency or the display

    tone(PIN_CW_TONE, sideTone);
    itoa(sideTone, b, 10);
    printLine2(b);

    checkCAT();
    active_delay(20);
  }
  noTone(PIN_CW_TONE);
  //save the setting
  if (digitalRead(PIN_PTT) == LOW) {
    EEPROM.put(CW_SIDETONE, sideTone);
    printStateChange(F("Sidetone set!"));
  }
  else {
    sideTone = prev_sideTone;
    printStateChange(F(""));
  }

}

void menuReadADC(int btn) {
  int adc;

  if (!btn) {
    printLineF2(F("Setup:Read ADC \x7E"));
    return;
  }
  delay(500);

  while (!btnDown()) {
    adc = analogRead(PIN_ANALOG_KEYER);
    itoa(adc, b, 10);
    printLine1(b);
  }

  printLine1("");
  updateDisplay();
}

void menuSetupCwDelay(int btn) {
  int knob = 0;
  int prev_cw_delay;

  if (!btn) {
    printLineF2(F("Setup:CW Delay \x7E"));
    return;
  }

  active_delay(500);
  prev_cw_delay = cwDelayTime;
  cwDelayTime = getValueByKnob(10, 1000, 50,  cwDelayTime, "CW Delay> ", " msec");

  printStateChange(F("CW Delay Set!"));
}

void menuSetupKeyer(int btn) {
  int tmp_key, knob;

  if (!btn) {
    if (!Iambic_Key)
      printLineF2(F("Setup:CW(Hand)\x7E"));
    else if (keyerControl & IAMBICB)
      printLineF2(F("Setup:CW(IambA)\x7E"));
    else
      printLineF2(F("Setup:CW(IambB)\x7E"));
    return;
  }

  active_delay(500);

  if (!Iambic_Key)
    tmp_key = 0; //hand key
  else if (keyerControl & IAMBICB)
    tmp_key = 2; //Iambic B
  else
    tmp_key = 1;

  while (!btnDown())
  {
    knob = enc_read();
    if (knob < 0 && tmp_key > 0)
      tmp_key--;
    if (knob > 0)
      tmp_key++;

    if (tmp_key > 2)
      tmp_key = 0;

    if (tmp_key == 0)
      printLineF1(F("Hand Key?"));
    else if (tmp_key == 1)
      printLineF1(F("Iambic A?"));
    else if (tmp_key == 2)
      printLineF1(F("Iambic B?"));
  }

  active_delay(500);
  if (tmp_key == 0)
    Iambic_Key = false;
  else if (tmp_key == 1) {
    Iambic_Key = true;
    keyerControl &= ~IAMBICB;
  }
  else if (tmp_key == 2) {
    Iambic_Key = true;
    keyerControl |= IAMBICB;
  }

  EEPROM.put(PIN_CW_KEY_TYPE, tmp_key);

  printStateChange(F("Keyer Set!"));
}

