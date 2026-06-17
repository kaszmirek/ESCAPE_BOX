
#include "puzzles.h"
#include "mp3.h"

// Cable puzzle variables
int cable[CABLES_COUNT] = {CABLE5, CABLE4, CABLE3, CABLE2, CABLE1, CABLE0};
int cableDownLim[CABLES_COUNT] = {73, 218, 364, 509, 654, 799};
int cableUpLim[CABLES_COUNT] = {217, 363, 508, 653, 798, 945};
int cablePartialSolve[CABLES_COUNT] = {false, false, false, false, false, false};
// Tow car puzzle variables
uint8_t reedCorrectSeq[REED_SEQ_COUNT] = {WORKSHOP, FARMLAND, HALL, STATION, FARMLAND, HALL, FARMLAND, STATION, HALL, WORKSHOP};
// Buttons puzzle variables
uint8_t buttonCorrectSeq[BUTTON_SEQ_COUNT] = {YELLOW, RED, GREEN, YELLOW, BLUE, GREEN, BLUE, RED};
// Buttons seq after win (to reset puzzle)
uint8_t buttonCorrectSeqAfterWin[4] = {GREEN, BLUE, RED, YELLOW};
// Blow variables
static float ambientPressure = 0;
// Knock variables
int knockCorrectSeq[KNOCK_SEQ_COUNT] = {400, 1600, 200, 200, 1000}; // intervals in ms between knocks
int lastPress = 0;
// 2 dim array of correct UIDs
uint8_t correctUID[6][UID_SIZE] = {{0x66, 0xDF, 0xFA, 0x22},
                                   {0xC3, 0xAE, 0x7D, 0x40},
                                   {0xD6, 0x20, 0x2A, 0x2B},
                                   {0xB2, 0x80, 0x89, 0x1B},
                                   {0xB6, 0xD3, 0x96, 0x2B},
                                   {0xB2, 0x7D, 0x2F, 0x1B}};

ezButton buttonArray[BUTTON_COUNT] = {
    ezButton(BUTTON0),
    ezButton(BUTTON1),
    ezButton(BUTTON2),
    ezButton(BUTTON3)};

ezButton reedArray[REED_COUNT] = {
    ezButton(REED0),
    ezButton(REED1),
    ezButton(REED2),
    ezButton(REED3)};

ezButton knockSensor(KNOCK);

BMP280_DEV bmp280;
MFRC522 rfid(SS, RST);

CircularBuffer<int, BUTTON_SEQ_COUNT> buttonBuffer;
CircularBuffer<int, REED_SEQ_COUNT> reedBuffer;
CircularBuffer<int, KNOCK_SEQ_COUNT> knockBuffer;
CircularBuffer<int, 4> buttonBufferAfterWin;

bool CheckCables()
{
  int readTmp;
  Serial.print("Kabelki: ");
  for (int i = 0; i < CABLES_COUNT; i++)
  {
    readTmp = analogRead(cable[i]);
    Serial.print(" ");
    Serial.print(i);
    Serial.print(" = ");
    Serial.print(readTmp);
  }
  Serial.println(".");

  for (int i = 0; i < CABLES_COUNT; i++)
  {
    readTmp = analogRead(cable[i]);
    if (cableDownLim[i] < readTmp && readTmp < cableUpLim[i])
      cablePartialSolve[i] = true;
    else
      cablePartialSolve[i] = false;
    if (cablePartialSolve[i] == false)
      return false;
  }
  return true;
}

bool CheckRFID(uint8_t n)
{
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial())
  {
    if (rfid.uid.size == UID_SIZE)
    {
      Serial.print("UID = ");
      for (int i = 0; i < UID_SIZE; i++)
      {
        Serial.print(rfid.uid.uidByte[i]);
        Serial.print(":");
      }
      Serial.println(".");

      if (!memcmp(correctUID[n], rfid.uid.uidByte, UID_SIZE))
      {
        return true;
      }
      else
        return false;
    }
  }
  return false;
}

void SetAmbientPressure()
{
  bmp280.getCurrentPressure(ambientPressure);
}

bool ChceckBlowSensor(float diff)
{
  float tempPressure = 0;
  bmp280.getCurrentPressure(tempPressure);
  Serial.print("Ciśnienie = ");
  Serial.println(tempPressure);
  if (tempPressure - ambientPressure >= diff)
    return true;
  else
    return false;
}

void UpdateCarRide()
{
  for (int i = 0; i < REED_COUNT; i++)
  {
    if (reedArray[i].isPressed())
    {
      reedBuffer.push(i);
      mp3.playMp3FolderTrack(HONK_HONK);
      Serial.print("Car Buffer: ");
      for (int j = 0; j < REED_SEQ_COUNT; j++)
      {
        Serial.print(reedBuffer[j]);
      }
      Serial.println(".");
    }
  }
}

bool CheckCarRide()
{
  for (int i = 0; i < REED_SEQ_COUNT; i++)
  {
    if (reedCorrectSeq[i] != reedBuffer[i])
      return false;
  }
  return true;
}

void UpdateButtons()
{
  for (int i = 0; i < BUTTON_COUNT; i++)
  {
    if (buttonArray[i].isPressed())
    {
      buttonBuffer.push(i);
      mp3.playMp3FolderTrack(BUTTON_SOUND);
      Serial.print("Button Buffer: ");
      for (int j = 0; j < BUTTON_SEQ_COUNT; j++)
      {
        Serial.print(buttonBuffer[j]);
      }
      Serial.println(".");
    }
  }
}
bool CheckButtons()
{
  for (int i = 0; i < BUTTON_SEQ_COUNT; i++)
  {
    if (buttonCorrectSeq[i] != buttonBuffer[i])
      return false;
  }
  return true;
}

void UpdateKnock()
{
  if (knockSensor.isPressed())
  {
    uint64_t timeFromLast = millis() - lastPress;
    knockBuffer.push(timeFromLast);
    lastPress = millis();
    Serial.print("Knock Buffer: ");
    for (int i = 0; i < KNOCK_SEQ_COUNT; i++)
    {
      Serial.print(knockBuffer[i]);
      Serial.print(" : ");
    }
    Serial.println(".");
  }
}

bool CheckKnock()
{
  // uint64_t knockCorrectSeq[KNOCK_SEQ_COUNT] = {400, 1600, 200, 200, 1000}; // intervals in ms between knocks
  int error = KNOCK_ERROR;
  for (uint8_t i = 0; i < KNOCK_SEQ_COUNT; i++)
  {
    if (!(knockCorrectSeq[i] - error < knockBuffer[i] && knockBuffer[i] < knockCorrectSeq[i] + error))
    {
      // Serial.print("git:");
      // Serial.println(i); 
      return false;
    }
  }
  return true;
}

void UpdateButtonsAfterWin()
{
  for (int i = 0; i < BUTTON_COUNT; i++)
  {
    if (buttonArray[i].isPressed())
    {
      buttonBufferAfterWin.push(i);
      mp3.playMp3FolderTrack(BUTTON_SOUND);
      Serial.print("Button Buffer: ");
      for (int j = 0; j < 4; j++)
      {
        Serial.print(buttonBufferAfterWin[j]);
      }
      Serial.println(".");
    }
  }
}
bool CheckButtonsAfterWin()
{
  for (int i = 0; i < 4; i++)
  {
    if (buttonCorrectSeqAfterWin[i] != buttonBufferAfterWin[i])
      return false;
  }
  return true;
}