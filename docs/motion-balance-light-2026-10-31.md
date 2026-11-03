# Animation, bảng giá và ánh sáng — 31/10/2026

## Thay đổi

- Tách clip idle khỏi clip vận hành: Water Slides 425 (47/46), Skull River 427 (49/48), Wave Rider 515 (151/150), Log Flume 522 (159/158). Hiệu ứng nước tiếp tục khi không có khách. Water Slides thêm dịch UV riêng dải tia nước, không dịch vùng atlas của vật trang trí.
- Mở rộng liên kết clip gốc theo thứ tự catalog và cấu trúc node. Đây là mapping suy luận, không phải bảng liên kết máy chủ được khôi phục. 76 ứng viên được xuất; 52 ride hiện trong danh mục đạt kiểm tra runtime. Bouncy Castle 404 giữ cơ chế riêng đã có.
- Mua thực tế dùng giá và thời gian xây mới; danh mục sắp theo giá tăng dần, hiển thị thời gian xây. Bouncy Castle $2.600 / 3s, Teacups $6.300 / 10s, Ferris Wheel $6.800 / 15s. Các mức còn lại là cân bằng tạm theo nhóm kích thước và ID, chưa cân bằng theo thời gian hoàn vốn. Không thay tiền trong save.
- Minigame 20s phát một tiếng khách khi đạt 16,5s (còn 3,5s), tuân theo bật/tắt âm thanh. Dùng mẫu gốc sample 10 của nhóm giọng khách; chưa xác nhận đây là tiếng reo chính xác của trò gốc bằng nghe đối chiếu.
- Thử chu kỳ ngày/đêm 360 giây mô phỏng, thay hướng đèn và ánh sáng môi trường. Settings có nút bật/tắt, tắt trở về ban ngày. Chưa có đổ bóng hoặc đèn điểm riêng; lựa chọn bật/tắt chưa lưu qua lần mở game.

## Giới hạn cần xử lý tiếp

- 414 và 415: chưa tìm được clip/assembly đáng tin cậy; không quay toàn bộ mô hình để giả animation.
- 422: clip ứng viên 43 có transform cực lớn (tới hàng tỷ), bị loại để tránh biến dạng. Chưa khôi phục đầy đủ vận hành.
- Hedge Maze 432: chưa có đường đi nội bộ/clip phù hợp.
- Kiểm tra chuyển động node/ghế không chứng minh mọi tư thế NPC hoặc toàn bộ chu kỳ đã giống bản gốc. Các dòng có 0 ghế chuyển động chỉ xác nhận chuyển động mô hình, chưa xác nhận tương tác NPC di chuyển.

## Kiểm chứng

53 Python tests, simulation C++ và native self-test đạt. Native kiểm tra 52 ride tại 12 thời điểm: root đứng yên, ma trận hữu hạn và có node chuyển động; kiểm tra idle nước và tiếng khách chỉ kích hoạt một lần. Ảnh ngày/đêm, Water Slides và Ferris Wheel đã xem trực tiếp. Các ảnh mẫu bổ sung nằm trong analysis/runtime_test/expanded-motion-*.png.

Executable: build/ThemeParkGame.exe, native AMD64, 1.024.000 byte, SHA-256 `03e4e616b01ef5ed3c78e0ee42857668429e91e77eab8997382864bf87145d5b`.

## Từng ride có clip được kiểm tra

