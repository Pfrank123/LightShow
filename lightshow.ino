#include <math.h>
#include <FastLED.h>
FASTLED_USING_NAMESPACE; 

#define NUM_LEDS 100
#define DATA_PIN 6
#define DEBUG_PIN 7
#define PI 3.14159265

// this must be a float: https://stackoverflow.com/a/16221859
#define STANDARD_DURATION 400.0

#define IDLE -1
#define FADE 1
#define CLIMB 2
#define SIN 3

CRGB leds[NUM_LEDS];

short currentEffect = IDLE;
long currentRequestedColor = -1;
long currentRequestStartTime = -1;
float currentEffectDuration = -1.0;

// the LED Strips that we're using seem to address their Green and Blue
// channels opposite of what FastLED does. Or there's some bug in the 
// FastLED library port into Particle land. Or Something. This work-around
// simply swaps the characters in a string which correspond to the Green
// and Blue channels. 
void swapGreenAndBlue(String colorCode){
    char g1 = colorCode[2];
    char g2 = colorCode[3];
    colorCode[2] = colorCode[4];
    colorCode[3] = colorCode[5];
    colorCode[4] = g1;
    colorCode[5] = g2;
}

void setDurationFromCommand(String command){
    float duration = strtof(command.substring(7).c_str(),NULL);
    currentEffectDuration = duration * 1000;
}

int show(String command) {
    currentRequestStartTime = millis();
    currentEffectDuration = STANDARD_DURATION;
    currentEffect = CLIMB;
    
    swapGreenAndBlue(command);
    currentRequestedColor = strtol(command.c_str(), NULL, 16);
    return currentRequestedColor;
}

int climbWithDuration(String command) {
    currentEffect = CLIMB;
    currentRequestStartTime = millis();
    
    swapGreenAndBlue(command);
    currentRequestedColor = strtol(command.substring(0,6).c_str(), NULL, 16);
    
    setDurationFromCommand(command);
    return currentRequestedColor;
}

int sinWithDuration(String command) {
    currentEffect = SIN;
    currentRequestStartTime = millis();
    
    swapGreenAndBlue(command);
    currentRequestedColor = strtol(command.substring(0,6).c_str(), NULL, 16);
    
    setDurationFromCommand(command);
    return currentRequestedColor;
}

void setup() { 
    pinMode(D0, INPUT_PULLDOWN);
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
    Particle.function("show", show);
    Particle.function("climbWithDuration", climbWithDuration);
    Particle.function("sinWithDuration", sinWithDuration);
    pinMode(DEBUG_PIN, OUTPUT);
}

