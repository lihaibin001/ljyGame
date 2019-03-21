#include "maxtrixApp.h"
#include "RGBMatrix.h"
#include "rand.h"
#include "matrix_config.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "canApp.h"
#include "string.h"
#include "sync.h"

typedef struct {
	uint8_t lock :1;
	uint8_t status :3;
	uint8_t test_cnt :4;
} Palte_status_t;

#ifdef APP_ERR_LOG
#define APP_ERROR(...) printf(...)
#else
#define APP_ERROR(...)
#endif

static uint16_t gTime;
static uint8_t gGameMode;
static uint8_t gScore[2] = { 0 };
static TimerHandle_t xTimers;

static Palte_status_t palteStatus[PLATE_AMOUNT];

const uint8_t singleMode[] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x82,0x00,0x01,0xA2,0x00,0x14,0x00,
		0x00,0x44,0x00,0x01,0xFA,0x03,0x24,0x00,0x00,0xFF,0x01,0x01,0xA2,0x00,0x04,0x00,
		0x00,0x11,0x01,0x81,0xF7,0xF9,0x3F,0x00,0x00,0xFF,0x01,0x01,0x12,0x01,0x04,0x00,
		0x00,0x11,0x81,0x02,0xF3,0x01,0x04,0x00,0x00,0xFF,0x81,0x02,0x17,0xF1,0x05,0x00,
		0x00,0x10,0x40,0x84,0xF2,0x41,0x04,0x00,0x80,0xFF,0x43,0x04,0x42,0x40,0x08,0x00,
		0x00,0x10,0x20,0x08,0xFA,0x43,0x28,0x00,0x00,0x10,0x10,0x10,0xA2,0xC0,0x33,0x00,
		0x00,0x10,0x08,0x20,0x1A,0x3B,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,


};

const uint8_t mulitMode[] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0xA2,0x00,0x14,0x00,
		0x80,0xEF,0x03,0x01,0xFA,0x03,0x24,0x00,0x00,0x28,0x02,0x01,0xA2,0x00,0x04,0x00,
		0x00,0x28,0x02,0x81,0xF7,0xF9,0x3F,0x00,0x80,0x28,0x02,0x01,0x12,0x01,0x04,0x00,
		0x00,0x45,0x81,0x02,0xF3,0x01,0x04,0x00,0x00,0x45,0x81,0x02,0x17,0xF1,0x05,0x00,
		0x00,0x82,0x40,0x84,0xF2,0x41,0x04,0x00,0x00,0x82,0x40,0x04,0x42,0x40,0x08,0x00,
		0x00,0x45,0x21,0x08,0xFA,0x43,0x28,0x00,0x00,0x25,0x11,0x10,0xA2,0xC0,0x33,0x00,
		0x80,0x10,0x0A,0x20,0x1A,0x3B,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

};

const uint8_t snatch_led[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x10, 0x00, 0x22, 0x00, 0x0A, 0x00,
		0x00, 0x21, 0x90, 0x5F, 0x24, 0x00, 0x12, 0x00, 0x00, 0x51, 0x10, 0x84,
		0xEF, 0x7D, 0x02, 0x00, 0xC0, 0x8B, 0x54, 0x04, 0x12, 0x40, 0x1E, 0x00,
		0x00, 0x05, 0x35, 0x04, 0xE2, 0xC4, 0x03, 0x00, 0x00, 0xF9, 0x14, 0x44,
		0x8E, 0x28, 0x0A, 0x00, 0x00, 0x8B, 0x10, 0x84, 0x4A, 0x28, 0x0A, 0x00,
		0xC0, 0x89, 0x10, 0x04, 0xFA, 0x11, 0x0A, 0x00, 0x00, 0xC9, 0x10, 0x04,
		0x4A, 0x10, 0x04, 0x00, 0x00, 0x09, 0x28, 0x84, 0x4A, 0x28, 0x16, 0x00,
		0x00, 0x09, 0x49, 0x44, 0x4A, 0x28, 0x19, 0x00, 0x80, 0xF1, 0x45, 0x07,
		0x6D, 0x84, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };
