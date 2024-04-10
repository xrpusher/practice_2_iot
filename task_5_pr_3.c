#include <stdio.h>
#include "periph/gpio.h"
#include "periph/adc.h"
#include "periph/pwm.h"
#include "xtimer.h"

#define LED_PIN GPIO_PIN(PORT_B, 4) // Порт A, пин 8
#define LED_CHANNEL 0 // Канал PWM для светодиода
#define ADC_PIN ADC_LINE(0)            // АЦП линия для фоторезистора
#define PWM_FREQUENCY 1000             // Частота ШИМ в Гц
#define PWM_RESOLUTION 1024            // Разрешение ШИМ (8 бит)
#define PWM_PERIOD_US (1000000 / PWM_FREQUENCY) // Период ШИМ в микросекундах
#define MIN_DUTY_CYCLE 100             // Минимальный duty cycle в микросекундах
#define ADC_RESOLUTION ADC_RES_12BIT   // Разрешение АЦП
#define FILTER_SIZE 30                 // Размер фильтра для сглаживания

static xtimer_t adc_timer;
static uint16_t adc_readings[FILTER_SIZE];
static uint8_t filter_index = 0;
pwm_t pwm_dev;

static void adc_timer_callback(void *arg);
static uint16_t read_adc(void);
static uint16_t get_filtered_adc_value(void);

int mapValue(int x) {
    // Обратный маппинг: значения с АЦП ниже порога приведут к увеличению яркости светодиода.
    
    return (int)((1023.0 / (2659 )) * (2659 - x));
}


static void adc_timer_callback(void *arg) {
    (void)arg;
    adc_readings[filter_index] = read_adc();
    filter_index = (filter_index + 1) % FILTER_SIZE;

    // Получаем фильтрованное значение с АЦП
    uint16_t filtered_adc_value = get_filtered_adc_value();

 
    // Устанавливаем яркость светодиода
    pwm_set(pwm_dev, LED_CHANNEL, mapValue(filtered_adc_value));

    // Перезапускаем таймер АЦП
    xtimer_set(&adc_timer, PWM_PERIOD_US);
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
    // Инициализация PWM с наименьшей частотой и разрешением
    uint32_t freq = 33000; // Частота в Гц
    uint16_t res = PWM_RESOLUTION; // Разрешение
    pwm_dev = PWM_DEV(0); // Используем первое устройство PWM
    pwm_init(pwm_dev, PWM_LEFT, freq, res);

    // Активация работы PWM
    pwm_poweron(pwm_dev);

    // Инициализация ADC
    if (adc_init(ADC_PIN) < 0) {
        printf("Ошибка инициализации ADC\n");
        pwm_set(pwm_dev, LED_CHANNEL, 0); // Устанавливаем минимальную яркость
        return 1;
    }

    adc_timer.callback = adc_timer_callback;
    xtimer_set(&adc_timer, PWM_PERIOD_US); // Запустить таймер для чтения АЦП

    while (1) {
 
    }

    return 0;
}
