/**
    Menus
    The Radio menus are accessed by tapping on the function button.
    The main loop() constantly looks for a button press and calls doMenu() when it detects a function button press.
*/

//this is used by the si5351 routines in the ubitx_5351 file
extern int32_t calibration;
extern uint32_t si5351bx_vcoa;


int getValueByKnob(int minimum, int maximum, int step_size,  int initial, char* prefix, char *postfix) {
  int knob = 0;
  int knob_value = initial;

  waitForFuncButtonUp();

  printIntValue1(prefix, postfix, knob_value);

  while (!funcButtonState()) {
    knob = enc_read();

    if (knob != 0) {
      if (knob < 0) {
        knob_value -= step_size;
      } else if (knob > 0) {
        knob_value += step_size;
      }

      if (knob_value < minimum) {
        knob_value = minimum;
      } else if (knob_value > maximum) {
        knob_value = maximum;
      }

      printIntValue1(prefix, postfix, knob_value);
    }
    active_delay(20);
  }

  return knob_value;
}

void doMenu() {
  int select = 0;
  int menuOn = 2;

  waitForFuncButtonUp();

  while (menuOn) {
    int btnState = funcButtonState();
    select += enc_read();

    if (select > 150) {
      select = 150;
    }
    if (select < 0) {
      select = 0;
    }

    switch (select / 10) {
      case 1:
        menuBand(btnState);
        break;
      case 2:
        menuRit_print();
        if (btnState) {
          menuRit_toggle();
        }
        break;
      case 3:
        menuVfo_print();
        if (btnState) {
          menuVfo_toggle();
        }
        break;
      case 4:
        menuSideband_print();
        if (btnState) {
          menuSideband_toggle();
        }
        break;
      case 5:
        menuSplit_print();
        if (btnState) {
          menuSplit_toggle();
        }
        break;
      case 6:
        menuCWSpeed(btnState);
        break;
      case 8:
        menuSetupCalibration(btnState);
        break;
      case 9:
        menuSetupCarrier(btnState);
        break;
      case 10:
        menuSetupCwTone(btnState);
        break;
      case 11:
        menuSetupCwDelay(btnState);
        break;
      case 12:
        menuReadADC(btnState);
        break;
      case 13:
        menuSetupKeyer(btnState);
        break;
      default:
        menuExit(btnState);
    }
    if (btnState) {
      waitForFuncButtonUp();
      return;
    } else {
      active_delay(20);
    }
  }
}

