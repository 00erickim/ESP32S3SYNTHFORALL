#ifndef FACTORY_TEST_H
#define FACTORY_TEST_H

#include "Globals.h"
#include <AceButton.h>

using namespace ace_button;

// 테스트 모드 전용 버튼 핸들러가 필요할 수 있으나, 
// 기존 버튼 객체를 활용하되 폴링 방식으로 간단히 구현하겠습니다.

void drawTestUI(int enc1Val, int enc2Val, bool* btnStates) {
    tft.fillScreen(TFT_BLACK);
    
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("FACTORY TEST", 120, 10);
    
    tft.setTextSize(1);
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.drawString("Press Reset to Exit", 120, 35);

    // ---------------------------------------------------------
    // 1. 엔코더 값 표시
    // ---------------------------------------------------------
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString("ENC 1: " + String(enc1Val), 20, 60);
    tft.drawString("ENC 2: " + String(enc2Val), 140, 60);

    // ---------------------------------------------------------
    // 2. 버튼 상태 표시 (그리드 형태)
    // ---------------------------------------------------------
    const char* labels[] = {"UP", "DN", "LT", "RT", "A", "B", "X", "Y"};
    
    int startY = 90;
    int gapX = 50;
    
    for(int i=0; i<8; i++) {
        int r = i / 4;
        int c = i % 4;
        int x = 20 + (c * gapX);
        int y = startY + (r * 25);
        
        if (btnStates[i]) {
            tft.setTextColor(TFT_GREEN, TFT_BLACK);
            tft.fillCircle(x - 5, y + 4, 3, TFT_GREEN);
        } else {
            tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
            tft.drawCircle(x - 5, y + 4, 3, TFT_DARKGREY);
        }
        tft.drawString(labels[i], x, y);
    }
    
    // 엔코더 버튼 확인
    // encBtns[0] -> ENC1_SW, encBtns[1] -> ENC2_SW
    // 여기서는 digitalRead로 직접 확인
    bool e1 = (digitalRead(ENC1_SW) == LOW); // Pull-up active low
    bool e2 = (digitalRead(ENC2_SW) == LOW);
    
    if(e1) { tft.setTextColor(TFT_MAGENTA, TFT_BLACK); tft.drawString("SW1 ON", 20, 130); } // ENC1 밑
    else   { tft.setTextColor(TFT_DARKGREY, TFT_BLACK); tft.drawString("SW1 --", 20, 130); }
    
    if(e2) { tft.setTextColor(TFT_MAGENTA, TFT_BLACK); tft.drawString("SW2 ON", 140, 130); } // ENC2 밑
    else   { tft.setTextColor(TFT_DARKGREY, TFT_BLACK); tft.drawString("SW2 --", 140, 130); }
}

void runFactoryTest() {
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.drawCentreString("Keep Holding A...", 120, 60, 2);
    delay(1000); // 사용자가 버튼을 확실히 누르고 있는 상태 확인용
    
    // 테스트용 핀 배열 정의 (UP, DOWN, LEFT, RIGHT, A, B, X, Y)
    uint8_t TEST_BTN_PINS[8] = {BTN_UP, BTN_DOWN, BTN_LEFT, BTN_RIGHT, BTN_A, BTN_B, BTN_X, BTN_Y};

    // 핀 설정 (이미 setup() 전에 호출될 수도 있으므로 안전하게 다시 설정)
    for (int i=0; i<8; i++) pinMode(TEST_BTN_PINS[i], INPUT_PULLUP);
    pinMode(ENC1_SW, INPUT_PULLUP);
    pinMode(ENC2_SW, INPUT_PULLUP);
    
    // 엔코더 초기화
    ESP32Encoder::useInternalWeakPullResistors = puType::up;
    enc1.attachSingleEdge(ENC1_A, ENC1_B);
    enc2.attachSingleEdge(ENC2_A, ENC2_B);
    enc1.setCount(0);
    enc2.setCount(0);

    bool btnStates[8];
    int lastE1 = -999;
    int lastE2 = -999;
    
    while(1) {
        bool dirty = false;
        
        // 1. 버튼 스캔
        for(int i=0; i<8; i++) {
            bool pressed = (digitalRead(TEST_BTN_PINS[i]) == LOW);
            if (btnStates[i] != pressed) {
                btnStates[i] = pressed;
                dirty = true;
            }
        }
        
        // 2. 엔코더 스캔
        int e1 = (int)enc1.getCount();
        int e2 = (int)enc2.getCount();
        
        // 엔코더 버튼 스캔 (화면 갱신용)
        static bool lastSw1 = false;
        static bool lastSw2 = false;
        bool sw1 = (digitalRead(ENC1_SW) == LOW);
        bool sw2 = (digitalRead(ENC2_SW) == LOW);
        
        if (e1 != lastE1 || e2 != lastE2 || sw1 != lastSw1 || sw2 != lastSw2) {
            lastE1 = e1;
            lastE2 = e2;
            lastSw1 = sw1;
            lastSw2 = sw2;
            dirty = true;
        }
        
        if (dirty) {
            drawTestUI(e1, e2, btnStates);
        }
        
        delay(10); // 과부하 방지
    }
}

#endif