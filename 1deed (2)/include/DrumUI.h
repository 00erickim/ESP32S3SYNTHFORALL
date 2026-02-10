#ifndef DRUM_UI_H
#define DRUM_UI_H

#include <TFT_eSPI.h>
#include "Globals.h"
#include "FileManager.h" 

// 디자인 상수
const int LABEL_W = 42;  
const int START_X = 46;  
const int START_Y = 14;  
const int TRACK_H = 15;  
const int STEP_H_CONST = 10; 

const uint16_t COLOR_ON = TFT_WHITE;        
const uint16_t COLOR_OFF = 0x39E7;          
const uint16_t COLOR_BG_PLAY = 0x2124;      

int getStepsPerPage() { return (noteResolution == 8) ? 32 : 16; }

// [★수정됨] 트랙 라벨 그리기 (선택 시 강조)
void drawDrumLabels() {
    tft.setTextSize(1);
    tft.setTextDatum(TL_DATUM);
    
    for(int i=0; i<8; i++) {
        int y = START_Y + (i * TRACK_H) + 3; 
        int boxY = START_Y + (i * TRACK_H);
        
        // 1. 커서가 헤더(-1)에 있고, 현재 트랙인 경우 -> 선택됨 (빨간 배경)
        if (drumCursorStep == -1 && i == drumCursorTrack) {
            tft.fillRect(0, boxY, LABEL_W + 2, TRACK_H - 1, TFT_RED);
            tft.setTextColor(TFT_WHITE, TFT_RED);
        }
        // 2. 그냥 현재 트랙 줄에 있는 경우 -> 노란 글씨
        else if (i == drumCursorTrack) {
            tft.fillRect(0, boxY, LABEL_W + 2, TRACK_H - 1, TFT_BLACK); // 배경 지우기
            tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        }
        // 3. 선택되지 않음 -> 어두운 글씨
        else {
            tft.fillRect(0, boxY, LABEL_W + 2, TRACK_H - 1, TFT_BLACK); // 배경 지우기
            tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
        }
        
        // 현재는 무조건 "SYTH"만 표시
        String label = String(i + 1) + " SYTH";
        tft.drawString(label, 2, y);
    }
}

void drawDrumBackground() { 
    tft.fillScreen(TFT_BLACK); drawDrumLabels();
    tft.setTextSize(1); tft.setTextDatum(TR_DATUM); tft.setTextColor(TFT_DARKGREY, TFT_BLACK); 
    int spp = getStepsPerPage();
    int stride = (noteResolution == 4) ? 2 : 1;
    // -1일 때는 0으로 취급하여 페이지 계산
    int validCursor = (drumCursorStep == -1) ? 0 : drumCursorStep;
    int totalVisualSteps = sequenceLength / stride;
    int currentVisualStep = validCursor / stride;
    int currentPage = (currentVisualStep / spp) + 1;
    int totalPages = (totalVisualSteps + spp - 1) / spp; 
    char buf[32]; sprintf(buf, "BPM %d  (%d/%d)", bpm, currentPage, totalPages); 
    tft.drawString(buf, 235, 3);
}

void drawShape(int t, int x, int y, int w, int h, uint16_t color) { 
    int cx = x + w/2; int cy = y + h/2; 
    switch(t % 4) { 
        case 0: tft.fillRect(x, y, w, h, color); break; 
        case 1: tft.fillCircle(cx, cy, w/2 - 1, color); break; 
        case 2: tft.fillTriangle(cx, y, x, y+h-1, x+w, y+h-1, color); break; 
        case 3: tft.drawLine(x, y, x+w, y+h, color); tft.drawLine(x, y+h, x+w, y, color); 
                tft.drawLine(x+1, y, x+w+1, y+h, color); tft.drawLine(x+1, y+h, x+w+1, y, color); break; 
    } 
}