const uint8_t road_block[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x22, 0x02, 0x04, 0x00, 0x00, 0x02, 0x00,
		0x00, 0xF4, 0xF7, 0x3C, 0xEF, 0xF3, 0x7F, 0x00, 0x00, 0x40, 0x90, 0x22,
		0x29, 0x92, 0x48, 0x00, 0x00, 0xF0, 0x97, 0x15, 0x29, 0xE2, 0x3F, 0x00,
		0x00, 0x17, 0xF4, 0x08, 0xE5, 0x83, 0x08, 0x00, 0x00, 0xF4, 0x47, 0x14,
		0x25, 0xE2, 0x3F, 0x00, 0x00, 0x14, 0x44, 0x63, 0x29, 0x82, 0x08, 0x00,
		0x00, 0xF4, 0xD7, 0x3E, 0xE9, 0xF3, 0x7F, 0x00, 0x00, 0x14, 0x54, 0x22,
		0x29, 0x42, 0x12, 0x00, 0x00, 0xF4, 0x57, 0x22, 0x27, 0xA2, 0x2F, 0x00,
		0x00, 0x0A, 0xD0, 0x3E, 0x21, 0x12, 0x42, 0x00, 0x00, 0xF1, 0x37, 0x22,
		0xF1, 0xE7, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

};
const uint8_t wipe_led[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x01, 0x00, 0x00,
		0x00, 0xF0, 0x7F, 0x11, 0x10, 0x01, 0x00, 0x00, 0x00, 0x00, 0x02, 0xF2,
		0x23, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x02, 0x08, 0x82, 0x20, 0x00, 0x00,
		0x00, 0x20, 0x22, 0xF4, 0x42, 0x2F, 0x00, 0x00, 0x00, 0x20, 0x22, 0x91,
		0x12, 0x29, 0x00, 0x00, 0x00, 0x20, 0x12, 0x92, 0x22, 0x29, 0x00, 0x00,
		0x00, 0x10, 0x05, 0xF0, 0x02, 0x2F, 0x00, 0x00, 0x00, 0x00, 0x05, 0x14,
		0x43, 0x31, 0x00, 0x00, 0x00, 0x80, 0x08, 0x12, 0x24, 0x41, 0x00, 0x00,
		0x00, 0x40, 0x10, 0x11, 0x14, 0x41, 0x00, 0x00, 0x00, 0x30, 0x60, 0xE0,
		0x07, 0x7E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };
const uint8_t agil_train[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x82, 0x40, 0x08, 0x12, 0x44, 0x08, 0x00,
		0x00, 0xBE, 0x40, 0x7E, 0x94, 0x44, 0x08, 0x00, 0x00, 0x81, 0x40, 0x08,
		0x90, 0x24, 0x7F, 0x00, 0x00, 0xBE, 0xF7, 0x3E, 0x90, 0x94, 0x04, 0x00,
		0x00, 0x62, 0x42, 0x28, 0x93, 0x74, 0x0F, 0x00, 0x00, 0xAA, 0x42, 0x7E,
		0x92, 0x44, 0x0A, 0x00, 0x00, 0xFF, 0xC2, 0x28, 0x92, 0x24, 0x7E, 0x00,
		0x00, 0xA2, 0x72, 0x3E, 0x92, 0xF4, 0x08, 0x00, 0x00, 0xAA, 0x42, 0x08,
		0x9A, 0x04, 0x2A, 0x00, 0x00, 0x7E, 0x41, 0x7A, 0x96, 0xC4, 0x4A, 0x00,
		0x00, 0xA0, 0x42, 0x0A, 0x8A, 0x34, 0x49, 0x00, 0x00, 0x58, 0x64, 0x7D,
		0x04, 0x04, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

};

const uint8_t num0[] = { 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEF, 0x01, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0xC7, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC7,
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x01, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC7,
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC7, 0x01, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0xEF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

};

const uint8_t num1[] = { 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };

const uint8_t num2[] = { 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEF, 0x01, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0xC0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0,
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x01, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };

