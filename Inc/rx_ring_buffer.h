/*
 * rx_ring_buffer.h
 *
 *  Created on: 05-Oct-2026
 *      Author: LENOVO
 */


#ifndef RX_RING_BUFFER_H_
#define RX_RING_BUFFER_H_

#include <stdint.h>

#define RX_RING_BUFFER_MAX_SIZE 32

typedef struct {
    uint8_t buffer[RX_RING_BUFFER_MAX_SIZE];
    uint8_t head;
    uint8_t tail;
    uint8_t count;
} RX_ring_buffer_t;

void RX_ring_buffer_init(RX_ring_buffer_t *bf);
void RX_ring_buffer_push(RX_ring_buffer_t *bf, uint8_t data);
uint8_t RX_ring_buffer_pop(RX_ring_buffer_t *bf, uint8_t *ptr);


#endif /* RX_RING_BUFFER_H_ */
