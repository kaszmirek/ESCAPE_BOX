#pragma once

//#include <Adafruit_BMP280.h>
#include <BMP280_DEV.h>
#include "config.h"
#include <ezButton.h>
#include <CircularBuffer.h>
#include "MFRC522.h"

#define CABLES_COUNT 6

#define REED_SEQ_COUNT 10
#define REED_COUNT 4
#define BUTTON_SEQ_COUNT 8
#define BUTTON_COUNT 4

#define KNOCK_SEQ_COUNT 5
#define KNOCK_ERROR 500

#define WORKSHOP 2
#define FARMLAND 1
#define HALL 0
#define STATION 3

#define RED 1
#define GREEN 3
#define BLUE 2
#define YELLOW 0

extern BMP280_DEV bmp280;
extern MFRC522 rfid;

extern ezButton buttonArray[BUTTON_COUNT];
extern ezButton reedArray[REED_COUNT];
extern ezButton knockSensor;

extern CircularBuffer<int, BUTTON_SEQ_COUNT> buttonBuffer;
extern CircularBuffer<int, REED_SEQ_COUNT> reedBuffer;
extern CircularBuffer<int, KNOCK_SEQ_COUNT> knockBuffer;
extern CircularBuffer<int, 4> buttonBufferAfterWin;

bool CheckCables();
// void RfidChandler(uint8_t);
bool CheckRFID(uint8_t);
void SetAmbientPressure();
bool ChceckBlowSensor(float);
void UpdateCarRide();
bool CheckCarRide();
void UpdateButtons();
bool CheckButtons();
void UpdateKnock();
bool CheckKnock();
void UpdateButtonsAfterWin();
bool CheckButtonsAfterWin();
