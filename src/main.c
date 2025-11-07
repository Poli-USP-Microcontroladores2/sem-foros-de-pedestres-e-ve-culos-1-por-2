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

// Semáforo para sincronização do verde
K_SEM_DEFINE(green_sem, 1, 1);

// Nodes do device tree
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)
#define EXT_SIG_NODE DT_ALIAS(sw0)

// Variáveis Globais
bool nightMode = false;
bool external_active = false;

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

#if DT_NODE_HAS_STATUS(EXT_SIG_NODE, okay)
static const struct gpio_dt_spec ext_sig = GPIO_DT_SPEC_GET(EXT_SIG_NODE, gpios);
#else
#error "Unsupported board: ext_sig devicetree alias undefined"
#endif

// ============================================================================
// THREAD DE MONITORAMENTO DO SINAL EXTERNO
// ============================================================================
void external_signal_thread(void)
{
    while (1) {

        int val = gpio_pin_get_dt(&ext_sig);

        if(val == 0) {
            external_active = true;
        } else {
            external_active = false;
        }

        k_msleep(10);
    }
}

// ============================================================================
// THREAD DO LED VERDE
// ============================================================================
void G_LED(void)
{
    for (;;) {

        if (!external_active) {
            k_msleep(10);
            continue;
        }

        // Espera permissão da sincronização
        k_sem_take(&green_sem, K_FOREVER);

        k_mutex_lock(&led_mutex, K_FOREVER);

        LOG_INF("Semáforo Verde");
        gpio_pin_set_dt(&ledG, 1);
        gpio_pin_set_dt(&ledR, 0);
        gpio_pin_set_dt(&ledB, 0);

        k_mutex_unlock(&led_mutex);

        k_busy_wait(4000);

        k_sem_give(&green_sem);

        // Se ainda há sinal externo, força verde novamente
        if (external_active) {
            continue;
        }
    }
}

// ============================================================================
// THREAD DO LED VERMELHO
// ============================================================================
void R_LED(void)
{
    for (;;) {

        // Se há prioridade externa, pula o vermelho
        if (external_active) {
            k_msleep(10);
            continue;
        }

        k_sem_take(&green_sem, K_FOREVER);

        k_mutex_lock(&led_mutex, K_FOREVER);

        LOG_INF("Semáforo Vermelho");
        gpio_pin_set_dt(&ledR, 1);
        gpio_pin_set_dt(&ledG, 0);
        gpio_pin_set_dt(&ledB, 0);

        k_mutex_unlock(&led_mutex);

        k_busy_wait(4000);

        k_sem_give(&green_sem);
    }
}

// ============================================================================
// DEFINIÇÃO DE THREADS
// ============================================================================
K_THREAD_DEFINE(GreenLedT, 512, G_LED, NULL, NULL, NULL, 6, 0, 0);
K_THREAD_DEFINE(Red_LedT,   512, R_LED, NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(ExtSigT,    512, external_signal_thread, NULL, NULL, NULL, 5, 0, 0);

// ============================================================================
// MAIN
// ============================================================================
void main(void)
{
    // Verifica dispositivos
    if (!gpio_is_ready_dt(&ledG) || !gpio_is_ready_dt(&ledR) || !gpio_is_ready_dt(&ledB)) {
        LOG_ERR("ERRO: LED device not ready!");
        return;
    }

    if (!gpio_is_ready_dt(&ext_sig)) {
        LOG_ERR("ERRO: External pin not ready!");
        return;
    }

    // Configura pinos
    gpio_pin_configure_dt(&ledG, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&ledR, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&ledB, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&ext_sig, GPIO_INPUT | GPIO_PULL_UP);

    LOG_INF("Sistema Iniciado");
    LOG_INF("Semáforo Pronto!");
}