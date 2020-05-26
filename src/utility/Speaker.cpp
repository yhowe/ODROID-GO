#include <sys/time.h>
#include "Speaker.h"
#define KEEPBUFFSZ (60 * 1024)

uint8_t _volume;
static void Task1code(void * parameter);
static void beepthread(void * parameter);
static void beepthread2(void * parameter);
static void beepthread3(void * parameter);
static void beepmix(void * parameter);
static int _beeptone = 0;
static int _beeptone2 = 0;
static int _beeptone3 = 0;
static uint8_t dacVal = 0;
static struct audiotask myaudiotask;
static bool sndbusy = false;
static int keepbuffcount = 0;
uint8_t dac1 = 0, dac2 = 0, dac3 = 0;

SPEAKER::SPEAKER(void) {
    _volume = 8;
}

void SPEAKER::begin() {
    pinMode(25, OUTPUT);
    digitalWrite(25, 0);
    digitalWrite(SPEAKER_PIN, 0);
    setBeep(1000, 100);
    speaker_on = 0;
    speaker_on2 = 0;
    speaker_on3 = 0;
    mute();
    xTaskCreatePinnedToCore(beepthread, "BeepGen", 10000, &_beeptone, 0,
	&BeepTask, xPortGetCoreID() == 1 ? 0 : 1);
    #if 0
    xTaskCreatePinnedToCore(beepthread2, "BeepGen2", 10000, &_beeptone2, 0,
	&BeepTask2, xPortGetCoreID() == 1 ? 0 : 1);
    xTaskCreatePinnedToCore(beepthread3, "BeepGen3", 10000, &_beeptone3, 0,
        &BeepTask3, xPortGetCoreID() == 1 ? 0 : 1);
    xTaskCreatePinnedToCore(beepmix, "BeepMix", 10000, NULL, 1,
        &BeepTask4, xPortGetCoreID() == 1 ? 0 : 1);
    #endif
    xTaskCreatePinnedToCore(
	  Task1code, /* Function to implement the task */
	"Task1", /* Name of the task */
      10000,  /* Stack size in words */
    &myaudiotask,  /* Task input parameter */
	  0,  /* Priority of the task */
		&Task1,  /* Task handle. */
      xPortGetCoreID() == 1 ? 0 : 1); /* Core where the task should run */
}

static void beepthread3(void * parameter) {
	for(;;) {
		struct timeval now;
		long int lasttime;
		static int mycount = 0;
		static long int next_update = 0;
		int frequency = *(int *)parameter;
		if (frequency <= 0) {
			vTaskDelay(1);
            	//	dacWrite(SPEAKER_PIN,0);
			next_update = 0;
			continue;
		}
		uint16_t delay_interval = ((uint32_t)1000000/frequency / 2);
		gettimeofday(&now, NULL);
		lasttime = now.tv_sec * 1000000 + now.tv_usec;
		if (lasttime > next_update) {
			next_update = lasttime + delay_interval;

			if(_volume != 11 && mycount % 2 == 0)
            		    dac3 = 32 / _volume;
			else if (mycount % 2)
            		    dac3 = 0;
			mycount++;
            		dacWrite(SPEAKER_PIN, dac1 + dac2 + dac3);
		}
		usleep(delay_interval);
	}
}

static void beepthread2(void * parameter) {
	for(;;) {
		struct timeval now;
		long int lasttime;
		static int mycount = 0;
		static long int next_update = 0;
		int frequency = *(int *)parameter;
		if (frequency <= 0) {
			vTaskDelay(1);
            	//	dacWrite(SPEAKER_PIN,0);
			next_update = 0;
			continue;
		}
		uint16_t delay_interval = ((uint32_t)1000000/frequency / 2);
		gettimeofday(&now, NULL);
		lasttime = now.tv_sec * 1000000 + now.tv_usec;
		if (lasttime > next_update) {
			next_update = lasttime + delay_interval;

			if(_volume != 11 && mycount % 2 == 0)
            		    dac2 = 32 / _volume;
			else if (mycount % 2)
            		    dac2 = 0;
			mycount++;
            		dacWrite(SPEAKER_PIN, dac1 + dac2 + dac3);
		}
		usleep(delay_interval);
	}
}

static void beepthread(void * parameter) {
	for(;;) {
		struct timeval now;
		long int lasttime;
		static int mycount = 0;
		static long int next_update = 0;
		int frequency = *(int *)parameter;
		if (frequency <= 0) {
			vTaskDelay(1);
            	//	dacWrite(SPEAKER_PIN,0);
			next_update = 0;
			continue;
		}
		uint16_t delay_interval = ((uint32_t)1000000/frequency / 2);
		gettimeofday(&now, NULL);
		lasttime = now.tv_sec * 1000000 + now.tv_usec;
		if (lasttime > next_update) {
			next_update = lasttime + delay_interval;

			if(_volume != 11 && mycount % 2 == 0)
            		    dac1 = 255 / _volume;
			else if (mycount % 2)
            		    dac1 = 0;
			mycount++;
            		dacWrite(SPEAKER_PIN, dac1);
		}
		usleep(delay_interval);
	}
}

static void beepmix(void * parameter) {
	for(;;) {
		struct timeval now;
		long int lasttime;
		static long int next_update = 0;
		uint16_t delay_interval = ((uint32_t)1000000/32000);
		usleep(1);
		#if 0
		if (dac1 + dac2 + dac3 <= 0) {
			vTaskDelay(1);
            		dacWrite(SPEAKER_PIN,0);
			next_update = 0;
			continue;
		}
		#endif
		gettimeofday(&now, NULL);
		lasttime = now.tv_sec * 1000000 + now.tv_usec;
		if (lasttime > next_update) {
			next_update = lasttime + delay_interval;

			if(_volume != 11)
            			dacWrite(SPEAKER_PIN, dac1 + dac2 + dac3);
		}
	}
}

