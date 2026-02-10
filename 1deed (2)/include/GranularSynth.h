#ifndef GRANULAR_SYNTH_H
#define GRANULAR_SYNTH_H

#include "Globals.h"

struct WavHeader {
    char riff[4]; uint32_t overallSize; char wave[4];
    char fmt[4]; uint32_t fmtLength; uint16_t formatType; uint16_t channels;
    uint32_t sampleRate; uint32_t byteRate; uint16_t blockAlign; uint16_t bitsPerSample;
    char data[4]; uint32_t dataSize;
};

float currentPlayHead = 0.0f;

// ------------------------------------------------------------------
// 1. WAV 파일을 PSRAM(램)으로 로딩하는 함수
// ------------------------------------------------------------------
// [스마트 WAV 로더] 메타데이터가 있어도 data 청크를 찾아냄
bool loadWavToPsram(String filename) {
    size_t totalRam = ESP.getPsramSize();
    size_t freeRam = ESP.getFreePsram();

    // 1. 메모리 초기화
    if (isSampleLoaded && sampleBuffer != NULL) {
        free(sampleBuffer);
        isSampleLoaded = false;
    }

    // 2. 파일 열기
    File32 file = sd.open(filename);
    if (!file) { 
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.drawCentreString("File Error!", 120, 120, 2);
        delay(1000); return false; 
    }

    // 3. RIFF 헤더 확인 (첫 12바이트)
    char id[5]; id[4] = 0;
    file.read(id, 4); // "RIFF"
    if (String(id) != "RIFF") { file.close(); return false; }
    
    file.seek(8);
    file.read(id, 4); // "WAVE"
    if (String(id) != "WAVE") { file.close(); return false; }

    // 4. 청크 탐색 (fmt와 data 찾기)
    uint32_t dataSize = 0;
    bool fmtFound = false;
    
    // 무한 루프로 청크를 하나씩 검사
    while (file.available()) {
        char chunkID[5]; chunkID[4] = 0;
        file.read(chunkID, 4); // 청크 이름 읽기 (fmt , data, JUNK 등)
        
        uint32_t chunkSize;
        file.read(&chunkSize, 4); // 청크 크기 읽기

        // [fmt ] 청크 발견: 포맷 확인
        if (String(chunkID) == "fmt ") {
            uint16_t formatCode; file.read(&formatCode, 2);
            uint16_t channels;   file.read(&channels, 2);
            uint32_t sampleRate; file.read(&sampleRate, 4);
            file.seek(file.position() + 6); // byteRate, blockAlign 건너뜀
            uint16_t bitsPerSample; file.read(&bitsPerSample, 2);
            
            // 16비트가 아니면 탈락
            if (bitsPerSample != 16) {
                tft.setTextColor(TFT_RED, TFT_BLACK);
                tft.drawCentreString("16bit Only!", 120, 120, 2);
                delay(1000); file.close(); return false;
            }
            
            // 남은 fmt 청크 데이터 건너뛰기
            int readSoFar = 16; 
            if (chunkSize > readSoFar) file.seek(file.position() + (chunkSize - readSoFar));
            fmtFound = true;
        }
        // [data] 청크 발견: 여기가 진짜 오디오 데이터!
        else if (String(chunkID) == "data") {
            dataSize = chunkSize;
            break; // 찾았으니 루프 종료 (현재 파일 포인터는 데이터 시작점)
        }
        // [기타] 쓸모없는 청크 (JUNK, LIST, ID3 등) -> 건너뛰기
        else {
            file.seek(file.position() + chunkSize);
        }
    }

    if (!fmtFound || dataSize == 0) {
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.drawCentreString("Bad WAV Format", 120, 120, 2);
        delay(1000); file.close(); return false;
    }

    // 5. 메모리 할당 및 로딩 (기존 로직과 동일)
    uint32_t numSamples = dataSize / 2;
    if (numSamples > 3000000) numSamples = 3000000; 

    // 화면 UI 업데이트
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawCentreString("LOADING...", 120, 20, 2);
    tft.setTextColor(TFT_WHITE);
    tft.drawString(String(numSamples * 2 / 1024) + " KB", 20, 60);

    sampleBuffer = (int16_t*) ps_malloc(numSamples * sizeof(int16_t));
    if (sampleBuffer == NULL) { 
        tft.drawCentreString("RAM FULL!", 120, 100, 2);
        delay(1000); file.close(); return false; 
    }

    // 데이터 읽기
    uint8_t buf[512];
    uint32_t loaded = 0;
    int loopCount = 0;
    
    tft.drawRect(20, 100, 200, 10, TFT_WHITE); // 로딩바

    while(file.available() && loaded < numSamples) {
        int bytesRead = file.read(buf, 512);
        for(int i=0; i<bytesRead; i+=2) {
            if (loaded >= numSamples) break;
            int16_t sample = (int16_t)(buf[i] | (buf[i+1] << 8));
            sampleBuffer[loaded++] = sample;
        }
        
        loopCount++;
        if (loopCount % 100 == 0) {
            delay(1); // 워치독 방지
            int p = (loaded * 200) / numSamples;
            tft.fillRect(20, 100, p, 10, TFT_GREEN);
        }
    }
    
    sampleLength = loaded;
    isSampleLoaded = true;
    file.close();
    
    grainPosition = 0.5f; 
    grainSize = 5000.0f;  
    grainPitch = 1.0f;
    return true;
}

// ------------------------------------------------------------------
// 2. 오디오 출력 함수
// ------------------------------------------------------------------
int16_t getGranularSample() {
    if (!isSampleLoaded || sampleBuffer == NULL) return 0;

    float startIdx = (float)sampleLength * grainPosition;
    float endIdx = startIdx + grainSize;

    if (endIdx >= sampleLength) endIdx = sampleLength - 1;
    if (startIdx >= endIdx) startIdx = 0;

    int index = (int)currentPlayHead;
    
    if (index >= sampleLength) index = sampleLength - 1;
    if (index < 0) index = 0;

    int16_t output = sampleBuffer[index];

    currentPlayHead += grainPitch;

    if (currentPlayHead >= endIdx) currentPlayHead = startIdx;
    if (currentPlayHead < startIdx) currentPlayHead = startIdx;

    return output;
}

// ------------------------------------------------------------------
// 3. 화면 UI
// ------------------------------------------------------------------
void drawGranularUI() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("GRANULAR SYNTH", 120, 10);
    
    tft.drawRect(20, 40, 200, 60, TFT_WHITE);
    int barPos = (int)(grainPosition * 200);
    if(barPos < 0) barPos = 0;
    if(barPos > 200) barPos = 200;
    
    tft.drawLine(20 + barPos, 40, 20 + barPos, 100, TFT_RED);
    
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("POS: " + String((int)(grainPosition*100)) + "%", 20, 120);
    tft.drawString("SIZE: " + String((int)grainSize), 20, 140);
    tft.drawString("PITCH: " + String(grainPitch), 20, 160);
    
    // [보너스] 현재 로딩된 샘플 용량 표시
    if (isSampleLoaded) {
        tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
        String memStr = "MEM: " + String(sampleLength * 2 / 1024) + " KB";
        tft.drawString(memStr, 20, 185);
    }

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawCentreString("A: Reload  B: Exit", 120, 220, 1);
}

#endif