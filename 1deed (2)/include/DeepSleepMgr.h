#ifndef DEEP_SLEEP_MGR_H
#define DEEP_SLEEP_MGR_H

#include "Globals.h"
#include <driver/rtc_io.h>
#include <esp_sleep.h>

// [딥슬립 진입 함수]
void enterDeepSleep() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("SLEEPING...", 120, 60);
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.drawString("Press Btn to Wake", 120, 90);

    // 1. 손 뗄 때까지 대기
    unsigned long s = millis();
    while ((digitalRead(BTN_A) == LOW || digitalRead(BTN_DOWN) == LOW) && (millis() - s < 3000)) {
        delay(50);
    }
    
    // -----------------------------------------------------------
    // [백라이트 잔광 제거] 끄고 -> 얼린다
    // -----------------------------------------------------------
    digitalWrite(TFT_BL, LOW);
    gpio_hold_en((gpio_num_t)TFT_BL); // 핀 상태 고정 (Deep Sleep 중에도 LOW 유지)
    tft.writecommand(0x10); 

    // [중요 1] RTC 주변기기 전원 켜기 (버튼 풀업 유지용)
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);

    // [중요 2] 깨울 핀 설정 (엔코더 제외! 오직 버튼만!)
    int wakePins[] = { 
        BTN_UP, BTN_DOWN, BTN_LEFT, BTN_RIGHT, 
        BTN_A, BTN_B, BTN_X, BTN_Y 
    };
    
    uint64_t mask = 0;

    for (int i = 0; i < 8; i++) {
        int pin = wakePins[i];
        
        // 1. 핀 리셋 (AceButton 등 기존 설정 제거)
        gpio_reset_pin((gpio_num_t)pin);
        
        // 2. RTC 모드로 전환
        rtc_gpio_init((gpio_num_t)pin);
        rtc_gpio_set_direction((gpio_num_t)pin, RTC_GPIO_MODE_INPUT_ONLY);
        
        // 3. 풀업 켜기
        rtc_gpio_pullup_en((gpio_num_t)pin);
        rtc_gpio_pulldown_dis((gpio_num_t)pin);

        mask |= (1ULL << pin);
    }

    // [중요 3] 깨우기 트리거 (ANY_LOW)
    esp_sleep_enable_ext1_wakeup(mask, ESP_EXT1_WAKEUP_ANY_LOW);
    
    Serial.println("Entering Deep Sleep...");
    Serial.flush();
    
    esp_deep_sleep_start();
}

// [깨어난 이유 확인 및 복구]
void checkWakeupReason() {
    esp_sleep_wakeup_cause_t reason = esp_sleep_get_wakeup_cause();
    
    // [★매우 중요] 깨어나자마자 백라이트 잠금부터 무조건 푼다!
    // (if문 안에 넣지 말고 무조건 실행해야 함)
    gpio_hold_dis((gpio_num_t)TFT_BL); 

    // RTC 핀 설정 해제 (일반 GPIO로 복귀)
    int wakePins[] = { BTN_UP, BTN_DOWN, BTN_LEFT, BTN_RIGHT, BTN_A, BTN_B, BTN_X, BTN_Y };
    for(int i=0; i<8; i++) {
        rtc_gpio_deinit((gpio_num_t)wakePins[i]);
    }

    // 화면 켜기
    if (reason == ESP_SLEEP_WAKEUP_EXT1) {
        Serial.println("WAKEUP!");
        digitalWrite(TFT_BL, HIGH);
        tft.writecommand(0x11); 
        delay(120);
    }
}

#endif