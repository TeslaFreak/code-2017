/**
 * Nathan Cheek
 * 2016-11-15
 * Interface with dashboard lights, buttons, and buzzer. Read pedal sensor values.
 */
#include <FlexCAN.h>
#include <Metro.h>

/*
 * State definitions
 */
#define PCU_STATE_WAITING_BMS_IMD 1
#define PCU_STATE_WAITING_DRIVER 2
#define PCU_STATE_LATCHING 3
#define PCU_STATE_SHUTDOWN_CIRCUIT_INITIALIZED 4
#define PCU_STATE_FATAL_FAULT 5
#define TCU_STATE_WAITING_SHUTDOWN_CIRCUIT_INITIALIZED 1
#define TCU_STATE_TRACTIVE_SYSTEM_ACTIVE 2
#define TCU_STATE_TRACTIVE_SYSTEM_NOT_ACTIVE 3
#define TCU_STATE_ENABLING_INVERTER 4
#define TCU_STATE_WAITING_READY_TO_DRIVE_SOUND 5
#define TCU_STATE_READY_TO_DRIVE 6

/*
 * Pin definitions
 */
#define BTN_TOGGLE 9
#define BTN_CYCLE 10
#define BTN_BOOST 11
#define BTN_START 12
#define LED_START 7
#define LED_IMD 6
#define LED_BMS 5
#define READY_SOUND 8
#define PEDAL_SIGNAL_A A3
#define PEDAL_SIGNAL_B A4
#define PEDAL_SIGNAL_C A5

/*
 * CAN ID definitions
 */
#define PCU_STATUS 0xD0
#define TCU_STATUS 0xD1

/*
 * Timers
 */
Metro timer_printer = Metro(500);
Metro timer_state_send = Metro(50);
Metro timer_motor_controller_send = Metro(50);
Metro timer_led_start_blink_fast = Metro(150);
Metro timer_led_start_blink_slow = Metro(400);
Metro timer_ready_sound = Metro(2000);

/*
 * Global variables
 */
boolean LED_START_ACTIVE = false;
boolean LED_IMD_ACTIVE = false;
boolean LED_BMS_ACTIVE = false;
boolean READY_SOUND_ACTIVE = false;
boolean BTN_START_PRESSED = false;
uint8_t LED_START_TYPE = 0; // 0 for steady, 1 for fast blink, 2 for slow blink
uint8_t STATE = TCU_STATE_WAITING_SHUTDOWN_CIRCUIT_INITIALIZED;

FlexCAN CAN(500000);
static CAN_message_t msg;

void setup() {
  pinMode(BTN_TOGGLE, INPUT_PULLUP);
  pinMode(BTN_CYCLE, INPUT_PULLUP);
  pinMode(BTN_BOOST, INPUT_PULLUP);
  pinMode(BTN_START, INPUT_PULLUP);
  pinMode(LED_START, OUTPUT);
  pinMode(LED_IMD, OUTPUT);
  pinMode(LED_BMS, OUTPUT);
  pinMode(READY_SOUND, OUTPUT);

  CAN.begin();
  delay(1000);
  Serial.println("CAN transceiver initialized");
  Serial.begin(115200);
}

