/*
 * Консольная версия программы для измерения и настройки аппаратуры 
 * радиоуправления v 1.00 (c) UA6EM 2018-2019 год.
 * Для измерения канала используется пин D3 (INT1) atmega328
 * Особая благодарность разработчику библиотеки ЕвгенийП (arduino.ru)
 * Не забываем прогреть должным образом радиопередающее устройство
 * для установления стабильных показаний. Точность измерения  на
 * программном уровне лучше 0.1 микросекнды, для улучшения этого 
 * показателя возможно внешнее тактирование от высокостабильного 
 * кварцевого генератора.
 */
 
#include  "TimeMeasure.h"
float pwm_f = 0;
unsigned int pwm = 0;

void setup() {
  Serial.begin(115200);
  initTimeMeasuring();
}

void loop() {
  const uint16_t res = measureResult();
  if (res){
   pwm_f = res; 
   pwm_f = pwm_f/TICKS_PER_MICROSECOND;
   pwm = ticks2Microseconds(res);
     }
   Serial.print(pwm);
   Serial.print(" - ");
   Serial.println(pwm_f); 
   delay(1000);
}
