/*
 * rx_ring_buffer.c
 *
 *  Created on: 05-Oct-2026
 *      Author: LENOVO
 */


#include "rx_ring_buffer.h"
#include "stm32f446xx.h"

void RX_ring_buffer_init(RX_ring_buffer_t *bf)
{
    bf->head = 0;
    bf->tail = 0;
    bf->count = 0;
}

void RX_ring_buffer_push(RX_ring_buffer_t *bf, uint8_t data)
{
    bf->buffer[bf->head] = data;
    bf->head = (bf->head + 1) % RX_RING_BUFFER_MAX_SIZE;

    if (bf->count == RX_RING_BUFFER_MAX_SIZE) {
        bf->tail = (bf->tail + 1) % RX_RING_BUFFER_MAX_SIZE;
    } else {
        bf->count++;
    }
}

uint8_t RX_ring_buffer_pop(RX_ring_buffer_t *bf, uint8_t *ptr)
{
    if (bf->count == 0) {
        return 0;
    }
    *ptr = bf->buffer[bf->tail];
    bf->tail = (bf->tail + 1) % RX_RING_BUFFER_MAX_SIZE;
    bf->count--;
    return 1;
}
