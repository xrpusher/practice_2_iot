#include "ztimer.h"
#include "periph/gpio.h"
#include "xtimer.h"

#define DEBOUNCE_DELAY 300000
#define NUM_BUTTONS 4
#define CODE_LENGTH 4
#define LONG_PRESS 3000000 

static gpio_t led_pin = GPIO_PIN(PORT_C, 13);
static gpio_t button_pins[NUM_BUTTONS] = {GPIO_PIN(PORT_B, 5), GPIO_PIN(PORT_B, 1), GPIO_PIN(PORT_B, 15), GPIO_PIN(PORT_B, 12)};
static uint32_t correct_code[CODE_LENGTH] = {0, 1, 2, 3}; // Пример правильного кода
static uint32_t entered_code[CODE_LENGTH];
static int code_index = 0;
static bool led_state = false;
static xtimer_t debounce_timers[NUM_BUTTONS];

void toggle_led(void) {
    led_state = !led_state;
    if (led_state) {
        gpio_set(led_pin);
    } else {
        gpio_clear(led_pin);
    }
}

void debounce_callback(void *arg) {
    int button_index = (int)arg;
    gpio_irq_enable(button_pins[button_index]);
}

void button_handler(void *arg) {
    int button_index = (int)arg;
    entered_code[code_index++] = button_index;
    if (code_index == CODE_LENGTH) {
        code_index = 0;
        bool is_correct = true;
        for (int i = 0; i < CODE_LENGTH; i++) {
            if (entered_code[i] != correct_code[i]) {
                is_correct = false;
                printf("Неверный код\n");
                break;
            }
        }
        if (is_correct) {
            printf("Код верный\n");
            toggle_led();
        }
    }
    gpio_irq_disable(button_pins[button_index]);
    xtimer_set(&debounce_timers[button_index], DEBOUNCE_DELAY);
}

int main(void) {
    if (gpio_init(led_pin, GPIO_OUT) < 0) {
        printf("Ошибка инициализации светодиода\n");
        return 1;
    }

    for (int i = 0; i < NUM_BUTTONS; i++) {
        if (gpio_init_int(button_pins[i], GPIO_IN_PU, GPIO_FALLING, button_handler, (void *)i) < 0) {
            printf("Ошибка инициализации кнопки %d\n", i);
            return 1;
        }
        debounce_timers[i].callback = debounce_callback;
        debounce_timers[i].arg = (void *)i;
    }

    while (1) {
        ztimer_sleep(ZTIMER_MSEC, 1000);
    }
    return 0;
}