void redrawStep(int stepIndex) { 
    int spp = getStepsPerPage(); 
    if (noteResolution == 4 && (stepIndex % 2 != 0)) return;
    int stride = (noteResolution == 4) ? 2 : 1;
    int visualIdx = stepIndex / stride;
    
    // -1일 때는 0페이지를 보여줌
    int validCursor = (drumCursorStep == -1) ? 0 : drumCursorStep;
    int currentVisualPage = (validCursor / stride) / spp;
    int stepVisualPage = visualIdx / spp;
    
    if (stepVisualPage != currentVisualPage) return;
    
    int stepW, stepH, gap, beatGap;
    stepH = STEP_H_CONST; 
    if (spp == 32) { stepW = 4; gap = 1; beatGap = 2; } 
    else { stepW = 9; gap = 2; beatGap = 3; }
    
    int displayIdx = visualIdx % spp;
    for(int t=0; t<8; t++) { 
        int groupGap = (displayIdx / stepsPerBeat) * beatGap; 
        int x = START_X + (displayIdx * (stepW + gap)) + groupGap; 
        int y = START_Y + (t * TRACK_H) + 2; 
        uint16_t bgColor = TFT_BLACK; 
        if(isSequencerPlaying && stepIndex == currentStep) bgColor = COLOR_BG_PLAY; 
        tft.fillRect(x-1, y-1, stepW+2, stepH+2, bgColor); 
        if(isSequencerPlaying && stepIndex == currentStep) tft.drawRect(x-1, y-1, stepW+2, stepH+2, TFT_DARKGREY);
        if(drumPattern[t][stepIndex]) drawShape(t, x, y, stepW, stepH, COLOR_ON); 
        else tft.drawPixel(x + stepW/2, y + stepH/2, COLOR_OFF);
        
        // 커서 그리기: stepIndex가 현재 커서와 같을 때만 (헤더 선택 중일 땐 여기 안 그려짐)
        if(t == drumCursorTrack && stepIndex == drumCursorStep) tft.drawRect(x-1, y-1, stepW+2, stepH+2, TFT_WHITE);
    }
}

// (drawSoundEditor, drawDrumSettings, drawDrumMachine, drawFingerDrumUI는 기존과 동일 - 생략 없이 전체 코드 필요시 이전 답변 참고)
void drawSoundEditor() {
    DrumSound* ds = (currentMode == MODE_DRUM) ? &drumSounds[editTargetIndex] : (currentMode == MODE_TRACKER) ? &drumSounds[editTargetIndex] : &fingerSounds[editTargetIndex];
    img.fillSprite(TFT_BLACK); int graphX = 10; int graphY = 80; int graphH = 50;
    int wA = map(ds->attack, 0, 500, 5, 60); int wD = map(ds->decay,  10, 2000, 5, 60); int wS = 40; int wR = map(ds->release,0, 2000, 5, 60); int hS = map(ds->sustain, 0, 255, 0, graphH);
    int x0 = graphX; int y0 = graphY; int x1 = x0 + wA; int y1 = graphY - graphH; int x2 = x1 + wD; int y2 = graphY - hS; int x3 = x2 + wS; int y3 = y2; int x4 = x3 + wR; int y4 = graphY;         
    uint16_t colFill = 0x18E3; uint16_t colLine = TFT_WHITE;
    img.fillTriangle(x0, y0, x1, y1, x1, y0, colFill); img.fillTriangle(x1, y1, x2, y2, x1, y0, colFill); img.fillTriangle(x1, y0, x2, y2, x2, y0, colFill); img.fillRect(x2, y2, wS, hS, colFill); img.fillTriangle(x3, y3, x4, y4, x3, y0, colFill); 
    img.drawLine(x0, y0, x1, y1, colLine); img.drawLine(x1, y1, x2, y2, colLine); img.drawLine(x2, y2, x3, y3, colLine); img.drawLine(x3, y3, x4, y4, colLine);
    img.setTextDatum(TL_DATUM); img.setTextColor(TFT_CYAN, TFT_BLACK); img.drawString("TRACK " + String(editTargetIndex + 1) + " EDITOR", 10, 5);
    const char* labels[] = {"ATK", "DEC", "SUS", "REL", "PIT", "WAV", "MOD"}; int values[] = {ds->attack, ds->decay, ds->sustain, ds->release, ds->pitch, ds->waveform, ds->pitchMod}; const char* waveNames[] = {"SIN", "TRI", "SAW", "SQR", "NSE"}; int startY = 90;
    for(int i=0; i<7; i++) { int r = i / 4; int c = i % 4; int px = 10 + (c * 58); int py = startY + (r * 20); uint16_t color = (i == editParamIndex) ? TFT_YELLOW : TFT_DARKGREY; img.setTextColor(color, TFT_BLACK); String valStr = String(values[i]); if(i == 5) valStr = waveNames[values[i]]; if(i == editParamIndex) img.drawString(">", px-8, py); img.drawString(labels[i], px, py); img.setTextColor((i == editParamIndex) ? TFT_WHITE : TFT_SILVER, TFT_BLACK); img.drawString(valStr, px, py + 10); } img.pushSprite(0, 0);
}