| ID | Tên hiện tại | Clip | Node chuyển động | Locator ghế chuyển động |
|---|---|---:|---:|---:|
| 425 | Water Slides | 46 | 25 | 5 |
| 427 | Skull River | 48 | 25 | 10 |
| 515 | Wave Rider | 150 | 4 | 1 |
| 522 | Log Flume | 158 | 56 | 12 |
| 403 | Stone Castle | 32 | 13 | 4 |
| 409 | Pirate Ship | 34 | 13 | 12 |
| 410 | Fountain Ride | 35 | 8 | 6 |
| 416 | Original Ride 416 | 38 | 15 | 10 |
| 418 | Original Ride 418 | 39 | 22 | 8 |
| 420 | Original Ride 420 | 41 | 56 | 3 |
| 421 | Original Ride 421 | 42 | 21 | 0 |
| 423 | Original Ride 423 | 44 | 6 | 1 |
| 424 | Original Ride 424 | 45 | 15 | 10 |
| 428 | Original Ride 428 | 50 | 18 | 5 |
| 429 | Original Ride 429 | 51 | 8 | 4 |
| 430 | Original Ride 430 | 52 | 19 | 6 |
| 431 | Original Ride 431 | 55 | 10 | 5 |
| 433 | Original Ride 433 | 60 | 3 | 1 |
| 434 | Original Ride 434 | 61 | 3 | 1 |
| 436 | Original Ride 436 | 63 | 16 | 9 |
| 438 | Original Ride 438 | 66 | 16 | 9 |
| 440 | Original Ride 440 | 69 | 17 | 12 |
| 441 | Original Ride 441 | 70 | 18 | 12 |
| 443 | Original Ride 443 | 71 | 1 | 1 |
| 444 | Original Ride 444 | 72 | 2 | 0 |
| 445 | Original Ride 445 | 73 | 9 | 8 |
| 447 | Original Ride 447 | 74 | 54 | 30 |
| 449 | Teacups | 75 | 13 | 8 |
| 450 | Original Ride 450 | 78 | 13 | 8 |
| 452 | Original Ride 452 | 81 | 37 | 16 |
| 453 | Original Ride 453 | 82 | 98 | 16 |
| 454 | Original Ride 454 | 83 | 17 | 8 |
| 455 | Original Ride 455 | 86 | 5 | 0 |
| 456 | Original Ride 456 | 87 | 10 | 8 |
| 458 | Original Ride 458 | 88 | 12 | 6 |
| 460 | Original Ride 460 | 89 | 10 | 8 |
| 461 | Original Ride 461 | 90 | 12 | 9 |
| 463 | Parasol Chairs | 91 | 37 | 12 |
| 465 | Carousel | 94 | 16 | 12 |
| 467 | Original Ride 467 | 97 | 14 | 8 |
| 469 | Original Ride 469 | 98 | 19 | 1 |
| 470 | Original Ride 470 | 99 | 33 | 2 |
| 471 | Original Ride 471 | 100 | 43 | 12 |
| 472 | Original Ride 472 | 101 | 42 | 20 |
| 474 | Original Ride 474 | 103 | 3 | 0 |
| 476 | Ferris Wheel | 107 | 25 | 12 |
| 477 | Original Ride 477 | 108 | 13 | 0 |
| 479 | Original Ride 479 | 113 | 25 | 12 |
| 517 | Original Ride 517 | 152 | 17 | 12 |
| 519 | Original Ride 519 | 155 | 26 | 0 |
| 520 | Original Ride 520 | 156 | 12 | 8 |
| 521 | Original Ride 521 | 157 | 7 | 4 |

## Bảng giá đầy đủ

Bao gồm cả mục ẩn để giữ tương thích catalog/save. Chỉ mục có artwork đủ điều kiện mới hiện trong shop.

