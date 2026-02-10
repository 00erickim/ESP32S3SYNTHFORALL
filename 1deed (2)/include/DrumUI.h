#ifndef DRUM_UI_H
#define DRUM_UI_H

#include <TFT_eSPI.h>
#include "Globals.h"

// 디자인 상수
const int START_X = 4;   
const int START_Y = 15;  
const int STEP_W = 12;   
const int STEP_H = 12;   
const int GAP = 2;
const int BEAT_GAP = 4;
const int TRACK_H = 20;  

const uint16_t COLOR_ON = TFT_WHITE;        
const uint16_t COLOR_OFF = 0x39E7;          
const uint16_t COLOR_BG_PLAY = 0x2124;      

// 1. 배경 그리기 (페이지 정보 표시 추가)
void drawDrumBackground() { 
    tft.fillScreen(TFT_BLACK); 
    tft.setTextSize(1); 
    tft.setTextDatum(TR_DATUM); 
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK); 
    
    // 현재 페이지 / 총 페이지 표시
    int currentPage = (drumCursorStep / 16) + 1;
    int totalPages = sequenceLength / 16;
    
    char buf[32]; 
    sprintf(buf, "BPM %d  (%d/%d)", bpm, currentPage, totalPages); 
    tft.drawString(buf, 235, 3);
}

// 2. 도형 그리기
void drawShape(int t, int x, int y, uint16_t color) { 
    int cx = x + STEP_W/2; int cy = y + STEP_H/2; 
    switch(t % 4) { 
        case 0: tft.fillRect(x, y, STEP_W, STEP_H, color); break; 
        case 1: tft.fillCircle(cx, cy, STEP_W/2 - 1, color); break; 
        case 2: tft.fillTriangle(cx, y, x, y+STEP_H-1, x+STEP_W, y+STEP_H-1, color); break; 
        case 3: tft.drawLine(x, y, x+STEP_W, y+STEP_H, color); tft.drawLine(x, y+STEP_H, x+STEP_W, y, color); 
                tft.drawLine(x+1, y, x+STEP_W+1, y+STEP_H, color); tft.drawLine(x+1, y+STEP_H, x+STEP_W+1, y, color); break; 
    } 
}

// 3. 스텝 그리기 (페이지 개념 적용)
void redrawStep(int step) {
    // 현재 커서가 위치한 페이지(0~3) 계산
    int currentCursorPage = drumCursorStep / 16;
    
    // 요청 들어온 step이 현재 보여야 할 페이지에 속해 있는지 확인
    int stepPage = step / 16;
    
    // 다른 페이지의 스텝이라면 그리지 않음 (화면 갱신 방지)
    if (stepPage != currentCursorPage) return;

    // 화면 그리기용 상대 좌표 (0~15)
    int displayIdx = step % 16;

    for(int t=0; t<8; t++) { 
        int groupGap = (displayIdx / 4) * BEAT_GAP; 
        int x = START_X + (displayIdx * (STEP_W + GAP)) + groupGap; 
        int y = START_Y + (t * TRACK_H);
        
        uint16_t bgColor = TFT_BLACK; 
        // 현재 재생 중인 스텝 강조 (현재 페이지에 있을 때만)
        if(isSequencerPlaying && step == currentStep) bgColor = COLOR_BG_PLAY; 
        
        tft.fillRect(x-2, y-2, STEP_W+4, STEP_H+4, bgColor);
        
        if(isSequencerPlaying && step == currentStep) tft.drawRect(x-1, y-1, STEP_W+2, STEP_H+2, TFT_DARKGREY);
        
        if(drumPattern[t][step]) drawShape(t, x, y, COLOR_ON); 
        else tft.drawPixel(x + STEP_W/2, y + STEP_H/2, COLOR_OFF);
        
        // 커서 그리기
        if(t == drumCursorTrack && step == drumCursorStep) tft.drawRect(x-2, y-2, STEP_W+4, STEP_H+4, TFT_WHITE);
    }
}

