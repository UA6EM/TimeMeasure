#ifndef  TimeMeasure_h
#define TimeMeasure_h
//
//  Измеряем время высокого уровня на пине 2 (aka PD2) или 3 (PD3)
//  (см. комментарий в начале файла TimeMeasure.cpp)
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

//
// Константа для 
//    1. пересчёта тактов в микросекунды (нужно поделить количество тактов на неё)
//    2. пересчёта микросекунд в такты (нужно умножить количество микросекунд на неё)
static const uint8_t TICKS_PER_MICROSECOND = static_cast <uint8_t> (F_CPU / 1000000UL);
//
// Интервал (в тактах), который считаем ошибкой (слишком большой)
static const uint16_t ENDLESS_INTERVAL = UINT16_MAX;

//
//  Инициализация всего (нужно вызвать из setup)
//
extern void initTimeMeasuring(void);

//
//  Функция возвращает результат измерения, заодно обнуляя его
//
extern uint16_t measureResult(void);

//
//  Пересчёт тактов в целые микросекунды с округлением до ближайшего целого
//
inline uint16_t ticks2Microseconds(const uint16_t ticks) {
  return static_cast <uint16_t> ((static_cast <uint32_t> (ticks) + TICKS_PER_MICROSECOND / 2) / TICKS_PER_MICROSECOND);
}

//
#endif  //  TimeMeasure_h
