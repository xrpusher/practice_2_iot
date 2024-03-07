#include "ztimer.h"
#include "periph/gpio.h"
#include "xtimer.h"

#define DEBOUNCE_DELAY 300000   

int myInt = 0; // Инициализация переменной
int expected = 0b000000000000;

#define MASK(n) (~(7 << (3 * (n)))) // Макрос для создания маски (7 = 111 в двоичной системе)

void setBits(int *value, int position, unsigned int bits) {
    if (bits > 7) {
        printf("Ошибка: значение битов должно быть меньше 8 (3 бита).\n");
        return;
    }
    *value = (*value & MASK(position)) | (bits << (3 * position));
}


static gpio_t led_pin = GPIO_PIN(PORT_C, 13);
static gpio_t button_pin = GPIO_PIN(PORT_B, 11);
static gpio_t button_pin_2 = GPIO_PIN(PORT_B, 5);
static gpio_t button_pin_3 = GPIO_PIN(PORT_B, 12);
static gpio_t button_pin_4 = GPIO_PIN(PORT_B, 1);

static xtimer_t debounce_timer;
static xtimer_t debounce_timer_2;
static xtimer_t debounce_timer_3;
static xtimer_t debounce_timer_4;
static uint32_t press_start_time;


int a = 3;


uint32_t blink_interval;
bool blink_interval_state = false;

void toggle_led(void *arg) {
    (void)arg;

        gpio_toggle(led_pin);

}

void debounce_callback(void *arg) {
    (void)arg;
    gpio_irq_enable(button_pin);
    xtimer_remove(&debounce_timer);
}

void debounce_callback_2(void *arg) {
    (void)arg;
    gpio_irq_enable(button_pin_2);
    xtimer_remove(&debounce_timer_2);
}

void debounce_callback_3(void *arg) {
    (void)arg;
    gpio_irq_enable(button_pin_3);
    xtimer_remove(&debounce_timer_3);
}

void debounce_callback_4(void *arg) {
    (void)arg;
    gpio_irq_enable(button_pin_4);
    xtimer_remove(&debounce_timer_4);
}
void button_press_handler(void *arg) {
    (void)arg;
    press_start_time = xtimer_now_usec();
}

void button_handler(void *arg) {
    (void)arg;
    gpio_irq_disable(button_pin);
    a = (a + 1) % 4;
    setBits(&myInt, a, 0);

    if (myInt == expected){
        gpio_clear(led_pin);
    }else{
        gpio_set(led_pin);
        if (a == 3){
            setBits(&myInt, 0, 5); // Устанавливаем 001 в позиции 0
            setBits(&myInt, 1, 5); // Устанавливаем 100 в позиции 1
            setBits(&myInt, 2, 5); // Устанавливаем 101 в позиции 2
            setBits(&myInt, 3, 5); // Устанавливаем 101 в позиции 2     
        }
    }

    xtimer_set(&debounce_timer, DEBOUNCE_DELAY);

}

void button_handler_2(void *arg) {
    (void)arg;
    a = (a + 1) % 4;
    setBits(&myInt, a, 1);

    if (myInt == expected){
        gpio_clear(led_pin);
    }else{
        gpio_set(led_pin);
        if (a == 3){
            setBits(&myInt, 0, 5); // Устанавливаем 001 в позиции 0
            setBits(&myInt, 1, 5); // Устанавливаем 100 в позиции 1
            setBits(&myInt, 2, 5); // Устанавливаем 101 в позиции 2
            setBits(&myInt, 3, 5); // Устанавливаем 101 в позиции 2     
        }
    }
    gpio_irq_disable(button_pin_2);
    xtimer_set(&debounce_timer_2, DEBOUNCE_DELAY);
}

void button_handler_3(void *arg) {
    (void)arg;
    a = (a + 1) % 4;
    setBits(&myInt, a, 2);
    if (myInt == expected){
        gpio_clear(led_pin);
    }else{
        gpio_set(led_pin);
        if (a == 3){
            setBits(&myInt, 0, 5); // Устанавливаем 001 в позиции 0
            setBits(&myInt, 1, 5); // Устанавливаем 100 в позиции 1
            setBits(&myInt, 2, 5); // Устанавливаем 101 в позиции 2
            setBits(&myInt, 3, 5); // Устанавливаем 101 в позиции 2     
        }
    }
    gpio_irq_disable(button_pin_3);
   // gpio_clear(led_pin);
    xtimer_set(&debounce_timer_3, DEBOUNCE_DELAY);
}

void button_handler_4(void *arg) {
    (void)arg;
    a = (a + 1) % 4;
    setBits(&myInt, a, 3);


    if (myInt == expected){
        gpio_clear(led_pin);
    }else {
        gpio_set(led_pin);
        if (a == 3){
            setBits(&myInt, 0, 5); // Устанавливаем 001 в позиции 0
            setBits(&myInt, 1, 5); // Устанавливаем 100 в позиции 1
            setBits(&myInt, 2, 5); // Устанавливаем 101 в позиции 2
            setBits(&myInt, 3, 5); // Устанавливаем 101 в позиции 2     
        }

    }

    gpio_irq_disable(button_pin_4);
    xtimer_set(&debounce_timer_4, DEBOUNCE_DELAY);
}

int main(void) {

    setBits(&myInt, 0, 5); // Устанавливаем 001 в позиции 0
    setBits(&myInt, 1, 5); // Устанавливаем 100 в позиции 1
    setBits(&myInt, 2, 5); // Устанавливаем 101 в позиции 2
    setBits(&myInt, 3, 5); // Устанавливаем 101 в позиции 2

    setBits(&expected, 0, 0); 
    setBits(&expected, 1, 2); 
    setBits(&expected, 2, 2); 
    setBits(&expected, 3, 2); 

    if (gpio_init(led_pin, GPIO_OUT) < 0) {
        printf("Ошибка инициализации светодиода\n");
        return 1;
    }

    if (gpio_init_int(button_pin, GPIO_IN_PU, GPIO_BOTH, button_handler, NULL) < 0) {
        printf("Ошибка инициализации кнопки\n");     
        return 1;
    }
    if (gpio_init_int(button_pin_4, GPIO_IN_PU, GPIO_BOTH, button_handler_4, NULL) < 0) {
        printf("Ошибка инициализации кнопки\n");     
        return 1;
    }
    if (gpio_init_int(button_pin_2, GPIO_IN_PU, GPIO_BOTH, button_handler_2, NULL) < 0) {
        printf("Ошибка инициализации кнопки\n");     
        return 1;
    }
        if (gpio_init_int(button_pin_3, GPIO_IN_PU, GPIO_BOTH, button_handler_3, NULL) < 0) {
        printf("Ошибка инициализации кнопки\n");     
        return 1;
    }

    debounce_timer.callback = debounce_callback;
    debounce_timer_2.callback = debounce_callback_2;
    debounce_timer_3.callback = debounce_callback_3;
    debounce_timer_4.callback = debounce_callback_4;

    while (1) {
  
    }
    return 0;
}
