#include "gameApp.h"
#include "maxtrixApp.h"
#include "RGBMatrix.h"
#include "rand.h"
void gameLevelIncrease(void)
{
    
}

void gameSetLevel(uint8_t level)
{
	uint8_t xoffset = 0, xoffset1 = 0;
	uint32_t i;
	uint8_t j;
	uint8_t tmp[256] = "";
	const uint8_t *pLevel1 = matrixAppGetNum(level / 10);
	const uint8_t *pLevel2 = matrixAppGetNum(level % 10);
	RGBClearBuff();
	uint32_t color = rand();
	for(i = 0; i < sizeof(gameLevelImage); i++)
	{
		for(j = 0; j < 8; j++)
		{
			if((gameLevelImage[i]) & (0x01 << j))
			{
				GRBSetCell(8 * i + j, color);
			}

		}
	}
	color = rand();
	if(level == 1)
	{
		xoffset = 46;
	}
	else if(level <= 9)
	{
		xoffset = 44;
	}
	else if(level > 9)
	{
		xoffset = 48;
		xoffset1 = 38;
		for(i = 0; i < sizeof(gameLevel); i++)
		{

			for(j = 0; j < 8; j++)
			{
				if((pLevel1[i]) & (0x01 << j))
				{

					GRBSetCell(8 * i + j + 10 * 64 + xoffset1, color);
				}
			}
		}
	}
	for(i = 0; i < sizeof(gameLevel); i++)
	{

		for(j = 0; j < 8; j++)
		{
			if((pLevel2[i]) & (0x01 << j))
			{

				GRBSetCell(8 * i + j + 10 * 64 + xoffset, color);
			}
		}
	}

}

uint8_t gameSetMode(uint8_t cnt)
{
    uint32_t i;
    uint8_t j;
    uint8_t *pMode = (uint8_t *)singleMode;
    if(cnt > 1)
    {
            pMode = (uint8_t *)mulitMode;
    }
    RGBClearBuff();
	uint32_t color = rand();
	for(i = 0; i < sizeof(gameLevel); i++)
	{
		for(j = 0; j < 8; j++)
		{

			if((gameMode[i]) & (0x01 << j))
			{
				GRBSetCell(8 * i + j, color);
			}

		}
	}   
    color = rand();
	for(i = 0; i < sizeof(gameLevel); i++)
	{
		for(j = 0; j < 8; j++)
		{

			if((pMode[i]) & (0x01 << j))
			{
				GRBSetCell(8 * i + j + 64*10 + 37,  color);
			}

		}
	}   
    return 0;
}
