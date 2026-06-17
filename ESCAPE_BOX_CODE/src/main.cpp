#include <Arduino.h>
#include <EEPROM.h>
#include <SPI.h>
#include <MFRC522.h>
//#include <Adafruit_BMP280.h>
#include <DFMiniMp3.h>
#include <ezButton.h>
#include <Servo.h>
#include <BMP280_DEV.h>

#include "puzzles.h"
#include "mp3.h"

enum State
{
  BEGIN,
  CABLES,
  RFID_EAGLE,
  RFID_OWL,
  RFID_SEAGUL,
  RFID_STORK,
  RFID_DUCK,
  RFID_SPARROW,
  BLOW,
  TOWCAR,
  BUTTONS,
  WOODPECKER,
  SOLVED
};
State puzzle_state = BEGIN;
boolean state_change = true;
boolean report_state = true;

uint64_t last_reset = 0;
uint64_t lastMillis = 0;
uint64_t waitTime = 1000;
uint64_t winDebounce = 1000;

bool hasSaid = false;
bool hasFailed = false;

void ChangeState(State s);
void StateChangeHandler();

Servo servo[3];

void setup()
{
  Serial.begin(115200);
  Serial.println("Booting");
  SPI.begin();

  // LED MATRIX
  pinMode(LED_MATRIX, OUTPUT);
  digitalWrite(LED_MATRIX, LOW);

  // SERVO CONFIGURATION
  servo[0].attach(SERVO0);
  servo[1].attach(SERVO1);
  servo[2].attach(SERVO2);

  for (int i = 0; i < 3; i++)
  {
    servo[i].write(SERVO_CLOSED);
  }

  // BUTTONS DEBOUNCE
  for (int i = 0; i < BUTTON_COUNT; i++)
  {
    buttonArray[i].setDebounceTime(50);
    reedArray[i].setDebounceTime(200);
  }
  knockSensor.setDebounceTime(10);

  // MP3 CONFIGURATION
  mp3.begin();
  mp3.setEq(DfMp3_Eq_Normal);
  mp3.setVolume(20);
  int count = mp3.getTotalTrackCount(DfMp3_PlaySource_Sd);
  Serial.print("files: ");
  Serial.println(count);

  // BMP CONFIGURATION
  bmp280.begin(BMP280_I2C_ALT_ADDR);           // Default initialisation with alternative I2C address (0x76), place the BMP280 into SLEEP_MODE
  bmp280.setPresOversampling(OVERSAMPLING_X1); // Set the pressure oversampling to X4
  bmp280.setTempOversampling(OVERSAMPLING_X1); // Set the temperature oversampling to X1
  bmp280.setIIRFilter(IIR_FILTER_OFF);         // Set the IIR filter to setting 4
  bmp280.setTimeStandby(TIME_STANDBY_05MS);    // Set the standby time to 2 seconds
  bmp280.startNormalConversion();
  // RFID SELFTEST
  rfid.PCD_Init();
  delay(4);
  rfid.PCD_DumpVersionToSerial();

  // KNOCK BUFFOR FLUSH
  for (uint8_t i = 0; i < 6; i++)
  {
    knockBuffer.push(0);
  }

  // EEPROM
  // EEPROM.write(0, 0);
  delay(3000);
  ChangeState(static_cast<State>(EEPROM.read(0)));
}

void loop()
{

  StateChangeHandler();                  // chandler for changing state in main state machine
  mp3.loop();                            // notification handler for mp3
  knockSensor.loop();                    // knock sensor loop (chcecks state)
  for (int i = 0; i < BUTTON_COUNT; i++) // buttons loop (chcecks buttons state)
  {
    buttonArray[i].loop();
    reedArray[i].loop();
  }

  // state machine loop
  switch (puzzle_state)
  {
  case BEGIN:
    for (int i = 0; i < 3; i++)
    {
      servo[i].write(SERVO_CLOSED);
    }
    Serial.println("czekam...");
    delay(waitTime);
    ChangeState(CABLES);
    break;
  case CABLES:
    if (!CheckCables())
      lastMillis = millis();

    if (millis() - lastMillis >= winDebounce)
      ChangeState(RFID_EAGLE);

    break;
  case RFID_EAGLE:
    if (millis() - lastMillis >= SOUND_INTERVAL)
    {
      Serial.println("playing Eagle sound");
      mp3.playMp3FolderTrack(EAGLE);
      lastMillis = millis();
    }
    if (CheckRFID(UID_EAGLE))
      ChangeState(RFID_OWL);
    break;
  case RFID_OWL:
    if (millis() - lastMillis >= SOUND_INTERVAL)
    {
      Serial.println("playing Owl sound");
      mp3.playMp3FolderTrack(OWL);
      lastMillis = millis();
    }
    if (CheckRFID(UID_OWL))
      ChangeState(RFID_SEAGUL);
    break;
  case RFID_SEAGUL:
    if (millis() - lastMillis >= SOUND_INTERVAL)
    {
      Serial.println("playing Seagul sound");
      mp3.playMp3FolderTrack(SEAGUL);
      lastMillis = millis();
    }
    if (CheckRFID(UID_SEAGUL))
      ChangeState(RFID_STORK);
    break;
  case RFID_STORK:
    if (millis() - lastMillis >= SOUND_INTERVAL)
    {
      Serial.println("playing Stork sound");
      mp3.playMp3FolderTrack(STORK);
      lastMillis = millis();
    }
    if (CheckRFID(UID_STORK))
      ChangeState(RFID_DUCK);
    break;
  case RFID_DUCK:
    if (millis() - lastMillis >= SOUND_INTERVAL)
    {
      Serial.println("playing Duck sound");
      mp3.playMp3FolderTrack(DUCK);
      lastMillis = millis();
    }
    if (CheckRFID(UID_DUCK))
      ChangeState(RFID_SPARROW);
    break;
  case RFID_SPARROW:
    if (millis() - lastMillis >= SOUND_INTERVAL)
    {
      Serial.println("playing Sparrow sound");
      mp3.playMp3FolderTrack(SPARROW);
      lastMillis = millis();
    }
    if (CheckRFID(UID_SPARROW))
      ChangeState(BLOW);
    break;
  case BLOW:
    if (ChceckBlowSensor(5))
      ChangeState(TOWCAR);
    break;
  case TOWCAR:
    UpdateCarRide();
    if (CheckCarRide())
      ChangeState(BUTTONS);
    break;
  case BUTTONS:
    UpdateButtons();
    if (CheckButtons())
      ChangeState(WOODPECKER);
    break;
  case WOODPECKER:
    if (millis() - lastMillis >= 10000)
    {
      Serial.println("playing Woodpecker sound");
      mp3.playMp3FolderTrack(KNOCK_SOUND);
      lastMillis = millis();
    }
    UpdateKnock();
    if (CheckKnock())
      ChangeState(SOLVED);
    break;
  case SOLVED:
    UpdateButtonsAfterWin();
    if (CheckButtonsAfterWin())
      ChangeState(BEGIN);
    break;
  }
}

