int enc_prev_state = 3;

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

byte enc_state (void) {
  return (analogRead(PIN_ENC_A) > 500 ? 1 : 0) + (analogRead(PIN_ENC_B) > 500 ? 2 : 0);
}

int enc_read(void) {
  int result = 0;
  byte newState;
  int enc_speed = 0;

  long stop_by = millis() + 50;

  while (millis() < stop_by) { // check if the previous state was stable
    newState = enc_state(); // Get current state

    if (newState != enc_prev_state)
      delay (1);

    if (enc_state() != newState || newState == enc_prev_state)
      continue;
    //these transitions point to the encoder being rotated anti-clockwise
    if ((enc_prev_state == 0 && newState == 2) ||
        (enc_prev_state == 2 && newState == 3) ||
        (enc_prev_state == 3 && newState == 1) ||
        (enc_prev_state == 1 && newState == 0)) {
      result--;
    }
    //these transitions point o the enccoder being rotated clockwise
    if ((enc_prev_state == 0 && newState == 1) ||
        (enc_prev_state == 1 && newState == 3) ||
        (enc_prev_state == 3 && newState == 2) ||
        (enc_prev_state == 2 && newState == 0)) {
      result++;
    }
    enc_prev_state = newState; // Record state for next pulse interpretation
    enc_speed++;
    active_delay(1);
  }
  return (result);
}

