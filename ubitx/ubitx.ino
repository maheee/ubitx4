#include <Wire.h>
#include <EEPROM.h>


/**
   Helper
*/
// short cut to print first or second line
#define printLineF1(x) (printLineF(1, x))
#define printLineF2(x) (printLineF(0, x))

#define printLine1(x) (printLine(1, x))
#define printLine2(x) (printLine(0, x))


/**
    Pin Setup
*/
#define PIN_ENC_A (A0)
#define PIN_ENC_B (A1)
#define PIN_FUNC_FBUTTON (A2)
#define PIN_PTT   (A3)
// A4 ->  Si5351 as I2C
// A5 ->  Si5351 as I2C
#define PIN_ANALOG_KEYER (A6)
// A7 -> unused

#define PIN_TX_RX (7)
#define PIN_CW_TONE (6)
#define PIN_TX_LPF_A (5)
#define PIN_TX_LPF_B (4)
#define PIN_TX_LPF_C (3)
#define PIN_CW_KEY (2)

#define PIN_LCD_RESET (8)
#define PIN_LCD_ENABLE (9)
#define PIN_LCD_D4 (10)
#define PIN_LCD_D5 (11)
#define PIN_LCD_D6 (12)
#define PIN_LCD_D7 (13)


/**
   LCD Setup
*/
#include <LiquidCrystal.h>
LiquidCrystal lcd(PIN_LCD_RESET, PIN_LCD_ENABLE, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7);


/**
   Global buffers as  kitchen counters where we can slice and dice our strings. These strings are mostly used to control the display or handle
   the input and output from the USB port. We must keep a count of the bytes used while reading
   the serial port as we can easily run out of buffer space. This is done in the serial_in_count variable.
*/
char c[30], b[30];
char printBuff[2][31];  //mirrors what is showing on the two lines of the display
int count = 0;          //to generally count ticks, loops, etc


/**
   EEPROM storage offsets
*/
#define MASTER_CAL   0
#define LSB_CAL      4
#define USB_CAL      8
#define SIDE_TONE   12
#define VFO_A       16
#define VFO_B       20
#define CW_SIDETONE 24
#define CW_SPEED    28

// These are defines for the new features back-ported from KD8CEC's software
// these start from beyond 256 as Ian, KD8CEC has kept the first 256 bytes free for the base version
#define VFO_A_MODE 256 // 2: LSB, 3: USB
#define VFO_B_MODE 257

//values that are stored for the VFO modes
#define VFO_MODE_LSB 2
#define VFO_MODE_USB 3

// handkey, iambic a, iambic b : 0,1,2f
#define PIN_CW_KEY_TYPE 358


/**
   Oscillator Setup
*/
// the second oscillator should ideally be at 57 MHz, however, the crystal filter's center frequency
// is shifted down a little due to the loading from the impedance matching L-networks on either sides
#define SECOND_OSC_USB (56995000l)
#define SECOND_OSC_LSB (32995000l)

//these are the two default USB and LSB frequencies. The best frequencies depend upon your individual taste and filter shape
#define INIT_USB_FREQ (11996500l)
// limits the tuning and working range of the ubitx between 3 MHz and 30 MHz
#define LOWEST_FREQ     (100000l)
#define HIGHEST_FREQ  (30000000l)

//we directly generate the CW by programmin the Si5351 to the cw tx frequency, hence, both are different modes
//these are the parameter passed to startTx
#define TX_SSB 0
#define TX_CW 1

char ritOn = 0;
char vfoActive = VFO_A;
int8_t meter_reading = 0; // a -1 on meter makes it invisible
unsigned long vfoA = 7150000L, vfoB = 14200000L, sideTone = 800, usbCarrier;
char isUsbVfoA = 0, isUsbVfoB = 1;
unsigned long frequency, ritRxFrequency, ritTxFrequency;  //frequency is the current frequency on the dial
unsigned long firstIF =   45000000L;

//these are variables that control the keyer behaviour
int cwSpeed = 100; //this is actuall the dot period in milliseconds
extern int32_t calibration;
byte cwDelayTime = 60;
bool Iambic_Key = true;
#define IAMBICB 0x10 // 0 for Iambic A, 1 for Iambic B
unsigned char keyerControl = IAMBICB;


