/*
 * mp3.h
 *
 *  Created on: Apr 5, 2019
 *      Author: murphy
 */

#ifndef HARDWARE_MP3_H_
#define HARDWARE_MP3_H_

#include "uart.h"

void mp3_init(void);
void mp3_send_command(uint8_t command, uint16_t data) ;

#endif /* HARDWARE_MP3_H_ */
