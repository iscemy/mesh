#include "routing.h"
#include "mesh_io.h"
#include <string.h>
#include "esp_log.h"

#define LISTEN_NUM_OF_PACKETS_ROUTING 50
#define REPEAT_NUM_OF_ROUTING_START_SEQ 2
#define MAX_NUM_OF_DISCARDED_PACKETS_IN_START_SEQ 500
#define MAX_NUM_ROUTING_PACKET 50
#define TAG "routing.c"

//code is redundant and ugly fix it

routing_packet_info_t *routing_packet_array[LISTEN_NUM_OF_PACKETS_ROUTING];
int routing_packet_array_index = 0;


int send_routing_packets(){
	int ret = 0, len;
	routing_packet_t* routing_packet;
	for (int index = 0; index < routing_packet_array_index; index++){
		routing_packet = routing_packet_array[index];
		len =  routing_packet->routing_packet_info_data.len;
		ret = tx_data_blocking((unsigned char *)routing_packet, sizeof(general_payload_t) + sizeof(routing_packet_info_data_t) + len);
	}
	return ret;
}



routing_packet_info_t* init_routing_packets_table(uint16_t tx_from, uint16_t rx_by, int8_t rssi, char *data){
	routing_packet_info_t* new_routing_packet_info = malloc(sizeof(routing_packet_info_t) + sizeof(routing_packet_info_data_t) + 
																((routing_packet_info_data_t*)data)->len + 2);
	if(new_routing_packet_info != NULL){
		new_routing_packet_info->tx_from = tx_from;
		new_routing_packet_info->rx_by = rx_by;	
		new_routing_packet_info->rssi = rssi;	
		memcpy(new_routing_packet_info->data, data, ((routing_packet_info_data_t*)data)->len);
		*(uint16_t*)(new_routing_packet_info->data + ((routing_packet_info_data_t*)data)->len) = DEV_ID;
		return new_routing_packet_info;
	}else{
		return NULL;
	}
}

int gather_routing_packets(){
	//tüm cihazlar routing durumuna geldiği zaman
	int get_status,failsafe = 0,recved_packets = 0, rloop = 0;

	data_unit* recvd_data;
	general_payload_t* recved_payload;
	routing_packet_info_t* rpi_s;

	while (recved_packets < LISTEN_NUM_OF_PACKETS_ROUTING){
		recvd_data = get_data(&get_status); // (a)
		recved_payload = (general_payload_t*)recvd_data->payload;
		vTaskDelay(1 / portTICK_PERIOD_MS); //it has to be more abstarct and not dependent to target chips sdk
		if((recved_payload->destination == 0xFFFF)&&(recved_payload->type == routing_broadcast)){
			if(get_status == 0){
				rloop = 0;
				for(int index = 0; index < recved_payload->data[0]; index++){
					if(recved_payload->data[index+1] == DEV_ID){	//for preventing loops in rtable
						rloop = 1;
						break;
					}
				}
				if(rloop == 0){
					rpi_s = init_routing_packets_table(recved_payload->sender, DEV_ID, recvd_data->rssi, (char*)recved_payload->data); //allocating/copying routing info for table
					if(rpi_s != NULL){
						routing_packet_array[recved_packets] = rpi_s;
						recved_packets++;
					}else{
						//NOT ENOUGH MEM FOR ALLOCATION
					}
				}
				rloop = 0;
			}
		}
		if(failsafe < MAX_NUM_ROUTING_PACKET){
			routing_packet_array_index = recved_packets;
			return -1; //maybe problematic with (a)
		}
		failsafe++;
		routing_packet_array_index = recved_packets;
	}

	return 0;
}



int send_routing_start_pkts(){

	int ret;
	general_payload_t* routing_start_frame = malloc(sizeof(general_payload_t));
	if(routing_start_frame != NULL){
		routing_start_frame->destination = BROADCAST_ADDR;
		routing_start_frame->sender = 0xbeef;
		routing_start_frame->type = routing_seq_start;
		ret = tx_data_blocking((uint8_t*)routing_start_frame, sizeof(general_payload_t));
		free(routing_start_frame);	//it may cause race conditions
									//send function will be better blocking until copies and not depended to passed address
									//altough function named as blocking but its not

	}else{
		ret = -1;
	}
	return ret;
}

int repeatw_routing_start_packets(){
	//this function may cause data losses
	//cagirildigi zaman queue de olan ve routing paketi olmayan paketler kaybolacak.
	int get_status = -1, start_seq_cnt = 0;
	general_payload_t* recved_payload;
	data_unit* recved_data = NULL;
	for(int s = 0; s < MAX_NUM_OF_DISCARDED_PACKETS_IN_START_SEQ; s++){
		recved_data = get_data(&get_status);
		vTaskDelay(30 / portTICK_PERIOD_MS); //it has to be more abstarct and not dependent to target chips sdk
		if (get_status == 0){
			recved_payload = (general_payload_t*)recved_data->payload;
			ESP_LOGI(TAG,"%d %d %d",recved_payload->destination,((data_unit*) recved_data)->payload[1],((data_unit*) recved_data)->payload[2]);
			if((recved_payload->destination == 0xFFFF)&&(recved_payload->type == routing_seq_start)){
				ESP_LOGI(TAG,"testetetet %d", start_seq_cnt);
				if (start_seq_cnt < REPEAT_NUM_OF_ROUTING_START_SEQ){
					send_routing_start_pkts();
					start_seq_cnt++;
				}else{
					return 0;
				}
			}
			
		}
	}
	return -1;
}


int start_routing_seq(){
	ESP_LOGI(TAG,"start_routing_seq");
	while (repeatw_routing_start_packets() != 0){

	}
	ESP_LOGI(TAG,"received routing_start_pkts");
	while (gather_routing_packets() != 0){

	}
	ESP_LOGI(TAG,"received routing_packets");

	return 0;

}

uint16_t get_next_node_addr(uint16_t destination){
	if(destination == 0){
		return 1;
	}

	return 1;
}