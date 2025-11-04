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
// Sinal externo solicitando prioridade
K_SEM_DEFINE(ext_req_sem, 0, 1);
K_SEM_DEFINE(green_sem, 1, 1);

// Nodes do device tree
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)
#define EXT_SIG_NODE DT_ALIAS(sw1)   // <-- novo input

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

void external_signal_thread(void)
{
    while(1) {

        int val = gpio_pin_get_dt(&ext_sig);

        if(val == 1) { // solicita prioridade
            external_active = true;
            k_sem_give(&ext_req_sem);
        } else {
            external_active = false;
            k_sem_reset(&ext_req_sem); // libera prioridade
        }

        k_msleep(100);
    }
}

// ============================================================================
// THREAD DO LED VERDE
// ============================================================================
void G_LED(void)
{
    for (;;) {
       if(external_active) {
         // Pede prioridade sobre o verde
         k_sem_take(&green_sem, K_FOREVER);
        if(nightMode == false){
		k_msleep(4000);

        k_mutex_lock(&led_mutex, K_FOREVER);

        LOG_INF("Semáforo Verde");
        gpio_pin_set_dt(&ledG, 1);
        gpio_pin_set_dt(&ledR, 0);
        gpio_pin_set_dt(&ledB, 0);

        k_mutex_unlock(&led_mutex);

        k_sem_give(&green_sem);

		k_msleep(2000);
        }
        else {
            k_sleep(K_FOREVER);
        }
    }
    }
}

// ============================================================================
// THREAD DO LED VERMELHO
// ============================================================================
void R_LED(void)
{
    for (;;) {
    k_sem_take(&green_sem, K_FOREVER);

	if (nightMode == false){
		k_msleep(2000);

		k_mutex_lock(&led_mutex, K_FOREVER);

        LOG_INF("Semáforo Vermelho");
        gpio_pin_set_dt(&ledR, 1);
        gpio_pin_set_dt(&ledG, 0);
        gpio_pin_set_dt(&ledB, 0);

        k_mutex_unlock(&led_mutex);

        k_sem_give(&green_sem);
        k_msleep(4000);

    }
    else{
        k_mutex_lock(&led_mutex, K_FOREVER);
         LOG_INF("Modo Noturno");
        gpio_pin_set_dt(&ledG, 0);
        gpio_pin_set_dt(&ledB, 0);
        gpio_pin_toggle_dt(&ledR);
        k_mutex_unlock(&led_mutex);
        k_msleep(1000);
    }
    }
}

// ============================================================================
// DEFINIÇÃO DE THREADS
// ============================================================================
K_THREAD_DEFINE(GreenLedT, 512, G_LED, NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(Red_LedT,   512, R_LED, NULL, NULL, NULL, 6, 0, 0);
K_THREAD_DEFINE(ExtSigT,    512, external_signal_thread, NULL, NULL, NULL, 5, 0, 0);

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

    if (!gpio_is_ready_dt(&ext_sig)) {
        LOG_ERR("ERRO: External pin not ready!");
        return;
    }

    // Configura pinos
    gpio_pin_configure_dt(&ledG, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&ledR, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&ledB, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&ext_sig, GPIO_INPUT | GPIO_PULL_DOWN);

    LOG_INF("Sistema Iniciado");
    LOG_INF("Semáforo Pronto!");
}