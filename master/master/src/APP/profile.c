#include "eeprom.h"
#include "profile.h"
#include "crc16.h"
#include "string.h"
#include "debug.h"
#define GAME_MODE_CNT 4
#define GAME_LEVEL_CNT 4

typedef union {
	struct _s{
		uint32_t idle_duration;
		uint32_t displayoff_duration;
		uint32_t slaver_blink_interval[GAME_MODE_CNT][GAME_LEVEL_CNT];
		uint8_t volume;
		uint16_t crc;
	}s;
	uint8_t b[sizeof(struct _s)];
}profile_t;

static profile_t local_profile;
const static profile_t default_profile = {
		.s = {
			.idle_duration = 1000*60*60*2,
			.displayoff_duration = 1000*60*60,
			.slaver_blink_interval = {{3500, 2500, 1600, 1000}, {3500, 2500, 1600, 1000}, {2000, 2000, 3000, 0}, {3000, 2300, 1500, 0}},
			.volume = 15,
		}
};

void profile_load(void) {
	uint16_t crc16;
//	EE_Write_Data(0, default_profile.b, sizeof(profile_t));
	EE_Read_Data(0, local_profile.b, sizeof(profile_t));
	crc16 = crc16_cal(local_profile.b, sizeof(profile_t) - 2);
	if(crc16 != local_profile.s.crc) {
		memcpy(local_profile.b, default_profile.b, sizeof(profile_t));
	}

}

void profile_save(void) {
	uint16_t crc16 =  crc16_cal(local_profile.b, sizeof(profile_t) - 2);
	local_profile.s.crc = crc16;
	EE_Write_Data(0, local_profile.b, sizeof(profile_t));
}

uint32_t profile_get_idle_duration(void) {
	return local_profile.s.idle_duration;
}

void profile_set_idle_duratoin(uint32_t duration) {
	if(duration < 60 * 1000) {
		local_profile.s.idle_duration = 0xFFFFFFFF;
	} else {
		local_profile.s.idle_duration = duration;
	}
}

uint32_t profile_get_displayoff_duration(void) {
	return local_profile.s.displayoff_duration;
}

void profile_set_displayoff_duratoin(uint32_t duration) {
	if(duration < 60 * 1000) {
		local_profile.s.displayoff_duration = 0xFFFFFFFF;
	} else {
		local_profile.s.displayoff_duration = duration;
	}
}

uint8_t profile_get_volume(void) {
	return local_profile.s.volume;
}

void profile_set_volume(uint8_t valume) {
	local_profile.s.volume = valume;
	profile_save();
}

uint32_t profile_get_blink_interval(uint8_t gamemode, uint8_t gamelevel) {
	if(gamemode >= GAME_MODE_CNT || gamelevel >= GAME_LEVEL_CNT) {
		DEBUG_ERROR("PARA ERR:profile_set_blink_interval:%d,%d\r\n",(int)gamemode, (int)gamelevel);
		return 0;
	}
	return local_profile.s.slaver_blink_interval[gamemode][gamelevel];
}

void profile_set_blink_interval(uint8_t gamemode, uint8_t gamelevel, uint32_t interval) {
	if(gamemode >= GAME_MODE_CNT || gamelevel >= GAME_LEVEL_CNT) {
		DEBUG_ERROR("PARA ERR:profile_set_blink_interval:%d,%d,%d\r\n",(int)gamemode, (int)gamelevel, (int)interval);
		return ;
	}
	local_profile.s.slaver_blink_interval[gamemode][gamelevel] = interval;
}

void profile_factory_reset(void) {
	memcpy(local_profile.b, default_profile.b, sizeof(profile_t));
	profile_save();
}