static void Task1code(void * parameter) {
	uint8_t *keepbuff = (uint8_t *)malloc(KEEPBUFFSZ);
	for(;;) {
		struct timeval now;
		long int lasttime;
		static int mycount = 0;
		static long int next_update = 0;
		struct audiotask *thisaudtask = (struct audiotask *)parameter;
		uint16_t sample_rate = thisaudtask->samplerate;
		uint8_t *myaudbuf = thisaudtask->buff;
		static uint8_t *oldbuff = NULL;
		int length = thisaudtask->len;
		if (myaudbuf == NULL) {
			vTaskDelay(1);
			continue;
		}
		if (oldbuff != myaudbuf) {
			sndbusy = true;
			oldbuff = myaudbuf;
			int parts2 = KEEPBUFFSZ - keepbuffcount;
			int parts = keepbuffcount + length - KEEPBUFFSZ;
			if (parts > 0) {
				memcpy (keepbuff + keepbuffcount, myaudbuf, parts2);
				memcpy (keepbuff, &myaudbuf[parts2], length - parts2);
			} else
				memcpy (keepbuff + keepbuffcount, myaudbuf, length);
			keepbuffcount += length;
			sndbusy = false;
		}
		if (mycount >= KEEPBUFFSZ)
			mycount = 0;
		uint16_t delay_interval = ((uint32_t)1000000/sample_rate);
		gettimeofday(&now, NULL);
		lasttime = now.tv_sec * 1000000 + now.tv_usec;
		if (lasttime > next_update) {
		fprintf(stderr, "AUDME %x %x\n", keepbuff[mycount], keepbuff[mycount + 1]);
			fprintf(stderr, "HERE\n");
			next_update = lasttime + delay_interval;

			if(_volume != 11)
            		    dacWrite(SPEAKER_PIN,keepbuff[mycount++]/_volume);
			keepbuffcount--;
		}
	}
}

void SPEAKER::end() {
}

void SPEAKER::tone(uint16_t frequency) {
    setVolume(_volume);
    _beeptone = frequency;
}

void SPEAKER::tone2(uint16_t frequency) {
    setVolume(_volume);
    _beeptone2 = frequency;
}

void SPEAKER::tone3(uint16_t frequency) {
    setVolume(_volume);
    _beeptone3 = frequency;
}

void SPEAKER::tone(uint16_t frequency, uint32_t duration) {
    tone(frequency);
    _count = millis() + duration + 100;
    speaker_on = 1;
}

void SPEAKER::tone2(uint16_t frequency, uint32_t duration) {
    tone2(frequency);
    _count2 = millis() + duration + 100;
    speaker_on2 = 1;
}
void SPEAKER::tone3(uint16_t frequency, uint32_t duration) {
    tone3(frequency);
    _count3 = millis() + duration + 100;
    speaker_on3 = 1;
}

void SPEAKER::beep() {
    tone(_beep_freq, _beep_duration);
}

void SPEAKER::setBeep(uint16_t frequency, uint16_t duration) {
    _beep_freq = frequency;
    _beep_duration = duration;
}

void SPEAKER::setVolume(uint8_t volume) {
    _volume = 11 - volume;
    if (volume)
    	digitalWrite(25, HIGH);
}

void SPEAKER::mute() {
    digitalWrite(25, 0);
    digitalWrite(SPEAKER_PIN, 0);
    tone(0);
    tone2(0);
    tone3(0);
}

void SPEAKER::update() {
    if(speaker_on) {
        if(millis() > _count) {
            speaker_on = 0;
            tone(0);
        }
    }
    if(speaker_on2) {
        if(millis() > _count2) {
            speaker_on2 = 0;
            tone2(0);
        }
    }
    if(speaker_on3) {
        if(millis() > _count3) {
            speaker_on3 = 0;
            tone3(0);
        }
    }
}

void SPEAKER::write(uint8_t value) {
    dacWrite(SPEAKER_PIN, value);
}

void SPEAKER::playMusicThreaded(const uint8_t* music_data, uint16_t sample_rate, int length) {
    delay(4);
    while (sndbusy || keepbuffcount + length > KEEPBUFFSZ)
		vTaskDelay(1);
    myaudiotask.buff = (uint8_t *)music_data;
    myaudiotask.len = length;
    myaudiotask.samplerate = sample_rate;
    //mute();
}
void SPEAKER::playMusic(const uint8_t* music_data, uint16_t sample_rate, int length) {
    uint16_t delay_interval = ((uint32_t)1000000/sample_rate);
        for(int i=0; i<length; i++) {
    	    if(_volume != 11)
            	dacWrite(SPEAKER_PIN, music_data[i]/_volume);
            delayMicroseconds(delay_interval);
        }
    
    #if 0
        for(int t=music_data[length-1]/_volume; t>=0; t--) {
            dacWrite(SPEAKER_PIN, t);
            delay(2);
        }
    #endif
    //mute();
}
void SPEAKER::playMusic(const uint8_t* music_data, uint16_t sample_rate) {
    uint32_t length = strlen((char*)music_data);
    uint16_t delay_interval = ((uint32_t)1000000/sample_rate);
    digitalWrite(25, HIGH);
        for(int i=0; i<length; i++) {
    	    if(_volume != 11)
            	dacWrite(SPEAKER_PIN, music_data[i]/_volume);
            delayMicroseconds(delay_interval);
        }
    
    #if 0
        for(int t=music_data[length-1]/_volume; t>=0; t--) {
            dacWrite(SPEAKER_PIN, t);
            delay(2);
        }
    #endif
    //mute();
}
