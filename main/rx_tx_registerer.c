#include "rx_tx_registerer.h"
#include "esp_libc.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include <string.h>
#include "mesh_io.h"
#include "mesh_defs.h"

#define MAX_TX_SIZE 300
#define MAX_RX_SIZE 255
#define TAG "rx_tx_registerer"
#define mesh_word "cemcem"



/*promiscuous modunda olan bir takım sıkıntılardan dolayı 75 bayt ile sınırlı */

int (*tx_func)(int,unsigned char*, int, int);
uint8_t unique[] = {0xDE,0xAD,0xBE,0xEF,0xCA,0xB1};

typedef struct __attribute__((packed)){
	unsigned frame_ctrl:16;
	unsigned duration_id:16;
	uint8_t addr1[6]; 
	uint8_t addr2[6]; 
	uint8_t addr3[6]; 
	unsigned sequence_ctrl:16;
} wifi_ieee80211_mac_hdr_t;

typedef struct __attribute__((packed)){
    uint8_t dest[6];
    uint8_t src[6];
    uint16_t eth_type;
}eth_frame_t;

typedef struct __attribute__((packed)){
	wifi_ieee80211_mac_hdr_t hdr;
    eth_frame_t eth_frame;
	uint8_t payload[0];
} wifi_ieee80211_packet_t;

typedef struct __attribute__((packed)){
    uint8_t time_stamp[8];
    uint16_t beacon_interval;
    uint16_t capabilities;
    uint8_t tag_number;
    uint8_t tag_len;
    uint8_t rates[10];
    uint8_t ds_parameters[3];
    
}beacon_frame_t;



typedef struct __attribute__((packed)){
    uint8_t timestamp[8];
    uint16_t beacon_interval;
    uint16_t cap_info;
    uint8_t tag_number;
    uint8_t tag_len;
   
} wifi_ieee80211_beacon_top_t;

typedef struct __attribute__((packed)){
    wifi_ieee80211_mac_hdr_t hdr;
    wifi_ieee80211_beacon_top_t b_top;
    uint8_t payload[0];
} fuesp_t;



void rx_callback(void* buf, wifi_promiscuous_pkt_type_t type){
    wifi_promiscuous_pkt_t *promiscuous_pkt = (wifi_promiscuous_pkt_t*)buf;
    fuesp_t *wifi_ieee80211_packet = (fuesp_t*)promiscuous_pkt->payload;
    uint16_t lenght = promiscuous_pkt->rx_ctrl.legacy_length; /*only in 802.11b*/
    uint8_t payload_lenght;
    int8_t rssi = promiscuous_pkt->rx_ctrl.rssi;
    if(wifi_ieee80211_packet->hdr.frame_ctrl == 0x0080  ){
        if(memcmp(wifi_ieee80211_packet->hdr.addr1,unique,sizeof(unique)) == 0){
            payload_lenght = wifi_ieee80211_packet->b_top.tag_len;
            #ifdef DEBUG_BUILD
            ESP_LOGI("sniff","GOT PACKET %hX, len:%u, rssi:%hd, payload len:%u \n RECEIVED:",wifi_ieee80211_packet->hdr.frame_ctrl, 
                     lenght,rssi,payload_lenght);
            if(lenght<MAX_RX_SIZE){
                printf("\n");
                for(int i = 0; i < payload_lenght; i++){
                    printf("%c",*(wifi_ieee80211_packet->payload+i));
                }
                printf("\n");
            }
            #endif
            post_data(wifi_ieee80211_packet->payload,payload_lenght, rssi, 0);
        }   
    }
}

int hw_tx_adapter(unsigned char *buf, unsigned int len){
    int ret = (*tx_func)(0, buf, len, 1);
    if (ret == -1){
	    ESP_LOGE("sniffer","GONDERILEMEDI");
    }
    ESP_LOGI("sniffer","GONDERIL%d",ret);
    return ret;
}

void* register_rx_tx(void *tx){
	tx_func = tx;
	return rx_callback;
}

char buf[300];
fuesp_t *buffer = (fuesp_t*)buf;

int tx_data_blocking(unsigned char *buf, unsigned short len){
    if(len < 75){
        buffer->hdr.frame_ctrl = 0x0080;
        buffer->b_top.tag_len = len;
        memcpy(buffer->payload,buf,len); 
        memcpy(buffer->hdr.addr1,unique,6);
	    return hw_tx_adapter(buffer, sizeof(wifi_ieee80211_mac_hdr_t)+sizeof(wifi_ieee80211_beacon_top_t)+len);
    }else{

        return -1;//bigger then max tx size
    }
}

