#include <M5StickCPlus.h>
#include "img.h"
#include "tb.h"
#include "hakutaka.h"
#include "hokuto.h"
#include "nis.h"
#include "sre.h"
#include "oexp.h"
#include "bexp.h"
#include "Unit_MiniEncoderC.h"


UNIT_MINIENCODERC sensor;
TFT_eSprite sprite(&M5.Lcd);

uint16_t delay_time = 0;
int32_t last_value  = 0;

uint8_t i2c_address = 0;

uint8_t scroll_steps[] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  1,1,2,4,6,5,4,2,1,1
};

static bool hold_toggle = false;

static bool hat_button_value = false;
static uint32_t released_for_timer = 0;
 
void page_get_encoder(void) {
  int32_t encoder_value = sensor.getEncoderValue();
  bool btn_status       = sensor.getButtonStatus();
  //Serial.println(encoder_value);
}

void get_inc_value(void) {
  int32_t inc_value = sensor.getIncrementValue();
  //Serial.printf("Inc:%d", inc_value);
}

void setup() {
  M5.begin();
  pinMode(GPIO_NUM_10, OUTPUT);
  M5.Axp.ScreenBreath(30); // 画面の明るさ7〜12
  M5.Lcd.setRotation(3); // 画面を横向きにする
  M5.Lcd.fillScreen(WHITE); // 背景色
  M5.Lcd.setSwapBytes(true); // 色がおかしい場合に変更する

  sensor.begin(&Wire, MINIENCODERC_ADDR, 0, 26, 100000UL);
  delay_time  = 20;
  i2c_address = sensor.getI2CAddress();

  sprite.createSprite(imgWidth, imgHeight);

  
}

void loop() {
  M5.update();

  static unsigned long last_time = 0;
  static uint8_t scroll_step_index = 0;
  unsigned long current_time = millis();

  bool a_btn_sence = M5.BtnA.wasPressed();
  if (a_btn_sence) {
    hold_toggle = !hold_toggle;
  }
  if (hold_toggle) {
    digitalWrite(GPIO_NUM_10, LOW);
  } else {
    digitalWrite(GPIO_NUM_10, HIGH);
  }

  bool isInTransition = false;
  bool now_button_value = sensor.getButtonStatus();
  if (!now_button_value && hat_button_value) {
    if (millis() - released_for_timer < 500) {
      isInTransition = true;
      Serial.println("button released");
    } else {
      isInTransition = false;
    }
    released_for_timer = millis();
  }
    
  //ここまで状態検知


  if ((current_time - last_time >= 100) && (!hold_toggle)) {
    last_value += scroll_steps[scroll_step_index];
    scroll_step_index = (scroll_step_index + 1) % (sizeof(scroll_steps) / sizeof(scroll_steps[0]));
    last_time = current_time;
  }

  // エンコーダの値を手動で設定
  sensor.setEncoderValue(last_value);
  page_get_encoder();

  const uint16_t* images[] = {tb, nis, sre, oexp, bexp};
  static int32_t last_y_offset = -1;
  int32_t encoder_value = sensor.getEncoderValue();
  const uint8_t image_count = sizeof(images) / sizeof(images[0]);
  int y_offset = encoder_value % (image_count * imgHeight);

  if (y_offset != last_y_offset) {
    TFT_eSprite sprite(&M5.Lcd);
    sprite.createSprite(imgWidth, imgHeight);
    sprite.fillSprite(WHITE);

    int corrected_offset = (y_offset * 5) % (image_count * imgHeight);
    if (corrected_offset < 0) corrected_offset += (image_count * imgHeight);

    for (int i = 0; i < image_count; i++) {
        sprite.pushImage(0, i * imgHeight - corrected_offset, imgWidth, imgHeight, images[i % image_count]);
    }
    sprite.pushImage(0, (image_count) * imgHeight - corrected_offset, imgWidth, imgHeight, images[0]);

    M5.Lcd.startWrite();
    sprite.pushSprite(0, 0);
    M5.Lcd.endWrite();
    sprite.deleteSprite();
    last_y_offset = y_offset;
  }
}