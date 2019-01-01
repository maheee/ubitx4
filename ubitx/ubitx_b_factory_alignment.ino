/**
   This procedure is only for those who have a signal generator/transceiver tuned to exactly 7.150 and a dummy load
*/
void btnWaitForClick() {
  while (!btnDown())
    active_delay(50);
  while (btnDown())
    active_delay(50);
  active_delay(50);
}

void testFrequency(long frequency) {
  setFrequency(frequency);
  updateDisplay();
  while (!funcButtonState()) {
    checkPTT();
    active_delay(20);
  }
  waitForFuncButtonUp();
}

void factory_alignment() {
  printInfoF(F("#1: Calibration"), F(""));
  calibrateClock();

  if (calibration == 0) {
    printStateChangeF(F("Setup Aborted"));
    return;
  }

  //move it away to 7.160 for an LSB signal
  setFrequency(7170000l);
  updateDisplay();

  printInfoF(F("#2: BFO"), F(""));

  usbCarrier = 11994999l;
  menuSetupCarrier(1);

  if (usbCarrier == 11994999l) {
    printStateChangeF(F("Setup Aborted"));
    return;
  }

  printInfoF(F("#3: Test 3.5MHz"), F(""));
  isUSB = false;
  testFrequency(3500000l);

  printInfoF(F("#4: Test 7MHz"), F(""));
  testFrequency(7150000l);

  printInfoF(F("#5: Test 14MHz"), F(""));
  isUSB = true;
  testFrequency(14000000l);

  printInfoF(F("#6: Test 28MHz"), F(""));
  testFrequency(28000000l);
  
  printInfoF(F("Alignment done!"), F(""));

  isUSB = false;
  setFrequency(7150000l);
  updateDisplay();
}

