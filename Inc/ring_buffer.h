/*
 * ring_buffer.h
 *
 *  Created on: 05-Oct-2026
 *      Author: LENOVO
 */

#ifndef RING_BUFFER_H_
#define RING_BUFFER_H_


#include "stm32f446xx.h"
#include "bmp280.h"

#define RING_BUFFER_SIZE 60

typedef struct {
	int32_t temperature;
	uint32_t pressure;
} sensor_sample_t;



/*         The mental model to keep
 * head = "the next empty slot I should write into"
 * tail = "the oldest slot that still holds valid, unread data"
 * count = "the single source of truth for how many valid entries exist right now
 */

typedef struct{
	sensor_sample_t  buffer[RING_BUFFER_SIZE];
	uint16_t head;
	uint16_t tail;
	uint16_t count;
}ring_buffer_t;


void ring_buffer_init(ring_buffer_t *rb);
void ring_buffer_push(ring_buffer_t *rb, sensor_sample_t sample);
uint8_t ring_buffer_pop(ring_buffer_t *rb, sensor_sample_t *out);



#endif /* RING_BUFFER_H_ */
