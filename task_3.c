#include <stdio.h>
#include "periph/gpio.h"
#include "xtimer.h"

#define DEBOUNCE_DELAY 300000   // Задержка для антидребезга
#define LONG_PRESS 30000      // Время для определения долгого нажатия

// Определение пинов для светодиодов
static gpio_t red_led_pin = GPIO_PIN(PORT_C, 13);
static gpio_t yellow_led_pin = GPIO_PIN(PORT_B, 14);
static gpio_t green_led_pin = GPIO_PIN(PORT_B, 7);

// Определение пина для кнопки
static gpio_t button_pin = GPIO_PIN(PORT_B, 11);

// Таймеры
static xtimer_t debounce_timer;
static xtimer_t led_timer;

// Временные метки
static uint32_t last_green_time = 0;
static uint32_t current_phase_time = 5000000; // 5 секунд для каждой фазы

enum { RED, YELLOW, GREEN } current_light = RED;

void toggle_leds(void)  {
    switch (current_light) {
        case RED:
            gpio_clear(red_led_pin);
            gpio_clear(yellow_led_pin);
            gpio_clear(green_led_pin);
            current_light = GREEN;

            break;
        case GREEN:
            gpio_set(red_led_pin);
            gpio_clear(yellow_led_pin);
            gpio_set(green_led_pin);
            last_green_time = xtimer_now_usec(); 
            current_light = YELLOW;
            break;
        case YELLOW:
            gpio_set(red_led_pin);
            gpio_set(yellow_led_pin);
            gpio_clear(green_led_pin);
            current_light = RED;
            break;
    }
    xtimer_set(&led_timer, current_phase_time);
}

void debounce_callback(void *arg) {
    (void)arg;
    gpio_irq_enable(button_pin);
}


void button_handler(void *arg) {
    (void)arg;
    uint32_t now = xtimer_now_usec();
    uint32_t time_since_green;

    if (last_green_time > now) { // Учет переполнения таймера
        time_since_green = (UINT32_MAX - last_green_time) + now;
    } else {
        time_since_green = now - last_green_time;
    }

    if (current_light != GREEN && time_since_green + LONG_PRESS < current_phase_time) {
        // Ускоряем переключение на зеленый свет
        xtimer_set(&led_timer, LONG_PRESS);
    }

    gpio_irq_disable(button_pin);
    xtimer_set(&debounce_timer, DEBOUNCE_DELAY);
}

int main(void) {
    // Инициализация светодиодов
    gpio_init(red_led_pin, GPIO_OUT);
    gpio_init(yellow_led_pin, GPIO_OUT);
    gpio_init(green_led_pin, GPIO_OUT);

    // Инициализация кнопки
    if (gpio_init_int(button_pin, GPIO_IN_PU, GPIO_FALLING, button_handler, NULL) < 0) {
        printf("Ошибка инициализации кнопки\n");
        return 1;
    }

    // Настройка таймеров
    debounce_timer.callback = debounce_callback;
    led_timer.callback = (void (*)(void *))toggle_leds;

    // Начальное состояние светофора
    toggle_leds();

    while (1) {
        xtimer_sleep(1);
    }

    return 0;
}
