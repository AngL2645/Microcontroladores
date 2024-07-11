// Librerias 
#include <stdio.h> 
#include <stdlib.h>
#include <freertos/FreeRTOS.h> 
#include <freertos/task.h>
#include <driver/gpio.h>     
#include <freertos/timers.h> 
#include "esp_log.h"         

/* ******************************************************************************************************************************************** */
// Variables
static const char *tag = "Main";

#define TRUE 1
#define FALSE 0
#define ESTADO_INIT 0
#define ESTADO_ABRIENDO 1
#define ESTADO_CERRANDO 2
#define ESTADO_CERRADO 3
#define ESTADO_ABIERTO 4
#define ESTADO_EMERGENCIA 5
#define ESTADO_ERROR 6
#define ESTADO_ESPERA 7

volatile int CONTADOR = 0;
volatile int ESTADO_ACTUAL = ESTADO_INIT;
volatile int ESTADO_SIGUIENTE = ESTADO_INIT;
volatile int ESTADO_ANTERIOR = ESTADO_INIT;
volatile unsigned int TimeCa = 0;

volatile struct INPUTS
{
    unsigned int LSA : 1;
    unsigned int LSC : 1;
    unsigned int CA : 1;
    unsigned int CC : 1;
    unsigned int FC : 1;
} inputs;
volatile struct OUTPUTS
{
    unsigned int MC : 1;
    unsigned int MA : 1;
    unsigned int LED_EMERGENCIA : 1;
    unsigned int LED_MOVIMIENTO : 1;
} outputs;

