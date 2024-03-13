#include <stdio.h>
#include "periph/gpio.h"
#include "xtimer.h"

#define DEBOUNCE_DELAY 300000   // Задержка для антидребезга
#define LONG_PRESS 300000       // Время для определения долгого нажатия

// Определение пинов для светодиодов
static gpio_t green_led_pin = GPIO_PIN(PORT_C, 13);//green
static gpio_t yellow_led_pin = GPIO_PIN(PORT_B, 14);
static gpio_t red_led_pin = GPIO_PIN(PORT_B, 7);

// Определение пина для кнопки
static gpio_t button_pin = GPIO_PIN(PORT_B, 11);

static bool led_timer_active = true; // Добавляем флаг активности таймера

// Таймеры
static xtimer_t debounce_timer;
static xtimer_t led_timer;

// Временные метки
static uint32_t last_green_time = 0;
static uint32_t current_phase_time = 5000000; // 5 секунд для каждой фазы

enum { RED, YELLOW, GREEN } current_light = GREEN;

void toggle_leds(void)  {
    if (!led_timer_active) {
        return; // Не выполняем переключение, если таймер неактивен
    }
    switch (current_light) {
        case RED:
            gpio_set(red_led_pin);
            gpio_clear(yellow_led_pin);
            gpio_set(green_led_pin);
            current_light = YELLOW;

            break;
        
        case YELLOW:
            gpio_clear(red_led_pin);
            gpio_set(yellow_led_pin);
            gpio_set(green_led_pin);
            current_light = GREEN;
            break;
        case GREEN:
            gpio_clear(red_led_pin);
            gpio_clear(yellow_led_pin);
            gpio_clear(green_led_pin);
            // новое время - зеленого цвета
            last_green_time = xtimer_now_usec(); 
            current_light = RED;
            break;
    }
    xtimer_set(&led_timer, current_phase_time);
}

void debounce_callback(void *arg) {
    (void)arg;
    gpio_irq_enable(button_pin);
}
uint32_t time_since_green;

void button_handler(void *arg) {
    (void)arg;
    uint32_t now = xtimer_now_usec(); 

    // Учет переполнения таймера
    if (last_green_time > now) { 
        time_since_green = (UINT32_MAX - last_green_time) + now;
    } else {
        time_since_green = now - last_green_time;
    }

  if ((current_light != RED) && ((time_since_green >= 7000000)&& (time_since_green <= 9000000))) {
        xtimer_remove(&led_timer); // Удаляем текущий таймер
        led_timer_active = false;  // Устанавливаем флаг таймера в неактивное состояние
        current_light = GREEN;     // Устанавливаем зеленый свет
        toggle_leds();   
        led_timer_active = true;           // Мгновенно переключаем на зеленый свет
        xtimer_set(&led_timer, LONG_PRESS); // Устанавливаем новый таймер
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
    current_light = GREEN; 
    // Начальное состояние светофора
    toggle_leds();

    while (1) {
        xtimer_sleep(1);
    }

    return 0;
}
