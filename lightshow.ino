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
#define BOUNCE 3

CRGB leds[NUM_LEDS];

short currentEffect = IDLE;
long currentRequestedColor = -1;
long currentRequestStartTime = -1;
float currentEffectDuration = -1.0;
float currentNumBounces = 3.0;

char debugStr[160];

// the LED Strips that we're using seem to address their Green and Blue
// channels opposite of what FastLED does. Or there's some bug in the 
// FastLED library port into Particle land. Or Something. This work-around
// simply swaps the characters in a string which correspond to the Green
// and Blue channels. 
const char *swapGreenAndBlue(String colorCode){
    char g1 = colorCode[2];
    char g2 = colorCode[3];
    colorCode[2] = colorCode[4];
    colorCode[3] = colorCode[5];
    colorCode[4] = g1;
    colorCode[5] = g2;
    return colorCode.substring(0,6).c_str(); 
}

void setDurationFromCommand(String command){
    // note the arguments to substring are (from,to) not (from,length)
    // https://docs.particle.io/reference/device-os/api/string-class/substring/
    float duration = command.substring(7,9).toFloat();
    sprintf(debugStr, "SET DURATION with '%s', substring '%s' to %f", command.c_str(), command.substring(7,9).c_str(), duration);
    Particle.publish("set duration", debugStr);
    currentEffectDuration = duration * 1000;
}

int show(String command) {
    currentRequestStartTime = millis();
    currentEffectDuration = STANDARD_DURATION;
    currentEffect = CLIMB;
    
    const char *swapped = swapGreenAndBlue(command);
    currentRequestedColor = strtol(swapped, NULL, 16);
    //currentRequestedColor = strtol("FF0000",NULL,16);

    sprintf(debugStr, 
            "CLIMB start:%i color:%s swapped:%s duration:%f", 
            currentRequestStartTime, 
            command.c_str(), 
            swapped,
            currentEffectDuration);
    Particle.publish("effect start", debugStr);
    return currentRequestedColor;
}

int climbWithDuration(String command) {
    currentEffect = CLIMB;
    currentRequestStartTime = millis();
    
    const char *swapped = swapGreenAndBlue(command);
    currentRequestedColor = strtol(swapped, NULL, 16);
    
    setDurationFromCommand(command);
    sprintf(debugStr, 
            "CLIMB-DURATION start:%i color:%s swapped:'%s' duration:%f", 
            currentRequestStartTime, 
            command.substring(0,6).c_str(), 
            swapped,
            currentEffectDuration);
    Particle.publish("effect start", debugStr);
    return currentRequestedColor;
}

int bounceWithDuration(String command) {
    currentEffect = BOUNCE;
    currentRequestStartTime = millis();
    
    const char *swapped = swapGreenAndBlue(command);
    currentRequestedColor = strtol(swapped, NULL, 16);
    
    setDurationFromCommand(command);
    currentNumBounces = command.substring(10,12).toFloat();

    sprintf(debugStr, 
            "BOUNCE start:%i color:%s swapped:%s duration:%f bounces:%f", 
            currentRequestStartTime, 
            command.substring(0,6).c_str(), 
            swapped,
            currentEffectDuration,
            currentNumBounces);
    Particle.publish("effect start", debugStr);
    
    return currentRequestedColor;
}

void setup() { 
    pinMode(D0, INPUT_PULLDOWN);
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
    Particle.function("show", show);
    Particle.function("climbWithDuration", climbWithDuration);
    Particle.function("bounceWithDuration", bounceWithDuration);
    pinMode(DEBUG_PIN, OUTPUT);
}

void loop()
{
    switch(currentEffect){
        case IDLE:
            //what makes the button work
            if(digitalRead(D0) == 1) {
            	show("990000");
            }
            
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
                char debugStr[160];
                sprintf(debugStr, "FADE ended:%i", millis());
                Particle.publish("effect end", debugStr);
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
                sprintf(debugStr, "CLIMB started:%i ended:%i", currentRequestStartTime, millis());
                Particle.publish("effect end", debugStr);

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
        case BOUNCE:
        {
            digitalWrite(DEBUG_PIN, HIGH);
            long timeSinceShowStart = millis() - currentRequestStartTime;
            if(timeSinceShowStart > currentEffectDuration){
                sprintf(debugStr, "BOUNCE started:%i ended:%i", currentRequestStartTime, millis());
                Particle.publish("effect end", debugStr);

                currentRequestedColor = -1;
                currentRequestStartTime = -1;
                currentEffectDuration = -1.0;
                for(short dot = 0; dot < NUM_LEDS; dot++){
                    leds[dot] = CRGB::Black;
                }
                currentEffect = IDLE;
            } else {
                float effectTimeComplete = timeSinceShowStart / (float) currentEffectDuration;
                float x = effectTimeComplete * currentNumBounces * PI * 2;
                
                // https://www.wolframalpha.com/input?i=50*cos%28x+%2B+pi%29%2B50
                // https://www.math.csi.cuny.edu/~ikofman/Polking/degrad.gif
                short numDotsToLight = 16;
                short whichDotsToLight[numDotsToLight] = 
                    { (NUM_LEDS/2) * cos(x+PI*1.150) + (NUM_LEDS/2)
                    , (NUM_LEDS/2) * cos(x+PI*1.140) + (NUM_LEDS/2)
                    , (NUM_LEDS/2) * cos(x+PI*1.130) + (NUM_LEDS/2)
                    , (NUM_LEDS/2) * cos(x+PI*1.120) + (NUM_LEDS/2)
                    , (NUM_LEDS/2) * cos(x+PI*1.110) + (NUM_LEDS/2)
                    , (NUM_LEDS/2) * cos(x+PI*1.100) + (NUM_LEDS/2)
                    , (NUM_LEDS/2) * cos(x+PI*1.090) + (NUM_LEDS/2)
                    , (NUM_LEDS/2) * cos(x+PI*1.080) + (NUM_LEDS/2)
                    , (NUM_LEDS/2) * cos(x+PI*1.070) + (NUM_LEDS/2)
                    , (NUM_LEDS/2) * cos(x+PI*1.060) + (NUM_LEDS/2)
                    , (NUM_LEDS/2) * cos(x+PI*1.050) + (NUM_LEDS/2)
                    , (NUM_LEDS/2) * cos(x+PI*1.040) + (NUM_LEDS/2)
                    , (NUM_LEDS/2) * cos(x+PI*1.030) + (NUM_LEDS/2)
                    , (NUM_LEDS/2) * cos(x+PI*1.020) + (NUM_LEDS/2)
                    , (NUM_LEDS/2) * cos(x+PI*1.010) + (NUM_LEDS/2)
                    , (NUM_LEDS/2) * cos(x+PI*1.000) + (NUM_LEDS/2)};
                short indexInLit = -1;
                CRGB dotColor;
                    
                for(short dot = 0; dot < NUM_LEDS; dot++){
                    indexInLit = -1;
                    
                    for(short i = 0; i < numDotsToLight; ++i){
                        if(dot == whichDotsToLight[i]){
                            indexInLit = i;
                        }
                    }
                    
                    if(indexInLit > -1){
                        dotColor = CRGB(currentRequestedColor);
                        leds[dot] = dotColor.fadeToBlackBy(indexInLit*16);
                        
                        // make the leading edge of the wave sparkle with random bright colors
                        if(indexInLit == 0){
                            leds[dot] = leds[dot].setHue(rand()%255);
                        }
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