void loop() {
  /*
   * Handle incoming CAN messages
   */
  while (CAN.read(msg)) {
    if (msg.id > 0xCA) { // Ignore Motor Controller messages
      Serial.print(msg.id, HEX);
      Serial.print(": ");
      for (unsigned int i = 0; i < msg.len; i++) {
        Serial.print(msg.buf[i], HEX);
        Serial.print(" ");
      }
      Serial.println();
    }
    
    if (msg.id == PCU_STATUS) {
      if (msg.buf[0] >> 3 & 0x1) { // BMS Fault
        digitalWrite(LED_BMS, HIGH);
        Serial.println("BMS Fault detected");
      }
      if (msg.buf[0] >> 2 & 0x1) { // IMD Fault
        digitalWrite(LED_IMD, HIGH);
        Serial.println("IMD Fault detected");
      }
      // Waiting for driver to press start button, flash start button
      if (msg.buf[0] >> 4 == PCU_STATE_WAITING_DRIVER && LED_START_TYPE != 2) {
        LED_START_TYPE = 2;
        LED_START_ACTIVE = true;
        timer_led_start_blink_slow.reset();
        digitalWrite(LED_START, HIGH);
        Serial.println("Enabling slow blink start button");
      }
      // Shutdown circuit initialized, and not waiting for driver to enable inverter, enable start button steady
      if (msg.buf[0] >> 4 == PCU_STATE_SHUTDOWN_CIRCUIT_INITIALIZED && STATE != TCU_STATE_TRACTIVE_SYSTEM_ACTIVE && LED_START_TYPE != 1) {
        STATE = TCU_STATE_TRACTIVE_SYSTEM_ACTIVE; // TODO check that Tractive System is really active
        LED_START_TYPE = 1;
        LED_START_ACTIVE = true;
        timer_led_start_blink_fast.reset();
        digitalWrite(LED_START, HIGH);
        Serial.println("Enabling fast blink start button");
      }
    }

    if (msg.id == 0xAA && msg.buf[6] == 1 && STATE == TCU_STATE_ENABLING_INVERTER) {
      STATE = TCU_STATE_WAITING_READY_TO_DRIVE_SOUND;
      timer_ready_sound.reset();
      READY_SOUND_ACTIVE = true;
      digitalWrite(READY_SOUND, HIGH);
      Serial.println("Inverter enabled");
      Serial.println("RTDS enabled");
    }
  }

  /*
   * Send state over CAN
   */
  if (timer_state_send.check()) {
    msg.id = TCU_STATUS;
    msg.len = 1;
    msg.buf[0] = !digitalRead(BTN_TOGGLE) << 7 | !digitalRead(BTN_CYCLE) << 6 | !digitalRead(BTN_BOOST) << 5 | !digitalRead(BTN_START) << 4;
    msg.buf[0] |= LED_START_ACTIVE << 3 | LED_IMD_ACTIVE << 2 | LED_BMS_ACTIVE << 1 | READY_SOUND_ACTIVE;
    CAN.write(msg);
  }

  /*
   * Send a message to the Motor Controller over CAN
   */
  if (timer_motor_controller_send.check()) {
    msg.id = 0xC0;
    msg.len = 8;
    if (STATE < TCU_STATE_ENABLING_INVERTER) {
      generate_MC_message(msg.buf, 0, false, false);
    } else {
      generate_MC_message(msg.buf, 0, false, true);
    }
    CAN.write(msg);
  }

  /*
   * Blink start led
   */
  if ((LED_START_TYPE == 1 && timer_led_start_blink_fast.check()) || (LED_START_TYPE == 2 && timer_led_start_blink_slow.check())) {
    if (LED_START_ACTIVE) {
      digitalWrite(LED_START, LOW);
    } else {
      digitalWrite(LED_START, HIGH);
    }
    LED_START_ACTIVE = !LED_START_ACTIVE;
  }

  if (timer_printer.check()) {
    /*Serial.print(analogRead(PEDAL_SIGNAL_A));
    Serial.print(" ");
    Serial.print(analogRead(PEDAL_SIGNAL_B));
    Serial.print(" ");
    Serial.println(analogRead(PEDAL_SIGNAL_C));
    Serial.println(analogRead(BTN_START));*/
  }

  /*
   * Temporarily assume Tractive System is active and wait for driver to enable inverter
   */
  if (STATE == TCU_STATE_TRACTIVE_SYSTEM_ACTIVE && !BTN_START_PRESSED && !digitalRead(BTN_START)) {
    Serial.println("Enabling inverter");
    LED_START_TYPE = 0;
    LED_START_ACTIVE = true;
    digitalWrite(LED_START, HIGH);
    msg.id = 0xC0;
    msg.len = 8;    
    for(int i = 0; i < 10; i++) {
      generate_MC_message(msg.buf,0,false,true); // many enable commands
      CAN.write(msg);
    }
    generate_MC_message(msg.buf,0,false,false); // disable command
    CAN.write(msg);
    for(int i = 0; i < 10; i++) {
      generate_MC_message(msg.buf,0,false,true); // many more enable commands
      CAN.write(msg);
    }
    Serial.println("Send enable command");
    STATE = TCU_STATE_ENABLING_INVERTER;
  }

  /*
   * Deactivate Ready to Drive Sound after timer completes
   */
  if (READY_SOUND_ACTIVE && timer_ready_sound.check()) {
    READY_SOUND_ACTIVE = false;
    digitalWrite(READY_SOUND, LOW);
    STATE = TCU_STATE_READY_TO_DRIVE;
    Serial.println("RTDS deactivated");
  }

  /*
   * Differentiate separate button presses
   * TODO this needs to be completely rewritten
   */
  if (!BTN_START_PRESSED && !digitalRead(BTN_START)) {
    BTN_START_PRESSED = true;
  }
  if (BTN_START_PRESSED && digitalRead(BTN_START)) {
    BTN_START_PRESSED = false;
  }
}

void generate_MC_message(unsigned char* message, int thatTorque, boolean backwards, boolean enable) {
  message[0] = thatTorque & 0xFF;
  message[1] = thatTorque>>(8);
  message[2] = 0;
  message[3] = 0;
  message[4] = char(backwards);
  message[5] = char(enable);
  message[6] = 0;
  message[7] = 0;
}