// [사운드 에디터 UI] (SoundDesign 의존성 제거됨)
void drawSoundEditor() {
    DrumSound* ds = (currentMode == MODE_DRUM) ? &drumSounds[editTargetIndex] : 
                    (currentMode == MODE_TRACKER) ? &drumSounds[editTargetIndex] : 
                    &fingerSounds[editTargetIndex];

    img.fillSprite(TFT_BLACK);

    int graphX = 10;
    int graphY = 80;  
    int graphH = 50;  
    int graphW = 220; 

    int wA = map(ds->attack, 0, 500, 5, 60); 
    int wD = map(ds->decay,  10, 2000, 5, 60);
    int wS = 40; 
    int wR = map(ds->release,0, 2000, 5, 60);
    int hS = map(ds->sustain, 0, 255, 0, graphH);

    int x0 = graphX;
    int y0 = graphY;
    int x1 = x0 + wA; int y1 = graphY - graphH; 
    int x2 = x1 + wD; int y2 = graphY - hS;     
    int x3 = x2 + wS; int y3 = y2;             
    int x4 = x3 + wR; int y4 = graphY;         

    uint16_t colFill = 0x18E3; 
    uint16_t colLine = TFT_WHITE;

    img.fillTriangle(x0, y0, x1, y1, x1, y0, colFill); 
    img.fillTriangle(x1, y1, x2, y2, x1, y0, colFill); 
    img.fillTriangle(x1, y0, x2, y2, x2, y0, colFill); 
    img.fillRect(x2, y2, wS, hS, colFill);             
    img.fillTriangle(x3, y3, x4, y4, x3, y0, colFill); 

    img.drawLine(x0, y0, x1, y1, colLine);
    img.drawLine(x1, y1, x2, y2, colLine);
    img.drawLine(x2, y2, x3, y3, colLine);
    img.drawLine(x3, y3, x4, y4, colLine);

    img.setTextDatum(TL_DATUM);
    img.setTextColor(TFT_CYAN, TFT_BLACK); 
    img.drawString("TRACK " + String(editTargetIndex + 1) + " EDITOR", 10, 5);

    const char* labels[] = {"ATK", "DEC", "SUS", "REL", "PIT", "WAV", "MOD"};
    int values[] = {ds->attack, ds->decay, ds->sustain, ds->release, ds->pitch, ds->waveform, ds->pitchMod};
    const char* waveNames[] = {"SIN", "TRI", "SAW", "SQR", "NSE"};

    int startY = 90;
    
    for(int i=0; i<7; i++) {
        int r = i / 4; 
        int c = i % 4;
        int px = 10 + (c * 58);
        int py = startY + (r * 20);

        uint16_t color = (i == editParamIndex) ? TFT_YELLOW : TFT_DARKGREY;
        img.setTextColor(color, TFT_BLACK);
        
        String valStr = String(values[i]);
        if(i == 5) valStr = waveNames[values[i]];
        
        if(i == editParamIndex) img.drawString(">", px-8, py);
        
        img.drawString(labels[i], px, py);
        img.setTextColor((i == editParamIndex) ? TFT_WHITE : TFT_SILVER, TFT_BLACK);
        img.drawString(valStr, px, py + 10);
    }
    img.pushSprite(0, 0);
}

