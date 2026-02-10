#ifndef SYNTH_UI_H
#define SYNTH_UI_H

#include "Globals.h"
#include "AudioEngine.h" // setSynthWaveform 사용을 위해

void drawKeyboardUI() { 
    tft.fillScreen(TFT_BLACK); 
    tft.setTextSize(2); 
    tft.setTextColor(TFT_MAGENTA, TFT_BLACK); 
    tft.drawString("POLY-SYNTH", 20, 10); 
    tft.setTextSize(1); 
    tft.setTextColor(TFT_WHITE, TFT_BLACK); 
    
    String waveName = "SINE"; 
    if(synthWaveform==1) waveName="SAW"; 
    else if(synthWaveform==2) waveName="SQUARE"; 
    
    tft.drawString("Wave: "+waveName, 20, 40); 
    tft.drawString("Filter: "+String(synthFilterVal), 130, 40); 
    
    int centerY=100; 
    for(int i=0; i<240; i+=2) { 
        int amp=(currentNote!=-1)?30:2; 
        int y=centerY; 
        if(synthWaveform==0) y+=sin(i*0.1)*amp; 
        else if(synthWaveform==1) y+=((i%20)-10)*amp/10; 
        else if(synthWaveform==2) y+=((i%40)<20?amp:-amp); 
        tft.drawPixel(i,y,TFT_GREEN); 
    } 
    
    tft.setTextColor(TFT_DARKGREY, TFT_BLACK); 
    tft.drawCentreString("Play Chords (Max 3)", 120, 150, 1); 
    
    if(currentNote!=-1) { 
        tft.setTextSize(3); 
        tft.setTextColor(TFT_YELLOW, TFT_BLACK); 
        const char* notes[]={"C","D","E","F","G","A","B","Hi-C"}; 
        tft.drawCentreString(notes[currentNote], 120, 180, 1); 
    } 
}

void drawDroneUI() { 
    tft.fillScreen(TFT_BLACK); 
    tft.setTextSize(2); 
    tft.setTextDatum(TC_DATUM); 
    tft.setTextColor(TFT_MAGENTA, TFT_BLACK); 
    tft.drawString("DRONE SYNTH", 120, 10); 
    
    int movingBar = 120 + (sin(millis() * lfoRate * 0.001) * lfoDepth / 2); 
    tft.drawRect(20, 50, 200, 20, TFT_DARKGREY); 
    tft.fillRect(movingBar - 5, 50, 10, 20, TFT_GREEN); 
    
    tft.setTextSize(1); 
    tft.setTextColor(TFT_GREEN, TFT_BLACK); 
    tft.drawCentreString("FILTER CUTOFF (LFO)", 120, 35, 1); 
    
    tft.setTextDatum(TL_DATUM); 
    tft.setTextColor(TFT_WHITE, TFT_BLACK); 
    char buf[32]; 
    sprintf(buf, "PITCH (Enc1): %d Hz", dronePitch); tft.drawString(buf, 20, 90); 
    sprintf(buf, "LFO RATE (Enc2): %d", lfoRate); tft.drawString(buf, 20, 110); 
    sprintf(buf, "DEPTH (Up/Dn): %d", lfoDepth); tft.drawString(buf, 20, 140); 
    sprintf(buf, "RES (Lt/Rt): %d", filterResonance); tft.drawString(buf, 20, 160); 
    
    tft.setTextColor(TFT_DARKGREY); 
    tft.drawCentreString("Press B to Exit", 120, 200, 1); 
}

#endif