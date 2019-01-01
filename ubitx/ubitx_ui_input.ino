/**
   The A7 And A6 are purely analog lines on the Arduino Nano
   These need to be pulled up externally using two 10 K resistors

   The enc_state returns a two-bit number such that each bit reflects the current
   value of each of the two phases of the encoder

   The enc_read returns the number of net pulses counted over 50 msecs.
   If the puluses are -ve, they were anti-clockwise, if they are +ve, the
   were in the clockwise directions. Higher the pulses, greater the speed
   at which the enccoder was spun
*/

//returns true if the button is pressed
int btnDown() {
  if (digitalRead(PIN_FUNC_FBUTTON) == HIGH)
    return 0;
  else
    return 1;
}

int funcButtonState() {
  static int buttonState = HIGH;
  static int lastButtonState = HIGH;
  static unsigned long lastDebounceTime = 0;
  static unsigned long debounceDelay = 50;

  int reading = digitalRead(PIN_FUNC_FBUTTON);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != buttonState) {
      buttonState = reading;
    }
  }

  lastButtonState = reading;

  return buttonState == HIGH ? 0 : 1;
}

void waitForFuncButtonUp() {
  while (funcButtonState() == 1) {
    active_delay(50);
  }
}

byte enc_state (void) {
  // 0 none
  // 1 A
  // 2 B
  // 3 both
  return
    (analogRead(PIN_ENC_A) > 500 ? 1 : 0) +
    (analogRead(PIN_ENC_B) > 500 ? 2 : 0);
}

int enc_read(void) {
  static byte prevState = 0;
  byte newState = prevState;

  unsigned long stop_by = millis() + 50;
  int result = 0;

  while (millis() < stop_by) {
    prevState = newState;
    newState = enc_state();

    if (newState == prevState) {
      active_delay(1);
      continue;
    }

    //these transitions point to the encoder being rotated anti-clockwise
    if ((prevState == 0 && newState == 2) ||
        (prevState == 2 && newState == 3) ||
        (prevState == 3 && newState == 1) ||
        (prevState == 1 && newState == 0)) {
      result--;
    }
    //these transitions point o the enccoder being rotated clockwise
    if ((prevState == 0 && newState == 1) ||
        (prevState == 1 && newState == 3) ||
        (prevState == 3 && newState == 2) ||
        (prevState == 2 && newState == 0)) {
      result++;
    }
  }
#ifdef ENC_SPEED_MULTIPLIER
  result *= ENC_SPEED_MULTIPLIER;
#endif
  return result;
}

