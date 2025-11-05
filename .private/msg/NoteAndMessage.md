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


M sẽ add, commit, push

## Thêm toàn bộ thay đổi
```
git add -Av
```

## Commit nhanh

```
git commit -m "Nội dung ngắn"
```

## Push 
```
git push -u origin HA
```

M chạy theo thứ tự từ trên xuống