#include "ztimer.h"
#include "periph/gpio.h"
#include "xtimer.h"

#define DEBOUNCE_DELAY 300000
#define LONG_PRESS 3000000

static gpio_t green_led = GPIO_PIN(PORT_C, 13);
static gpio_t yellow_led = GPIO_PIN(PORT_C, 14);
static gpio_t red_led = GPIO_PIN(PORT_C, 15);
static gpio_t button_pin = GPIO_PIN(PORT_B, 1);

static xtimer_t debounce_timer;
static uint32_t press_start_time;
static uint32_t press_duration;

static uint32_t green_led_interval = 5000;  // время свечения зеленого света
static uint32_t yellow_led_interval = 2000; // время свечения желтого света
static uint32_t red_led_interval = 5000;    // время свечения красного света
static uint32_t current_phase_time = 0;     // текущее время фазы
static uint32_t next_green_time = 0;        // время до следующего включения зеленого света

enum TrafficLightPhase {
    GREEN,
    YELLOW,
    RED
} current_phase = RED;

void traffic_light_update(void) {
    gpio_clear(green_led);
    gpio_clear(yellow_led);
    gpio_clear(red_led);

    switch (current_phase) {
        case GREEN:
            gpio_set(green_led);
            current_phase_time = green_led_interval;
            current_phase = YELLOW;
            break;
        case YELLOW:
            gpio_set(yellow_led);
            current_phase_time = yellow_led_interval;
            current_phase = RED;
            break;
        case RED:
            gpio_set(red_led);
            current_phase_time = red_led_interval;
            current_phase = GREEN;
            break;
    }

    next_green_time -= current_phase_time;
    if (next_green_time < 0) {
        next_green_time = green_led_interval + yellow_led_interval + red_led_interval;
    }
}

void button_handler(void *arg) {
    (void)arg;
    uint32_t time_to_green = next_green_time - current_phase_time;
    printf("Время до зеленого света: %lu мс\n", time_to_green);
    if (time_to_green > LONG_PRESS && current_phase != GREEN) {
        printf("Планирование нового переключения на зеленый\n");
        traffic_light_update();  // немедленное переключение светофора
    }
}

void debounce_callback(void *arg) {
    (void)arg;
    gpio_irq_enable(button_pin);
    xtimer_remove(&debounce_timer);
}

int main(void) {
    if (gpio_init(green_led, GPIO_OUT) < 0 || gpio_init(yellow_led, GPIO_OUT) < 0 || gpio_init(red_led, GPIO_OUT) < 0) {
        printf("Ошибка инициализации светодиодов\n");
        return 1;
    }
    traffic_light_update();

    if (gpio_init_int(button_pin, GPIO_IN_PU, GPIO_FALLING, button_handler, NULL) < 0) {
        printf("Ошибка инициализации кнопки\n");
        return 1;
    }
    debounce_timer.callback = debounce_callback;

    while (1) {
        ztimer_sleep(ZTIMER_MSEC, current_phase_time);
        traffic_light_update();
    }

    return 0;
}
