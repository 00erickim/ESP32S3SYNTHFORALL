#include <Arduino.h>
#include <TFT_eSPI.h>
#include <AceButton.h>
#include <ESP32Encoder.h>
#include <driver/i2s.h>
#include <FS.h>
#include <SPI.h>

#include "Globals.h"

// [함수 전방 선언]
void AudioTask(void *pvParams);
void restoreLastMode(); 

#include "GranularSynth.h" 
#include "AudioEngine.h"
#include "MixerUI.h"       
#include "DrumUI.h"  
#include "TrackerUI.h"
#include "SynthUI.h"       
#include "SpaceVisualizer.h" 
#include "FileManager.h"
#include "DeepSleepMgr.h"   // 수정된 파일
#include "InputHandlers.h"  
#include "AppControl.h"     
#include "FactoryTest.h"

// ---------------------------------------------------------
// [로컬 객체]
// ---------------------------------------------------------
SpaceVisualizer* spaceVis = nullptr;
AceButton buttons[8];
uint8_t BTN_PINS[8] = {BTN_UP, BTN_DOWN, BTN_LEFT, BTN_RIGHT, BTN_A, BTN_B, BTN_X, BTN_Y};
AceButton encBtns[2];
uint8_t ENC_BTN_PINS[2] = {ENC1_SW, ENC2_SW};

// [부팅 애니메이션]
void playStartupAnimation() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2); tft.setTextDatum(MC_DATUM); tft.setTextColor(TFT_CYAN, TFT_BLACK); tft.drawString("GRANULAR", 120, 60);
  tft.setTextColor(TFT_WHITE, TFT_BLACK); tft.drawString("SYNTHESIZER", 120, 90);
  tft.setTextSize(1); tft.setTextColor(TFT_DARKGREY, TFT_BLACK); tft.drawString("System Booting...", 120, 120); delay(1000);
}

// [화면 복구 함수]
void restoreLastMode() {
    tft.writecommand(0x11); // Sleep Out
    delay(120);
    digitalWrite(TFT_BL, HIGH);

    switch(currentMode) {
        case MODE_MENU: initMixerUI(); break;
        case MODE_DRUM: drawDrumMachine(true); break;
        case MODE_TRACKER: drawTrackerUI(); break;
        case MODE_PIANO_ROLL: drawPianoRoll(); break;
        case MODE_GRANULAR_PLAY: drawGranularUI(); break;
        case MODE_KEYBOARD: drawKeyboardUI(); break;
        case MODE_DRONE: drawDroneUI(); break;
        case MODE_FINGER_DRUM: drawFingerDrumUI(); break;
        default: initMixerUI(); break;
    }
}

// ----------------------------------------------------
// [SETUP]
void setup() {
  Serial.begin(115200);

  // 1. 하드웨어 핀 설정 (먼저 해야 함)
  // 버튼 핀들을 INPUT_PULLUP으로 확실히 초기화
  for (int i=0; i<8; i++) pinMode(BTN_PINS[i], INPUT_PULLUP);
  pinMode(ENC1_SW, INPUT_PULLUP);
  pinMode(ENC2_SW, INPUT_PULLUP);

  // [★중요] 깨어난 이유 확인 및 '핀 얼음 땡(Un-hold)'
  // DeepSleepMgr.h의 checkWakeupReason() 안에서 gpio_hold_dis()가 실행됩니다.
  checkWakeupReason(); 

  // [FACTORY TEST]
  if (digitalRead(BTN_A) == LOW) runFactoryTest();

  // 2. 주변기기 초기화
  SPI.begin(12, 13, 11); 
  SdSpiConfig sdConfig(SD_CS_PIN, SHARED_SPI, SD_SCK_MHZ(16)); 
  sd.begin(sdConfig);

  tft.init(); tft.setRotation(1); tft.invertDisplay(true); 
  img.setColorDepth(16); img.createSprite(240, 135);

  ButtonConfig* config = ButtonConfig::getSystemButtonConfig(); 
  config->setEventHandler(handleButtonEvent); 
  config->setFeature(ButtonConfig::kFeatureClick); 
  config->setFeature(ButtonConfig::kFeatureRepeatPress); 
  config->setFeature(ButtonConfig::kFeatureLongPress);

  ESP32Encoder::useInternalWeakPullResistors = puType::up; 
  enc1.attachSingleEdge(ENC1_A, ENC1_B); 
  enc2.attachSingleEdge(ENC2_A, ENC2_B);
  
  for (int i=0; i<8; i++) { buttons[i].init(BTN_PINS[i], HIGH, i); }
  for (int i=0; i<2; i++) { encBtns[i].init(ENC_BTN_PINS[i], HIGH, i+8); }
  
  // 3. 오디오 시작
  xTaskCreatePinnedToCore(AudioTask, "MozziTask", 10000, NULL, 24, NULL, 0);

  // 4. 화면 표시 로직
  // 만약 딥슬립에서 깬 거라면(EXT1) 복구하고, 아니면 로고 재생
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT1) {
      Serial.println("Wakeup Restore!");
      enc1.setCount(0); 
      restoreLastMode();
  } else {
      Serial.println("Normal Boot");
      // 전역변수 초기화 (Globals.cpp에 있는 RTC 변수가 아닌 일반 변수들)
      currentMode = MODE_MENU; 
      currentMixerTrack = 0;
      playStartupAnimation(); 
      initMixerUI(); 
      enc1.setCount(0);
  }
}

// ----------------------------------------------------
// [LOOP]
void loop() {
  for(int i=0; i<8; i++) buttons[i].check(); 
  encBtns[0].check(); 
  encBtns[1].check(); 
  updateAppLogic();
}