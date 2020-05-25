#ifndef _SPEAKER_H_
#define _SPEAKER_H_

#include "Arduino.h"
#include "Config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern "C" {
#include "esp32-hal-dac.h"
}

struct audiotask {
	uint8_t *buff;
	int len;
	int pos;
	int bufsize;
	int samplerate;
};

class SPEAKER {
public:
    SPEAKER(void);
    
    void begin();
    void end();
    void mute();
    void tone(uint16_t frequency);
    void tone(uint16_t frequency, uint32_t duration);
    void tone2(uint16_t frequency);
    void tone2(uint16_t frequency, uint32_t duration);
    void tone3(uint16_t frequency);
    void tone3(uint16_t frequency, uint32_t duration);
    void beep();
    void setBeep(uint16_t frequency, uint16_t duration);
    void update();
    
    void write(uint8_t value);
    void setVolume(uint8_t volume);
    void playMusic(const uint8_t* music_data, uint16_t sample_rate);
    void playMusic(const uint8_t* music_data, uint16_t sample_rate, int length);
    void playMusicThreaded(const uint8_t* music_data, uint16_t sample_rate, int length);

private:
    uint32_t _count;
    uint32_t _count2;
    uint32_t _count3;
    uint16_t _beep_duration;
    uint16_t _beep_freq;
    bool speaker_on;
    bool speaker_on2;
    bool speaker_on3;
    TaskHandle_t Task1;
    TaskHandle_t BeepTask;
    TaskHandle_t BeepTask2;
    TaskHandle_t BeepTask3;
    TaskHandle_t BeepTask4;
};

#endif
