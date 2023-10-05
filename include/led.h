#ifndef HAVELED_H
#define HAVELED_H

#include <FastLED.h>

// FastLED
#define NUM_LEDS 1         // Number of RGB LED beads
#define DATA_PIN 27        // The pin for controlling RGB LED
#define LED_TYPE NEOPIXEL  // RGB LED strip type

class Led{
  private:
    CRGB leds[NUM_LEDS];       // Instantiate RGB LED
  
  public:
    Led(){
      FastLED.addLeds<SK6812, DATA_PIN, RGB>(leds, NUM_LEDS);  // GRB ordering is typical
      FastLED.setBrightness(127);
    }
    void showGreen(){
      leds[0] = CRGB::Green;  // LED shows red light
      FastLED.show();
    }
    void showRed(){
      leds[0] = CRGB::Red;  // LED shows red light
      FastLED.show();
    }
    void showBlue(){
      leds[0] = CRGB::Blue;  // LED shows red light
      FastLED.show();
    }

    void clear(){
      leds[0] = CRGB(0, 0, 0);  // LED oFF
      FastLED.show();
    }

};

#endif