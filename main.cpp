#include <M5StickCPlus.h>
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

void insert_logo(std::string name);

UNIT_MINIENCODERC sensor;
TFT_eSprite sprite(&M5.Lcd);

uint16_t delay_time = 0;
int32_t last_value  = 0;

uint8_t i2c_address = 0;

uint8_t scroll_steps[] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  1,1,2,4,6,5,4,2,1,1
};

static const uint16_t imgWidth = 240;
static const uint16_t imgHeight = 135;

static bool hold_toggle = false;

static bool hat_button_value = false;
static uint32_t released_for_timer = 0;

static std::string mode = "namecard";

const uint16_t* images[] = {ogawa, ogawa_aff, tw};
const uint16_t* denchas[] = {tb, nis, sre, oexp, bexp};
 
void page_get_encoder(void) {
  int32_t encoder_value = sensor.getEncoderValue();
  bool btn_status       = sensor.getButtonStatus();
  //Serial.println(encoder_value);
}

void get_inc_value(void) {
  int32_t inc_value = sensor.getIncrementValue();
  //Serial.printf("Inc:%d", inc_value);
}

void show_transitions(std::string now, std::string next) {
  sprite.setSwapBytes(false);
  mode = next;
  for (int i = 0; i < 7; i++) {
    sprite.fillScreen(WHITE);
    sprite.setTextColor(BLACK);
    sprite.setCursor(0, 0);
    insert_logo(now);
    if (i % 3 == 0) {
      insert_logo("0");
    } else if (i % 3 == 1) {
      insert_logo("1");
    } else {
      insert_logo("2");
    }
    insert_logo(next);
    sprite.pushSprite(0, 0);
    delay(120);
  }
  sprite.setSwapBytes(true);
}

void insert_logo(std::string name) {
  sprite.setTextFont(2);
  if (name == "now") {
    sprite.setTextSize(2);
    sprite.setTextColor(CYAN);
    sprite.print("N");
    sprite.setTextColor(BLACK);
    sprite.println("ow");
  } else if (name == "next") {
    sprite.setTextSize(2);
    sprite.setTextColor(PINK);
    sprite.print("N");
    sprite.setTextColor(BLACK);
    sprite.println("ext");
  } else if (name == "0") {
    sprite.setTextSize(2);
    sprite.setTextColor(YELLOW);
    sprite.print(">");
    sprite.setTextColor(BLACK);
    sprite.println(">>");
  } else if (name == "1") {
    sprite.setTextSize(2);
    sprite.setTextColor(BLACK);
    sprite.print(">");
    sprite.setTextColor(YELLOW);
    sprite.print(">");
    sprite.setTextColor(BLACK);
    sprite.println(">");
  } else if (name == "2") {
    sprite.setTextSize(2);
    sprite.setTextColor(BLACK);
    sprite.print(">>");
    sprite.setTextColor(YELLOW);
    sprite.println(">");
  } else if (name == "namecard") {
    sprite.setTextSize(2);
    sprite.setTextColor(BLACK);
    sprite.print("Name");
    sprite.setTextColor(YELLOW);
    sprite.println("Card");
  } else if (name == "dencha") {
    sprite.setTextSize(2);
    sprite.setTextColor(BLACK);
    sprite.print("Den");
    sprite.setTextColor(YELLOW);
    sprite.println("cha");
  } else {
    sprite.setTextSize(2);
    sprite.setTextColor(BLACK);
    sprite.print(name.c_str());
    sprite.setTextColor(BLACK);
    sprite.println(name.c_str());
  }
}

void setup() {
  M5.begin();
  pinMode(GPIO_NUM_10, OUTPUT);
  M5.Axp.ScreenBreath(30); // 画面の明るさ7〜12
  M5.Lcd.setRotation(3); // 画面を横向きにする
  M5.Lcd.fillScreen(WHITE); // 背景色
  M5.Lcd.setTextFont(4);
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

  bool now_button_value = sensor.getButtonStatus();
  if (!now_button_value && hat_button_value) {
    Serial.println("button pressed?");
    if (millis() - released_for_timer < 300) {
      if (mode == "namecard") {
        show_transitions("namecard", "dencha");
      } else if (mode == "dencha") {
        show_transitions("dencha", "namecard");
      }
      Serial.println("button released");
    }
    released_for_timer = millis();
  }
  hat_button_value = now_button_value;
    
  //ここまで状態検知
  if ((current_time - last_time >= 100) && (!hold_toggle)) {
    last_value += scroll_steps[scroll_step_index];
    scroll_step_index = (scroll_step_index + 1) % (sizeof(scroll_steps) / sizeof(scroll_steps[0]));
    last_time = current_time;
  }

    
  if (mode == "namecard") {
    // エンコーダの値を手動で設定
    sensor.setEncoderValue(last_value);
    page_get_encoder();

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
  } else if (mode == "dencha") {
    sensor.setEncoderValue(last_value);
    page_get_encoder();

    static int32_t last_y_offset = -1;
    int32_t encoder_value = sensor.getEncoderValue();
    const uint8_t image_count = sizeof(denchas) / sizeof(denchas[0]);
    int y_offset = encoder_value % (image_count * imgHeight);

    if (y_offset != last_y_offset) {
      TFT_eSprite sprite(&M5.Lcd);
      sprite.createSprite(imgWidth, imgHeight);
      sprite.fillSprite(WHITE);

      int corrected_offset = (y_offset * 5) % (image_count * imgHeight);
      if (corrected_offset < 0) corrected_offset += (image_count * imgHeight);

      for (int i = 0; i < image_count; i++) {
          sprite.pushImage(0, i * imgHeight - corrected_offset, imgWidth, imgHeight, denchas[i % image_count]);
      }
      sprite.pushImage(0, (image_count) * imgHeight - corrected_offset, imgWidth, imgHeight, denchas[0]);

      M5.Lcd.startWrite();
      sprite.pushSprite(0, 0);
      M5.Lcd.endWrite();
      sprite.deleteSprite();
      last_y_offset = y_offset;
    }
  }
}

