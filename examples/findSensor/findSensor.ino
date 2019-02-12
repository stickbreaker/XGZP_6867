/*
Copyright 2019 stickbreaker@github

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <XGZP.h>
#include <Wire.h>

#define DEVID 0x6D
static uint8_t foundSda=255, foundScl=255;

bool searchI2C(){
    bool found = true;
    uint8_t _sda, _scl;
    uint8_t pinlist[]={0,2,4,5,12,13,14,15,16,17,18,19,21,22,23,25,26,27,32,33}; //standard i/o pins on ESP32

    if(foundScl < 255) { // second+ scan, so start off where left off.
        _scl = 0;
        while( (_scl<sizeof(pinlist)) && ( pinlist[_scl] <= foundScl)) _scl++;
        if( _scl == sizeof(pinlist)){
            _scl= 0;
            found = false;
        }
    }
    else _scl = 0;

    if(foundSda < 255) { // second+ scan, so start off where left off.
        _sda = 0;
        while( (_sda<sizeof(pinlist)) && ( pinlist[_sda] < foundSda)) _sda++;
        if( !found ) { // inc to next sda because scl rolled over
            _sda++;
            found = true;
        }
        if( _sda >= sizeof(pinlist) ){
            found = false;
        }
    }
    else _sda = 0;

    if(_sda == _scl) {
        _scl++;
        if(_scl >= sizeof(pinlist)) found = false;
    }

    if( !found ) { // all possibles have been searched
        foundSda = 255;
        foundScl = 255;
        return false;
    }
    found = false;
    while((_sda < sizeof(pinlist)) && !found){
        while(( _scl <sizeof(pinlist)) && !found){
            if(_scl==_sda) {
                _scl++;
                continue;
            }
            found =(Wire.begin(pinlist[_sda],pinlist[_scl],100000));
            if(found ){
                foundSda = pinlist[_sda];
                foundScl = pinlist[_scl];
                return true;
            }
            else {
                _scl++;
            }
        }
        _scl = 0;
        _sda++;
    }
}
     
int scanI2C( uint8_t sda, uint8_t scl, bool display=false){
    uint8_t cnt=0;
    if(display) Serial.printf("(SDA=%d, SCL=%d)\nScanning I2C Addresses\n",sda,scl);
    for(uint8_t i=0;i<128;i++){
        Wire.beginTransmission(i);
        uint8_t ec=Wire.endTransmission(true);
        if(ec==0){
            if(display){
                if(i<16)Serial.print('0');
                Serial.print(i,HEX);
            }
            cnt++;
        }
        else {
            if(display) Serial.print("..");
        }
        if(display){
            Serial.print(' ');
            if ((i&0x0f)==0x0f)Serial.println();
        }
    } 
    if(display){
        Serial.print("Scan Completed, ");
        Serial.print(cnt);
        Serial.println(" I2C Devices found.");
    }
    return cnt;
}

XGZP sensor;

void setup(){
Serial.begin(115200);
Serial.printf("\n Scanning all pins, looking for I2C slave Device with id=0x%02X\n",DEVID);
do{
    if(searchI2C()){
        if(scanI2C(foundSda,foundScl,false)) {
            Serial.printf("\n(%8dms) I2C bus devices found on ",millis());
            scanI2C(foundSda,foundScl,true);
            Wire.beginTransmission(DEVID);
            if(Wire.endTransmission() ==I2C_ERROR_OK) {
                Serial.printf(" Sensor Found, use:\n\n Wire.begin(%d,%d,100000);\n\n",foundSda, foundScl);
                sensor.begin(DEVID);
                return;
            }
        }
    }
}while((foundSda != 255)&&(foundScl!=255));
Serial.printf(" Not Found\n locking up\n");
while(1);
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