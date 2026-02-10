#ifndef INSTRUMENT_MENU_H
#define INSTRUMENT_MENU_H

#include <TFT_eSPI.h>
#include "Globals.h"

// 메뉴 커서 변수 (0: Synth, 1: Sampler, 2: Effects)
// main.cpp나 InputHandlers.h에서 이 변수를 참조(extern)해서 사용합니다.
int instSelectCursor = 0;

void drawInstrumentMenu() {
    tft.fillScreen(TFT_BLACK);
    
    // 1. 상단 제목 (현재 선택된 트랙 번호 표시)
    tft.drawRect(20, 20, 200, 100, TFT_WHITE); // 테두리
    tft.fillRect(20, 20, 200, 25, TFT_WHITE);  // 제목 배경
    
    tft.setTextSize(1);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_BLACK, TFT_WHITE);
    
    String title = "TRACK " + String(drumCursorTrack + 1) + " SETUP";
    tft.drawString(title, 120, 32);

    // 2. 메뉴 항목들 [★수정됨: SAMPLER 항목 추가]
    const char* menuItems[] = { "1. SYNTHESIZER", "2. SAMPLER", "3. EFFECTS" };
    int itemCount = 3; // 항목 수 3개

    int menuY = 60;
    int gap = 25;

    tft.setTextDatum(TL_DATUM);
    
    for (int i = 0; i < itemCount; i++) {
        int y = menuY + (i * gap);
        
        if (i == instSelectCursor) {
            // 선택된 항목 강조 (회색 배경 + 노란 글씨)
            tft.fillRect(30, y - 2, 180, 20, TFT_DARKGREY);
            tft.setTextColor(TFT_YELLOW, TFT_DARKGREY);
            tft.drawString("> " + String(menuItems[i]), 40, y);
        } else {
            // 일반 항목 (검은 배경 + 회색 글씨)
            tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
            tft.drawString("  " + String(menuItems[i]), 40, y);
        }
    }

    // 3. 하단 안내 문구
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString("A: SELECT  /  B: BACK", 120, 110);
    
    // (선택사항) 현재 설정된 타입 표시
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    String currentType = (trackType[drumCursorTrack] == 1) ? "Type: SAMPLER" : "Type: SYNTH";
    tft.drawString(currentType, 120, 135);
}

#endif