TimerHandle_t xTimers;
int timerID = 1;
int INTERVALO = 50;
/* ******************************************************************************************************************************************** */
//FUNCIONES
esp_err_t Escaneo(void); // Funcion de lectura de los pines cada interrupcion
esp_err_t SET_TIMER(void);
void pines(void);
int contador();
int Func_ESTADO_INIT();
int Func_ESTADO_ABRIENDO();
int Func_ESTADO_CERRANDO();
int Func_ESTADO_CERRADO();
int Func_ESTADO_ABIERTO();
int Func_ESTADO_EMERGENCIA();
int Func_ESTADO_ERROR();
int Func_ESTADO_ESPERA();
/* ******************************************************************************************************************************************** */
void vTimerCallback(TimerHandle_t pxTimer)
{
    contador();
    Escaneo();
    ESP_LOGE(tag, "INTERRUPCION COMPLETADA.");
}
/* ******************************************************************************************************************************************** */
void app_main()
{
    pines();
    ESTADO_SIGUIENTE = Func_ESTADO_INIT();
    SET_TIMER();

    while(1)
    {
        if (ESTADO_SIGUIENTE == ESTADO_INIT)
        {
            ESTADO_SIGUIENTE = Func_ESTADO_INIT();
        }
        if (ESTADO_SIGUIENTE == ESTADO_ESPERA)
        {
            ESTADO_SIGUIENTE = Func_ESTADO_ESPERA();
        }
        if (ESTADO_SIGUIENTE == ESTADO_ABRIENDO)
        {
            ESTADO_SIGUIENTE = Func_ESTADO_ABRIENDO();
        }
        if (ESTADO_SIGUIENTE == ESTADO_CERRANDO)
        {
            ESTADO_SIGUIENTE = Func_ESTADO_CERRANDO();
        }
        if (ESTADO_SIGUIENTE == ESTADO_CERRADO)
        {
            ESTADO_SIGUIENTE = Func_ESTADO_CERRADO();
        }
        if (ESTADO_SIGUIENTE == ESTADO_ABIERTO)
        {
            ESTADO_SIGUIENTE = Func_ESTADO_ABIERTO();
        }
        if (ESTADO_SIGUIENTE == ESTADO_EMERGENCIA)
        {
            ESTADO_SIGUIENTE = Func_ESTADO_EMERGENCIA();
        }
        if (ESTADO_SIGUIENTE == ESTADO_ERROR)
        {
            ESTADO_SIGUIENTE = Func_ESTADO_ERROR();
        }
    }
}
/* ******************************************************************************************************************************************** */
int Func_ESTADO_INIT()
{

    ESP_LOGE(tag, "INICIANDO PROGRAMA");

    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_INIT;

    return ESTADO_ESPERA;
}
/* ******************************************************************************************************************************************** */
int Func_ESTADO_ABRIENDO()
{

    ESP_LOGE(tag,"ABRIENDO EL PUERTON");

    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_ABRIENDO;


    outputs.LED_MOVIMIENTO = TRUE;
    outputs.LED_EMERGENCIA = FALSE;
    outputs.MA = TRUE;
    outputs.MC = FALSE;

    if (inputs.LSA == TRUE)
    {

        return ESTADO_ABIERTO; 
    }
    if (inputs.LSA == TRUE && inputs.LSC == TRUE)
    {
        return ESTADO_ERROR;
    }
    if (inputs.FC == TRUE)
    {
        return ESTADO_EMERGENCIA;
    }
    if (inputs.CC == TRUE)
    {

        return ESTADO_CERRANDO;
    }
    if (CONTADOR == 3600)
    {
        return ESTADO_ERROR;
    }
}
/* ******************************************************************************************************************************************** */
int Func_ESTADO_CERRANDO()
{

    ESP_LOGE(tag,"CERRANDO EL PUERTON");


    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_CERRANDO;
    outputs.LED_MOVIMIENTO = TRUE;
    outputs.LED_EMERGENCIA = FALSE;
    outputs.MA = FALSE;
    outputs.MC = TRUE;

    
        if (inputs.LSC == TRUE)
        {
            return ESTADO_CERRADO;
        }

        if (inputs.LSA == TRUE && inputs.LSC == TRUE)
        {
            return ESTADO_ERROR;
        }

        if (inputs.FC == TRUE)
        {
            return ESTADO_EMERGENCIA;
        }
        if (inputs.CA == TRUE)
        {
            return ESTADO_ABRIENDO;
        }
        if (CONTADOR == 3600)
        {
            return ESTADO_ERROR;
        }

    
}
/* ******************************************************************************************************************************************** */
int Func_ESTADO_CERRADO()
{

    ESP_LOGE(tag,"PUERTON CERRADO ");
    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_CERRADO;

    outputs.LED_MOVIMIENTO = FALSE;
    outputs.LED_EMERGENCIA = FALSE;
    outputs.MA = FALSE;
    outputs.MC = FALSE;

   
        return ESTADO_ESPERA;
    
}
/* ******************************************************************************************************************************************** */
int Func_ESTADO_ABIERTO()
{

    ESP_LOGE(tag,"PUERTON ABIERTO ");
    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_ABIERTO;

    outputs.LED_MOVIMIENTO = FALSE;
    outputs.LED_EMERGENCIA = FALSE;
    outputs.MA = FALSE;
    outputs.MC = FALSE;

    
    
        return ESTADO_ESPERA;
    
}
/* ******************************************************************************************************************************************** */
int Func_ESTADO_EMERGENCIA()
{

    ESP_LOGE(tag,"EMERGENCIA!!!!!!!");

    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_EMERGENCIA;

    outputs.LED_MOVIMIENTO = FALSE;
    outputs.LED_EMERGENCIA = TRUE;
    outputs.MA = FALSE;
    outputs.MC = FALSE;

    
        vTaskDelay(1500 / portTICK_PERIOD_MS);
        if (inputs.FC == FALSE)
        {
            return ESTADO_ANTERIOR;
        }
    
}
/* ******************************************************************************************************************************************** */
int Func_ESTADO_ERROR()
{

    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_ERROR;

    outputs.LED_MOVIMIENTO = FALSE;
    outputs.LED_EMERGENCIA = TRUE;
    outputs.MA = FALSE;
    outputs.MC = FALSE;

    ESP_LOGE(tag,"\nERROR!!!!");
    ESP_LOGE(tag,"\nERROR!!!!");
    ESP_LOGE(tag,"\nERROR!!!!");

    
        vTaskDelay(500 / portTICK_PERIOD_MS);
        outputs.LED_EMERGENCIA = FALSE;
        vTaskDelay(500 / portTICK_PERIOD_MS);
        outputs.LED_EMERGENCIA = TRUE;
        vTaskDelay(500 / portTICK_PERIOD_MS);
        outputs.LED_EMERGENCIA = FALSE;

        return ESTADO_ESPERA;
    
}
/* ******************************************************************************************************************************************** */
int Func_ESTADO_ESPERA()
{

    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_ESPERA;


    outputs.LED_EMERGENCIA = FALSE;
    outputs.LED_MOVIMIENTO = FALSE;
    outputs.MA = FALSE;
    outputs.MC = FALSE;
    ESP_LOGE(tag, "ESTADO ESPERA");

    

        if (inputs.LSA == FALSE && inputs.FC == FALSE && inputs.LSC == FALSE) 
                                                                              
        {
            return ESTADO_CERRANDO;
        }
        if (inputs.CA == TRUE && inputs.FC == FALSE && inputs.LSA == FALSE && inputs.FC == FALSE)
        {
            return ESTADO_ABRIENDO;
        }
        if (inputs.CC == TRUE && inputs.FC == FALSE)
        {
            return ESTADO_CERRANDO;
        }
        if (inputs.CA == TRUE && inputs.FC == FALSE)
        {
            return ESTADO_ABRIENDO;
        }

        if (inputs.FC == TRUE)
        {
            return ESTADO_EMERGENCIA;
        }
        if (inputs.LSA == TRUE && inputs.LSC == TRUE)
        {
            return ESTADO_ERROR;
        }
    
}
/* ******************************************************************************************************************************************** */
esp_err_t SET_TIMER(void)
{

    ESP_LOGE(tag,"Inicializando configuracion del timer...");

    xTimers = xTimerCreate("Timer",
                           (pdMS_TO_TICKS(INTERVALO)),
                           pdTRUE,
                           (void *)timerID,
                           vTimerCallback);

    if (xTimers == NULL)
    {
        ESP_LOGE(tag,"El timer no fue creado");
    }
    else
    {
        if (xTimerStart(xTimers, 0) != pdPASS)
        {
            ESP_LOGE(tag,"El timer podria no ser seteado en el estado activo");
        }
    }

    return ESP_OK;
}
/* ******************************************************************************************************************************************** */
esp_err_t Escaneo(void)
{


    inputs.LSC = (gpio_get_level(13) == TRUE) ? TRUE : FALSE;

    inputs.LSA = (gpio_get_level(12) == TRUE) ? TRUE : FALSE;

    inputs.FC = (gpio_get_level(14) == TRUE) ? TRUE : FALSE;

    inputs.CC = (gpio_get_level(27) == TRUE) ? TRUE : FALSE;

    inputs.CA = (gpio_get_level(26) == TRUE) ? TRUE : FALSE;


    if (outputs.LED_MOVIMIENTO == TRUE)
    {
        gpio_set_level(4, TRUE);
    }
    else
    {
        gpio_set_level(4, FALSE);
    }

    if (outputs.LED_EMERGENCIA == TRUE)
    {
        gpio_set_level(16, TRUE);
    }
    else
    {
        gpio_set_level(16, FALSE);
    }


    if (outputs.MC == TRUE)
    {
        gpio_set_level(17, TRUE);
    }
    else
    {
        gpio_set_level(17, FALSE);
    }

    if (outputs.MA == TRUE)
    {
        gpio_set_level(5, TRUE);
    }
    else
    {
        gpio_set_level(5, FALSE);
    }

    return ESP_OK;
}
/* ******************************************************************************************************************************************** */
void pines(void)
{
/* ******************************************************************************************************************************************** */
    ESP_LOGE(tag, "CONFIGURACION DE LOS PINES");

    gpio_config_t IO_CONFIG;

    IO_CONFIG.mode = GPIO_MODE_INPUT;
    IO_CONFIG.pin_bit_mask = (1 << 13) | (1 << 12) | (1 << 14) | (1 << 27) | (1 << 26);
    IO_CONFIG.pull_down_en = GPIO_PULLDOWN_ENABLE;
    IO_CONFIG.pull_up_en = GPIO_PULLUP_DISABLE;
    IO_CONFIG.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&IO_CONFIG);

    IO_CONFIG.mode = GPIO_MODE_OUTPUT;
    IO_CONFIG.pin_bit_mask = (1 << 4) | (1 << 16) | (1 << 17) | (1 << 5);
    IO_CONFIG.pull_down_en = GPIO_PULLDOWN_DISABLE;
    IO_CONFIG.pull_up_en = GPIO_PULLUP_DISABLE;
    IO_CONFIG.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&IO_CONFIG);
}
/* ******************************************************************************************************************************************** */
int contador()
{

    if (ESTADO_ACTUAL == ESTADO_CERRANDO || ESTADO_ACTUAL == ESTADO_ABRIENDO)
    {
        if (inputs.CA = TRUE || inputs.CA == TRUE)
        {

            CONTADOR = 0;
        }
        CONTADOR++;
    }
    else
    {

        CONTADOR = 0;
    }
}
/* ******************************************************************************************************************************************** */