void loop()
{
    //what makes the button work
    if(digitalRead(D0) == 1) {
    	show("990000");
    }
    switch(currentEffect){
        case IDLE:
            digitalWrite(DEBUG_PIN, LOW);
            // setting this delay means that the button check will happen 20x/second,
            // which should feel pretty responsive.
            delay(50);
            break;
        case FADE:
        {
            digitalWrite(DEBUG_PIN, HIGH);
            bool isAnyColorShowing = false;
            for(short dot = 0; dot < NUM_LEDS; dot++) {
                leds[dot] = leds[dot].subtractFromRGB(5);
                // you can compare a color vs. black with the boolean operator.
                // so if this light has any color to it, the boolean operator
                // on the next line will return true.
                isAnyColorShowing = isAnyColorShowing || leds[dot];
            }
            if(!isAnyColorShowing){
                // we dont' need to keep updating every LED value from now on.
                currentEffect = IDLE;
            }
            delay(100);
            break;
        }
        case CLIMB:
        {
            digitalWrite(DEBUG_PIN, HIGH);
            long timeSinceShowStart = millis() - currentRequestStartTime;
            if(timeSinceShowStart > currentEffectDuration){
                currentRequestedColor = -1;
                currentRequestStartTime = -1;
                currentEffectDuration = -1.0;
                currentEffect = FADE;
            } else {
                float effectTimeComplete = timeSinceShowStart / (float) currentEffectDuration;
                for(short dot = 0; dot < NUM_LEDS; dot++){
                    float amountCompleteThisDot = (float)dot / NUM_LEDS;
                    
                    if(effectTimeComplete > amountCompleteThisDot){
                        leds[dot] = currentRequestedColor;
                    } else {
                        leds[dot] = CRGB::Black;
                    }
                }
            }
            delay(5);
            break;
        }
        case SIN:
        {
            digitalWrite(DEBUG_PIN, HIGH);
            long timeSinceShowStart = millis() - currentRequestStartTime;
            if(timeSinceShowStart > currentEffectDuration){
                currentRequestedColor = -1;
                currentRequestStartTime = -1;
                currentEffectDuration = -1.0;
                for(short dot = 0; dot < NUM_LEDS; dot++){
                    leds[dot] = CRGB::Black;
                }
                currentEffect = IDLE;
            } else {
                float effectTimeComplete = timeSinceShowStart / (float) currentEffectDuration;
                float x = effectTimeComplete * PI * 12;
                
                // https://www.wolframalpha.com/input?i=50*sin%28x-pi%2F2%29%2B50
                // https://www.math.csi.cuny.edu/~ikofman/Polking/degrad.gif
                short whichIsLit[16] = { (NUM_LEDS/2) * cos(x+PI*1.000) + (NUM_LEDS/2)
                                      , (NUM_LEDS/2) * cos(x+PI*1.010) + (NUM_LEDS/2)
                                      , (NUM_LEDS/2) * cos(x+PI*1.020) + (NUM_LEDS/2)
                                      , (NUM_LEDS/2) * cos(x+PI*1.030) + (NUM_LEDS/2)
                                      , (NUM_LEDS/2) * cos(x+PI*1.040) + (NUM_LEDS/2)
                                      , (NUM_LEDS/2) * cos(x+PI*1.050) + (NUM_LEDS/2)
                                      , (NUM_LEDS/2) * cos(x+PI*1.060) + (NUM_LEDS/2)
                                      , (NUM_LEDS/2) * cos(x+PI*1.070) + (NUM_LEDS/2)
                                      , (NUM_LEDS/2) * cos(x+PI*1.080) + (NUM_LEDS/2)
                                      , (NUM_LEDS/2) * cos(x+PI*1.090) + (NUM_LEDS/2)
                                      , (NUM_LEDS/2) * cos(x+PI*1.100) + (NUM_LEDS/2)
                                      , (NUM_LEDS/2) * cos(x+PI*1.110) + (NUM_LEDS/2)
                                      , (NUM_LEDS/2) * cos(x+PI*1.120) + (NUM_LEDS/2)
                                      , (NUM_LEDS/2) * cos(x+PI*1.130) + (NUM_LEDS/2)
                                      , (NUM_LEDS/2) * cos(x+PI*1.140) + (NUM_LEDS/2)
                                      , (NUM_LEDS/2) * cos(x+PI*1.150) + (NUM_LEDS/2)};
                short indexInLit = -1;
                CRGB dotColor;
                    
                for(short dot = 0; dot < NUM_LEDS; dot++){
                    indexInLit = -1;
                    for(short i = 0; i < 16; ++i){
                        if(dot == whichIsLit[i]){
                            indexInLit = i;
                        }
                    }
                    
                    if(indexInLit > -1){
                        dotColor = CRGB(currentRequestedColor);
                        leds[dot] = dotColor.fadeToBlackBy(indexInLit*16);
                    } else {
                        leds[dot] = CRGB::Black;
                    }
                }
            }
            break;
        }
    }
    FastLED.show();
}

/*
// sample debug message with sprintf
char debugStr[160];
sprintf(debugStr, 
        "start:%i now:%i timeSince:%i complete:%f", 
        currentRequestStartTime, 
        millis(), 
        timeSinceShowStart, 
        effectTimeComplete);

// note that Particle.publish() is rate limited to 4x / second.
// so we are going to try to throttle ourselves by only 
// printing out every 50th message, more or less.
if(rand()%50 == 0){
    Particle.publish("effect debug", debugStr);
}

*/