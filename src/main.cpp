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


/*
Note for using terminal:(Only for HoangAnh's PC)
- Build:    C:\Users\Admin\.platformio\penv\Scripts\pio.exe run
- Upload:   C:\Users\Admin\.platformio\penv\Scripts\pio.exe run -t upload
- Upload:   C:\Users\Admin\.platformio\penv\Scripts\pio.exe run --target monitor

For shorter command, you need add the `C:\Users\Admin\.platformio\penv\Scripts` to environment.
*/