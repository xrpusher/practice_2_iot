#include <stdio.h>
#include "periph/gpio.h"
#include "xtimer.h"

// Определяем GPIO пин кнопки и светодиода
#define BTN_PIN GPIO_PIN(PORT_B, 1)
#define LED_PIN GPIO_PIN(PORT_C, 13)
// Задержка для устранения дребезга контактов кнопки
#define DEBOUNCE_TIME (50U * US_PER_MS) // 50 миллисекунд

int main(void) {
    // Инициализируем GPIO пин светодиода как выход
    gpio_init(LED_PIN, GPIO_OUT);
    // Инициализируем GPIO пин кнопки как вход
    gpio_init(BTN_PIN, GPIO_IN);

    // Состояние светодиода (вкл/выкл)
    bool led_state = false;
    // Выключаем светодиод в начале
    gpio_clear(LED_PIN);

    // Бесконечный цикл
    while (1) {
        // Проверяем, нажата ли кнопка
        if (gpio_read(BTN_PIN)) {
            // Ждем для устранения дребезга контактов
            xtimer_usleep(DEBOUNCE_TIME);
            // Проверяем кнопку еще раз после задержки
            if (gpio_read(BTN_PIN)) {
                // Переключаем состояние светодиода
                led_state = !led_state;
                // Включаем или выключаем светодиод
                if (led_state) {
                    gpio_set(LED_PIN);
                } else {
                    gpio_clear(LED_PIN);
                }
                // Ждем, пока кнопка будет отпущена
                while (gpio_read(BTN_PIN));
            }
        }
        // Небольшая задержка, чтобы снизить нагрузку на процессор
        xtimer_usleep(10 * US_PER_MS);
    }

    return 0;
}
