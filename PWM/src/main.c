#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <driver/gpio.h>
#include <driver/adc.h>
#include<driver/ledc.h>

adc1_channel_t adc_pot = ADC1_CHANNEL_6;

ledc_channel_config_t pwm;

#define PIN_LED 2 


void init_hw(void);

void app_main() {

    init_main();
    while (1)
    {
        int pot = adc1_get_raw(adc_pot);
        printf("ADC: %d\n", pot);
        ledc_set_duty(pwm.speed_mode, pwm.channel, pot);
        ledc_update_duty(pwm.speed_mode, pwm.channel);


        vTaskDelay(300/ portTICK_PERIOD_MS);
    }
    

}

void init_hw(void){
adc1_config_width(ADC_WIDTH_BIT_11);
adc1_config_channel_atten(adc_pot, ADC_ATTEN_DB_11);

ledc_timer_config_t timer = {

.duty_resolution = LEDC_TIMER_12_BIT,
.freq_hz = 1000,
.speed_mode = LEDC_HIGH_SPEED_MODE,
.timer_num = LEDC_TIMER_0,
.clk_cfg = LEDC_AUTO_CLK,


};
ledc_timer_config(&timer);

pwm.channel = LEDC_CHANNEL_0;
pwm.duty = 0;
pwm.gpio_num = PIN_LED;
pwm.hpoint = 0; 
pwm.timer_sel = LEDC_TIMER_0;
pwm.speed_mode = LEDC_HIGH_SPEED_MODE;

ledc_channel_config(&pwm);
}

