#include "peridic.h"

static QueueSetHandle_t xQueue;

typedef void (*pFnc)(void);

static void peridic5ms(void);
static void peridic10ms(void);
static void peridic15ms(void);
static void peridic20ms(void);
static void peridic25ms(void);
static void peridic30ms(void);
const pFnc peridicFnc[] = 
{
    peridic5ms,
    peridic10ms,
    peridic15ms,
    peridic20ms,
    peridic25ms,
    peridic30ms,
};

static void peridic5ms(void){}
static void peridic10ms(void){}
static void peridic15ms(void){}
static void peridic20ms(void){}
static void peridic25ms(void){}
static void peridic30ms(void){}

static void peridicTask( void *pvParameters )
{
    uint8_t trash;
    static uint32_t  time_slice; 
    xQueueReceive(xQueue, &trash, portMAX_DELAY);
    peridic5ms();
    time_slice = time_slice << 1;
}

void peridicTaskCreate(void)
{
    xQueue = xQueueCreate(5,1);
    xTaskCreate( peridicTask, "peridic", 128, NULL, 1, NULL );
}

void peridicTriger(void)
{
    
}
