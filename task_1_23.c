#include "periph/gpio.h"
#include "xtimer.h"

#define DEBOUNCE_DELAY 300000   
#define LONG_PRESS 3000000 
#define SHORT_BLINK 100000  // 100 мс
#define LONG_BLINK 500000   // 500 мс

static gpio_t led_pin = GPIO_PIN(PORT_C, 13);
static gpio_t button_pin = GPIO_PIN(PORT_B, 1);
static xtimer_t debounce_timer;
static xtimer_t led_timer;
static uint32_t press_start_time;
static uint32_t press_duration;
static bool led_state = false;
static bool led_fire = false;
static bool is_button_pressed = false;

void toggle_led(void *arg) {
    (void)arg;
    if (led_state) {
        gpio_toggle(led_pin);
        // Запускаем таймер снова для следующего мигания
        xtimer_set(&led_timer, press_duration >= LONG_PRESS ? LONG_BLINK : SHORT_BLINK);
    }
}

void debounce_callback(void *arg) {
    (void)arg;
    gpio_irq_enable(button_pin);
}

void button_handler(void *arg) {
    (void)arg;
    if (!is_button_pressed) {
        press_start_time = xtimer_now_usec();
        is_button_pressed = true;
    } else {
        uint32_t press_end_time = xtimer_now_usec();
        press_duration = (press_end_time < press_start_time) 
                         ? (UINT32_MAX - press_start_time + press_end_time) 
                         : (press_end_time - press_start_time);

        led_fire = !led_fire;
        led_state = led_fire;

        if (led_fire) {
            // Начать мигание
            toggle_led(NULL);
        } else {
            // Остановить мигание
            xtimer_remove(&led_timer);
            gpio_set(led_pin);
        }

        is_button_pressed = false;
        gpio_irq_disable(button_pin);
        xtimer_set(&debounce_timer, DEBOUNCE_DELAY);
    }
}

int main(void) {
    gpio_init(led_pin, GPIO_OUT);
    gpio_init_int(button_pin, GPIO_IN_PU, GPIO_BOTH, button_handler, NULL);

    debounce_timer.callback = debounce_callback;
    led_timer.callback = toggle_led;

    while (1) {
    }
    return 0;
}
