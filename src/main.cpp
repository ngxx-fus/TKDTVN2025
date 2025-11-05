#include "../include/localHelperAndUtil.h"

void setup(){
    __entry("setup()");

    Serial.begin(115200);

    __exit("setup()");
}

void loop(){
    __log("running...");
    delay(1000);
}
