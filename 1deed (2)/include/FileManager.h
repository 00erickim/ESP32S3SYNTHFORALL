// FileManager.h
#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include "Globals.h"

const int MAX_FILES = 50; // 최대 파일 개수
String fileList[MAX_FILES];
int fileCount = 0;

// [함수] SD카드 파일 목록 읽기 (배열에 저장)
void loadFileList() {
    fileCount = 0;
    File32 root = sd.open("/");
    if (!root) return;

    File32 file;
    while (file.openNext(&root, O_RDONLY)) {
        if (fileCount >= MAX_FILES) { file.close(); break; }
        
        char name[32];
        file.getName(name, sizeof(name));
        
        // 숨김 파일(.) 제외
        if (name[0] != '.') {
            if (file.isDirectory()) fileList[fileCount] = "[D] " + String(name);
            else fileList[fileCount] = String(name);
            fileCount++;
        }
        file.close();
    }
    root.close();
}

// [함수] 파일 선택 화면 그리기
void drawFileBrowser() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("LOAD WAV", 120, 10);
    tft.drawFastHLine(20, 35, 200, TFT_WHITE);
    
    tft.setTextSize(1);
    tft.setTextDatum(TL_DATUM);

    if (fileCount == 0) {
        tft.setTextColor(TFT_ORANGE, TFT_BLACK);
        tft.drawCentreString("No Files Found", 120, 100, 2);
        return;
    }

    // 스크롤 처리 (한 화면에 8개 표시)
    int displayCount = 8;
    for (int i = 0; i < displayCount; i++) {
        int idx = fileScrollTop + i;
        if (idx >= fileCount) break;

        int y = 50 + (i * 20);
        
        // 커서 위치 강조
        if (idx == fileCursor) {
            tft.setTextColor(TFT_GREEN, TFT_BLACK);
            tft.drawString("> " + fileList[idx], 10, y);
        } else {
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            tft.drawString("  " + fileList[idx], 10, y);
        }
    }
    
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft.drawCentreString("A: Load to RAM  B: Back", 120, 220, 1);
}

#endif