/**
   Transceiver state variables
*/
boolean txCAT = false;        // turned on if the transmitting due to a CAT command
char inTx = 0;                // it is set to 1 if in transmit mode (whatever the reason : cw, ptt or cat)
char splitOn = 0;             // working split, uses VFO B as the transmit frequency, (NOT IMPLEMENTED YET)
char keyDown = 0;             // in cw mode, denotes the carrier is being transmitted
char isUSB = 0;               // upper sideband was selected, this is reset to the default for the frequency when it crosses the frequency border of 10 MHz
byte menuOn = 0;              // set to 1 when the menu is being displayed, if a menu item sets it to zero, the menu is exited
unsigned long cwTimeout = 0;  // milliseconds to go before the cw transmit line is released and the radio goes back to rx mode
unsigned long dbgCount = 0;   // not used now
unsigned char txFilter = 0;   // which of the four transmit filters are in use
boolean modeCalibrate = false;// this mode of menus shows extended menus to calibrate the oscillators and choose the proper beat frequency


/**
   Our own delay. During any delay, the raduino should still be processing a few times.
*/
void active_delay(int delay_by) {
  unsigned long timeStart = millis();

  while (millis() - timeStart <= delay_by) {
    //Background Work
    checkCAT();
  }
}


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
   Basic User Interface Routines. These check the front panel for any activity
*/

/**
   The PTT is checked only if we are not already in a cw transmit session
   If the PTT is pressed, we shift to the ritbase if the rit was on
   flip the T/R line to T and update the display to denote transmission
*/

void checkPTT() {
  //we don't check for ptt when transmitting cw
  if (cwTimeout > 0)
    return;

  if (digitalRead(PIN_PTT) == 0 && inTx == 0) {
    startTx(TX_SSB);
    active_delay(50); //debounce the PTT
  }

  if (digitalRead(PIN_PTT) == 1 && inTx == 1)
    stopTx();
}

void checkButton() {
  int i, t1, t2, knob, new_knob;

  //only if the button is pressed
  if (!btnDown())
    return;
  active_delay(50);
  if (!btnDown()) //debounce
    return;

  doMenu();
  //wait for the button to go up again
  while (btnDown())
    active_delay(10);
  active_delay(50);//debounce
}


/**
   The tuning jumps by 50 Hz on each step when you tune slowly
   As you spin the encoder faster, the jump size also increases
   This way, you can quickly move to another band by just spinning the
   tuning knob
*/
void doTuning() {
  int s;
  unsigned long prev_freq;

  s = enc_read();
  if (s != 0) {
    prev_freq = frequency;

    if (s > 4)
      frequency += 10000l;
    else if (s > 2)
      frequency += 500l;
    else if (s > 0)
      frequency +=  50l;
    else if (s > -2)
      frequency -= 50l;
    else if (s > -4)
      frequency -= 500l;
    else
      frequency -= 10000l;

    if (prev_freq < 10000000l && frequency > 10000000l)
      isUSB = true;

    if (prev_freq > 10000000l && frequency < 10000000l)
      isUSB = false;

    setFrequency(frequency);
    updateDisplay();
  }
}


/**
   RIT only steps back and forth by 100 hz at a time
*/
void doRIT() {
  unsigned long newFreq;

  int knob = enc_read();
  unsigned long old_freq = frequency;

  if (knob < 0)
    frequency -= 100l;
  else if (knob > 0)
    frequency += 100;

  if (old_freq != frequency) {
    setFrequency(frequency);
    updateDisplay();
  }
}


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
      if (vfoA > 10000000l)
        isUsbVfoA = 1;
      else
        isUsbVfoA = 0;
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
      if (vfoA > 10000000l)
        isUsbVfoB = 1;
      else
        isUsbVfoB = 0;
  }

  //set the current mode
  isUSB = isUsbVfoA;

  //The keyer type splits into two variables
  EEPROM.get(PIN_CW_KEY_TYPE, x);

  if (x == 0)
    Iambic_Key = false;
  else if (x == 1) {
    Iambic_Key = true;
    keyerControl &= ~IAMBICB;
  }
  else if (x == 2) {
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


void setup() {
  Serial.begin(38400);
  Serial.flush();
  lcd.begin(16, 2);

  //we print this line so this shows up even if the raduino
  //crashes later in the code
  printLineF2(F("uBITX v4.?"));
  //active_delay(500);

  //  initMeter(); //not used in this build
  initSettings();
  initPorts();
  initOscillators();

  frequency = vfoA;
  setFrequency(vfoA);
  updateDisplay();

  if (btnDown())
    factory_alignment();
}


/**
   The loop checks for keydown, ptt, function button and tuning.
*/
void loop() {
  cwKeyer();
  if (!txCAT)
    checkPTT();
  checkButton();

  //tune only when not tranmsitting
  if (!inTx) {
    if (ritOn)
      doRIT();
    else
      doTuning();
  }

  //we check CAT after the encoder as it might put the radio into TX
  checkCAT();
}

