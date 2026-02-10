#ifndef SAMPLE_LOADER_H
#define SAMPLE_LOADER_H

#include "Globals.h"

// 특정 트랙에 WAV 파일 로드 (헤더 파싱 포함)
bool loadSampleToTrack(int trackIdx, String filename) {
    if (trackIdx < 0 || trackIdx >= 8) return false;

    // 1. 기존 메모리 해제
    if (trackSampleBuffers[trackIdx] != NULL) {
        free(trackSampleBuffers[trackIdx]);
        trackSampleBuffers[trackIdx] = NULL;
    }
    isTrackSampleLoaded[trackIdx] = false;

    // 2. 파일 열기
    File32 file = sd.open(filename);
    if (!file) {
        tft.fillScreen(TFT_BLACK);
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.drawCentreString("File Error!", 120, 120, 2);
        delay(1000); 
        return false;
    }

    // 3. RIFF/WAVE 확인 및 DATA 청크 탐색
    char id[5]; id[4] = 0;
    file.read(id, 4); 
    if (String(id) != "RIFF") { file.close(); return false; }
    
    file.seek(8); file.read(id, 4);
    if (String(id) != "WAVE") { file.close(); return false; }

    uint32_t dataSize = 0;
    bool dataFound = false;

    // 청크 탐색 루프
    while (file.available()) {
        char chunkID[5]; chunkID[4] = 0;
        file.read(chunkID, 4);
        uint32_t chunkSize;
        file.read(&chunkSize, 4);

        if (String(chunkID) == "data") {
            dataSize = chunkSize;
            dataFound = true;
            break; // 현재 file pointer는 data 시작점
        } else {
            file.seek(file.position() + chunkSize); // 건너뛰기
        }
    }

    if (!dataFound || dataSize == 0) {
        tft.drawCentreString("No Data Chunk!", 120, 120, 2);
        delay(1000); file.close(); return false;
    }

    // 4. 메모리 할당 (최대 1MB 제한 예시)
    uint32_t numSamples = dataSize / 2;
    if (numSamples > 500000) numSamples = 500000; // 약 1MB

    // UI 표시
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawCentreString("LOADING...", 120, 60, 2);

    trackSampleBuffers[trackIdx] = (int16_t*) ps_malloc(numSamples * sizeof(int16_t));
    if (trackSampleBuffers[trackIdx] == NULL) { 
        tft.drawCentreString("PSRAM FULL!", 120, 100, 2);
        delay(1000); file.close(); return false; 
    }

    // 5. 데이터 읽기
    file.read((uint8_t*)trackSampleBuffers[trackIdx], numSamples * 2);
    
    trackSampleLengths[trackIdx] = numSamples;
    isTrackSampleLoaded[trackIdx] = true;
    
    // Trim 초기화 (전체 구간)
    drumSounds[trackIdx].sampleStart = 0;
    drumSounds[trackIdx].sampleEnd = 1000;
    
    file.close();
    return true;
}

#endif