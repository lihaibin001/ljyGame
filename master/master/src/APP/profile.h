#ifndef APP_PROFILE_H_
#define APP_PROFILE_H_

#include <stdint.h>

#define PROFILE_SNATCH_LED 0
#define PROFILE_ROAD_BLOCK 1
#define PROFILE_WIPE_LED 2
#define PROFILE_AGIL_TRAIN 3

void profile_load(void);
void profile_save(void);
uint32_t profile_get_idle_duration(void);
void profile_set_idle_duratoin(uint32_t duration);
uint32_t profile_get_displayoff_duration(void);
void profile_set_displayoff_duratoin(uint32_t duration);
uint8_t profile_get_volume(void);
void profile_set_volume(uint8_t valume);
uint32_t profile_get_blink_interval(uint8_t gamemode, uint8_t gamelevel);
void profile_set_blink_interval(uint8_t gamemode, uint8_t gamelevel, uint32_t interval);
void profile_factory_reset(void);
#endif /* APP_PROFILE_H_ */
