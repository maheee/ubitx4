#include <Wire.h>
#include <EEPROM.h>

/**
   Switches for testing
   should all be commented out when building for prod
*/
// #define DISABLE_CAT
// #define DISABLE_KEYER
// #define DISABLE_SI5351
// #define ENC_SPEED_MULTIPLIER 2


/**
   Config
*/
#define HEADER "uBITX v4.?"

/**
   Helper
*/
// short cut to print first or second line
#define printLineF1(x) (printLineF(1, x))
#define printLineF2(x) (printLineF(0, x))

#define printLine1(x) (printLine(1, x))
#define printLine2(x) (printLine(0, x))

#define printIntValue1(prefix, suffix, value) (printIntValue(1, prefix, suffix, value))
#define printIntValue2(prefix, suffix, value) (printIntValue(0, prefix, suffix, value))

#define printLongValue1(prefix, suffix, value) (printLongValue(1, prefix, suffix, value))
#define printLongValue2(prefix, suffix, value) (printLongValue(0, prefix, suffix, value))

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
    checkCAT();
    delay(1);
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
  if (cwTimeout > 0) {
    return;
  }

  if (digitalRead(PIN_PTT) == 0 && inTx == 0) {
    startTx(TX_SSB);
    active_delay(50); //debounce the PTT
  }

  if (digitalRead(PIN_PTT) == 1 && inTx == 1) {
    stopTx();
  }
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

    if (s > 4) {
      frequency += 10000l;
    } else if (s > 2) {
      frequency += 500l;
    } else if (s > 0) {
      frequency +=  50l;
    } else if (s > -2) {
      frequency -= 50l;
    } else if (s > -4) {
      frequency -= 500l;
    } else {
      frequency -= 10000l;
    }

    if (prev_freq < 10000000l && frequency > 10000000l) {
      isUSB = true;
    }

    if (prev_freq > 10000000l && frequency < 10000000l) {
      isUSB = false;
    }

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

  if (knob < 0) {
    frequency -= 100l;
  } else if (knob > 0) {
    frequency += 100;
  }

  if (old_freq != frequency) {
    setFrequency(frequency);
    updateDisplay();
  }
}

void setup() {
  Serial.begin(38400);
  Serial.flush();
  lcd.begin(16, 2);

  // we print this line so this shows up even if the raduino
  //crashes later in the code
  printLineF2(F(HEADER));

  // initMeter(); //not used in this build
  initSettings();
  initPorts();
  initOscillators();

  frequency = vfoA;
  setFrequency(vfoA);
  updateDisplay();

  if (btnDown()) {
    factory_alignment();
  }
}

void loop() {
  printLineF2(F(HEADER));

  cwKeyer();

  if (!txCAT) {
    checkPTT();
  }

  // enter menu when button is pressed
  if (funcButtonState()) {
    doMenu();
  }

  // tune only when not tranmsitting
  if (!inTx) {
    if (ritOn) {
      doRIT();
    } else {
      doTuning();
    }
  }

  active_delay(20);
}