| ID | Tên | Cỡ | Giá tiền | Xây (giây) |
|---|---|---:|---:|---:|
| 425 | Water Slides | 16 | 60000 | 60 |
| 427 | Skull River | 16 | 62000 | 90 |
| 515 | Wave Rider | 6 | 15000 | 20 |
| 522 | Log Flume | 16 | 57000 | 90 |
| 404 | Bouncy Castle | 4 | 2600 | 3 |
| 403 | Stone Castle | 4 | 7740 | 30 |
| 409 | Pirate Ship | 4 | 8820 | 35 |
| 410 | Fountain Ride | 4 | 9000 | 15 |
| 414 | Original Ride 414 | 4 | 9720 | 35 |
| 415 | Original Ride 415 | 4 | 9900 | 15 |
| 416 | Original Ride 416 | 16 | 51000 | 75 |
| 418 | Original Ride 418 | 16 | 53000 | 105 |
| 419 | Original Ride 419 | 16 | 54000 | 120 |
| 420 | Original Ride 420 | 16 | 55000 | 60 |
| 421 | Original Ride 421 | 4 | 10980 | 20 |
| 422 | Original Ride 422 | 6 | 17800 | 40 |
| 423 | Original Ride 423 | 6 | 18200 | 50 |
| 424 | Original Ride 424 | 16 | 59000 | 120 |
| 428 | Original Ride 428 | 16 | 63000 | 105 |
| 429 | Original Ride 429 | 6 | 20600 | 60 |
| 430 | Original Ride 430 | 4 | 12600 | 15 |
| 431 | Original Ride 431 | 6 | 21400 | 30 |
| 432 | Hedge Maze | 6 | 21800 | 40 |
| 433 | Original Ride 433 | 4 | 13140 | 30 |
| 434 | Original Ride 434 | 4 | 13320 | 35 |
| 436 | Original Ride 436 | 6 | 23400 | 30 |
| 438 | Original Ride 438 | 6 | 24200 | 50 |
| 440 | Original Ride 440 | 4 | 14400 | 15 |
| 441 | Original Ride 441 | 4 | 14580 | 20 |
| 443 | Original Ride 443 | 4 | 14940 | 30 |
| 444 | Original Ride 444 | 4 | 15120 | 35 |
| 445 | Original Ride 445 | 4 | 15300 | 15 |
| 447 | Original Ride 447 | 6 | 27800 | 40 |
| 449 | Teacups | 4 | 6300 | 10 |
| 450 | Original Ride 450 | 4 | 16200 | 15 |
| 452 | Original Ride 452 | 6 | 29800 | 40 |
| 453 | Original Ride 453 | 6 | 30200 | 50 |
| 454 | Original Ride 454 | 4 | 16920 | 35 |
| 455 | Original Ride 455 | 6 | 31000 | 20 |
| 456 | Original Ride 456 | 6 | 31400 | 30 |
| 458 | Original Ride 458 | 6 | 32200 | 50 |
| 460 | Original Ride 460 | 16 | 95000 | 60 |
| 461 | Original Ride 461 | 16 | 96000 | 75 |
| 463 | Parasol Chairs | 4 | 18540 | 30 |
| 465 | Carousel | 6 | 35000 | 20 |
| 467 | Original Ride 467 | 4 | 19260 | 25 |
| 469 | Original Ride 469 | 6 | 36600 | 60 |
| 470 | Original Ride 470 | 6 | 37000 | 20 |
| 471 | Original Ride 471 | 6 | 37400 | 30 |
| 472 | Original Ride 472 | 6 | 37800 | 40 |
| 473 | Original Ride 473 | 16 | 108000 | 105 |
| 474 | Original Ride 474 | 6 | 38600 | 60 |
| 475 | Original Ride 475 | 6 | 39000 | 20 |
| 476 | Ferris Wheel | 6 | 6800 | 15 |
| 477 | Original Ride 477 | 6 | 39800 | 40 |
| 479 | Original Ride 479 | 6 | 40600 | 60 |
| 484 | Original Ride 484 | 4 | 22320 | 35 |
| 486 | Original Ride 486 | 4 | 22680 | 20 |
| 487 | Original Ride 487 | 6 | 43800 | 40 |
| 488 | Original Ride 488 | 4 | 23040 | 30 |
| 489 | Original Ride 489 | 16 | 124000 | 120 |
| 490 | Original Ride 490 | 6 | 45000 | 20 |
| 491 | Original Ride 491 | 16 | 126000 | 75 |
| 492 | Original Ride 492 | 6 | 45800 | 40 |
| 493 | Original Ride 493 | 6 | 46200 | 50 |
| 495 | Original Ride 495 | 6 | 47000 | 20 |
| 497 | Original Ride 497 | 6 | 47800 | 40 |
| 498 | Original Ride 498 | 6 | 48200 | 50 |
| 499 | Original Ride 499 | 6 | 48600 | 60 |
| 500 | Original Ride 500 | 6 | 9000 | 20 |
| 502 | Original Ride 502 | 16 | 37000 | 90 |
| 503 | Original Ride 503 | 16 | 38000 | 105 |
| 508 | Original Ride 508 | 6 | 12200 | 50 |
| 509 | Original Ride 509 | 6 | 12600 | 60 |
| 511 | Original Ride 511 | 6 | 13400 | 30 |
| 512 | Original Ride 512 | 4 | 9360 | 25 |
| 513 | Original Ride 513 | 16 | 48000 | 105 |
| 514 | Original Ride 514 | 6 | 14600 | 60 |
| 517 | Original Ride 517 | 4 | 10260 | 25 |
| 519 | Original Ride 519 | 6 | 16600 | 60 |
| 520 | Original Ride 520 | 16 | 55000 | 60 |
| 521 | Original Ride 521 | 16 | 56000 | 75 |
| 529 | Original Ride 529 | 6 | 20600 | 60 |
| 531 | Original Ride 531 | 16 | 66000 | 75 |
| 533 | Original Ride 533 | 6 | 22200 | 50 |
| 534 | Original Ride 534 | 4 | 13320 | 35 |
| 535 | Original Ride 535 | 16 | 70000 | 60 |
| 536 | Original Ride 536 | 6 | 23400 | 30 |
| 537 | Original Ride 537 | 6 | 23800 | 40 |
| 538 | Original Ride 538 | 4 | 14040 | 30 |
| 539 | Original Ride 539 | 6 | 24600 | 60 |
| 540 | Original Ride 540 | 6 | 25000 | 20 |
| 541 | Original Ride 541 | 16 | 76000 | 75 |
| 542 | Original Ride 542 | 4 | 14760 | 25 |
| 543 | Original Ride 543 | 6 | 26200 | 50 |
| 544 | Original Ride 544 | 6 | 26600 | 60 |
| 545 | Candy Floss | 4 | 6600 | 10 |
| 546 | Candy Shop | 6 | 6720 | 15 |
| 547 | Hot Dogs | 4 | 6840 | 20 |
| 548 | Octopus Shop | 4 | 6960 | 25 |
| 549 | Pumpkin Shop | 4 | 7080 | 30 |
| 550 | Fries | 4 | 7200 | 10 |
| 551 | Balloons | 4 | 7320 | 15 |
| 552 | Coffee | 4 | 7440 | 20 |
| 553 | Soft Drinks | 4 | 7560 | 25 |
| 554 | Gingerbread | 6 | 7680 | 30 |
| 555 | Burgers | 4 | 7800 | 10 |
| 556 | Christmas Shop | 4 | 7920 | 15 |
| 557 | Fish Shop | 6 | 8040 | 20 |
| 558 | Ice Cream | 6 | 8160 | 25 |
| 559 | Pizza | 4 | 8280 | 30 |
| 560 | Pirate Shop | 6 | 8400 | 10 |
| 561 | Gift Shop | 6 | 8520 | 15 |
| 562 | Treasure Shop | 6 | 8640 | 20 |
| 563 | Saloon | 6 | 8760 | 25 |
| 564 | Chicken | 6 | 8880 | 30 |
| 565 | Tropical Shop | 6 | 9000 | 10 |
| 566 | Toys | 6 | 9120 | 15 |
| 567 | Dr Pepper Drinks | 4 | 9240 | 20 |
| 568 | Dr Pepper Dice | 6 | 9360 | 25 |
| 569 | Dr Pepper Lounge | 4 | 9480 | 30 |
| 570 | Party Shop | 4 | 9600 | 10 |
| 571 | Tea Garden | 16 | 9720 | 15 |
| 572 | Lemonade | 4 | 9840 | 20 |
| 573 | Popcorn | 6 | 9960 | 25 |
| 574 | Kites | 6 | 10080 | 30 |
| 575 | Torch | 4 | 10200 | 10 |
| 576 | Workshop | 4 | 10320 | 15 |
| 577 | Play Frame | 4 | 10440 | 20 |
| 578 | Volcano | 4 | 10560 | 25 |
| 579 | Flowers | 4 | 10680 | 30 |
| 580 | Picnic Garden | 6 | 10800 | 10 |
| 581 | Totem | 4 | 10920 | 15 |
| 582 | Tiki Shop | 4 | 11040 | 20 |
| 592 | Original Shop 592 | 4 | 12240 | 20 |
| 593 | Original Shop 593 | 4 | 12360 | 25 |
| 594 | Original Shop 594 | 4 | 12480 | 30 |
| 595 | Original Shop 595 | 4 | 12600 | 10 |
| 596 | Original Shop 596 | 6 | 12720 | 15 |
| 597 | Original Shop 597 | 16 | 12840 | 20 |
| 598 | Original Shop 598 | 4 | 12960 | 25 |
| 599 | Original Shop 599 | 4 | 13080 | 30 |
| 600 | Original Shop 600 | 4 | 1200 | 10 |
| 601 | Original Shop 601 | 6 | 1320 | 15 |
| 602 | Original Shop 602 | 4 | 1440 | 20 |
| 603 | Original Shop 603 | 4 | 1560 | 25 |
| 604 | Original Shop 604 | 4 | 1680 | 30 |
| 605 | Original Shop 605 | 6 | 1800 | 10 |
| 606 | Original Shop 606 | 4 | 1920 | 15 |
| 607 | Original Shop 607 | 4 | 2040 | 20 |
| 608 | Original Shop 608 | 4 | 2160 | 25 |
| 609 | Original Shop 609 | 4 | 2280 | 30 |
| 610 | Original Shop 610 | 6 | 2400 | 10 |
| 611 | Original Shop 611 | 6 | 2520 | 15 |
| 612 | Original Shop 612 | 6 | 2640 | 20 |
| 613 | Original Shop 613 | 6 | 2760 | 25 |
| 614 | Original Shop 614 | 6 | 2880 | 30 |
| 615 | Original Shop 615 | 6 | 3000 | 10 |
| 616 | Original Shop 616 | 16 | 3120 | 15 |
| 617 | Original Shop 617 | 4 | 3240 | 20 |
| 618 | Wooden Kiosk | 1 | 3360 | 25 |
| 619 | Phone Booth | 1 | 3480 | 30 |
| 620 | Toilets | 1 | 3600 | 10 |
