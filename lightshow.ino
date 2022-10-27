// This #include statement was automatically added by the Particle IDE.
#include <FastLED.h>

// This #include statement was automatically added by the Particle IDE.
#include <FastLED.h>


#include <FastLED.h>
FASTLED_USING_NAMESPACE; 

#define NUM_LEDS 100
#define DATA_PIN 6
#define DEBUG_PIN 7
#define PI 3.14159265


// this must be a float: https://stackoverflow.com/a/16221859
#define STANDARD_DURATION 700.0



CRGB leds[NUM_LEDS];

long currentRequestedColor = -1;
long currentRequestStartTime = -1;
float currentEffectDuration = -1.0;

//expecting a HEX color 
int show(String command) {
    // FF0000 
    char g1 = command[2];
    char g2 = command[3];
    command[2] = command[4];
    command[3] = command[5];
    command[4] = g1;
    command[5] = g2;
    
    currentRequestedColor = strtol(command.c_str(), NULL, 16);
    currentRequestStartTime = millis();
    currentEffectDuration = STANDARD_DURATION;
    return currentRequestedColor;
}

//expecting a HEX color 
int climbWithDuration(String command) {
    // FF0000 
    char g1 = command[2];
    char g2 = command[3];
    command[2] = command[4];
    command[3] = command[5];
    command[4] = g1;
    command[5] = g2;
    
    currentRequestedColor = strtol(command.substring(0,6).c_str(), NULL, 16);
    currentRequestStartTime = millis();
    float duration = strtof(command.substring(7).c_str(),NULL);
    currentEffectDuration = duration * 1000;
    return currentRequestedColor;
}

void setup() { 
    pinMode(D0, INPUT_PULLDOWN);
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
    Particle.function("show", show);
    Particle.function("climbWithDuration", climbWithDuration);
    pinMode(DEBUG_PIN, OUTPUT);
}

void loop()
{
    //what makes the button work
    if(digitalRead(D0) == 1) {
    	show("990000");
    }
    if(currentRequestedColor < 0){
        digitalWrite(DEBUG_PIN, LOW);
        for(short dot = 0; dot < NUM_LEDS; dot++) {
            leds[dot].r = leds[dot].r * 0.98;
            leds[dot].g = leds[dot].g * 0.98;
            leds[dot].b = leds[dot].b * 0.98;
        }    
        //strtol(command.c_str(), NULL, 16);
        delay(50);
        
    } else {
        digitalWrite(DEBUG_PIN, HIGH);
        long timeSinceShowStart = millis() - currentRequestStartTime;
        if(timeSinceShowStart > currentEffectDuration){
            currentRequestedColor = -1;
            currentRequestStartTime = -1;
            currentEffectDuration = -1.0;
        } else {
            float effectTimeComplete = timeSinceShowStart / currentEffectDuration;
            char debugStr[160];
            for(short dot = 0; dot < NUM_LEDS; dot++){
                float amountCompleteThisDot = (float)dot / NUM_LEDS;
                
                if(effectTimeComplete > amountCompleteThisDot){
                    leds[dot] = currentRequestedColor;
                } else {
                    leds[dot] = CRGB::Black;
                }
                
                /*
                // sample debug message with sprintf
                sprintf(debugStr, "start:%i now:%i timeSince:%i complete:%f", currentRequestStartTime, millis(), timeSinceShowStart, effectTimeComplete);
                if(rand()%50 == 0){
                    Particle.publish("effect debug", debugStr);
                }
                */
                
            }

        }
        delay(5);
            
    }
    
    FastLED.show();
}