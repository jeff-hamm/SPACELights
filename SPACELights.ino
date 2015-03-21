#include <SPI.h>
#define BASS_ONLY
#include "EDP8266Wifi.h"
#include "LightingController.h"
#include "WebInput.h"
#define RESOLUTION 8

#define BOXES 10
#ifdef BASS_ONLY
#define FREQ_BINS 1
#else 
#define FREQ_BINS BOXES
#endif

#include "Wire.h"
#include "FastLED.h"
#include "SD.h"
//#include "Audio.h"
#include "RF24.h"
#include "RFAudio.h"
#include "AudioProcessor.h"
#include "AudioVisualizer.h"
#include <ArduinoJson.h>
#include "LedFxUtilities.h"
#include "EEPROM.h"

#define NUM_LEDS 300
#define LED_PIN 1
#ifdef BASS_ONLY
#define VISUALIZERS 10
#else
#define VISUALIZERS 1
#endif

AudioInputAnalog ai;
AudioAnalyzeFFT1024 fft;
AudioConnection patch(ai,0,fft,0);
AudioProcessor ap(&fft);
LEDStripAudioVisualizer avs[VISUALIZERS];
FFTBinData data;
CRGB leds[NUM_LEDS];
LightingControllerClass<BOXES> controller;
ModeMessage currentMode(Sound,0,255,255);
//WebInput input(A7,A6);
#ifdef BASS_ONLY
int binCounts[FREQ_BINS] = {5};
#else
int binCounts[FREQ_BINS] = {2,2,2,4,8,16,32,64,128,253};
#endif

void setup() {
	ai.init(A7);
	Serial.begin(9600);
        delay(1000);
	//pin 0 got busted, we bridged to pin1 for data output
	pinMode(0, INPUT);
        pinMode(A2, INPUT);
        pinMode(A3, INPUT);
        pinMode(A6, INPUT);
        pinMode(A7, INPUT);
        

	LEDS.addLeds<WS2811, LED_PIN, BRG>(leds, NUM_LEDS);
	ap.init();
        ap.setBinCounts(binCounts);
        int binSize = NUM_LEDS/VISUALIZERS;
        for(int i = 0; i < VISUALIZERS; i++) {
        	avs[i].init(&leds[i*binSize], binSize);
        	avs[i].setSpeed(6,.00005);
	        ap.connectAudioVisualizer(&avs[i]);
        }
	controller.init(leds, NUM_LEDS, _BV(7));
}

void loop() {
	update();
//	checkInput();
}
static unsigned int soundCount = 0;
static unsigned long patternTime = 0;
void update() {
	CHSV hsv(currentMode.arg1,currentMode.arg2,min(currentMode.arg3,200));
	int peak = ap.analyzeData(data);
        ap.visualize(data);
        for(int i = 0; i < VISUALIZERS; i++) {
          if(!avs[i].isDrawing()) {
            updatePattern();
            break;
  	}
        }
        LEDS.show();
}
void updatePattern() {
	static unsigned long lastPop = 0;
	const unsigned long popTime = 2500;
	unsigned long currentTime = millis();
	for(int i = 0; i < BOXES; i++) {
		if(controller.isRendered(i)) {
			controller.setColor(i, tr(EMPTY_COLOR), true);
		}
	}
	if((currentTime-lastPop) > popTime) {
		Serial.println(currentTime);
		int box = random(0, BOXES);
		Serial.print("pop: ");
		Serial.println(box);
		controller.setColor(box, tr(LEDFxUtilities::randomColor(currentMode.arg2, 255)), true);
		lastPop = currentTime;
	}

	controller.render();
}

CRGB tr(Pixel p) {
	CRGB r(p.r, p.g, p.b);
	return r;
}

void checkInput() {
/*	input.update();
	if(input.inputAvailable()) {
		ModeMessage inputMessage;
		input.readInput(inputMessage);
		if(!inputMessage.duplicate) {
			currentMode.print();
			switch(inputMessage.mode) {
			case Sound:
				currentMode.copyFrom(inputMessage);
				av.setColorParameters((float)currentMode.arg1 / 255.0, (float)currentMode.arg2 / 255.0, (float)currentMode.arg3/255.0);
				break;
				case Brightness:
					LEDS.setBrightness(inputMessage.arg1);
					break;
				case Solid:
					currentMode.copyFrom(inputMessage);
					break;
				case Patterns:
					//TODO: Implement
					// for now we'll just toss a rainbow chase in here.
					currentMode.copyFrom(inputMessage);
					break;
			}
		}
		input.sendResponse();
	}*/
}
