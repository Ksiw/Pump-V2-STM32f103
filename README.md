# PumpV2f103

Данный проект реализует некое реле времени, предназначенное(в частном случае) для управления насосом. 
Сделано оно, чтобы автоматически выдерживать заданную паузу после последнего включения, неважно какого, ручного или автоматического. 
Особенности:

-камень stm32f103c8t6;

-2 режима работы(автоматический/ручное включение);

-светодиодная индикация режимов работы;

-сохранение текущих настроек в ПЗУ(последняя страница флеш памяти камня);

-экран для вывода информации nokia5110;

-датчик температуры ds18b20. По сути он здесь не нужен, а добавлен чтобы было информативнее и в целом веселей;

-Используется FreeRTOS, таким образом достигается независимоть тасков программы и в целом повышается стабильность и отзывчивость;

-Проект сделан на visual studio + visualGDB.
![Alt text](photo_2018-10-03_14-22-03.jpg?raw=true "Title")
P.S. Простите за мусор в репозитории, это чисто рабочая версия, плагин к студии же тащит за собой кучу зависимостей и разбираться с каждой, впадлу. Появились вопросы - не стесняйся, пиши, разберемся.