// [설정 팝업 그리기]
void drawDrumSettings() {
    int w = 180; int h = 130; 
    int x = (240 - w) / 2; int y = (135 - h) / 2;

    // 팝업 배경
    tft.fillRect(x, y, w, h, TFT_BLACK);
    tft.drawRect(x, y, w, h, TFT_WHITE);
    
    // 제목
    tft.setTextSize(1);
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString("DRUM SETTINGS", 120, y + 8);
    
    // 메뉴 항목들
    const char* items[] = {"Length", "Gap (Visual)", "Div (Speed)"};
    String values[3];
    
    // 0. Length 값
    values[0] = String(sequenceLength);
    
    // 1. Gap 값
    values[1] = (stepsPerBeat == 4) ? "4 (Normal)" : "3 (Triplet)";
    
    // 2. Div 값
    values[2] = (noteResolution == 4) ? "1/16" : "1/32";

    int startY = y + 35;
    int gapY = 25;

    tft.setTextDatum(TL_DATUM);

    for(int i=0; i<3; i++) {
        int itemY = startY + (i * gapY);
        
        // 커서 및 하이라이트 처리
        if (i == drumSettingsCursor) {
            tft.setTextColor(TFT_YELLOW, TFT_BLACK); // 선택됨: 노란색
            tft.drawString("> " + String(items[i]), x + 15, itemY);
            
            // 값 표시 (오른쪽 정렬 느낌으로)
            tft.setTextDatum(TR_DATUM);
            tft.drawString(values[i], x + w - 15, itemY);
            tft.setTextDatum(TL_DATUM); // 복구
        } else {
            tft.setTextColor(TFT_DARKGREY, TFT_BLACK); // 선택 안됨: 회색
            tft.drawString("  " + String(items[i]), x + 15, itemY);
            
            tft.setTextDatum(TR_DATUM);
            tft.drawString(values[i], x + w - 15, itemY);
            tft.setTextDatum(TL_DATUM);
        }
    }

    // 하단 안내
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("Y: Close", 120, y + 110);
}

// [메인 드럼 UI 함수]
void drawDrumMachine(bool fullRedraw = false) {
    if (isSoundEditMode) { drawSoundEditor(); return; }
    
    // 설정 메뉴가 열려있으면 메뉴만 그리고 리턴
    if (isDrumSettingsOpen) { drawDrumSettings(); return; }

    // 상단바
    tft.setTextSize(1); tft.setTextDatum(TR_DATUM); 
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK); 
    
    int currentPage = (drumCursorStep / 16) + 1;
    int totalPages = sequenceLength / 16;
    char buf[32];
    sprintf(buf, "BPM %d  (%d/%d)", bpm, currentPage, totalPages);
    tft.drawString(buf, 235, 3);
    
    tft.setTextDatum(TL_DATUM);
    if(isSequencerPlaying) { tft.setTextColor(TFT_GREEN, TFT_BLACK); tft.drawString("PLAY", 5, 3); } 
    else { tft.setTextColor(TFT_RED, TFT_BLACK); tft.drawString("STOP", 5, 3); }
    
    if (fullRedraw) { 
        drawDrumBackground(); 
        // 현재 페이지에 해당하는 16개 스텝만 다시 그림
        int pageStart = (drumCursorStep / 16) * 16;
        for(int i=0; i<16; i++) redrawStep(pageStart + i); 
    }
}

// FingerDrumUI (그대로 유지)
bool fingerBtnState[8] = {0}; 
void drawFingerDrumUI() {
    if (isSoundEditMode) { drawSoundEditor(); return; }
    tft.fillScreen(TFT_BLACK); tft.setTextSize(2); tft.setTextDatum(TC_DATUM); tft.setTextColor(TFT_CYAN, TFT_BLACK); tft.drawString("FINGER DRUM", 120, 10);
    tft.setTextSize(1); tft.setTextColor(TFT_DARKGREY, TFT_BLACK); tft.drawCentreString("Hold BTN + Click ENC1 to Edit", 120, 120, 1);
    int cx = 60; int cy = 70; int r = 15; int dpadX[] = {cx, cx, cx-25, cx+25}; int dpadY[] = {cy-25, cy+25, cy, cy};
    int bx = 180; int by = 70; int abxyX[] = {bx+25, bx, bx, bx-25}; int abxyY[] = {by, by+25, by-25, by};
    const char* labels[] = {"U", "D", "L", "R", "A", "B", "X", "Y"};
    for(int i=0; i<8; i++) {
        int x = (i<4) ? dpadX[i] : abxyX[i-4]; int y = (i<4) ? dpadY[i] : abxyY[i-4];
        if(fingerBtnState[i]) { tft.fillCircle(x, y, r, TFT_WHITE); tft.setTextColor(TFT_BLACK); } 
        else { tft.drawCircle(x, y, r, TFT_WHITE); tft.setTextColor(TFT_WHITE); }
        tft.setTextDatum(MC_DATUM); tft.drawString(labels[i], x, y);
    }
}

#endif