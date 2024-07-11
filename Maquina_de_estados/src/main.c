// Librerías
#include <stdio.h> 
#include <stdlib.h>
#include <freertos/FreeRTOS.h> 
#include <freertos/task.h>
#include <driver/gpio.h>     
#include <freertos/timers.h> 
#include "esp_log.h"         

/* ******************************************************************************************************************************************** */
// Variables
static const char *tag = "Main"; // Etiqueta para los mensajes de log

#define TRUE 1
#define FALSE 0

// Definición de estados del sistema
typedef enum {
    ESTADO_INIT,
    ESTADO_ABRIENDO,
    ESTADO_CERRANDO,
    ESTADO_CERRADO,
    ESTADO_ABIERTO,
    ESTADO_EMERGENCIA,
    ESTADO_ERROR,
    ESTADO_ESPERA
} ESTADO;

// Variables globales volátiles
volatile int CONTADOR = 0;
volatile ESTADO ESTADO_ACTUAL = ESTADO_INIT;
volatile ESTADO ESTADO_SIGUIENTE = ESTADO_INIT;
volatile ESTADO ESTADO_ANTERIOR = ESTADO_INIT;
volatile unsigned int TimeCa = 0;

// Estructuras de entradas y salidas
volatile struct INPUTS {
    unsigned int LSA : 1;
    unsigned int LSC : 1;
    unsigned int CA : 1;
    unsigned int CC : 1;
    unsigned int FC : 1;
} inputs;

volatile struct OUTPUTS {
    unsigned int MC : 1;
    unsigned int MA : 1;
    unsigned int LED_EMERGENCIA : 1;
    unsigned int LED_MOVIMIENTO : 1;
} outputs;

TimerHandle_t xTimers; // Manejador del timer
int timerID = 1;
int INTERVALO = 50; // Intervalo del timer en milisegundos

/* ******************************************************************************************************************************************** */
// Prototipos de funciones
esp_err_t Escaneo(void); // Función de lectura de los pines en cada interrupción
esp_err_t SET_TIMER(void);
void pines(void);
void contador(void);
ESTADO Func_ESTADO_INIT(void);
ESTADO Func_ESTADO_ABRIENDO(void);
ESTADO Func_ESTADO_CERRANDO(void);
ESTADO Func_ESTADO_CERRADO(void);
ESTADO Func_ESTADO_ABIERTO(void);
ESTADO Func_ESTADO_EMERGENCIA(void);
ESTADO Func_ESTADO_ERROR(void);
ESTADO Func_ESTADO_ESPERA(void);

/* ******************************************************************************************************************************************** */
// Función de callback del timer
void vTimerCallback(TimerHandle_t pxTimer) {
    contador();
    Escaneo();
    ESP_LOGE(tag, "INTERRUPCIÓN COMPLETADA.");
}

/* ******************************************************************************************************************************************** */
// Función principal
void app_main() {
    pines(); // Configuración de los pines
    ESTADO_SIGUIENTE = Func_ESTADO_INIT(); // Inicialización del estado
    SET_TIMER(); // Configuración del timer

    // Bucle principal
    while(1) {
        switch (ESTADO_SIGUIENTE) {
            case ESTADO_INIT:
                ESTADO_SIGUIENTE = Func_ESTADO_INIT();
                break;
            case ESTADO_ESPERA:
                ESTADO_SIGUIENTE = Func_ESTADO_ESPERA();
                break;
            case ESTADO_ABRIENDO:
                ESTADO_SIGUIENTE = Func_ESTADO_ABRIENDO();
                break;
            case ESTADO_CERRANDO:
                ESTADO_SIGUIENTE = Func_ESTADO_CERRANDO();
                break;
            case ESTADO_CERRADO:
                ESTADO_SIGUIENTE = Func_ESTADO_CERRADO();
                break;
            case ESTADO_ABIERTO:
                ESTADO_SIGUIENTE = Func_ESTADO_ABIERTO();
                break;
            case ESTADO_EMERGENCIA:
                ESTADO_SIGUIENTE = Func_ESTADO_EMERGENCIA();
                break;
            case ESTADO_ERROR:
                ESTADO_SIGUIENTE = Func_ESTADO_ERROR();
                break;
            default:
                ESP_LOGE(tag, "Estado desconocido");
                break;
        }
    }
}

/* ******************************************************************************************************************************************** */
// Función de inicialización del estado
ESTADO Func_ESTADO_INIT(void) {
    ESP_LOGE(tag, "INICIANDO PROGRAMA");

    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_INIT;

    return ESTADO_ESPERA;
}

