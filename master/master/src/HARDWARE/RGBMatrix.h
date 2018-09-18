#ifndef __RGBMatrix_H__
#define __RGBMatrix_H__

#include <stdio.h>
#include <stdint.h>

typedef struct
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
} RGB_t;


typedef enum rotationDegrees
{
	rotation0,
	rotation90,
	rotation180,
	rotation270
} rotationDegrees;

typedef enum fontChoices
{
	font3x5,
	font5x7,
	font6x10,
	font8x13,
} fontChoices;

typedef enum ScrollMode
{
	wrapForward,    //向前
	bounceForward,  //弹正
	bounceReverse,  //弹反
	showtime,
	stopped,        //停止
	off             //关闭
} ScrollMode;


typedef enum { KILL, COPY, NEW } CellAction;

void RGBProcessor(void);
void RGBSetupRGBMatrixPorts(void);
void RGBDisplayBuffer(RGB_t buffer[]);
void RGBRandomizeFramebuffer(uint32_t buffer[]);
void RGBSetRow(int row);
void RGBSetRGB(RGB_t rgb1, RGB_t rgb2, uint8_t plane);
void RGBShowLine(int amount);
void RGBprocessBuffer(uint32_t src[], uint32_t dst[]);
CellAction RGBAnalyzeCell(int offset, uint32_t buffer[]);
void RGBDrawChar(uint8_t x, uint8_t y, uint32_t Color, char character);
void RGBrawString(uint8_t x, uint8_t y, uint32_t Color, char *text);
void RGBDrawPixel(uint8_t x, uint8_t y, uint32_t Color);
void GRBSetCell(uint32_t cellNum, uint32_t color);
void RGBClearBuff(void);
#endif
