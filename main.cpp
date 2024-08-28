// #include <M5StickCPlus.h>
#include <M5Unified.h>
#include <string>
#include <vector>
#include "tb.h"
#include "hakutaka.h"
#include "hokuto.h"
#include "nis.h"
#include "sre.h"
#include "oexp.h"
#include "bexp.h"
#include "Unit_MiniEncoderC.h"

#include "ogawa.h"
#include "ogawa_aff.h"
#include "tw.h"

#define FREEZING_TIME 1000

enum State
{
  AUTO_SCROLL,
  MANUAL_SCROLL,
  START_ESCAPE,
  AUTO_ESCAPE,
};

State state = AUTO_SCROLL;

String StateToString(State state)
{
  switch (state)
  {
  case AUTO_SCROLL:
    return "AS";
  case MANUAL_SCROLL:
    return "MN";
  case START_ESCAPE:
    return "ST";
  case AUTO_ESCAPE:
    return "EC";
  }
}

uint8_t scroll_steps[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 2, 2, 3, 3, 4, 3, 2, 2, 2, 1, 1};

static const uint16_t imgWidth = 240;
static const uint16_t imgHeight = 135;
uint16_t currentOffset = 0;
uint8_t currentStep = 0;
UNIT_MINIENCODERC sensor;

uint32_t lastMovedTime = 0;
uint32_t encoderValue = 0;
uint32_t lastEncoderValue = 0;

std::vector<uint16_t> autoEscapeSteps;
uint16_t autoEscapeStep = 0;

void setup()
{
  M5.begin();
  sensor.begin(&Wire, MINIENCODERC_ADDR, 0, 26, 100000UL);
  state = AUTO_SCROLL;
}

void loop()
{
  M5.update();
  encoderValue = sensor.getEncoderValue();
  uint32_t currentTime = millis();

  if (state == AUTO_SCROLL)
  {
    Serial.println("AUTO_SCROLL");
    Serial.printf("currentTime: %d\n", currentTime);
    Serial.printf("lastMovedTime: %d\n", lastMovedTime);
    Serial.printf("EncoderValue: %d\n", encoderValue);
    Serial.printf("lastEncoderValue: %d\n", lastEncoderValue);

    if (encoderValue != lastEncoderValue)
    {
      // state = MANUAL_SCROLL;
    }
  }
  else if (state == MANUAL_SCROLL)
  {
    if (currentTime - lastMovedTime > FREEZING_TIME)
    {
      state = START_ESCAPE;
    }
  }
  else if (state == AUTO_ESCAPE)
  {
    if (encoderValue != lastEncoderValue)
    {
      state = MANUAL_SCROLL;
    }
    if (autoEscapeSteps.size() == 0)
    {
      state = AUTO_SCROLL;
    }
    else
    {
      currentOffset = autoEscapeSteps[autoEscapeStep];
      autoEscapeStep++;
      if (autoEscapeStep == autoEscapeSteps.size())
      {
        state = AUTO_SCROLL;
      }
    }
  }

  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextSize(2); // 文字サイズを大きくする
  M5.Lcd.setCursor(0, 0, 2);
  M5.Lcd.setTextColor(YELLOW);

  M5.Lcd.printf("TiD: %d\n", currentTime - lastMovedTime);
  M5.Lcd.setTextColor(WHITE);

  M5.Lcd.printf("EcVal: %d\n", encoderValue);

  currentOffset += scroll_steps[currentStep] * 5;
  if (currentOffset == imgHeight)
  {
    currentOffset = 0;
  }
  currentStep++;
  if (currentStep == sizeof(scroll_steps))
  {
    currentStep = 0;
  }
  M5.Lcd.setTextColor(PINK);
  M5.Lcd.printf("Ofs: %d\n", currentOffset);
  M5.Lcd.setTextColor(TFT_CYAN);
  M5.Lcd.printf("Step: %d\n", currentStep);

  M5.Lcd.setTextSize(4);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.print("S:");
  M5.Lcd.println(StateToString(AUTO_SCROLL));

  if (encoderValue != lastEncoderValue)
  {
    lastMovedTime = currentTime;
  }
  lastEncoderValue = encoderValue;
  delay(150);
}