/* ******************************************************************************************************************************************** */
// Función del estado "ABRIENDO"
ESTADO Func_ESTADO_ABRIENDO(void) {
    ESP_LOGE(tag, "ABRIENDO EL PUERTÓN");

    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_ABRIENDO;

    outputs.LED_MOVIMIENTO = TRUE;
    outputs.LED_EMERGENCIA = FALSE;
    outputs.MA = TRUE;
    outputs.MC = FALSE;

    if (inputs.LSA == TRUE) {
        return ESTADO_ABIERTO; 
    }
    if (inputs.LSA == TRUE && inputs.LSC == TRUE) {
        return ESTADO_ERROR;
    }
    if (inputs.FC == TRUE) {
        return ESTADO_EMERGENCIA;
    }
    if (inputs.CC == TRUE) {
        return ESTADO_CERRANDO;
    }
    if (CONTADOR == 3600) {
        return ESTADO_ERROR;
    }

    return ESTADO_ABRIENDO; // Mantener el estado si ninguna condición se cumple
}

/* ******************************************************************************************************************************************** */
// Función del estado "CERRANDO"
ESTADO Func_ESTADO_CERRANDO(void) {
    ESP_LOGE(tag, "CERRANDO EL PUERTÓN");

    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_CERRANDO;

    outputs.LED_MOVIMIENTO = TRUE;
    outputs.LED_EMERGENCIA = FALSE;
    outputs.MA = FALSE;
    outputs.MC = TRUE;

    if (inputs.LSC == TRUE) {
        return ESTADO_CERRADO;
    }
    if (inputs.LSA == TRUE && inputs.LSC == TRUE) {
        return ESTADO_ERROR;
    }
    if (inputs.FC == TRUE) {
        return ESTADO_EMERGENCIA;
    }
    if (inputs.CA == TRUE) {
        return ESTADO_ABRIENDO;
    }
    if (CONTADOR == 3600) {
        return ESTADO_ERROR;
    }

    return ESTADO_CERRANDO; // Mantener el estado si ninguna condición se cumple
}

/* ******************************************************************************************************************************************** */
// Función del estado "CERRADO"
ESTADO Func_ESTADO_CERRADO(void) {
    ESP_LOGE(tag, "PUERTÓN CERRADO");

    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_CERRADO;

    outputs.LED_MOVIMIENTO = FALSE;
    outputs.LED_EMERGENCIA = FALSE;
    outputs.MA = FALSE;
    outputs.MC = FALSE;

    return ESTADO_ESPERA;
}

/* ******************************************************************************************************************************************** */
// Función del estado "ABIERTO"
ESTADO Func_ESTADO_ABIERTO(void) {
    ESP_LOGE(tag, "PUERTÓN ABIERTO");

    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_ABIERTO;

    outputs.LED_MOVIMIENTO = FALSE;
    outputs.LED_EMERGENCIA = FALSE;
    outputs.MA = FALSE;
    outputs.MC = FALSE;

    return ESTADO_ESPERA;
}

/* ******************************************************************************************************************************************** */
// Función del estado "EMERGENCIA"
ESTADO Func_ESTADO_EMERGENCIA(void) {
    ESP_LOGE(tag, "¡EMERGENCIA!");

    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_EMERGENCIA;

    outputs.LED_MOVIMIENTO = FALSE;
    outputs.LED_EMERGENCIA = TRUE;
    outputs.MA = FALSE;
    outputs.MC = FALSE;

    vTaskDelay(1500 / portTICK_PERIOD_MS);
    if (inputs.FC == FALSE) {
        return ESTADO_ANTERIOR;
    }

    return ESTADO_EMERGENCIA; // Mantener el estado si la condición no se cumple
}

/* ******************************************************************************************************************************************** */
// Función del estado "ERROR"
ESTADO Func_ESTADO_ERROR(void) {
    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_ERROR;

    outputs.LED_MOVIMIENTO = FALSE;
    outputs.LED_EMERGENCIA = TRUE;
    outputs.MA = FALSE;
    outputs.MC = FALSE;

    ESP_LOGE(tag, "¡ERROR!");

    vTaskDelay(500 / portTICK_PERIOD_MS);
    outputs.LED_EMERGENCIA = FALSE;
    vTaskDelay(500 / portTICK_PERIOD_MS);
    outputs.LED_EMERGENCIA = TRUE;
    vTaskDelay(500 / portTICK_PERIOD_MS);
    outputs.LED_EMERGENCIA = FALSE;

    return ESTADO_ESPERA;
}

