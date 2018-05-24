#include <ButtonDebounce.h>

#include <Adafruit_NeoPixel.h>

#define BUTTON     13
#define PIN_PIXEL  6
#define NUM_PIXELS 32

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, PIN_PIXEL,
                                            NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

void onButtonChange(int state) {
  Serial.println(state);
}
ButtonDebounce button(BUTTON, 250); // PIN 3 with 250ms debounce time

void setup() {
    strip.begin();
    showInitSequence();

    pinMode(BUTTON, INPUT_PULLUP);
    button.setCallback(onButtonChange);
    
    Serial.begin(115200);
}

void showInitSequence() {
    int inc = 1;
    for (int i = 0; i > -1;) {
        strip.setPixelColor(i+0, 255, 0, 0);
        strip.setPixelColor(i+2, 0, 255, 0);
        strip.setPixelColor(i+4, 0, 0, 255);
        strip.show();

        delay(10);
        strip.setPixelColor(i+0, 0, 0, 0);
        strip.setPixelColor(i+2, 0, 0, 0);
        strip.setPixelColor(i+4, 0, 0, 0);
        strip.show();

        if (i == NUM_PIXELS-5) {
            inc = -1;
        }
        i += inc;
    }
}

void loop() {
    button.update();
    if (Serial.available()) {
        char command = Serial.read();
        switch (command) {
            case 'a':
                commandApply();
                break;
            case 'c':
                commandSetColor();
                break;
            case 'l':
                commandLedToColor();
                break;
            case 'p':
                commandPercent();
                break;
            case 'f':
                commandFlash();
                break;
            default:
                break;
        }
    }
}


uint8_t color_r = 0;
uint8_t color_g = 0;
uint8_t color_b = 0;

void commandFlash() {
    colorWipe(strip.Color(color_r, color_g, color_b), 0);
    delay(4000);
    colorWipe(strip.Color(0, 0, 0), 10);
}

void commandPercent() {
    byte r = Serial.read();
    unsigned cnt = (strip.numPixels() * r) / 100;
    for(uint16_t i=0; i<strip.numPixels(); i++) {
	if(i < cnt) {
	        strip.setPixelColor(i, strip.Color(color_r, color_g, color_b));
	} else {
	        strip.setPixelColor(i, strip.Color(0, 0, 0));
	}
    }
    strip.show();
}

void commandApply() {
    strip.show();
}

void commandSetColor() {
    while (!Serial.available());
    byte r = Serial.read();
    while (!Serial.available());
    byte g = Serial.read();
    while (!Serial.available());
    byte b = Serial.read();

    color_r = r;
    color_g = g;
    color_b = b;
}

void commandLedToColor() {
    while (!Serial.available());
    byte index = Serial.read();
    if (index >= 0 && index < NUM_PIXELS) {
        strip.setPixelColor(index, color_r, color_g, color_b);
    }
}


/*
void fullStripToColor(uint32_t color) {
    firstLedsToColor(strip.numPixels(), color);
}

void firstLedsToColor(uint16_t num, uint32_t color) {
    for(uint16_t i = 0; i < num; i++) {
        strip.setPixelColor(i, color);
    }
}
*/


void demo() {
    // Some example procedures showing how to display to the pixels:
    colorWipe(strip.Color(255, 0, 0), 50); // Red
    colorWipe(strip.Color(0, 255, 0), 50); // Green
    colorWipe(strip.Color(0, 0, 255), 50); // Blue
    // Send a theater pixel chase in...
    theaterChase(strip.Color(127, 127, 127), 50); // White
    theaterChase(strip.Color(127,   0,   0), 50); // Red
    theaterChase(strip.Color(  0,   0, 127), 50); // Blue

    rainbow(20);
    rainbowCycle(20);
    theaterChaseRainbow(50);
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
    for(uint16_t i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, c);
        strip.show();
        delay(wait);
    }
}

void rainbow(uint8_t wait) {
    uint16_t i, j;

    for(j=0; j<256; j++) {
        for(i=0; i<strip.numPixels(); i++) {
            strip.setPixelColor(i, Wheel((i+j) & 255));
        }
        strip.show();
        delay(wait);
    }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
    uint16_t i, j;

    for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
        for(i=0; i< strip.numPixels(); i++) {
            strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
        }
        strip.show();
        delay(wait);
    }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
    for (int j=0; j<10; j++) {  //do 10 cycles of chasing
        for (int q=0; q < 3; q++) {
            for (int i=0; i < strip.numPixels(); i=i+3) {
                strip.setPixelColor(i+q, c);    //turn every third pixel on
            }
            strip.show();

            delay(wait);

            for (int i=0; i < strip.numPixels(); i=i+3) {
                strip.setPixelColor(i+q, 0);        //turn every third pixel off
            }
        }
    }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
    for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
        for (int q=0; q < 3; q++) {
            for (int i=0; i < strip.numPixels(); i=i+3) {
                strip.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
            }
            strip.show();

            delay(wait);

            for (int i=0; i < strip.numPixels(); i=i+3) {
                strip.setPixelColor(i+q, 0);        //turn every third pixel off
            }
        }
    }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
    WheelPos = 255 - WheelPos;
    if(WheelPos < 85) {
        return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    } else if(WheelPos < 170) {
        WheelPos -= 85;
        return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    } else {
        WheelPos -= 170;
        return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
    }
}