const uint8_t num3[] = { 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEF, 0x01, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0xC0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0,
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x01, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0xEF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

};

const uint8_t num4[] = { 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEE,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEE, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0xEE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEE,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE7, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };

const uint8_t num5[] = { 0xFF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0xE0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0,
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x01, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0xEF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };

const uint8_t num6[] = { 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEF, 0x01, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEF,
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC7, 0x01, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0xEF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };

const uint8_t num7[] = { 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x38,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };

const uint8_t num8[] = { 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEF, 0x01, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0xC7, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEE,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEF,
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC7, 0x01, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0xEF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

};

const uint8_t num9[] = { 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEF, 0x01, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0xC7, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEF,
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0x01, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0xFC, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0,
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x01, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0xEF, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, };

const uint8_t *pNumber[] = { num0, num1, num2, num3, num4, num5, num6, num7,
		num8, num9, };

bool maxtrixAppSelfTest(void) {
	uint8_t i;
	RET_t status;
	return true;
	char drawBuff[16] = "init";
	static uint8_t test_loop_cnt;
	static uint32_t test_pre_tick;
	if (xTaskGetTickCount() - test_pre_tick <= 800) {
		return false;
	}
	if (test_loop_cnt >= PLATE_AMOUNT * 3) {
		//self test timeout
		test_loop_cnt = 0;
		return true;
	}

	memcpy(&drawBuff[4], "...", test_loop_cnt % 4);
	RGBClearBuff();
	RGBrawString(12, 12, 0x0000FF, drawBuff);
	test_loop_cnt++;
	for (i = 0; i < PLATE_AMOUNT; i++) {
		if (palteStatus[i].status == PLATE_STA_UNKNOW) {
			if (++palteStatus[i].test_cnt == 3) {
				palteStatus[i].status = PLATE_STA_FAULT;
			} else {
				can_frame_t msg;
//				msg.length = 8;
				msg.dataByte0 = PROTOCAL_SELF_TESET;
				msg.dataByte1 = i + 1;
//				msg.format = CAN_ID_STANDRD;
//				msg.type = CAN_TYPE_DATA;
				status = CanAppSendMsg(&msg);
				if (status != RET_OK) {
					APP_ERROR("[App] Send Can message error: %d\r\n", status);
					ERROR_HANDLER()
						;
				}
			}
		} else {
			continue;
		}
		break;
	}
	if (i == PLATE_AMOUNT && palteStatus[i - 1].status != PLATE_STA_UNKNOW) {
		test_loop_cnt = 0;
		return true; //self test complete
	}
	test_pre_tick = xTaskGetTickCount();
	return false;
}

void maxtrixAppFault(void) {
	uint8_t i;
	RET_t status;
	for (i = 0; i < PLATE_AMOUNT; i++) {
		if (palteStatus[i].status != PLATE_STA_OK) {
			can_frame_t msg;
			msg.dataByte0 = PROTOCAL_SELF_TESET;
			msg.dataByte1 = i;
			status = CanAppSendMsg(&msg);
			if (status != RET_OK) {
				APP_ERROR("[App] Send Can message error: %d\r\n", status);
				ERROR_HANDLER()
					;
			}
		}
		break;
	}
}

bool maxtrixApplockPlate(uint8_t i, uint32_t timeout) {
	if (i >= PLATE_AMOUNT) {
		return false;
	}
	while (palteStatus[i].lock && timeout--) {
		vTaskDelay(1);
	}
	return palteStatus[i].lock == 0;
}

void maxtrixAppUnlockPlate(uint8_t i) {
	palteStatus[i].lock = 0;
}

bool maxtrixAppSetPlateStatus(uint8_t i, uint8_t status) {
	if (maxtrixApplockPlate(i, 250)) {
		palteStatus[i].status = status;
		palteStatus[i].lock = 0;
		return true;
	}
	return false;
}

