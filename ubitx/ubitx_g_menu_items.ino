#define printCond(C, A, B) printLineF0(C ? F(A) : F(B));


int menuBand(int btn) {
  int knob = 0;
  int band;
  unsigned long offset;

  if (!btn) {
    printLineF0(F("Band Select    \x7E"));
    return;
  }

  printLineF0(F("Band Select"));
  waitForFuncButtonUp();
  ritDisable();

  while (!funcButtonState()) {
    knob = enc_read();
    if (knob != 0) {
      if (knob < 0 && frequency > 3000000l) {
        setFrequency(frequency - 200000l);
      }
      if (knob > 0 && frequency < 30000000l) {
        setFrequency(frequency + 200000l);
      }
      isUSB = frequency > 10000000l;
      updateDisplay();
    }
    active_delay(20);
  }

  printStateChangeF(F("Band set!"));
}

void menuRit_print() {
  printCond(ritOn == 1, "RIT On \x7E Off", "RIT Off \x7E On");
}

void menuRit_toggle() {
  if (ritOn == 0) {
    ritEnable(frequency);
    printStateChangeF(F("RIT is On!"));
  } else {
    ritDisable();
    printStateChangeF(F("RIT is Off!"));
  }
}

void menuVfo_print() {
  printCond(vfoActive == VFO_A, "VFO A \x7E B", "VFO B \x7E A");
}

void menuVfo_toggle() {
  if (vfoActive == VFO_B) {
    vfoB = frequency;
    isUsbVfoB = isUSB;
    EEPROM.put(VFO_B, frequency);
    if (isUsbVfoB) {
      EEPROM.put(VFO_B_MODE, VFO_MODE_USB);
    } else {
      EEPROM.put(VFO_B_MODE, VFO_MODE_LSB);
    }

    vfoActive = VFO_A;
    frequency = vfoA;
    isUSB = isUsbVfoA;
    printStateChangeF(F("Selected VFO A!"));
  } else {
    vfoA = frequency;
    isUsbVfoA = isUSB;
    EEPROM.put(VFO_A, frequency);
    if (isUsbVfoA) {
      EEPROM.put(VFO_A_MODE, VFO_MODE_USB);
    } else {
      EEPROM.put(VFO_A_MODE, VFO_MODE_LSB);
    }

    vfoActive = VFO_B;
    frequency = vfoB;
    isUSB = isUsbVfoB;
    printStateChangeF(F("Selected VFO B!"));
  }

  ritDisable();
  setFrequency(frequency);
}

void menuSideband_print() {
  printCond(isUSB == true, "USB \x7E LSB", "LSB \x7E USB");
}

void menuSideband_toggle() {
  if (isUSB == true) {
    isUSB = false;
    printStateChangeF(F("LSB Selected!"));
  } else {
    isUSB = true;
    printStateChangeF(F("USB Selected!"));
  }

  if (vfoActive == VFO_B) {
    isUsbVfoB = isUSB;
  } else {
    isUsbVfoB = isUSB;
  }
}

void menuSplit_print() {
  printCond(splitOn == 0, "Split Off \x7E On", "Split On \x7E Off");
}

void menuSplit_toggle() {
  if (splitOn == 1) {
    splitOn = 0;
    printStateChangeF(F("Split OFF!"));
  } else {
    splitOn = 1;
    if (ritOn == 1) {
      ritOn = 0;
    }
    printStateChangeF(F("Split ON!"));
  }
}

int menuCWSpeed(int btn) {
  int knob = 0;
  int wpm;

  wpm = 1200 / cwSpeed;

  if (!btn) {
    printIntValue0("CW Speed ", " WPM\x7E", wpm);
    return;
  }

  printLineF0(F("CW Speed"));
  wpm = getValueByKnob(1, 100, 1,  wpm, "         ", " WPM");

  cwSpeed = 1200 / wpm;
  EEPROM.put(CW_SPEED, cwSpeed);

  printStateChangeF(F("CW Speed set!"));
}

int calibrateClock() {
  int knob = 0;
  int32_t prev_calibration;

  waitForFuncButtonUp();

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

  printLineF0(F("Setup:Calibrate"));
  printLongValue1("10 MHz cal: ", "", calibration / 8750);

  while (!funcButtonState()) {
    if (digitalRead(PIN_PTT) == LOW && !keyDown) {
      cwKeydown();
    }
    if (digitalRead(PIN_PTT) == HIGH && keyDown) {
      cwKeyUp();
    }

    knob = enc_read();

    if (knob > 0) {
      calibration += 875;
    } else if (knob < 0) {
      calibration -= 875;
    } else {
      continue;
    }

    si5351_set_calibration(calibration);
    si5351bx_setfreq(2, 10000000l);

    printLongValue1("10 MHz cal: ", "", calibration / 8750);
  }

  cwTimeout = 0;
  keyDown = 0;
  stopTx();

  EEPROM.put(MASTER_CAL, calibration);
  initOscillators();
  setFrequency(frequency);

  printStateChangeF(F("Calibration set!"));
}

int menuSetupCalibration(int btn) {
  int knob = 0;
  int32_t prev_calibration;

  if (!btn) {
    printLineF0(F("Setup:Calibrate\x7E"));
    return 0;
  }

  printInfoF(F("Press PTT & tune"), F("to exactly 10 MHz"));
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
  printLine1(c);
}