void drawDrumSettings() {
    int w = 200; int h = 125; int x = (240 - w) / 2; int y = (135 - h) / 2;
    tft.fillRect(x, y, w, h, TFT_BLACK); tft.drawRect(x, y, w, h, TFT_WHITE);
    tft.setTextSize(1); tft.setTextDatum(TC_DATUM); tft.setTextColor(TFT_CYAN, TFT_BLACK); tft.drawString("SETTINGS & PROJECT", 120, y + 5);
    const char* items[] = {"Length", "Gap", "Div", "Slot", "SAVE", "LOAD"}; const int itemCount = 6;
    String values[itemCount];
    int displayLen = (noteResolution == 4) ? sequenceLength / 2 : sequenceLength;
    values[0] = String(displayLen); values[1] = (stepsPerBeat == 4) ? "4" : "3"; values[2] = (noteResolution == 4) ? "1/16" : "1/32";
    bool exists = checkSlotExists(drumProjectSlot); values[3] = String(drumProjectSlot + 1) + (exists ? " [DATA]" : " [EMPTY]");
    values[4] = "[Press A]"; values[5] = "[Press A]";
    int startY = y + 22; int gapY = 16; tft.setTextDatum(TL_DATUM);
    for(int i=0; i<itemCount; i++) { int itemY = startY + (i * gapY); if (i == drumSettingsCursor) { tft.setTextColor(TFT_YELLOW, TFT_BLACK); tft.drawString("> " + String(items[i]), x + 15, itemY); tft.setTextDatum(TR_DATUM); tft.drawString(values[i], x + w - 15, itemY); tft.setTextDatum(TL_DATUM); } else { tft.setTextColor(TFT_DARKGREY, TFT_BLACK); tft.drawString("  " + String(items[i]), x + 15, itemY); tft.setTextDatum(TR_DATUM); tft.drawString(values[i], x + w - 15, itemY); tft.setTextDatum(TL_DATUM); } }
    if (millis() - msgTime < 2000 && msgText != "") { int msgH = 24; int msgY = y + (h/2) - (msgH/2); tft.fillRect(x+20, msgY, w-40, msgH, TFT_RED); tft.drawRect(x+20, msgY, w-40, msgH, TFT_WHITE); tft.setTextColor(TFT_WHITE, TFT_RED); tft.setTextDatum(MC_DATUM); tft.drawString(msgText, 120, msgY + (msgH/2)); } 
    else { tft.setTextDatum(TC_DATUM); tft.setTextColor(TFT_WHITE, TFT_BLACK); tft.drawString("Y: Close", 120, y + h - 12); }
}

void drawDrumMachine(bool fullRedraw = false) {
    if (isSoundEditMode) { drawSoundEditor(); return; }
    if (isDrumSettingsOpen) { drawDrumSettings(); return; }
    
    // 라벨 업데이트를 위해 fullRedraw가 아니어도 라벨만 다시 그려야 함
    // (헤더 선택 시 빨간색 표시를 위해)
    drawDrumLabels(); 
    
    if (fullRedraw) {
        tft.fillScreen(TFT_BLACK); 
        drawDrumBackground(); // 배경과 라벨 다시 그리기
    }
    
    int stride = (noteResolution == 4) ? 2 : 1;
    int spp = getStepsPerPage();
    // -1일 때도 0페이지 보여줌
    int validCursor = (drumCursorStep == -1) ? 0 : drumCursorStep;
    int visualCursor = validCursor / stride;
    int pageStartVisual = (visualCursor / spp) * spp;
    
    for(int i=0; i<spp; i++) {
        int visualIdx = pageStartVisual + i;
        int dataIdx = visualIdx * stride;
        if(dataIdx < 64) redrawStep(dataIdx); 
    }
}
bool fingerBtnState[8] = {0}; 
void drawFingerDrumUI() {
    if (isSoundEditMode) { drawSoundEditor(); return; }
    tft.fillScreen(TFT_BLACK); tft.setTextSize(2); tft.setTextDatum(TC_DATUM); tft.setTextColor(TFT_CYAN, TFT_BLACK); tft.drawString("FINGER DRUM", 120, 10);
    tft.setTextSize(1); tft.setTextColor(TFT_DARKGREY, TFT_BLACK); tft.drawCentreString("Hold BTN + Click ENC1 to Edit", 120, 120, 1);
    int cx = 60; int cy = 70; int r = 15; int dpadX[] = {cx, cx, cx-25, cx+25}; int dpadY[] = {cy-25, cy+25, cy, cy}; int bx = 180; int by = 70; int abxyX[] = {bx+25, bx, bx, bx-25}; int abxyY[] = {by, by+25, by-25, by}; const char* labels[] = {"U", "D", "L", "R", "A", "B", "X", "Y"}; for(int i=0; i<8; i++) { int x = (i<4) ? dpadX[i] : abxyX[i-4]; int y = (i<4) ? dpadY[i] : abxyY[i-4]; if(fingerBtnState[i]) { tft.fillCircle(x, y, r, TFT_WHITE); tft.setTextColor(TFT_BLACK); } else { tft.drawCircle(x, y, r, TFT_WHITE); tft.setTextColor(TFT_WHITE); } tft.setTextDatum(MC_DATUM); tft.drawString(labels[i], x, y); }
}

#endif