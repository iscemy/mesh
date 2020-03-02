#include "mesh_defs.h"
#include "mesh_io.h"
#include "esp_libc.h"
#include <string.h>
#define MAX_QUEUE_SIZE 10
#define SWAP_SIZE 300

#ifdef DEBUG_BUILD
#include "esp_log.h"
#define TAG "mesh_io.c"
#endif

/*modülün dişari veri cıkaracağı buffer stack'de tanımlandı,
  düzgün bi algoritmayla heap'e alınabilir*/


int head = 0;
int tail = 0;

data_unit* data_unit_p_buffer[MAX_QUEUE_SIZE];
uint8_t swap[SWAP_SIZE];

int init_a_data_unit(uint8_t *buf,uint16_t len, int8_t rssi, uint32_t sys_time,data_unit *new_data){
    new_data->rssi = rssi;
    new_data->sys_time = sys_time;
    new_data->len = len;
    memcpy(new_data->payload,buf,len);
    return 0;
}


int copy_data_unit_to_swap(data_unit *from){
    if(from->len +	 sizeof(data_unit) > SWAP_SIZE){
        return -1;//data bigger than swap
    }
    data_unit *new_data = (data_unit *)swap;
    new_data->rssi = from->rssi;
    new_data->sys_time = from->sys_time;
    new_data->len = from->len;
    memcpy(new_data->payload,from->payload,from->len);
    return 0;
}

int post_data(uint8_t *buf,uint16_t len, int8_t rssi, uint32_t sys_time){
    int ret = 0;
    if(tail >= MAX_QUEUE_SIZE){
        ret = -1;//queue full
    }else if(len > SWAP_SIZE){
        ret = -2;//too big
    }else{
        data_unit *new_data = malloc(len+sizeof(data_unit));
        if(new_data != NULL){
            init_a_data_unit(buf,len,rssi,sys_time,new_data);
            data_unit_p_buffer[tail] = new_data;

            tail++;
        }else{
            ret = -3;//not enough mem
        }
    }
    #ifdef DEBUG_BUILD
        ESP_LOGI(TAG,"post_data status %d head:%d, tail:%d",ret, head,tail);
    #endif
    return ret;
}

//this function writes the received packet to a fixed location on memory and returns that address
//recalling this function without saving packet will cause data losses
//
data_unit* get_data(int* status){
    int ret = 0;
    if((head < tail)||((tail == MAX_QUEUE_SIZE - 1)&&(head == MAX_QUEUE_SIZE - 1))){
        copy_data_unit_to_swap(data_unit_p_buffer[head]);
        free(data_unit_p_buffer[head]);
        if((tail == MAX_QUEUE_SIZE - 1)&&(head == MAX_QUEUE_SIZE - 2)){
            head = 0;
            tail = 0;
        }else{
            head++;
        }
        ESP_LOGI(TAG,"get_data status %d head:%d, tail:%d", ret, head, tail);

    }else{
        //ESP_LOGI(TAG,"queue empty");
        ret = -1;//queue empty
    }
    #ifdef DEBUG_BUILD
        //ESP_LOGI(TAG,"swap %d", ((data_unit*)swap)->payload[7]);
    #endif
    *status = ret;
   	if(ret == 0){
   		 
   	} 
   
    return (data_unit*)swap;
}

