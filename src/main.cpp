void setup(){
    __entry("setup()");

    serial_init(115200);

    __exit("setup()");
}

void loop(){
    __log("running...");
    delay(1000);
}
