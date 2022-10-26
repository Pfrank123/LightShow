// This #include statement was automatically added by the Particle IDE.
#include <FastLED.h>

// This #include statement was automatically added by the Particle IDE.
#include <FastLED.h>


#include <FastLED.h>
FASTLED_USING_NAMESPACE; 

#define NUM_LEDS 100
#define DATA_PIN 7

CRGB leds[NUM_LEDS];

//expecting a HEX color 
int show(String command) {
    // FF0000 
    char g1 = command[2];
    char g2 = command[3];
    command[2] = command[4];
    command[3] = command[5];
    command[4] = g1;
    command[5] = g2;
    
    long color = strtol(command.c_str(), NULL, 16);
    short dot = 0;
    
    // light up from the bottom
    for(dot = 0; dot < NUM_LEDS; dot++) {
        leds[dot] = color;
        FastLED.show();
        delay(4);
    }
    
    // lower lights from top to bottom
/*    for(dot = NUM_LEDS-1; dot >= 0; dot--){
        leds[dot] = CRGB::Black;
        FastLED.show();
        delay(30);
    }
*/

    return color;
}
int show1(String command) {
    // FF0000 
    char g1 = command[2];
    char g2 = command[3];
    command[2] = command[4];
    command[3] = command[5];
    command[4] = g1;
    command[5] = g2;
    
    long color = strtol(command.c_str(), NULL, 16);
    short dot = 21;
    
    // light up from the bottom
    for(dot = 21; dot < 40; dot++) {
        leds[dot] = color;
        FastLED.show();
        delay(4);
    }
    
    // lower lights from top to bottom
/*    for(dot = NUM_LEDS-1; dot >= 0; dot--){
        leds[dot] = CRGB::Black;
        FastLED.show();
        delay(30);
    }
*/

    return color;
}
void setup() { 
    pinMode(D0, INPUT_PULLDOWN);
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
    Particle.function("show", show);
    Particle.function("show1", show1);

}

void loop()
{
    //what makes the button work
    if(digitalRead(D0) == 1) {
    	show("990000");
    }
    for(short dot = 0; dot < NUM_LEDS; dot++) {
        leds[dot].r = leds[dot].r * 0.98;
        leds[dot].g = leds[dot].g * 0.98;
        leds[dot].b = leds[dot].b * 0.98;
    }    
    delay(200);
    FastLED.show();
}