void menuSetupCarrier(int btn) {
  int knob = 0;
  unsigned long prevCarrier;

  if (!btn) {
    printLineF0(F("Setup:BFO      \x7E"));
    return;
  }

  prevCarrier = usbCarrier;

  printInfoF(F("Tune to best Signal"), F("Press to confirm."));
  printLineF0(F("Setup:BFO"));

  usbCarrier = 11995000l;
  si5351bx_setfreq(0, usbCarrier);
  printCarrierFreq(usbCarrier);

  waitForFuncButtonUp();

  //disable all clock 1 and clock 2
  while (!funcButtonState()) {
    knob = enc_read();

    if (knob > 0)
      usbCarrier -= 50;
    else if (knob < 0)
      usbCarrier += 50;
    else
      continue; //don't update the frequency or the display

    si5351bx_setfreq(0, usbCarrier);
    printCarrierFreq(usbCarrier);

    active_delay(20);
  }

  EEPROM.put(USB_CAL, usbCarrier);
  si5351bx_setfreq(0, usbCarrier);
  setFrequency(frequency);

  printStateChangeF(F("Carrier set!"));
}

void menuSetupCwTone(int btn) {
  int knob = 0;
  int prev_sideTone;

  if (!btn) {
    printLineF0(F("Setup:CW Tone  \x7E"));
    return;
  }

  prev_sideTone = sideTone;

  printInfoF(F("Tune CW tone"), F("PTT to confirm."));
  printLineF0(F("Setup:CW Tone"));

  tone(PIN_CW_TONE, sideTone);
  printIntValue1("", "", sideTone);

  //disable all clock 1 and clock 2
  while (digitalRead(PIN_PTT) == HIGH && !funcButtonState()) {
    knob = enc_read();

    if (knob > 0 && sideTone < 2000) {
      sideTone += 10;
    } else if (knob < 0 && sideTone > 100) {
      sideTone -= 10;
    } else {
      continue;
    }

    tone(PIN_CW_TONE, sideTone);
    printIntValue1("", "", sideTone);

    active_delay(20);
  }
  noTone(PIN_CW_TONE);

  //save the setting
  if (digitalRead(PIN_PTT) == LOW) {
    EEPROM.put(CW_SIDETONE, sideTone);
    printStateChangeF(F("Sidetone set!"));
  } else {
    sideTone = prev_sideTone;
    printStateChangeF(F("Aborted!"));
  }
}

void menuReadADC(int btn) {
  int adc;

  if (!btn) {
    printLineF0(F("Setup:Read ADC \x7E"));
    return;
  }

  printLineF0(F("Setup:Read ADC"));

  while (!funcButtonState()) {
    adc = analogRead(PIN_ANALOG_KEYER);
    itoa(adc, b, 10);
    printLine1(b);
  }

  printStateChangeF(F(""));
}

void menuSetupCwDelay(int btn) {
  int knob = 0;
  int prev_cw_delay;

  if (!btn) {
    printLineF0(F("Setup:CW Delay \x7E"));
    return;
  }

  printLineF0(F("Setup:CW Delay"));
  prev_cw_delay = cwDelayTime;
  cwDelayTime = getValueByKnob(10, 1000, 50,  cwDelayTime, "        ", " ms");

  printStateChangeF(F("CW Delay Set!"));
}

void menuSetupKeyer(int btn) {
  int tmp_key, knob;

  if (!btn) {
    if (!Iambic_Key) {
      printLineF0(F("Setup:CW(Hand)\x7E"));
    } else if (keyerControl & IAMBICB) {
      printLineF0(F("Setup:CW(IambA)\x7E"));
    } else {
      printLineF0(F("Setup:CW(IambB)\x7E"));
    }
    return;
  }

  if (!Iambic_Key) {
    printLineF0(F("Setup:CW"));
  } else if (keyerControl & IAMBICB) {
    printLineF0(F("Setup:CW"));
  } else {
    printLineF0(F("Setup:CW"));
  }

  if (!Iambic_Key) {
    tmp_key = 0; //hand key
  } else if (keyerControl & IAMBICB) {
    tmp_key = 2; //Iambic B
  } else {
    tmp_key = 1;
  }

  waitForFuncButtonUp();

  while (!funcButtonState()) {
    knob = enc_read();
    if (knob < 0 && tmp_key > 0) {
      tmp_key--;
    }
    if (knob > 0) {
      tmp_key++;
    }

    if (tmp_key > 2) {
      tmp_key = 0;
    }

    if (tmp_key == 0) {
      printLineF1(F("Hand Key?"));
    } else if (tmp_key == 1) {
      printLineF1(F("Iambic A?"));
    } else if (tmp_key == 2) {
      printLineF1(F("Iambic B?"));
    }
  }

  if (tmp_key == 0) {
    Iambic_Key = false;
  } else if (tmp_key == 1) {
    Iambic_Key = true;
    keyerControl &= ~IAMBICB;
  } else if (tmp_key == 2) {
    Iambic_Key = true;
    keyerControl |= IAMBICB;
  }

  EEPROM.put(PIN_CW_KEY_TYPE, tmp_key);

  printStateChangeF(F("Keyer Set!"));
}

void menuExit(int btn) {
  if (!btn) {
    printLineF0(F("Exit Menu      \x7E"));
  } else {
    printStateChangeF(F("Exiting ..."));
  }
}

