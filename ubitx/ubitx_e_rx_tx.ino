/**
   ritEnable is called with a frequency parameter that determines
   what the tx frequency will be
*/
void ritEnable(unsigned long f) {
  ritOn = 1;
  //save the non-rit frequency back into the VFO memory
  //as RIT is a temporary shift, this is not saved to EEPROM
  ritTxFrequency = f;
}

// this is called by the RIT menu routine
void ritDisable() {
  if (ritOn) {
    ritOn = 0;
    setFrequency(ritTxFrequency);
    updateDisplay();
  }
}


/**
   Select the properly tx harmonic filters

   LPF_A -> KT1 -> OFF = 21-30 MHz, ON = LPF_B
   LPF_B -> KT2 -> OFF = 14-18 MHz, ON = LPF_C
   LPF_C -> KT3 -> OFF = 3.5-5 MHz, ON = 7-10 MHz
*/
void setTXFilters(unsigned long freq) {
  if (freq > 21000000L) {
    digitalWrite(PIN_TX_LPF_A, 0);
    digitalWrite(PIN_TX_LPF_B, 0);
    digitalWrite(PIN_TX_LPF_C, 0);
  } else if (freq >= 14000000L) {
    digitalWrite(PIN_TX_LPF_A, 1);
    digitalWrite(PIN_TX_LPF_B, 0);
    digitalWrite(PIN_TX_LPF_C, 0);
  } else if (freq > 7000000L) {
    digitalWrite(PIN_TX_LPF_A, 1);
    digitalWrite(PIN_TX_LPF_B, 1);
    digitalWrite(PIN_TX_LPF_C, 0);
  } else {
    digitalWrite(PIN_TX_LPF_A, 1);
    digitalWrite(PIN_TX_LPF_B, 1);
    digitalWrite(PIN_TX_LPF_C, 1);
  }
}


/**
   Configure the radio to a particular frequeny, sideband and set up the transmit filters

   The carrier oscillator of the detector/modulator is permanently fixed at
   uppper sideband. The sideband selection is done by placing the second oscillator
   either 12 Mhz below or above the 45 Mhz signal thereby inverting the sidebands
   through mixing of the second local oscillator.
*/
void setFrequency(unsigned long f) {
  setTXFilters(f);

  if (isUSB) {
    si5351bx_setfreq(2, firstIF  + f);
    si5351bx_setfreq(1, firstIF + usbCarrier);
  }
  else {
    si5351bx_setfreq(2, firstIF + f);
    si5351bx_setfreq(1, firstIF - usbCarrier);
  }

  frequency = f;
}


/**
   startTx is called by the PTT, cw keyer and CAT protocol to
   put the uBitx in tx mode. It takes care of rit settings, sideband settings
   Note: In cw mode, doesnt key the radio, only puts it in tx mode
   CW offest is calculated as lower than the operating frequency when in LSB mode, and vice versa in USB mode
*/
void startTx(byte txMode) {
  unsigned long tx_freq = 0;

  digitalWrite(PIN_TX_RX, 1);
  inTx = 1;

  if (ritOn) {
    //save the current as the rx frequency
    ritRxFrequency = frequency;
    setFrequency(ritTxFrequency);
  }
  else
  {
    if (splitOn == 1) {
      if (vfoActive == VFO_B) {
        vfoActive = VFO_A;
        isUSB = isUsbVfoA;
        frequency = vfoA;
      }
      else if (vfoActive == VFO_A) {
        vfoActive = VFO_B;
        frequency = vfoB;
        isUSB = isUsbVfoB;
      }
    }
    setFrequency(frequency);
  }

  if (txMode == TX_CW) {
    //turn off the second local oscillator and the bfo
    si5351bx_setfreq(0, 0);
    si5351bx_setfreq(1, 0);

    //shif the first oscillator to the tx frequency directly
    //the key up and key down will toggle the carrier unbalancing
    //the exact cw frequency is the tuned frequency + sidetone
    if (isUSB)
      si5351bx_setfreq(2, frequency + sideTone);
    else
      si5351bx_setfreq(2, frequency - sideTone);
  }
  updateDisplay();
}

void stopTx() {
  inTx = 0;

  digitalWrite(PIN_TX_RX, 0);           //turn off the tx
  si5351bx_setfreq(0, usbCarrier);  //set back the cardrier oscillator anyway, cw tx switches it off

  if (ritOn)
    setFrequency(ritRxFrequency);
  else {
    if (splitOn == 1) {
      //vfo Change
      if (vfoActive == VFO_B) {
        vfoActive = VFO_A;
        frequency = vfoA;
        isUSB = isUsbVfoA;
      }
      else if (vfoActive == VFO_A) {
        vfoActive = VFO_B;
        frequency = vfoB;
        isUSB = isUsbVfoB;
      }
    }
    setFrequency(frequency);
  }
  updateDisplay();
}

