#ifndef INSTRUMENT_MENU_H
#define INSTRUMENT_MENU_H

#include <TFT_eSPI.h>
#include "Globals.h"

// 메뉴 커서 (0: Synthesizer, 1: Effects)
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

    // 2. 메뉴 항목들
    const char* menuItems[] = { "1. SYNTHESIZER", "2. EFFECTS" };
    int menuY = 60;
    int gap = 25;

    tft.setTextDatum(TL_DATUM);
    
    for (int i = 0; i < 2; i++) {
        int y = menuY + (i * gap);
        
        if (i == instSelectCursor) {
            // 선택된 항목 강조
            tft.fillRect(30, y - 2, 180, 20, TFT_DARKGREY);
            tft.setTextColor(TFT_YELLOW, TFT_DARKGREY);
            tft.drawString("> " + String(menuItems[i]), 40, y);
        } else {
            // 일반 항목
            tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
            tft.drawString("  " + String(menuItems[i]), 40, y);
        }
    }

    // 3. 하단 안내
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString("A: SELECT  /  B: BACK", 120, 100);
}

#endif