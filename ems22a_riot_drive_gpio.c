#include "periph/spi.h"
#include "periph/gpio.h"
#include "xtimer.h"

#define PIN_CS GPIO_PIN(PORT_A, 4) // Пин для CS
#define PIN_CLK GPIO_PIN(PORT_A, 5) // Пин для CLK
#define PIN_DO GPIO_PIN(PORT_A, 6) // Пин для DO
#define EMS22A_READINGS_PER_SEC 1000 // если вы хотите считывать данные 1000 раз в секунду

void init_pins(void) {
    gpio_init(PIN_CS, GPIO_OUT);
    gpio_init(PIN_CLK, GPIO_OUT);
    gpio_init(PIN_DO, GPIO_IN);
    gpio_set(PIN_CS); // Активируем энкодер (CS в высокое состояние)
}

uint16_t read_encoder(void) {
    uint16_t data = 0;
    gpio_clear(PIN_CS); // Начало передачи (CS в низкое состояние)
    
    for (int i = 0; i < 10; i++) { // Читаем только первые 10 бит данных угла
        gpio_clear(PIN_CLK);
        xtimer_usleep(1); // Время задержки должно соответствовать требованиям вашей системы
        gpio_set(PIN_CLK);
        xtimer_usleep(1);
        
        data <<= 1;
        if (gpio_read(PIN_DO)) {
            data |= 1;
        }
    }
    
    for (int i = 0; i < 6; i++) { // Пропускаем статусные и биты четности
        gpio_clear(PIN_CLK);
        xtimer_usleep(1);
        gpio_set(PIN_CLK);
        xtimer_usleep(1);
    }
    
    gpio_set(PIN_CS); // Конец передачи (CS в высокое состояние)
    return data;
}

int main(void) {
    init_pins();
    
    while (1) {
        uint16_t angle_data = read_encoder();
        printf("Угол: %u\n", angle_data); // Используем %u для вывода беззнакового значения
        xtimer_usleep(1000000 / EMS22A_READINGS_PER_SEC); // Задержка между чтениями
    }
    
    return 0;
}
