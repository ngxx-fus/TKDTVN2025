# NOTE

- Cái thư mục `.private` là chứa DOC, file ảnh để đẩy lên github, nên m cứ kệ nó.
- Để coi mấy file `.md` này á, m nhấm CTRL+SHIFT+V nó sẽ hiển thị ra dạng dễ xem hơn. 
- M có thể đọc qua cái startpoint để biết mà t mới viết có miếng à :v 
- M code đi, có gì cần thì t comment vào. 
- Giới thiệu trước về cấu trúc

# Cấu trúc thư mục

├───.pio
│   └───build
├───.private
│   ├───docs
│   ├───imgs
│   └───msg
├───include
├───lib
│   ├───cArithmeticMacro                : mấy này là các macro so sánh, check
│   ├───cBitWiseMacro                   : mấy này là các macro để thao thác bit
│   ├───cLoopMacro                      : mấy này là các macro để vòng lặp
│   ├───cReturnType                     : mấy này là các macro để chuẩn hóa kiểu trả về mặc định của hàm
│   └───espSerialWrap                   : mấy này là các macro để 
└───src

- Cái thư mục `include` là nó chứa các file.h tổng hợp các tiện ích từ thư mục lib

# msg

Hmmm, m code xong, m push lên github thì t sẽ xem được; Còn giờ tại m share seasion nên t coi được thôi

à là t push lên file HA à 
đúng r


M chạy theo thứ tự từ trên xuống
M sẽ add, commit, push; 

## Thêm toàn bộ thay đổi
```
git add -Av
```
VD:
```
PS C:\Users\Admin\Documents\GitHub\TKDTVN2025> git add -Av
add '.pio/build/project.checksum'
add '.private/msg/NoteAndMessage.md'
```

## Commit nhanh

```
git commit -m "Nội dung ngắn"
```
VD:
```
PS C:\Users\Admin\Documents\GitHub\TKDTVN2025> git commit -m "add note"
[HA 1817442] add note
 2 files changed, 55 insertions(+)
 create mode 100644 .pio/build/project.checksum
 create mode 100644 .private/msg/NoteAndMessage.md
```

## Push 
```
git push -u origin HA
```
VD
```
PS C:\Users\Admin\Documents\GitHub\TKDTVN2025> git push -u origin HA
Enumerating objects: 10, done.
Counting objects: 100% (10/10), done.
Delta compression using up to 12 threads
Compressing objects: 100% (4/4), done.
Writing objects: 100% (8/8), 1.47 KiB | 752.00 KiB/s, done.
Total 8 (delta 0), reused 0 (delta 0), pack-reused 0 (from 0)
To https://github.com/ngxx-fus/TKDTVN2025.git
   86c8171..1817442  HA -> HA
branch 'HA' set up to track 'origin/HA'.
```

m VÔ LINK này coi thử lại đi
https://github.com/ngxx-fus/TKDTVN2025.git

t có cài cái app github là t thấy nó trực tiếp với bên đây lun hay sao á? 
ồ t thấy r 
Thì nó là 1 mà
tại t dùng commentline quen rồi
m cứ dùng app
:)))
Như t remote qua m là t dùng command line chứ đâu có app :v

m bấm build lại thử
coi có lỗi gì k
với lúc build m copy cho t cái đường dẫn tới thằng pio, qua terminal bên này k có cái gì hết á


The terminal process "C:\Users\Admin\.platformio\penv\Scripts\platformio.exe


m kéo cái terminal cao cao tí

ủa lúc m bấm build
cso bị lỗi như v k?
                            build cái nào á? 
ủa chứmm 
nãy t build cái main.cpp

https://meet.google.com/dnp-agdk-qrd
ê meet đi
mà share screen cho t
chứ t k nói được haiz
tại bên này chỉ thấy có cái terminal à
ủa chứ m chưa 

https://meet.google.com/xbf-hrhc-zcx

--
Cái buildtask failed rồi :v --> Đang cài cái espressif32 lại
Có một cái terminal đang chạy á, nên đừng tắt

T k có nghe m nói gì á, tại k có đeo tai nghe nên kiểu có gì thì gõ vô

//ok, t quên tắt mic 


C:\Users\Admin\.platformio\penv\Scripts\pio.exe run

M qua main.cpp đi, với đóng cái terminal đnag mở (có 3 cái đang mở, cái build lỗi thì cho cook đi
)

Với cắm esp vào, lỗi này là chưa cắm esp, haowjc m chưa lôi esp từ trong máy ảo ra

m bấm vô cái chỗ chữ Auto dưới góc màn hình
coi nó đang hconj COM mấy? 
Kế bên shared
K có gì là chauw nhận á
M tắ máy ảo chưa?
Tắt hẳng luôn, tắt cái vmware luôn

hư rồi, để t đi mua cái khác


???
Ủa sao hư, ý là cắm nhầm RX TX thì xác suất hư cx khá thấp :>
M vào device manager
Xong check thử coi có cái com nào k


// t cắm vào r á, lên chưa
```
Checking size .pio\build\esp32dev\firmware.elf
Advanced Memory Usage is available via "PlatformIO Home > Project Inspect"
RAM:   [=         ]   6.6% (used 21464 bytes from 327680 bytes)
Flash: [==        ]  20.4% (used 267237 bytes from 1310720 bytes)
Configuring upload protocol...
AVAILABLE: cmsis-dap, esp-bridge, esp-prog, espota, esptool, iot-bus-jtag, jlink, minimodule, olimex-arm-us
sb-ocd, olimex-arm-usb-ocd-h, olimex-arm-usb-tiny-h, olimex-jtag-tiny, tumpa
CURRENT: upload_protocol = esptool
Looking for upload port...
Error: Please specify `upload_port` for environment or use global `--upload-port` option.
For some development platforms it can be a USB flash drive (i.e. /media/<user>/<device name>)
*** [upload] Explicit exit, status 1
```