void ChangeState(State s)
{
  if (puzzle_state != s)
  {
    puzzle_state = s;
    state_change = true;
    report_state = true;
    EEPROM.write(0, s);
  }
}

void StateChangeHandler()
{ // state machine (happens only one time after change)
  if (state_change)
  {
    switch (puzzle_state)
    {
    case BEGIN:
      Serial.println("BEGIN");
      break;
    case CABLES:
      lastMillis = millis();
      Serial.println("CABLES");
      break;
    case RFID_EAGLE:
      mp3.playMp3FolderTrack(EAGLE);
      lastMillis = millis();
      Serial.println("RFID_EAGLE");
      break;
    case RFID_OWL:
      mp3.playMp3FolderTrack(BIRDS_SOLVED);
      delay(2000);
      mp3.playMp3FolderTrack(OWL);
      lastMillis = millis();
      Serial.println("RFID_OWL");
      break;
    case RFID_SEAGUL:
      mp3.playMp3FolderTrack(BIRDS_SOLVED);
      delay(2000);
      mp3.playMp3FolderTrack(SEAGUL);
      lastMillis = millis();
      Serial.println("RFID_SEAGUL");
      break;
    case RFID_STORK:
      mp3.playMp3FolderTrack(BIRDS_SOLVED);
      delay(2000);
      mp3.playMp3FolderTrack(STORK);
      lastMillis = millis();
      Serial.println("RFID_STORK");
      break;
    case RFID_DUCK:
      mp3.playMp3FolderTrack(BIRDS_SOLVED);
      delay(2000);
      mp3.playMp3FolderTrack(DUCK);
      lastMillis = millis();
      Serial.println("RFID_DUCK");
      break;
    case RFID_SPARROW:
      mp3.playMp3FolderTrack(BIRDS_SOLVED);
      delay(2000);
      mp3.playMp3FolderTrack(SPARROW);
      lastMillis = millis();
      Serial.println("RFID_SPARROW");
      break;
    case BLOW:
      mp3.playMp3FolderTrack(BIRDS_SOLVED);
      Serial.println("BLOW");
      lastMillis = millis();
      delay(2000);
      digitalWrite(LED_MATRIX, HIGH);
      mp3.playMp3FolderTrack(LED_MATRIX_SOUND);
      delay(3000);
      SetAmbientPressure();
      break;
    case TOWCAR:
      mp3.playMp3FolderTrack(BLOW_SOUND);
      delay(3000);
      Serial.println("TOWCAR");
      lastMillis = millis();
      mp3.playMp3FolderTrack(CAR_SOUND);
      servo[0].write(SERVO_OPEN);
      break;
    case BUTTONS:
      mp3.playMp3FolderTrack(OPEN_BOX_SOUND);
      delay(3000);
      servo[0].write(SERVO_OPEN);
      servo[1].write(SERVO_OPEN);
      Serial.println("BUTTONS");
      lastMillis = millis();
      break;
    case WOODPECKER:
      Serial.println("WOODPECKER");
      lastMillis = SOUND_INTERVAL;
      break;
    case SOLVED:
      Serial.println("SOLVED");
      mp3.playMp3FolderTrack(WIN_SOUND);
      servo[0].write(SERVO_OPEN);
      servo[1].write(SERVO_OPEN);
      servo[2].write(SERVO_OPEN);
      break;
    default:
      break;
    }
    state_change = false;
  }
}
