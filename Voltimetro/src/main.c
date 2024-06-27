#include <stdio.h>
#include <math.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <driver/adc.h>
#include <driver/ledc.h>
adc1_channel_t adc_pot = ADC1_CHANNEL_6;
#define LED 2
float Obj= 18;
float voltajeRMS;
float Vmedido;
float voltajes[100];
void init_hw(void);
int i = 0;
void app_main()
{

    init_hw();
while (1)
{
    for (i = 0; i <= 99; i++)
    {

        int pot = adc1_get_raw(adc_pot);
        Vmedido = (pot * Obj / 4095);
        voltajes[i] = Vmedido;

    //    printf("Voltaje%d: %fV\n", i, (voltajes[i]));

        vTaskDelay(1 / portTICK_PERIOD_MS);
    }

    for (size_t j = 1; j < 101; j++)
    {
        voltajes[j] = pow(voltajes[j],2);
        voltajes[j] = (voltajes[j] + voltajes[j - 1]);
    //  printf("%d\n", j);
    }
    float vtemp = (voltajes[100] / 100);
    vtemp = sqrt(vtemp);
    printf("Voltaje: %4.2fV\n", vtemp);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

}}

void init_hw(void)
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(adc_pot, ADC_ATTEN_DB_12);

    gpio_config_t io_config;
    io_config.mode = GPIO_MODE_OUTPUT;
    io_config.pin_bit_mask = (1 << LED);
    io_config.pull_down_en = GPIO_PULLDOWN_ONLY;
    io_config.pull_up_en = GPIO_PULLUP_DISABLE;
    io_config.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_config);
}
