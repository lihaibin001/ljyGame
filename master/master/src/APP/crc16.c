#include "crc16.h"

uint16_t crc16_cal(uint8_t *pLcPtr, uint16_t LcLen) {
	uint8_t i;
	uint16_t lwCRC16 = 0;

	while (LcLen--) {
		for (i = 0x80; i != 0; i >>= 1) {
			if (0 != (lwCRC16 & 0x8000)) {
				lwCRC16 <<= 1;
				lwCRC16 ^= 0x1021;
			} else {
				lwCRC16 <<= 1;
			}

			if (0 != (*pLcPtr & i)) {
				lwCRC16 ^= 0x1021;
			}
		}
		pLcPtr++;
	}

	return (lwCRC16);
}
