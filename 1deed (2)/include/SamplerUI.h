#ifndef SAMPLER_UI_H
#define SAMPLER_UI_H

#include <TFT_eSPI.h>
#include "Globals.h"

// [샘플러 에디터 UI 그리기]
void drawSamplerEditor() {
    DrumSound* ds = &drumSounds[editTargetIndex];

    img.fillSprite(TFT_BLACK);
    
    // 헤더
    img.setTextDatum(TL_DATUM);
    img.setTextColor(TFT_MAGENTA, TFT_BLACK);
    img.drawString("TRACK " + String(editTargetIndex + 1) + " [SAMPLER]", 10, 5);

    // 그래프 (배경)
    int graphX = 10; int graphY = 80; int graphH = 50;
    int wA = map(ds->attack, 0, 500, 5, 60);
    int wD = map(ds->decay,  10, 2000, 5, 60);
    int wS = 40; 
    int wR = map(ds->release,0, 2000, 5, 60);
    int hS = map(ds->sustain, 0, 255, 0, graphH);

    int x0 = graphX; int y0 = graphY;
    int x1 = x0 + wA; int y1 = graphY - graphH;
    int x2 = x1 + wD; int y2 = graphY - hS;
    int x3 = x2 + wS; int y3 = y2;
    int x4 = x3 + wR; int y4 = graphY;
    
    uint16_t colFill = 0x5140; uint16_t colLine = TFT_WHITE;
    img.fillTriangle(x0, y0, x1, y1, x1, y0, colFill);
    img.fillTriangle(x1, y1, x2, y2, x1, y0, colFill);
    img.fillTriangle(x1, y0, x2, y2, x2, y0, colFill);
    img.fillRect(x2, y2, wS, hS, colFill);
    img.fillTriangle(x3, y3, x4, y4, x3, y0, colFill);
    img.drawLine(x0, y0, x1, y1, colLine); img.drawLine(x1, y1, x2, y2, colLine);
    img.drawLine(x2, y2, x3, y3, colLine); img.drawLine(x3, y3, x4, y4, colLine);

    // [★수정] 파라미터 목록 (LOAD, STRT, END 포함)
    const char* labels[] = {"ATK", "DEC", "SUS", "REL", "PIT", "LOAD", "STRT", "END"};
    int values[] = {
        ds->attack, ds->decay, ds->sustain, ds->release, 
        ds->pitch, 
        0, // LOAD (버튼)
        ds->sampleStart, 
        ds->sampleEnd
    };

    int startY = 90;
    for(int i=0; i<8; i++) {
        int r = i / 4;
        int c = i % 4;
        int px = 10 + (c * 58);
        int py = startY + (r * 20);
        
        uint16_t color = (i == editParamIndex) ? TFT_YELLOW : TFT_DARKGREY;
        img.setTextColor(color, TFT_BLACK);
        
        if(i == editParamIndex) img.drawString(">", px-8, py);
        img.drawString(labels[i], px, py);
        
        // 값 표시 처리
        String valStr;
        if (i == 5) {
            // LOAD 항목
            valStr = isTrackSampleLoaded[editTargetIndex] ? "[READY]" : "[EMPTY]";
        } else if (i == 6 || i == 7) {
            // STRT, END 항목 (% 표시)
            valStr = String(values[i] / 10.0, 1) + "%";
        } else {
            valStr = String(values[i]);
        }

        img.setTextColor((i == editParamIndex) ? TFT_WHITE : TFT_SILVER, TFT_BLACK);
        img.drawString(valStr, px, py + 10);
    }

    img.pushSprite(0, 0);
}

#endif