/* ******************************************************************************************************************************************** */
// Función del estado "ESPERA"
ESTADO Func_ESTADO_ESPERA(void) {
    ESTADO_ANTERIOR = ESTADO_ACTUAL;
    ESTADO_ACTUAL = ESTADO_ESPERA;

    outputs.LED_EMERGENCIA = FALSE;
    outputs.LED_MOVIMIENTO = FALSE;
    outputs.MA = FALSE;
    outputs.MC = FALSE;

    ESP_LOGE(tag, "ESTADO ESPERA");

    if (inputs.LSA == FALSE && inputs.FC == FALSE && inputs.LSC == FALSE) {
        return ESTADO_CERRANDO;
    }
    if (inputs.CA == TRUE && inputs.FC == FALSE && inputs.LSA == FALSE && inputs.FC == FALSE) {
        return ESTADO_ABRIENDO;
    }
    if (inputs.CC == TRUE && inputs.FC == FALSE) {
        return ESTADO_CERRANDO;
    }
    if (inputs.CA == TRUE && inputs.FC == FALSE) {
        return ESTADO_ABRIENDO;
    }
    if (inputs.FC == TRUE) {
        return ESTADO_EMERGENCIA;
    }
    if (inputs.LSA == TRUE && inputs.LSC == TRUE) {
        return ESTADO_ERROR;
    }

    return ESTADO_ESPERA; // Mantener el estado si ninguna condición se cumple
}

/* ******************************************************************************************************************************************** */
// Función para configurar el timer
esp_err_t SET_TIMER(void) {
    ESP_LOGE(tag, "Inicializando configuración del timer...");

    xTimers = xTimerCreate("Timer",
                           (pdMS_TO_TICKS(INTERVALO)),
                           pdTRUE,
                           (void *)timerID,
                           vTimerCallback);

    if (xTimers == NULL) {
        ESP_LOGE(tag, "El timer no fue creado");
    } else {
        if (xTimerStart(xTimers, 0) != pdPASS) {
            ESP_LOGE(tag, "El timer no pudo ser activado");
        }
    }

    return ESP_OK;
}

/* ******************************************************************************************************************************************** */
// Función de escaneo de pines
esp_err_t Escaneo(void) {
    inputs.LSC = (gpio_get_level(13) == TRUE) ? TRUE : FALSE;
    inputs.LSA = (gpio_get_level(12) == TRUE) ? TRUE : FALSE;
    inputs.FC = (gpio_get_level(14) == TRUE) ? TRUE : FALSE;
    inputs.CC = (gpio_get_level(27) == TRUE) ? TRUE : FALSE;
    inputs.CA = (gpio_get_level(26) == TRUE) ? TRUE : FALSE;

    gpio_set_level(4, outputs.LED_MOVIMIENTO);
    gpio_set_level(16, outputs.LED_EMERGENCIA);
    gpio_set_level(17, outputs.MC);
    gpio_set_level(5, outputs.MA);

    return ESP_OK;
}

/* ******************************************************************************************************************************************** */
// Función para configurar los pines
void pines(void) {
    ESP_LOGE(tag, "CONFIGURACIÓN DE LOS PINES");

    gpio_config_t IO_CONFIG;

    // Configuración de pines de entrada
    IO_CONFIG.mode = GPIO_MODE_INPUT;
    IO_CONFIG.pin_bit_mask = (1 << 13) | (1 << 12) | (1 << 14) | (1 << 27) | (1 << 26);
    IO_CONFIG.pull_down_en = GPIO_PULLDOWN_ENABLE;
    IO_CONFIG.pull_up_en = GPIO_PULLUP_DISABLE;
    IO_CONFIG.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&IO_CONFIG);

    // Configuración de pines de salida
    IO_CONFIG.mode = GPIO_MODE_OUTPUT;
    IO_CONFIG.pin_bit_mask = (1 << 4) | (1 << 16) | (1 << 17) | (1 << 5);
    IO_CONFIG.pull_down_en = GPIO_PULLDOWN_DISABLE;
    IO_CONFIG.pull_up_en = GPIO_PULLUP_DISABLE;
    IO_CONFIG.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&IO_CONFIG);
}

/* ******************************************************************************************************************************************** */
// Función del contador
void contador(void) {
    if (ESTADO_ACTUAL == ESTADO_CERRANDO || ESTADO_ACTUAL == ESTADO_ABRIENDO) {
        if (inputs.CA == TRUE || inputs.CC == TRUE) {
            CONTADOR = 0;
        }
        CONTADOR++;
    } else {
        CONTADOR = 0;
    }
}

