/* UART Echo Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#define LOG_LOCAL_LEVEL ESP_LOG_NONE

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

/**
 * This is an example which echos any data it receives on configured UART back to the sender,
 * with hardware flow control turned off. It does not use UART driver event queue.
 *
 * - Port: configured UART
 * - Receive (Rx) buffer: on
 * - Transmit (Tx) buffer: off
 * - Flow control: off
 * - Event queue: off
 * - Pin assignment: see defines below (See Kconfig)
 */

#define BUF_SIZE (1024)

#define TXD (17)
#define RXD (16)

#define M0 (32)
#define M1 (33)
static const char *TAG = "MAIN";


static void uart0_handler(void *arg){

    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);
    while (1) {
        int len = uart_read_bytes(UART_NUM_0, data, BUF_SIZE,1/portTICK_RATE_MS);

        uart_write_bytes(UART_NUM_2, (const char *) data, len);

    }
}

static void uart2_handler(void *arg){

    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);
    while (1) {
        int len = uart_read_bytes(UART_NUM_2, data, BUF_SIZE,100/portTICK_RATE_MS);

        uart_write_bytes(UART_NUM_0, (const char *) data, len);
    }
}



void app_main(void)
{
    ESP_LOGV(TAG,"starting");

    ESP_LOGV(TAG,"setting M0, M1 high");
    gpio_pad_select_gpio(M0);
    gpio_pad_select_gpio(M1);

    gpio_set_direction(M0, GPIO_MODE_OUTPUT);
    gpio_set_direction(M1, GPIO_MODE_OUTPUT);

    gpio_set_level(M0, 1);
    gpio_set_level(M1,1);


        /* Configure parameters of an UARTb driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    int intr_alloc_flags = 0;


    ESP_LOGV(TAG,"installing driver 2");
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, BUF_SIZE * 2, BUF_SIZE * 2, 0, NULL, intr_alloc_flags));
    ESP_LOGV(TAG,"installing driver 0");
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, BUF_SIZE * 2, BUF_SIZE * 2, 0, NULL, intr_alloc_flags));

    ESP_LOGV(TAG,"configuring driver 2");
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_2, &uart_config));
    ESP_LOGV(TAG,"configuring driver 0");
    ESP_ERROR_CHECK(uart_param_config(UART_NUM_0, &uart_config));
    //ESP_ERROR_CHECK(uart_set_baudrate(UART_NUM_0,115200)); // quitar esto al final (si queremos que el baudrate sea 9600)

    ESP_LOGV(TAG,"setting pins driver 2");
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, TXD, RXD,UART_PIN_NO_CHANGE , UART_PIN_NO_CHANGE));
    ESP_LOGV(TAG,"setting pins driver 0");
    ESP_ERROR_CHECK(uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE,UART_PIN_NO_CHANGE , UART_PIN_NO_CHANGE));



    ESP_LOGV(TAG,"creating tasks:");
    xTaskCreate(uart0_handler, "uart0_handler", 4096, NULL, 10, NULL);
    xTaskCreate(uart2_handler, "uart2_handler", 4096, NULL, 10, NULL);

}
