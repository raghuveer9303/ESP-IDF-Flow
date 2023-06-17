#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define FLOW_METER_PIN  GPIO_NUM_4  // GPIO pin connected to flow meter (white wire)
#define PULSES_PER_LITER  340  // Adjust this value based on your flow meter calibration
volatile float total_liters = 0.0;
uint32_t total_pulse_count = 0;

void flow_meter_task(void* pvParameters) {
    int previous_state = gpio_get_level(FLOW_METER_PIN);
    int current_state;
    uint32_t pulse_count = 0;
    uint32_t ppcount = 0;
    float liters_per_minute = 0.0;
    TickType_t last_time = xTaskGetTickCount();

    while (1) {
        current_state = gpio_get_level(FLOW_METER_PIN);

        if (current_state != previous_state && current_state == 1) {
            pulse_count++;
            total_pulse_count++;
            
            total_liters = (float)total_pulse_count / PULSES_PER_LITER;
        }
        
        previous_state = current_state;

        TickType_t current_time = xTaskGetTickCount();
        TickType_t elapsed_time = current_time - last_time;
        if (elapsed_time >= pdMS_TO_TICKS(1000)) {  // Calculate liters per minute every minute
            liters_per_minute = (float)pulse_count / PULSES_PER_LITER * 60;
            pulse_count = 0;
            last_time = current_time;
            ppcount = total_pulse_count;
            printf("Total pulses: %.2d\n",ppcount);
            printf("Total Liters: %.2f\n", total_liters);
            printf("Liters per Minute: %.2f\n", liters_per_minute);
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);  // Adjust delay as needed
    }
}

void app_main() {
    gpio_pad_select_gpio(FLOW_METER_PIN);
    gpio_set_direction(FLOW_METER_PIN, GPIO_MODE_INPUT);

    xTaskCreate(flow_meter_task, "flow_meter_task", 2048, NULL, 10, NULL);
}
