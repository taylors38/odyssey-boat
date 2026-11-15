
#include "pwm.h"

#include "driver/mcpwm_prelude.h"
#include "driver/gpio.h"

#define PWM_FREQ_HZ        2000
#define PWM_RESOLUTION_HZ  1000000
#define PWM_PERIOD_TICKS   (PWM_RESOLUTION_HZ / PWM_FREQ_HZ)

#define PWM_OPERATORS      3
#define PWM_CHANNELS	   6

// Example GPIO assignments.
// Verify these are available on your particular DevKit.
static const gpio_num_t pwm_pins[PWM_CHANNELS] = {
    GPIO_NUM_0,
    GPIO_NUM_1,
    GPIO_NUM_2,
    GPIO_NUM_3,
    GPIO_NUM_16,
    GPIO_NUM_17
};

// MCPWM handles
static mcpwm_timer_handle_t pwm_timer;

static mcpwm_oper_handle_t operators[PWM_OPERATORS];

static mcpwm_cmpr_handle_t comparators[PWM_CHANNELS];

static mcpwm_gen_handle_t generators[PWM_CHANNELS];


#define CHECK_ERROR(x)                  \
    do {                                \
        esp_err_t err = (x);            \
        if (err != ESP_OK) return err;  \
    } while (0)


esp_err_t pwm_init(void)
{
    /*
     * 1. Create shared timer
     */

    mcpwm_timer_config_t timer_config = {
        .group_id = 0,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = PWM_RESOLUTION_HZ,
        .period_ticks = PWM_PERIOD_TICKS,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP
    };

    CHECK_ERROR(mcpwm_new_timer(&timer_config, &pwm_timer));


    /*
     * 2. Create three operators
     */

    for (int i = 0; i < PWM_OPERATORS; i++)
    {
        mcpwm_operator_config_t oper_config = {
            .group_id = 0
        };

        CHECK_ERROR(mcpwm_new_operator(
            &oper_config,
            &operators[i]
        ));

        // Connect each operator to the same timer
        CHECK_ERROR(mcpwm_operator_connect_timer(
            operators[i],
            pwm_timer
        ));


        /*
         * 3. Create two independent PWM channels
         *    for each operator
         */

        for (int j = 0; j < 2; j++)
        {
            int channel = (i * 2) + j;

            /*
             * Create comparator
             */

            mcpwm_comparator_config_t cmp_config = {
                .flags.update_cmp_on_tez = true
            };

            CHECK_ERROR(mcpwm_new_comparator(
                operators[i],
                &cmp_config,
                &comparators[channel]
            ));

            CHECK_ERROR(mcpwm_comparator_set_compare_value(
                comparators[channel],
                1
            ));


            /*
             * Create generator
             */

            mcpwm_generator_config_t gen_config = {
                .gen_gpio_num = pwm_pins[channel]
            };

            CHECK_ERROR(mcpwm_new_generator(
                operators[i],
                &gen_config,
                &generators[channel]
            ));


            /*
             * Configure PWM waveform
             *
             * Timer reaches zero:
             *     Output HIGH
             *
             * Comparator match:
             *     Output LOW
             */

            CHECK_ERROR(mcpwm_generator_set_action_on_timer_event(
                generators[channel],
                MCPWM_GEN_TIMER_EVENT_ACTION(
                    MCPWM_TIMER_DIRECTION_UP,
                    MCPWM_TIMER_EVENT_EMPTY,
                    MCPWM_GEN_ACTION_HIGH
                )
            ));

            CHECK_ERROR(mcpwm_generator_set_action_on_compare_event(
                generators[channel],
                MCPWM_GEN_COMPARE_EVENT_ACTION(
                    MCPWM_TIMER_DIRECTION_UP,
                    comparators[channel],
                    MCPWM_GEN_ACTION_LOW
                )
            ));

            // Start with output forced LOW
            CHECK_ERROR(mcpwm_generator_set_force_level(
                generators[channel],
                0,
                true
            ));
        }
    }


    /*
     * 4. Enable and start timer
     */

    CHECK_ERROR(mcpwm_timer_enable(pwm_timer));

    CHECK_ERROR(mcpwm_timer_start_stop(
        pwm_timer,
        MCPWM_TIMER_START_NO_STOP
    ));

    return ESP_OK;
}


/*
 * Set PWM duty cycle
 *
 * channel: 0-5
 * duty:    0.0-100.0 (%)
 */

esp_err_t pwm_set_duty(uint8_t channel, float duty)
{
    if (channel >= PWM_CHANNELS ||
        duty < 0.0f ||
        duty > 100.0f)
    {
        return ESP_ERR_INVALID_ARG;
    }

    // Handle 0% and 100% explicitly
    if (duty == 0.0f)
    {
        return mcpwm_generator_set_force_level(
            generators[channel], 0, true
        );
    }

    if (duty == 100.0f)
    {
        return mcpwm_generator_set_force_level(
            generators[channel], 1, true
        );
    }


    // Convert percentage into timer ticks
    uint32_t compare_value =
        (uint32_t)((PWM_PERIOD_TICKS * duty) / 100.0f);

    if (compare_value == 0)
        compare_value = 1;

    if (compare_value >= PWM_PERIOD_TICKS)
        compare_value = PWM_PERIOD_TICKS - 1;


    // Update comparator
    CHECK_ERROR(mcpwm_comparator_set_compare_value(
        comparators[channel],
        compare_value
    ));

    // Release any previously forced output level
    return mcpwm_generator_set_force_level(
        generators[channel], -1, true
    );
}
