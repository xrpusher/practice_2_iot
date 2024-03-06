#include "ztimer.h"
#include "periph/gpio.h"
#include "xtimer.h"

#define DEBOUNCE_DELAY 300000   
#define LONG_PRESS 3000000 

static gpio_t led_pin = GPIO_PIN(PORT_C, 13);
static gpio_t button_pin = GPIO_PIN(PORT_B, 1);
static xtimer_t debounce_timer;
static uint32_t press_start_time;
static uint32_t press_duration;
static bool led_state = false;
static bool led_fire = false;
static bool is_button_pressed = false;

uint32_t blink_interval;
bool blink_interval_state = false;

void toggle_led(void *arg) {
    (void)arg;
    if (led_state) {
        gpio_toggle(led_pin);
    }
}

void debounce_callback(void *arg) {
    (void)arg;
    gpio_irq_enable(button_pin);
    xtimer_remove(&debounce_timer);
}

void button_press_handler(void *arg) {
    (void)arg;
    press_start_time = xtimer_now_usec();
}

void button_handler(void *arg) {
    (void)arg;
    if (!is_button_pressed) {
        press_start_time = xtimer_now_usec();
        is_button_pressed = true;
    } else {
        uint32_t press_end_time = xtimer_now_usec();
        if (press_end_time < press_start_time) {
            press_duration = UINT32_MAX - press_start_time + press_end_time;
        } else {
            press_duration = press_end_time - press_start_time;
        } 
        if (press_duration >= LONG_PRESS){
            blink_interval_state = true;
        }else{
            blink_interval_state = false;
        }
        led_fire  = (led_fire  + 1) % 2;
        is_button_pressed = false;
        gpio_irq_disable(button_pin);
        xtimer_set(&debounce_timer, DEBOUNCE_DELAY);
    }
}

int main(void) {
    if (gpio_init(led_pin, GPIO_OUT) < 0) {
        printf("Ошибка инициализации светодиода\n");
        return 1;
    }
    ztimer_t led_timer = { .callback = toggle_led };
    if (gpio_init_int(button_pin, GPIO_IN_PU, GPIO_BOTH, button_handler, NULL) < 0) {
        printf("Ошибка инициализации кнопки\n");
        return 1;
    }
    debounce_timer.callback = debounce_callback;
    while (1) {
        if (led_fire){
            if (blink_interval_state == false){
                led_state = !led_state;
                ztimer_set(ZTIMER_MSEC, &led_timer, 100);
                ztimer_sleep(ZTIMER_MSEC, 100);
            }else{
                led_state = !led_state;
                ztimer_set(ZTIMER_MSEC, &led_timer, 500);
                ztimer_sleep(ZTIMER_MSEC, 500);
            }
        }else{
            gpio_set(led_pin);
        }    
    }
    return 0;
}
