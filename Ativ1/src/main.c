#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>


#define RED_TIME_MS 3000
#define GREEN_TIME_MS 3000
#define YELLOW_TIME_MS 1000

#define GREEN_LED_NODE DT_ALIAS(led0)
#define YELLOW_LED_NODE DT_ALIAS(led1)
#define RED_LED_NODE DT_ALIAS(led2)

#if !DT_NODE_HAS_STATUS(GREEN_LED_NODE, okay)
#error "Unsupported board: led0 devicetree alias is not defined"
#endif

#if !DT_NODE_HAS_STATUS(YELLOW_LED_NODE, okay)
#error "Unsupported board: led1 devicetree alias is not defined"
#endif

#if !DT_NODE_HAS_STATUS(RED_LED_NODE, okay)
#error "Unsupported board: led2 devicetree alias is not defined"
#endif

static const struct gpio_dt_spec green_led = GPIO_DT_SPEC_GET(GREEN_LED_NODE, gpios);
static const struct gpio_dt_spec yellow_led = GPIO_DT_SPEC_GET(YELLOW_LED_NODE, gpios);
static const struct gpio_dt_spec red_led = GPIO_DT_SPEC_GET(RED_LED_NODE, gpios);

enum traffic_state {
    TRAFFIC_RED,
    TRAFFIC_GREEN,
    TRAFFIC_YELLOW,
};

static int configure_led(const struct gpio_dt_spec *led)
{
    int ret;

    if (!gpio_is_ready_dt(led)) {
        printk("Error: LED device %s is not ready\n", led->port->name);
        return -ENODEV;
    }

    ret = gpio_pin_configure_dt(led, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        printk("Error %d: failed to configure LED on %s pin %d\n",
               ret, led->port->name, led->pin);
    }

    return ret;
}

static int set_traffic_lights(bool red, bool yellow, bool green)
{
    int ret;

    if (red == 1) {
        ret = gpio_port_clear_bits_raw(red_led.port, BIT(red_led.pin));
    } else {
        ret = gpio_port_set_bits_raw(red_led.port, BIT(red_led.pin));
    }
    if (ret < 0) {
        return ret;
    }

    if (yellow == 1) {
        ret = gpio_port_clear_bits_raw(yellow_led.port, BIT(yellow_led.pin));
    } else {
        ret = gpio_port_set_bits_raw(yellow_led.port, BIT(yellow_led.pin));
    }
    if (ret < 0) {
        return ret;
    }

    if (green == 1) {
        ret = gpio_port_clear_bits_raw(green_led.port, BIT(green_led.pin));
    } else {
        ret = gpio_port_set_bits_raw(green_led.port, BIT(green_led.pin));
    }

    return ret;
}

int main(void)
{
    enum traffic_state state = TRAFFIC_RED;

    if (configure_led(&red_led) < 0 ||
        configure_led(&yellow_led) < 0 ||
        configure_led(&green_led) < 0) {
        return 0;
    }

    while (1) {
        switch (state) {
        case TRAFFIC_RED:
            set_traffic_lights(true, false, false);
            k_msleep(RED_TIME_MS);
            state = TRAFFIC_GREEN;
            break;

        case TRAFFIC_GREEN:
            set_traffic_lights(false, false, true);
            k_msleep(GREEN_TIME_MS);
            state = TRAFFIC_YELLOW;
            break;

        case TRAFFIC_YELLOW:
            set_traffic_lights(true, false, true);
            k_msleep(YELLOW_TIME_MS);
            state = TRAFFIC_RED;
            break;
        }
    }

    return 0;
}