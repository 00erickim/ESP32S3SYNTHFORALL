#ifndef MIXER_UI_H
#define MIXER_UI_H

#include "Globals.h"

const int MIXER_COL_W = 60;   
const int BAR_W = 24;         
const int BAR_H = 100;        
const int BAR_TOP = 40;       
const int SEL_BOX_PAD = 6;    

void redrawMixerTrack(int i) {
    int xBase = i * MIXER_COL_W; 
    int centerX = xBase + (MIXER_COL_W / 2);
    tft.fillRect(xBase, 26, MIXER_COL_W, 160, TFT_BLACK);
    bool isSelected = (i == currentMixerTrack);
    uint16_t color = isSelected ? TFT_WHITE : TFT_DARKGREY;
    if (isSelected) tft.drawRect(centerX - (BAR_W/2) - SEL_BOX_PAD, BAR_TOP - SEL_BOX_PAD, BAR_W + (SEL_BOX_PAD*2), BAR_H + (SEL_BOX_PAD*2), TFT_WHITE);
    tft.drawRect(centerX - (BAR_W/2), BAR_TOP, BAR_W, BAR_H, color);
    int volHeight = map(trackVolume[i], 0, 255, 0, BAR_H);
    if (volHeight > 0) tft.fillRect(centerX - (BAR_W/2) + 2, (BAR_TOP + BAR_H) - volHeight + 1, BAR_W - 4, volHeight - 2, color);
}

void redrawPlayStatus() {
    tft.fillRect(80, 200, 80, 25, TFT_BLACK); 
    if(isSequencerPlaying) { tft.fillRect(80, 205, 80, 20, TFT_WHITE); tft.setTextColor(TFT_BLACK, TFT_WHITE); tft.setTextDatum(MC_DATUM); tft.drawString(">> PLAY <<", 120, 215); } 
    else { tft.drawRect(80, 205, 80, 20, TFT_WHITE); tft.setTextColor(TFT_WHITE, TFT_BLACK); tft.setTextDatum(MC_DATUM); tft.drawString("STOPPED", 120, 215); }
}

void initMixerUI() {
    tft.fillScreen(TFT_BLACK);
    tft.fillRect(0, 0, 240, 25, TFT_WHITE); tft.setTextColor(TFT_BLACK, TFT_WHITE); tft.setTextDatum(MC_DATUM); tft.setTextSize(2); tft.drawString("MIXER", 120, 12); tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK); tft.setTextDatum(MC_DATUM); tft.drawString("[A] OPEN  [X] PLAY", 120, 190);
    for(int i=0; i<4; i++) redrawMixerTrack(i);
    redrawPlayStatus();
}

#endif