#include "main.h"

void setup(){
    Serial.begin(115200);
    __entry("setup()");
    lightTickInit();



    __exit("setup()");
}

void loop(){
    lightTickRun();
}