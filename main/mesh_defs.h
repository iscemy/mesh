 #pragma once
#include <stdint.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#define DEV_ID 0xF123 //TODO: mac'in son 2 byte'ı yada hashlenebilir, otomatik yap
#define BROADCAST_ADDR 0xFFFF

/*
TODO:Proper error check/debug 
*/
#define DEBUG_BUILD

/*
enum mesh_status {
	MESH_ERROR,
	MESH_OK
};
*/

typedef struct __attribute__((packed))
{
	uint16_t destination;
	uint16_t sender;
	uint8_t type;
	uint16_t data[0];
}general_payload_t;

enum packet_types{
	data = 0,
	routing_seq_start,
	routing_broadcast
	//
};
