/*
 * Semáforo de pedestres – Zephyr RTOS 4.2
 * FRDM-KL25Z
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(sem_mod, LOG_LEVEL_INF);

// Mutex para controlar acesso aos LEDs
K_MUTEX_DEFINE(led_mutex);
// Semáforo Binário para sincronizar os faróis
K_SEM_DEFINE(led_sem, 0, 1);

// Nodes do device tree
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)

// Bool do Modo Noturno
bool nightMode = true;

#if DT_NODE_HAS_STATUS(LED0_NODE, okay)
static const struct gpio_dt_spec ledG = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
#else
#error "Unsupported board: led0 devicetree alias undefined"
#endif

#if DT_NODE_HAS_STATUS(LED1_NODE, okay)
static const struct gpio_dt_spec ledB = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
#else
#error "Unsupported board: led1 devicetree alias undefined"
#endif

#if DT_NODE_HAS_STATUS(LED2_NODE, okay)
static const struct gpio_dt_spec ledR = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
#else
#error "Unsupported board: led2 devicetree alias undefined"
#endif

// ============================================================================
// THREAD DO LED VERDE
// ============================================================================
void G_LED(void)
{
    for (;;) {
        if (nightMode == false){
        k_mutex_lock(&led_mutex, K_FOREVER);

        LOG_INF("Semáforo Verde");
        gpio_pin_set_dt(&ledG, 1);
        gpio_pin_set_dt(&ledR, 0);
        gpio_pin_set_dt(&ledB, 0);

        k_msleep(3000);

        k_mutex_unlock(&led_mutex);
                                }
        else{
            k_sleep(K_FOREVER);
        }
    }
}

// ============================================================================
// THREAD DO LED VERMELHO
// ============================================================================
void R_LED(void)
{
    for (;;) {
    if (nightMode == false){
		k_mutex_lock(&led_mutex, K_FOREVER);

        LOG_INF("Semáforo Vermelho");
        gpio_pin_set_dt(&ledR, 1);
        gpio_pin_set_dt(&ledG, 0);
        gpio_pin_set_dt(&ledB, 0);

        k_msleep(4000);

        k_mutex_unlock(&led_mutex);

        k_msleep(4000);

                            }
        else{
            k_sleep(K_FOREVER);
        }
    }
}

void Y_LED(void)
{
    for (;;) {
         if (nightMode == false){

		k_mutex_lock(&led_mutex, K_FOREVER);

        LOG_INF("Semáforo Amarelo");
        gpio_pin_set_dt(&ledR, 1);
        gpio_pin_set_dt(&ledG, 1);
        gpio_pin_set_dt(&ledB, 0);

        k_msleep(1000);

        k_mutex_unlock(&led_mutex);

        k_msleep(3000);
         }
         else{
        gpio_pin_toggle_dt(&ledR);
        gpio_pin_toggle_dt(&ledG);
        k_msleep(1000);
         }
    }
}

// ============================================================================
// DEFINIÇÃO DE THREADS
// ============================================================================
K_THREAD_DEFINE(Green_LedT, 512, G_LED, NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(Red_LedT,   512, R_LED, NULL, NULL, NULL, 5, 0, 0);
K_THREAD_DEFINE(Yellow_LedT,   512, Y_LED, NULL, NULL, NULL, 6, 0, 0);

// ============================================================================
// MAIN
// ============================================================================
void main(void)
{
    int ret;

    // Verifica dispositivos
    if (!gpio_is_ready_dt(&ledG) || !gpio_is_ready_dt(&ledR) || !gpio_is_ready_dt(&ledB)) {
        LOG_ERR("ERRO: LED device not ready!");
        return;
    }

    // Configura pinos
    gpio_pin_configure_dt(&ledG, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&ledR, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&ledB, GPIO_OUTPUT_INACTIVE);

    LOG_INF("Sistema Iniciado");
    LOG_INF("Semáforo Pronto!");
}
