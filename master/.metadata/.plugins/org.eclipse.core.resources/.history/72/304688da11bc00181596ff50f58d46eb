#include "RGBMatrix.h"
#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "math.h"
#include "string.h"
#include "stdlib.h"
#include "matrix_config.h"
#include "rgbMaxtrixFront.h"

bitmap_font *font = (bitmap_font *) &apple3x5;
const bitmap_font *scrollFont = &apple5x7;
// Scrolling
ScrollMode scrollmode = bounceForward;
uint32_t ScrolColor; //颜色
uint32_t ScrolbgColor;
uint16_t textlen, textWidth; //字符个数 总移位数
u8 ScrollPosition = 0; //滚动位置
u8 Scroll_Y;
const int waits[] = { 5, 10, 20, 40, 80, 160, 320, 640 };
const int scan = MATRIX_HEIGHT / 2;
uint32_t birthRate;
uint8_t gammaTable[256];
RGB_t RGB[MATRIX_SIZE];

// ----- Timing definitions -------------------------------------------------


void RGBProcessor(void)
{
	RGBDisplayBuffer(RGB);
}

/**
 * Displays the buffer on the display using binary encoding (PWM equivalent).
 */

void RGBDisplayBuffer(RGB_t buffer[])
{
	for(int s = 0; s < scan; s++)
	{
		RGBSetRow(s);
		int plane, x;
		int offset1 = MATRIX_WIDTH * s;
		int offset2 = MATRIX_WIDTH * (s + scan);
		for(plane = 0; plane < 8; plane++)
		{
			for(x = 0; x < MATRIX_WIDTH; x++)
			{
				RGBSetRGB(buffer[offset1 + x], buffer[offset2 + x], plane);
				CLK_TOGGLE;
			}
			RGBShowLine(waits[plane]);
		}
	}
}

/**
 * generates some random junk for testing on the framebuffer.
 */
void RGBRandomizeFramebuffer(uint32_t buffer[])
{

	for(int i = 0; i < MATRIX_SIZE; i++)
	{
		buffer[i] = 0x00
		            | ((gammaTable[rand() % 255]) << 0)
		            | ((gammaTable[rand() % 255]) << 8)
		            | ((gammaTable[rand() % 255]) << 16)
		            | ((gammaTable[rand() % 255]) << 24);
	}
}


/**
 * sets the row on the row gpio ports
 */
void RGBSetRow(int row)
{
	// todo: perhaps a lookup table could give us a tiny boost here.

	if(row & 0b0001)
		MTX_PORT_SELSET->BSRR = MTX_PA;
	else
		MTX_PORT_SELSET->BRR = MTX_PA;

	if(row & 0b0010)
		MTX_PORT_SELSET->BSRR = MTX_PB;
	else
		MTX_PORT_SELSET->BRR = MTX_PB;

	if(row & 0b0100)
		MTX_PORT_SELSET->BSRR = MTX_PC;
	else
		MTX_PORT_SELSET->BRR = MTX_PC;

	if(row & 0b1000)
		MTX_PORT_SELSET->BSRR = MTX_PD;
	else
		MTX_PORT_SELSET->BRR = MTX_PD;
}

/**
 * loads rgb1 and rgb2 gpio ports with the given bitplane
 */
void RGBSetRGB(RGB_t rgb1, RGB_t rgb2, uint8_t plane)
{
	// using bitshifting seems to be faster due to gcc optimization
	// than using a bitmask lookup table here.

	if(rgb1.r & (1 << plane))
		MTX_PORT0->BSRR = MTX_PR0;
	else
		MTX_PORT0->BRR  = MTX_PR0;

	if(rgb1.g & (1 << plane))
		MTX_PORT0->BSRR = MTX_PG0;
	else
		MTX_PORT0->BRR  = MTX_PG0;

	if(rgb1.b & (1 << plane))
		MTX_PORT0->BSRR = MTX_PB0;
	else
		MTX_PORT0->BRR  = MTX_PB0;

	if(rgb2.r & (1 << plane))
		MTX_PORT1->BSRR = MTX_PR1;
	else
		MTX_PORT1->BRR  = MTX_PR1;

	if(rgb2.g & (1 << plane))
		MTX_PORT1->BSRR = MTX_PG1;
	else
		MTX_PORT1->BRR  = MTX_PG1;

	if(rgb2.b & (1 << plane))
		MTX_PORT1->BSRR = MTX_PB1;
	else
		MTX_PORT1->BRR  = MTX_PB1;
}

