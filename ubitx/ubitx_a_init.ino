/**
   The settings are read from EEPROM. The first time around, the values may not be
   present or out of range, in this case, some intelligent defaults are copied into the
   variables.
*/
void initSettings() {
  byte x;
  //read the settings from the eeprom and restore them
  //if the readings are off, then set defaults
  EEPROM.get(MASTER_CAL, calibration);
  EEPROM.get(USB_CAL, usbCarrier);
  EEPROM.get(VFO_A, vfoA);
  EEPROM.get(VFO_B, vfoB);
  EEPROM.get(CW_SIDETONE, sideTone);
  EEPROM.get(CW_SPEED, cwSpeed);

  if (usbCarrier > 12000000l || usbCarrier < 11990000l)
    usbCarrier = 11997000l;
  if (vfoA > 35000000l || 3500000l > vfoA)
    vfoA = 7150000l;
  if (vfoB > 35000000l || 3500000l > vfoB)
    vfoB = 14150000l;
  if (sideTone < 100 || 2000 < sideTone)
    sideTone = 800;
  if (cwSpeed < 10 || 1000 < cwSpeed)
    cwSpeed = 100;

  //The VFO modes are read in as either 2 (USB) or 3(LSB), 0, the default is taken as 'uninitialized
  EEPROM.get(VFO_A_MODE, x);

  switch (x) {
    case VFO_MODE_USB:
      isUsbVfoA = 1;
      break;
    case VFO_MODE_LSB:
      isUsbVfoA = 0;
      break;
    default:
      if (vfoA > 10000000l) {
        isUsbVfoA = 1;
      } else {
        isUsbVfoA = 0;
      }
  }

  EEPROM.get(VFO_B_MODE, x);
  switch (x) {
    case VFO_MODE_USB:
      isUsbVfoB = 1;
      break;
    case VFO_MODE_LSB:
      isUsbVfoB = 0;
      break;
    default:
      if (vfoA > 10000000l) {
        isUsbVfoB = 1;
      } else {
        isUsbVfoB = 0;
      }
  }

  //set the current mode
  isUSB = isUsbVfoA;

  //The keyer type splits into two variables
  EEPROM.get(PIN_CW_KEY_TYPE, x);

  if (x == 0) {
    Iambic_Key = false;
  } else if (x == 1) {
    Iambic_Key = true;
    keyerControl &= ~IAMBICB;
  } else if (x == 2) {
    Iambic_Key = true;
    keyerControl |= IAMBICB;
  }
}


void initPorts() {
  analogReference(DEFAULT);

  //??
  pinMode(PIN_ENC_A, INPUT_PULLUP);
  pinMode(PIN_ENC_B, INPUT_PULLUP);
  pinMode(PIN_FUNC_FBUTTON, INPUT_PULLUP);

  //configure the function button to use the external pull-up
  //  pinMode(PIN_FUNC_FBUTTON, INPUT);
  //  digitalWrite(PIN_FUNC_FBUTTON, HIGH);

  pinMode(PIN_PTT, INPUT_PULLUP);
  pinMode(PIN_ANALOG_KEYER, INPUT_PULLUP);

  pinMode(PIN_CW_TONE, OUTPUT);
  digitalWrite(PIN_CW_TONE, 0);

  pinMode(PIN_TX_RX, OUTPUT);
  digitalWrite(PIN_TX_RX, 0);

  pinMode(PIN_TX_LPF_A, OUTPUT);
  pinMode(PIN_TX_LPF_B, OUTPUT);
  pinMode(PIN_TX_LPF_C, OUTPUT);
  digitalWrite(PIN_TX_LPF_A, 0);
  digitalWrite(PIN_TX_LPF_B, 0);
  digitalWrite(PIN_TX_LPF_C, 0);

  pinMode(PIN_CW_KEY, OUTPUT);
  digitalWrite(PIN_CW_KEY, 0);
}