bool maxtrixAppGetPlateStatue(uint8_t *status) {
	uint8_t i;
	for (i = 0; i < PLATE_AMOUNT; i++) {
		if (maxtrixApplockPlate(i, 250)) {
			if (palteStatus[i].status != PLATE_STA_OK) {
				*status |= 1 << i;
			}
			palteStatus[i].lock = 0;
		} else {
			break;
			//ERROR_HANDLER();
		}
	}
	return i == PLATE_AMOUNT;
}

void maxtrixAppSetGameMode(uint8_t mode) {
	if(mode == 0) {
		RGBdrawImage(0, 0, 0xFF, singleMode);
	} else {
		RGBdrawImage(0, 0, 0xFF, mulitMode);
	}
	gGameMode = mode;
}

uint8_t maxtrixAppGetGameMode(void) {
	return gGameMode;
}

void maxtrixAppSetGameScoreRefresh(void) {
	uint8_t hundreds, tens, units;
	RGBClearBuff();
	if (gGameMode == 0) { //single mode
		hundreds = gScore[0] / 100;
		tens = (gScore[0] - hundreds * 100) / 10;
		units = gScore[0] % 10;
		//draw score
		RGBdrawImage(33, 10, 0xFF, pNumber[hundreds]);
		RGBdrawImage(43, 10, 0xFF, pNumber[tens]);
		RGBdrawImage(53, 10, 0xFF, pNumber[units]);

		//draw time
		hundreds = gTime / 100;
		tens = (gTime - hundreds * 100) / 10;
		units = gTime % 10;
		RGBdrawImage(1, 10, 0xFF, pNumber[hundreds]);
		RGBdrawImage(11, 10, 0xFF, pNumber[tens]);
		RGBdrawImage(21, 10, 0xFF, pNumber[units]);

	} else {
		//draw score
		hundreds = gScore[0] / 100;
		tens = (gScore[0] - hundreds * 100) / 10;
		units = gScore[0] % 10;
		RGBdrawImage(33, 2, 0xFF, pNumber[hundreds]);
		RGBdrawImage(43, 2, 0xFF, pNumber[tens]);
		RGBdrawImage(53, 2, 0xFF, pNumber[units]);

		hundreds = gScore[1] / 100;
		tens = (gScore[1] - hundreds * 100) / 10;
		units = gScore[1] % 10;
		RGBdrawImage(33, 18, 0xFF0000, pNumber[hundreds]);
		RGBdrawImage(43, 18, 0xFF0000, pNumber[tens]);
		RGBdrawImage(53, 18, 0xFF0000, pNumber[units]);

		//draw time
		hundreds = gTime / 100;
		tens = (gTime - hundreds * 100) / 10;
		units = gTime % 10;
		RGBdrawImage(1, 2, 0xFF, pNumber[hundreds]);
		RGBdrawImage(11, 2, 0xFF, pNumber[tens]);
		RGBdrawImage(21, 2, 0xFF, pNumber[units]);
		RGBdrawImage(1, 18, 0xFF0000, pNumber[hundreds]);
		RGBdrawImage(11, 18, 0xFF0000, pNumber[tens]);
		RGBdrawImage(21, 18, 0xFF0000, pNumber[units]);
	}
}

void maxtrixAppGameStart(void) {
	gScore[0] = 0;
	gScore[1] = 0;
	maxtrixAppSetGameScoreRefresh();
}

void maxtriAppScoreIncrease(uint8_t player) {
	gScore[player - 1]++;
	maxtrixAppSetGameScoreRefresh();
}

void maxtriAppScoreDecrease(uint8_t player) {
	if(gScore[player - 1] != 0) {
		gScore[player - 1]--;
		maxtrixAppSetGameScoreRefresh();
	}
}

static void vTimerCallback(void *p) {
	(void) p;
	gTime++;
	maxtrixAppSetGameScoreRefresh();
}

void maxtriAppInit(void) {
	xTimers = xTimerCreate("1stimer", pdMS_TO_TICKS(1000), pdTRUE,
			(void*) 0, vTimerCallback);
	if (xTimers == NULL) {
		while(1)
			;
	}
}

void maxtriAppStartTime(void) {
	gTime = 0;
	if(pdPASS != xTimerStart(xTimers, 100)) {
		while(1)
			;
	}
}