/**
 * strobes / shows a line for a n*nop amount of time.
 */
void RGBShowLine(int amount)
{
	STROBE;
	DISP_ON;
	for(int c = 0; c < amount; c++) asm("nop");
	DISP_OFF;
}

void RGBProcessBuffer(uint32_t src[], uint32_t dst[])
{
	// apply GOF rules on src and store result in dst.
	for(int i = 0; i < MATRIX_SIZE; i++)
	{
		CellAction action = RGBAnalyzeCell(i, src);
		if(COPY == action)
		{
			dst[i] = src[i];
		}
		else if(NEW == action)
		{
			dst[i] = ((gammaTable[rand() % 255]) << 0) | ((gammaTable[rand() % 255]) << 8) | ((gammaTable[rand() % 255]) << 16) | ((1) << 24);
			birthRate++;
		}
		else if(KILL == action)
		{
			dst[i] = 0x00ffffff & src[i];
		}
	}

	// fade out dead cells
	for(int i = 0; i < MATRIX_SIZE; i++)
	{
		if(!(0x01000000 & dst[i]))
		{
			dst[i] = (((dst[i]       & 0x000000ff) >> 1))       |
			         (((dst[i] >> 8  & 0x000000ff) >> 1) << 8)  |
			         (((dst[i] >> 16 & 0x000000ff) >> 1) << 16);
		}
	}
}

CellAction RGBAnalyzeCell(int offset, uint32_t buffer[])
{
	// skip the first row, first column, last column and last row to make alive neighbor
	// detection easier.
	if((offset    < MATRIX_WIDTH) ||
	        (offset    > MATRIX_SIZE - MATRIX_WIDTH) ||
	        (offset    % MATRIX_WIDTH == 0) ||
	        ((offset + 1) % MATRIX_WIDTH == 0)) return KILL;

	int neighbors = 0;
	int alive = buffer[offset] & 0x01000000;

	if(buffer[offset - 1] & 0x1000000) neighbors ++;
	if(buffer[offset + 1] & 0x1000000) neighbors ++;

	offset -= MATRIX_WIDTH;
	if(buffer[offset - 1] & 0x1000000) neighbors ++;
	if(buffer[offset]   & 0x1000000) neighbors ++;
	if(buffer[offset + 1] & 0x1000000) neighbors ++;

	offset += MATRIX_WIDTH * 2;
	if(buffer[offset - 1] & 0x1000000) neighbors ++;
	if(buffer[offset]   & 0x1000000) neighbors ++;
	if(buffer[offset + 1] & 0x1000000) neighbors ++;

	return (neighbors < 2) ? KILL :
	       (alive && (neighbors == 2 || neighbors == 3)) ? COPY :
	       (neighbors > 3) ? KILL :
	       (!alive && neighbors == 3) ? NEW :
	       COPY;
}

