/*
 * mp3.h
 *
 *  Created on: Apr 5, 2019
 *      Author: murphy
 */

#ifndef HARDWARE_MP3_H_
#define HARDWARE_MP3_H_

#include "uart.h"
#include "sys.h"
typedef enum {
	mp3_idle,
	mp3_playing,
}mp3_status_t;

void mp3_init(void);
bool mp3_is_device_ready(void);
void mp3_play_starting_music(void);
RET_t mp3_send_command(uint8_t command, uint16_t data) ;
mp3_status_t mp3_get_status(void);
#endif /* HARDWARE_MP3_H_ */
