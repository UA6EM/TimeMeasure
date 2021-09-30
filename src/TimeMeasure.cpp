#include  <arduino.h>
#include  "TimeMeasure.h"

//
//  Измеряем время высокого уровня на пине 2 (aka PD2)
//  (чтобы измерять на пине 3 (PD3), нужно раскомментировать следующую строку)
//  #define USE_INT_1
//
//  Измеряемый интервал - от 80 до 65534 тактов процессора
//  (при 20МГц это 4 - 3275 микросекунд, при 16МГц это 5 - 4095 микросекунд
//  Если высокий уровень держится 65535 тактов, то
//  считаем это ошибкой и выдаём результат ENDLESS_INTERVAL
//
//  Результат измерений - количество тактов процессора.
//  Чтобы получить время в микросекундах, нужно полученный результат
//  разделить на константу TICKS_PER_MICROSECOND
//  

#ifdef  USE_INT_1 // используем пин 3

  #define PIN_NUMBER  3
  #define PIN_MASK  (bit(PD3))
  #define INT_FLAG  INT1
  #define MODE_BIT  ISC10
  #define INT_VECT  INT1_vect

#else // используем пин 2

  #define PIN_NUMBER  2
  #define PIN_MASK  (bit(PD2))
  #define INT_FLAG  INT0
  #define MODE_BIT  ISC00
  #define INT_VECT  INT0_vect
  
#endif

//
//  Инициализация таймера-счётчика 1
//  Состоит из начальной инициализации, которая выполняется один раз
//  и дополнительной инициализации, которая выполняется перед каждым 
//  измерением (stopTimerCounter1)
//
static void stopTimerCounter1(void) {
  TCCR1B = 0; //  остановим таймер
  TCNT1 = 0;  // считаем с 0
}
static void initTimerCounter1(void) {
  stopTimerCounter1();
  TIMSK1 = bit(TOIE1);  //  Разрешить прерывание по переполнению
  TCCR1A = 0;
  TCCR1C = 0;
}

//
//  Инициализация внешнего прерывания 0/1 на смену состояния
//
static void initInterrupt0(void) {
  EICRA = bit(MODE_BIT);  // на смену состояния
  EIMSK = bit(INT_FLAG);  // прерывание 0/1
}

//
//  Инициализация всего (нужно вызвать из setup)
//
void initTimeMeasuring(void) {
  pinMode(PIN_NUMBER, INPUT);
  initTimerCounter1();
  initInterrupt0();
}

static volatile uint16_t result = 0;

//
//  Функция возвращает результат измерения, заодно обнуляя его
//
uint16_t measureResult(void) {
  cli();
  const uint16_t res = result;
  result = 0;
  sei();
  return res;
}

//
//  Если случилось переполнение, значит истёк таймаут
//  Ставим таймаут результатом и реинициализируем таймер 
//  для следующегг измерения
//
ISR(TIMER1_OVF_vect) {
  result = ENDLESS_INTERVAL;
  stopTimerCounter1();
}

//
//  Прилетело прерывание
//  Если на пинe PD2/PD3 высокий уровень - начинаем отсчёт
//  Если на пине PD2/PD3 низкий уровень - заканчиваем отсчёт
//
//  Примечание: в этой функции количество тактов до включения таймера
//  и до запоминания результата одинаковое, т.е. поправки не нужны.
//  Теоретически одинаковость может пропасть при новой версии компилятора
//  или при других опциях. Если есть сомнения, можно её убрать, а вместо ней
//  использовать закомментированную (ниже) функцию с ассемблерным кодом.
//  Там то уж никто не нагадит.
//
/*
ISR(INT_VECT) {
  if (PIND & PIN_MASK) {
    TCCR1B = 1; // запускаем таймер
  } else {
    result = TCNT1;
    stopTimerCounter1();
  }
}
/*
 */
ISR(INT_VECT) {
asm volatile(
   // if (PIND & bit(PDx))
      "sbis %[Prt],%[Mask]  \r\n" 
      "rjmp SignalEnds    \r\n"
  // {
      "ldi  r24, 1      \r\n" //  Единицу пихаем в TCCRB1
      "sts  %[TCCRB], r24 \r\n" //  т.е. запускаем таймер
      "rjmp AllDone     \r\n"  
  // } else {
    "SignalEnds: \r\n"
      "lds  r24, %[TCNTL] \r\n" // младший байт TCNT1 читаем раньше старшего
      "lds  r25, %[TCNTH] \r\n"
      "sts  %[res]+1, r25 \r\n" //  Значение TCNT1 пихаем 
      "sts  %[res], r24   \r\n" //  в переменнуюresult
      "sts  %[TCCRB], r1  \r\n" //  Пихаем 0 в TCCRB1 - останавливаем таймер
      "sts  %[TCNTH], r1  \r\n" // Пихаем 0 в TCNT1 на будущее 
      "sts  %[TCNTL], r1  \r\n" //  старший байт раньше младшего
  // }    
    "AllDone: \r\n"
    :
    [res] "=m" (result)
  : 
    [Prt] "I" (_SFR_IO_ADDR(PIND)), 
    [Mask] "M" (PIN_NUMBER),
    [TCCRB] "M" (_SFR_MEM_ADDR(TCCR1B)),
    [TCNTH] "M" (_SFR_MEM_ADDR(TCNT1H)),
    [TCNTL] "M" (_SFR_MEM_ADDR(TCNT1L))
  :
    "r24", "r25"
  );
}
/*
*/

//
