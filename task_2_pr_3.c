#include <stdio.h>

#include "dht.h"
#include "dht_params.h"
#include "fmt.h"

int main(void)
{
    dht_params_t my_params;
    my_params.pin = GPIO_PIN(PORT_A, 0);

    my_params.in_mode = DHT_PARAM_PULL;

    dht_t dev;
    if (dht_init(&dev, &my_params) == DHT_OK) {
        printf("DHT sensor connected\n");
    }
    else {
        printf("Failed to connect to DHT sensor\n");
        return 1;
    }
    int16_t temp, hum;
    if (dht_read(&dev, &temp, &hum) != DHT_OK) {
        printf("Error reading values\n");
    }

    char temp_s[10];
    size_t n = fmt_s16_dfp(temp_s, temp, -1);
    temp_s[n] = '\0';

    char hum_s[10];
    n = fmt_s16_dfp(hum_s, hum, -1);
    hum_s[n] = '\0';
    
    printf("DHT values - temp: %s°C - relative humidity: %s%%\n",
       temp_s, hum_s);

    return 0;
}