void RGBSetupRGBMatrixPorts()
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(MTX_RCCP0 | MTX_RCCP1 | MTX_RCCPS, ENABLE);

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;

	GPIO_InitStructure.GPIO_Pin = MTX_PR0 | MTX_PG0 | MTX_PB0;
	GPIO_Init(MTX_PORT0, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = MTX_PR1 | MTX_PG1 | MTX_PB1;
	GPIO_Init(MTX_PORT1, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = MTX_PA | MTX_PB | MTX_PC | MTX_PD;
	GPIO_Init(MTX_PORT_SELSET, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = MTX_PSTB | MTX_POE | MTX_PCLK;
	GPIO_Init(MTX_PORT_CONTROL, &GPIO_InitStructure);
	for(int i = 0; i < 256; i++)
	{
		gammaTable[i] = 255 * pow((i / 256.0), 1.6);
	}

}

// order needs to match fontChoices enum
const bitmap_font *fontArray[] =
{
	apple3x5,
	&apple5x7,
	&apple6x10,
	&apple8x13,
};

const bitmap_font * fontLookup(fontChoices font)
{
	return fontArray[font];
}


void RGBDrawPixel(uint8_t x, uint8_t y, uint32_t Color)
{
	RGB[x + y * 64].r = gammaTable[(uint8_t)(Color & 0xFF)];
	RGB[x + y * 64].b = gammaTable[(uint8_t)((Color >> 8) & 0xff)];
	RGB[x + y * 64].g = gammaTable[(uint8_t)((Color >> 16) & 0xff)];
	//RGB[x+y*64].b |= gammaTable[Color ];
	//RGB[x+y*64].g |= Color>>8;
	//RGB[x+y*64].r |= Color>>16;
}


void RGBClearBuff(void)
{
	uint32_t num;
	for(num = 0; num < MATRIX_SIZE; num++)
	{
		RGB[num].r = 0x00;
		RGB[num].g = 0x00;
		RGB[num].b = 0x00;
	}
}
void RGBMaxtrixSetFont(fontChoices newFont)
{
	// Fonts are font3x5, font5x7, font6x10, font8x13
	font = (bitmap_font *)fontLookup(newFont);
}


u8 RGBGetBitmapFontPixelAtXY(unsigned char letter, unsigned char x, unsigned char y, char *font)
{
	int location = 0;

	if(__apple3x5_bitmap__[(location * 6) + y] & (0x80 >> x))
		return 1;
	else
		return 0;
}

void RGBDrawChar(uint8_t x, uint8_t y, uint32_t Color, char character)
{
	uint8_t xcnt, ycnt;
	character = character - 32;
	for(ycnt = 0; ycnt < font->Height; ycnt++)
	{
		for(xcnt = 0; xcnt < font->Width; xcnt++)
		{
			if(font->Bitmap[(character * (font->Height)) + ycnt] & (0x80 >> xcnt))
			{
				RGBDrawPixel(x + xcnt, y + ycnt, Color);
			}
		}
	}
}
void RGBrawString(uint8_t x, uint8_t y, uint32_t Color, char *text)
{
	uint8_t xcnt, ycnt, i = 0, offset = 0;
	char character;

	// limit text to 10 chars, why?
	for(i = 0; i < 20; i++)
	{
		character = text[offset++];
		if(character == '\0')
			return;
		character -= 32;
		for(ycnt = 0; ycnt < font->Height; ycnt++)
		{
			for(xcnt = 0; xcnt < font->Width; xcnt++)
			{
				//if (getBitmapFontPixelAtXY(character, xcnt, ycnt, font)) {
				if(font->Bitmap[(character * (font->Height)) + ycnt] & (0x80 >> xcnt))
				{
					RGBDrawPixel(x + xcnt, y + ycnt, Color);
				}
			}
		}
		x += font->Width;
	}
}

void drawImage(uint8_t x, uint8_t y, uint32_t Color, const uint8_t *addres)
{
	uint8_t b, c, d;
	//a = a / 8;

	for(b = 0; b < addres[1]; b++)
	{
		for(d = 0; d < addres[0] / 8; d++)
		{
			uint8_t num = addres[b * (addres[0] / 8) + d + 2];
			for(c = 0; c < 8; c++)
			{
				if((num & 0x80) == 0x80)
					RGBDrawPixel(d * 8 + c + x, b + y, Color);
				num = num << 1;
				//if(num & (1<<c))
				//	drawPixel(c+x,b+y,Color);
			}
		}
	}
}

void GRBSetCell(uint32_t cellNum, uint32_t color)
{
	RGB[cellNum].r = gammaTable[(uint8_t)(color & 0xFF)];
	RGB[cellNum].b = gammaTable[(uint8_t)((color >> 8) & 0xff)];
	RGB[cellNum].g = gammaTable[(uint8_t)((color >> 16) & 0xff)];
}
