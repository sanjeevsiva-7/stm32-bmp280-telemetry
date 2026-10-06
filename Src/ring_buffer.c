/*
 * ring_buffer.c
 *
 *  Created on: 05-Oct-2026
 *      Author: LENOVO
 */


#include "ring_buffer.h"
#include "stm32f446xx.h"

void ring_buffer_init(ring_buffer_t *rb){
	rb->head = 0;
	rb->tail = 0;
	rb->count = 0;
}
void ring_buffer_push(ring_buffer_t *rb, sensor_sample_t sample){

    rb->buffer[rb->head] = sample;
    rb->head = (rb->head + 1) % RING_BUFFER_SIZE;

    if (rb->count == RING_BUFFER_SIZE) {
        // buffer was already full — the write above just overwrote the oldest
        // entry, so tail must move up too, to stay pointing at the new oldest
        rb->tail = (rb->tail + 1) % RING_BUFFER_SIZE;
    } else {
        rb->count++;
    }


}

uint8_t ring_buffer_pop(ring_buffer_t *rb, sensor_sample_t *out)
{
    if (rb->count == 0) {
        return 0;   // empty, nothing to pop
    }
    *out = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) % RING_BUFFER_SIZE;
    rb->count--;
    return 1;
}
