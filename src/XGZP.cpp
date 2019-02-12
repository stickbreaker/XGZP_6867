/*
Copyright 2019 stickbreaker@github

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/



#include <XGZP.h>
#include <Wire.h>

XGZP::XGZP(void):
    _devID(0)
{}

bool XGZP::begin(uint8_t devID){
    _devID=0;
    if(!Wire.begin()) { //Bus init failed, abort
        return false;
    }
    Wire.beginTransmission(devID);
    Wire.write(0xA5); // Sys Config Register
    Wire.endTransmission();
    uint8_t count = Wire.requestFrom(devID,(uint8_t)1);
    if(count ==1){
        uint8_t reg = Wire.read();
        log_d(" Read System Configuration register =0x%02X\n",reg);
        
        Wire.beginTransmission(devID);
        Wire.write(0xA5); // sys configuration register
            // DAC ON, Single Conversion, Vout=Fixed by Vext*1.5, Vext=3.6V, Calibrated Output, Diag=on
        Wire.write(0xFD); //
        uint8_t err =Wire.endTransmission();
        if(err != 0){
            log_e("Writing to register 0xA5 value 0xFD failed=%d(%s)\n",err,Wire.getErrorText(err));
            return false; // fail
        }
        
        bool ready=false;
        uint32_t tick=millis(); // timout
        while(!ready && (millis()-tick<1000)){
            Wire.beginTransmission(devID);
            ready = I2C_ERROR_OK==Wire.endTransmission();
        }
        if(ready){
            Wire.beginTransmission(devID);
            Wire.write(0xA5);
            Wire.write(0x0A);
            err = Wire.endTransmission();
            if(err != 0){
                log_e("Writing to register 0xA5 value 0x0A failed=%d(%s)\n",err,Wire.getErrorText(err));
                return false; // fail
            }
        }
        else {
            log_e("Timeout, Sensor did not respond after first config set.\n");
            return false;
        }
    }
    else {
        log_e("Reading from 0xA5 failed=%d(%s)\n",Wire.lastError(),Wire.getErrorText(Wire.lastError()));
        return false;
    }
    _devID = devID;    
    return true;
}

bool XGZP::read(float * reading){
    if((! _devID ) || ( *reading == NULL )){ // not initialized, or no return value location, exit
        if( *reading != NULL )  *reading = NAN;
        return false;
    }
    float XGZPC_Value = 0;
    bool ready=false;
    uint32_t tick=millis();
    while(!ready && (millis()-tick<1000)){ // Wait upto 1sec for conversion to complete
        Wire.beginTransmission(_devID);
        Wire.write(0x02); // status register
        uint8_t err =Wire.endTransmission(); 
        if(err == I2C_ERROR_OK) {
            uint8_t count = Wire.requestFrom(_devID,(uint8_t)1); // get status byte
            if (count==1){ // got data
               uint8_t status = Wire.read();
               ready = (status & 0x01)==0x01; // data ready!
            }
        }
    }
    if(ready){
        Wire.beginTransmission(_devID);
        Wire.write(0x06); // data register
        uint8_t err =Wire.endTransmission(); 
        if(err == I2C_ERROR_OK) {
            uint8_t count = Wire.requestFrom(_devID,(uint8_t)3); // get data
            if (count==3){ // got data
                XGZPC_Value = Wire.read() * 65536.0 + Wire.read() * 256.0 + Wire.read();
                XGZPC_Value = (XGZPC_Value*100.0) / 8388608.0;
                *reading = XGZPC_Value;
                Wire.beginTransmission(_devID); // start next sample?
                Wire.write(0x30);
                Wire.write(0x0a); // Data and Temp conversion, Single shot, immediate
                uint8_t err = Wire.endTransmission();
                if(err != 0){
                    log_e(" next Sample start failed? i2cError = %d(%s)\n",err, Wire.getErrorText(err));
                    return true; // data valid, next sample will be a problem.
                }
                else return true;
            }
            else{
                log_e("Read data failed =%d(%s)\n",Wire.lastError(), Wire.getErrorText(Wire.lastError()));
                return false;
            }
        }
        else {
            log_e("setData address failed =%d(%s)\n",Wire.lastError(), Wire.getErrorText(Wire.lastError()));
            return false;
            }
    }
    else{
        log_e(" Timeout, Data not available, Re Sampling.");
        Wire.beginTransmission(_devID); // start next sample?
        Wire.write(0x30);
        Wire.write(0x0a); // Data and Temp conversion, Single shot, immediate
        uint8_t err = Wire.endTransmission();
        if(err != 0){
            log_e(" Next Sample start failed? i2cError = %d(%s)\n",err, Wire.getErrorText(err));
        }
        return false;
    }
}
