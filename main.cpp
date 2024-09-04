// #include <M5StickCPlus.h>
#include <M5Unified.h>
// #include <TFT_eSPI.h>
#include <string>
#include <vector>
// #include "tb.h"
// #include "hakutaka.h"
// #include "hokuto.h"
// #include "nis.h"
// #include "sre.h"
// #include "oexp.h"
// #include "bexp.h"
#include "Unit_MiniEncoderC.h"

#include "ogawa.h"
// #include "ogawa_aff.h"
// #include "tw.h"

#define FREEZING_TIME 1000
#define AUTO_ESCAPE_STEP 3

// #include "kaiso.h"
// #include "sinkai.h"
// #include "kaisoku.h"
// #include "huku.h"
// #include "wanman.h"
// #include "liner.h"
// #include "testr.h"

const uint16_t *imgs[] = {ogawa};//{kaiso, sinkai, kaisoku, huku, wanman, liner, testr};
const size_t imgCount = sizeof(imgs) / sizeof(imgs[0]);
uint8_t currentIndex = 0;

enum State
{
  AUTO_SCROLL,
  MANUAL_SCROLL,
  AUTO_ESCAPE,
};

State state = AUTO_SCROLL;
bool isHold = false;

String StateToString(State state)
{
  switch (state)
  {
  case AUTO_SCROLL:
    return "AS";
  case MANUAL_SCROLL:
    return "MN";
  case AUTO_ESCAPE:
    return "EC";
  }
}

uint8_t scroll_steps[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 2, 2, 3, 3, 4, 3, 2, 2, 2, 1, 1};

static const uint16_t imgWidth = 240;
static const uint16_t imgHeight = 135;
int16_t currentOffset = 0;
uint8_t currentStep = 0;
UNIT_MINIENCODERC sensor;

uint32_t lastMovedTime = 0;
uint32_t encoderValue = 0;
uint32_t lastEncoderValue = 0;

std::vector<uint16_t> autoEscapeSteps;
uint16_t autoEscapeStep = 0;

void drawImage(const uint16_t *img)
{
  M5.Lcd.startWrite();
  for (int x = 0; x < imgWidth; x++)
  {
    for (int y = 0; y < imgHeight; y++)
    {
      uint16_t color = img[y * imgWidth + x];
      // RGB565 (little endian) の色を正しく表示するためにバイト順を入れ替える
      color = (color >> 8) | (color << 8);
      M5.Lcd.drawPixel(imgHeight - y, x, color);
    }
  }
  M5.Lcd.endWrite();
}

void drawCombinedImage(const uint16_t *img1, const uint16_t *img2, uint16_t offset)
{
  M5.Lcd.startWrite();
  int pixelNum = 0;

  // img1からoffset分ずらしてコピー
  for (int y = 0; y < imgHeight - offset; y++)
  {
    for (int x = 0; x < imgWidth; x++)
    {
      uint16_t color = img1[y * imgWidth + x + offset * imgWidth];
      color = (color >> 8) | (color << 8); // RGB565 (little endian) の色を正しく表示するためにバイト順を入れ替える
      M5.Lcd.drawPixel(imgHeight - y, x, color);
    }
  }

  // img2から残りをコピー
  for (int y = imgHeight - offset; y < imgHeight; y++)
  {
    for (int x = 0; x < imgWidth; x++)
    {
      uint16_t color = img2[(y - (imgHeight - offset)) * imgWidth + x];
      color = (color >> 8) | (color << 8); // RGB565 (little endian) の色を正しく表示するためにバイト順を入れ替える
      M5.Lcd.drawPixel(imgHeight - y, x, color);
    }
  }

  M5.Lcd.endWrite();
}

void drawTape(const uint16_t *imgs[], uint8_t imgIndex, int16_t currentOffset)
{
  Serial.printf("ImageCount: %d\n", imgCount);
  Serial.printf("next imgIndex: %d\n", (imgIndex + 1) % imgCount);
  drawCombinedImage(imgs[imgIndex], imgs[(imgIndex + 1) % imgCount], currentOffset);
}

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

  if (M5.BtnA.wasPressed())
  {
    isHold = !isHold;
  }

  if (M5.BtnA.wasPressed() || isHold)
  {
    if (!isHold)
    {
      isHold = false;
    }
    else
    {
      state = MANUAL_SCROLL;
      isHold = true;
    }
  }

  if (state == AUTO_SCROLL)
  {
    if (encoderValue != lastEncoderValue)
    {
      state = MANUAL_SCROLL;
      lastMovedTime = currentTime;
    }
    else
    {
      currentOffset += scroll_steps[currentStep] * 5;
      if (currentOffset >= imgHeight)
      {
        currentOffset = 0;
        currentIndex = (currentIndex + 1) % imgCount; // インクリメント
      }
      currentStep++;
      if (currentStep >= sizeof(scroll_steps))
      {
        currentStep = 0;
      }
    }
  }
  else if (state == MANUAL_SCROLL)
  {
    if (currentTime - lastMovedTime > FREEZING_TIME)
    {
      state = AUTO_ESCAPE;
      autoEscapeStep = 0;
    }
    if (encoderValue != lastEncoderValue)
    {
      lastMovedTime = currentTime;
      currentOffset += (encoderValue - lastEncoderValue) * 2;
      if (currentOffset >= imgHeight)
      {
        currentOffset = 0;
        currentIndex = (currentIndex + 1) % imgCount; // インクリメント
      }
      else if (currentOffset < 0)
      {
        currentOffset = imgHeight - 1;
        currentIndex = (currentIndex == 0) ? imgCount - 1 : currentIndex - 1; // デクリメント
      }
    }
  }
  else if (state == AUTO_ESCAPE)
  {
    if (encoderValue != lastEncoderValue)
    {
      state = MANUAL_SCROLL;
      lastMovedTime = currentTime;
    }
    else if (autoEscapeStep < autoEscapeSteps.size())
    {
      currentOffset += autoEscapeSteps[autoEscapeStep] * 5;
      autoEscapeStep++;
    }
    else
    {
      currentOffset = currentOffset % imgHeight;
      if (currentOffset != 0)
      {
        if (abs(currentOffset) < AUTO_ESCAPE_STEP)
        {
          currentOffset = 0;
        }
        else if (currentOffset > 0)
        {
          currentOffset -= AUTO_ESCAPE_STEP;
        }
        else
        {
          currentOffset += AUTO_ESCAPE_STEP;
        }
        currentStep = 0;
      }
      else
      {
        state = AUTO_SCROLL;
      }
    }
  }

  drawTape(imgs, currentIndex, currentOffset);

  if (encoderValue != lastEncoderValue)
  {
    lastMovedTime = currentTime;
  }
  lastEncoderValue = encoderValue;
}
