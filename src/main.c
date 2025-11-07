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

// Semáforo binário para exclusão de acesso aos LEDs
K_SEM_DEFINE(led_sem, 1, 1);

// Semáforo para sincronizar farol com botão
K_SEM_DEFINE(button_sem, 0, 1);

// Nodes do device tree
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)
#define BUTTON_NODE DT_NODELABEL(user_button_0)
#define OUT_PORT DEVICE_DT_GET(DT_NODELABEL(gpioa))
#define OUT_PIN  1


// Bool do Modo Noturno
bool nightMode = false;

// Valor da entrada do Botão
int value;

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

#if DT_NODE_HAS_STATUS(BUTTON_NODE, okay)
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);
#else
#error "Unsupported board: button devicetree alias undefined"
#endif

static struct gpio_callback button_cb;

// ----------------------------------------------------------------------------
// Button ISR
// ----------------------------------------------------------------------------
void button_pressed(const struct device *dev, struct gpio_callback *cb,
                    uint32_t pins)
{
    value = true;
}

static void out_pin_set(int value)
{
    gpio_pin_set(OUT_PORT, OUT_PIN, value);
}


void force_red(void)
{
    k_mutex_lock(&led_mutex, K_FOREVER);

    LOG_INF("Travessia requisitada! Interrompendo ciclo.");

    gpio_pin_set_dt(&ledG, 0);
    gpio_pin_set_dt(&ledB, 0);
    gpio_pin_set_dt(&ledR, 1);
    out_pin_set(1);

    // tempo de travessia
    k_msleep(4000);

    value = false;
    out_pin_set(0);

    k_mutex_unlock(&led_mutex);
}

// ============================================================================
// THREAD DO LED VERDE
// ============================================================================

void G_LED(void)
{
    for (;;) {
        if(nightMode == false) {

            k_sem_take(&led_sem, K_FOREVER);

            LOG_INF("Semáforo Verde");
            gpio_pin_set_dt(&ledG, 1);
            gpio_pin_set_dt(&ledR, 0);
            gpio_pin_set_dt(&ledB, 0);

            if (value) {
                force_red();
                 LOG_INF("Voltando o Ciclo.");
                 value = 0;
            }

            k_msleep(3000);

            k_sem_give(&led_sem);

        } else {
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

        if (value == 1) {
            k_sem_take(&led_sem, K_FOREVER);

            gpio_pin_set_dt(&ledR, 1);
            gpio_pin_set_dt(&ledG, 0);
            gpio_pin_set_dt(&ledB, 0);
            out_pin_set(1);

            k_msleep(4000);

            out_pin_set(0);
            k_sem_give(&led_sem);
        }

        if(nightMode == false){

            k_sem_take(&led_sem, K_FOREVER);

            LOG_INF("Semáforo Vermelho");
            gpio_pin_set_dt(&ledR, 1);
            gpio_pin_set_dt(&ledG, 0);
            gpio_pin_set_dt(&ledB, 0);

            if (value) {
                force_red();
                 LOG_INF("Voltando o Ciclo.");
                value = 0;
            }


            k_msleep(4000);

            k_sem_give(&led_sem);

            k_msleep(4000);

        } else {
            k_sleep(K_FOREVER);
        }
    }
}

// ============================================================================
// THREAD DO LED AMARELO
// ============================================================================
void Y_LED(void)
{
    for (;;) {
        if(nightMode == false){

            k_sem_take(&led_sem, K_FOREVER);

            LOG_INF("Semáforo Amarelo");
            gpio_pin_set_dt(&ledR, 1);
            gpio_pin_set_dt(&ledG, 1);
            gpio_pin_set_dt(&ledB, 0);

            if (value) {
                force_red();
                 LOG_INF("Voltando o Ciclo.");
                value = 0;
            }

            k_msleep(1000);

            k_sem_give(&led_sem);

            k_msleep(3000);

        } else {

            LOG_INF("Modo Noturno");
            gpio_pin_toggle_dt(&ledR);
            gpio_pin_toggle_dt(&ledG);
            k_msleep(1000);
        }
    }
}
// ============================================================================
// DEFINIÇÃO DE THREADS
// ============================================================================
K_THREAD_DEFINE(Green_LedT, 512, G_LED, NULL, NULL, NULL, 6, 0, 0);
K_THREAD_DEFINE(Red_LedT,   512, R_LED, NULL, NULL, NULL, 5, 0, 0);
K_THREAD_DEFINE(Yellow_LedT,   512, Y_LED, NULL, NULL, NULL, 7, 0, 0);

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

    if (!gpio_is_ready_dt(&button)) {
    return;
    }

    
    if (!device_is_ready(OUT_PORT)) {
    LOG_ERR("Porta OUT não pronta!");
    return;
}

    ret = gpio_pin_configure(OUT_PORT, OUT_PIN,GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
    LOG_ERR("Falha ao configurar OUT_PIN");
    return;
}

    // Configura pinos
    gpio_pin_configure_dt(&ledG, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&ledR, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&ledB, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&button, GPIO_INPUT | GPIO_PULL_UP);


    gpio_pin_interrupt_configure_dt(&button,
        GPIO_INT_EDGE_TO_ACTIVE);

    gpio_init_callback(&button_cb, button_pressed,
                       BIT(button.pin));

    gpio_add_callback(button.port, &button_cb);


    LOG_INF("Sistema Iniciado");
    LOG_INF("Semáforo Pronto!");
}