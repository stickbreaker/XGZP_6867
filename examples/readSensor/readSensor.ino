/*
Copyright 2019 stickbreaker@github

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include <XGZP.h>
#include <Wire.h>

XGZP sensor;

void setup(){
    Serial.begin(115200);
    Wire.begin(SDA,SCL,100000);
    
    if(!sensor.begin()){
        Serial.printf("Error initializing Sensor\n Locking up.");
        while(1);
    }
}

void loop(){
    float reading;
    if(sensor.read(&reading)){
        Serial.printf("(%8dms) reading=%6.2f\n",millis(),reading);
    }
    else {
        Serial.printf(" problem reading sensor, Re-Initting\n");
        delay(500);
        if(!sensor.begin()){
            Serial.printf("Error initializing Sensor\n");
        }
    }
}    