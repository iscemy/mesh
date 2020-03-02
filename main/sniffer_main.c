/* sniffer example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include <driver/gpio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "rx_tx_registerer.h"
#include "esp_libc.h"
#include "mesh_io.h"
#include "routing.h"
#define TAG "sniffer"

#define MAC_HEADER_LEN 24
#define SNIFFER_DATA_LEN 112
#define MAC_HDR_LEN_MAX 40

static EventGroupHandle_t wifi_event_group;

static const int START_BIT = BIT0;

static void sniffer_task(void* pvParameters)
{
    wifi_promiscuous_filter_t sniffer_filter = {0};
    sniffer_filter.filter_mask |= WIFI_PROMIS_FILTER_MASK_CTRL;
    sniffer_filter.filter_mask |= WIFI_PROMIS_FILTER_MASK_DATA;
    sniffer_filter.filter_mask |= WIFI_PROMIS_FILTER_MASK_MGMT;
#if CONFIG_FILTER_MASK_MGMT
    
#endif

#if CONFIG_FILTER_MASK_CTRL
    
#endif

#if CONFIG_FILTER_MASK_DATA

#endif

#if CONFIG_FILTER_MASK_DATA_FRAME_PAYLOAD
    /*Enable to receive the correct data frame payload*/
    extern esp_err_t esp_wifi_set_recv_data_frame_payload(bool enable_recv);
    ESP_ERROR_CHECK(esp_wifi_set_recv_data_frame_payload(true));
#endif

#if CONFIG_FILTER_MASK_MISC
    sniffer_filter.filter_mask |= WIFI_PROMIS_FILTER_MASK_MISC;
#endif

    if (sniffer_filter.filter_mask == 0) {
        ESP_LOGI(TAG, "Please add one filter at least!");
        vTaskDelete(NULL);
    }

    xEventGroupWaitBits(wifi_event_group, START_BIT,
                        false, true, portMAX_DELAY);
    ESP_ERROR_CHECK(esp_wifi_set_channel(CONFIG_CHANNEL, 0));
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous_rx_cb(register_rx_tx(esp_wifi_80211_tx)));
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous_filter(&sniffer_filter));
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));
    vTaskDelete(NULL);
}

static esp_err_t event_handler(void* ctx, system_event_t* event)
{
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            xEventGroupSetBits(wifi_event_group, START_BIT);
            break;

        default:
            break;
    }

    return ESP_OK;
}

static void initialise_wifi(void)
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(0x0);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

static void print_task(void* pvParameters){

    int status;
    while(1){
        if(gpio_get_level(GPIO_Pin_1) == 1){
            data_unit* new_data = get_data(&status);
            if(status == 0){
                ESP_LOGI("recved data:","%s",new_data->payload);
            }else if(status == -1){
                //ESP_LOGI("print_task","empty queue:");
            }
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void app_main()
{
    ESP_ERROR_CHECK(nvs_flash_init());
    initialise_wifi();
    xTaskCreate(&sniffer_task, "sniffer_task", 2048, NULL, 10, NULL);
    //xTaskCreate(&start_routing_seq, "routing_task", 2048, NULL, 9, NULL);
    //xTaskCreate(&, "print_task", 2048, NULL, 10, NULL);
    char frame_buffer [400];
    frame_buffer[0] = 0x08; // Version (B0-1): 00 (802.11), Type (B2-3): 10 (Data) Subtype: 0000 (Data) _ 0000100 = 0x08
    frame_buffer[1] = 0x03;
    frame_buffer[2] = 0x00;
    frame_buffer[3] = 0x00;
  
    esp_wifi_set_channel(4,WIFI_SECOND_CHAN_NONE);
         
    unsigned char data_test[] = "0esdasdasdasdasdasdasdasdasdasd3trasdasdasd";
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = GPIO_Pin_0;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    gpio_config(&io_conf);
    int status;

    vTaskDelay(10 / portTICK_PERIOD_MS);
    start_routing_seq();
    while(1){
        frame_buffer[4] += 1;
        //ESP_LOGI(TAG,"%d",esp_wifi_80211_tx(WIFI_IF_STA, beacon_raw, sizeof(beacon_raw), 1));
        //ESP_LOGI(TAG,"%d",esp_wifi_80211_tx(WIFI_IF_STA, frame_buffer+frame_buffer[4], 40, 1));

        vTaskDelay(1000 / portTICK_PERIOD_MS);
        //send_routing_packets(); 

        if (data_test[0] == '9' ){
            data_test[0] = '0';
        }
        data_test[0] = data_test[0] + 1;
        
        //ESP_LOGI("sniffer","address esp_wifi_80211_tx() is :%p\n", esp_wifi_80211_tx);
   
        //(*tx_func)(WIFI_IF_STA, beacon_raw, sizeof(beacon_raw), 1);
        //(*tx_func)(0, beacon_raw, sizeof(beacon_raw), 1);
        
    }

}
