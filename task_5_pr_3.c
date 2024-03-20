#include <stdio.h>
#include "periph/gpio.h"
#include "periph/adc.h"
#include "xtimer.h"

#define PWM_PIN GPIO_PIN(PORT_A, 8)    // Замените на нужный GPIO-пин
#define ADC_PIN ADC_LINE(0)            // АЦП линия для фоторезистора
#define PWM_FREQUENCY 1000             // Частота ШИМ в Гц
#define PWM_RESOLUTION 255             // Разрешение ШИМ (8 бит)
#define PWM_PERIOD_US (1000000 / PWM_FREQUENCY) // Период ШИМ в микросекундах
#define MIN_DUTY_CYCLE 100             // Минимальный duty cycle в микросекундах
#define ADC_RESOLUTION ADC_RES_12BIT   // Разрешение АЦП
#define FILTER_SIZE 10                 // Размер фильтра для сглаживания

static xtimer_t timer_on;
static xtimer_t timer_off;
static xtimer_t adc_timer;
static uint32_t on_time;
static uint32_t off_time;
static uint16_t adc_readings[FILTER_SIZE];
static uint8_t filter_index = 0;

static void timer_on_callback(void *arg);
static void timer_off_callback(void *arg);
static void adc_timer_callback(void *arg);
static uint16_t read_adc(void);
static uint16_t get_filtered_adc_value(void);

static void timer_on_callback(void *arg) {
    gpio_t *pin = (gpio_t *)arg;
    gpio_set(*pin); // Включить светодиод

    off_time = PWM_PERIOD_US - on_time;
    timer_off.callback = timer_off_callback;
    timer_off.arg = pin;
    xtimer_set(&timer_off, off_time); // Запустить таймер для выключения светодиода
}

static void timer_off_callback(void *arg) {
    gpio_t *pin = (gpio_t *)arg;
    gpio_clear(*pin); // Выключить светодиод

    timer_on.callback = timer_on_callback;
    timer_on.arg = pin;
    xtimer_set(&timer_on, on_time); // Запустить таймер для включения светодиода
}

static void adc_timer_callback(void *arg) {
    (void)arg;
    adc_readings[filter_index] = read_adc();
    filter_index = (filter_index + 1) % FILTER_SIZE;

    uint16_t filtered_adc_value = get_filtered_adc_value();

    // Расчет on_time с использованием отфильтрованного значения
    on_time = MIN_DUTY_CYCLE + ((PWM_PERIOD_US - MIN_DUTY_CYCLE) * filtered_adc_value) / 1000;

        // Проверяем, включен ли светодиод
    bool is_led_on = gpio_read(PWM_PIN);

    // Если светодиод уже включен, обновляем таймеры
       // Если светодиод уже включен, обновляем таймеры
    if (is_led_on) {
        xtimer_remove(&timer_off); // Отменяем таймер выключения
        off_time = PWM_PERIOD_US - on_time;
        xtimer_set(&timer_off, off_time); // Запускаем таймер выключения с новым временем
    } else {
        xtimer_remove(&timer_on); // Отменяем таймер включения
        xtimer_set(&timer_on, on_time); // Запускаем таймер включения с новым временем
    }

    // Перезапустить таймер АЦП
    xtimer_set(&adc_timer, PWM_PERIOD_US);
    
    // Перезапустить таймер АЦП
   // xtimer_set(&adc_timer, PWM_PERIOD_US);

 

}

static uint16_t read_adc(void) {
    return adc_sample(ADC_PIN, ADC_RESOLUTION);
}


static uint16_t get_filtered_adc_value(void) {
    uint32_t sum = 0;
    for (int i = 0; i < FILTER_SIZE; i++) {
        sum += adc_readings[i];
    }
    return (uint16_t)(sum / FILTER_SIZE);
}

int main(void) {
    gpio_t led_pin = PWM_PIN;
    gpio_init(led_pin, GPIO_OUT);

    // Инициализация ADC
    if (adc_init(ADC_PIN) < 0) {
        printf("Ошибка инициализации ADC\n");
        gpio_clear(led_pin);
        return 1;
    }

    on_time = MIN_DUTY_CYCLE;

    timer_on.callback = timer_on_callback;
    timer_on.arg = &led_pin;
    xtimer_set(&timer_on, on_time); // Запустить процесс мигания

   adc_timer.callback = adc_timer_callback;
   xtimer_set(&adc_timer, PWM_PERIOD_US); // Запустить таймер для чтения АЦП

    while (1) {
        // Основной цикл может выполнять другие задачи
    }

    return 0;
}
