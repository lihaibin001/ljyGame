/*----------------------------------------------------------------------------/
 *  (C)Dedao, 2016
 *-----------------------------------------------------------------------------/
 *
 * Copyright (C) 2016, Dedao, all right reserved.
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this condition and the following disclaimer.
 *
 * This software is provided by the copyright holder and contributors "AS IS"
 * and any warranties related to this software are DISCLAIMED.
 * The copyright owner or contributors be NOT LIABLE for any damages caused
 * by use of this software.
 *----------------------------------------------------------------------------*/

/*===========================================================================
 * DESCRIPTION:
 *   This is the standard code file for UART module.
 *
 \*===========================================================================*/

#include "uart.h"

/******************************************************************************/
/* Constant and Macro Definitions using #define                               */
/******************************************************************************/

/******************************************************************************/
/* Enumerations and Structures and Typedefs                                   */
/******************************************************************************/
// Information for each UART channel is stored in variables of this type:
typedef struct uart_chan_tag {
	volatile uint16_t rx_in;                  // Rx buffer input index
	volatile uint16_t rx_out;                 // Rx buffer output index
	volatile uint16_t rx_count;               // Rx buffer byte count
	uint16_t rx_size;                // Rx buffer size
	uint8_t* rx_buf;                 // Rx ring buffer
	// uart_rx_func_ptr rx_func;        // Rx callback function pointer
	volatile uint16_t tx_in;                  // Tx buffer input index
	volatile uint16_t tx_out;                 // Tx buffer output index
	volatile uint16_t tx_count;               // Tx buffer byte counter
	uint16_t tx_size;                // Tx buffer size
	uint8_t* tx_buf;                 // Tx ring buffer
	bool tx_progress;            // Tx in progress
} uart_chan_T;

/******************************************************************************/
/* ROM Const Variables With File Level Scope                                  */
/******************************************************************************/

/******************************************************************************/
/* Static Variables and Const Variables With File Level Scope                 */
/******************************************************************************/
uart_chan_T uart_chan[UART_NUM_CHANNELS];

// Tx/Rx buffers (STM32F1 CL have 4 UART channels)
static uint8_t uart0_rx_buf[UART0_RX_BUF_SIZE];
static uint8_t uart0_tx_buf[UART0_TX_BUF_SIZE];
static uint8_t uart1_rx_buf[UART1_RX_BUF_SIZE];
static uint8_t uart1_tx_buf[UART1_TX_BUF_SIZE];
static uint8_t uart2_rx_buf[UART2_RX_BUF_SIZE];
static uint8_t uart2_tx_buf[UART2_TX_BUF_SIZE];
static uint8_t uart3_rx_buf[UART3_RX_BUF_SIZE];
static uint8_t uart3_tx_buf[UART3_TX_BUF_SIZE];

/******************************************************************************/
/* Function Prototypes for Private Functions with File Level Scope            */
/******************************************************************************/
static void u_do_tx(uint8_t chan);    // Transmit helper function
static void uart_initialize_hook(uint8_t channel);

void Uart_InitIO(uint8_t chan);

/******************************************************************************/
/* Add User defined functions                                                 */
/******************************************************************************/

/******************************************************************************/
/* Function Definitions                                                       */
/******************************************************************************/

void Uart_InitIO(uint8_t chan) {
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	switch (chan) {
	case 0:
		/* Configure USART1 Rx (PA10) as input floating */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init( GPIOA, &GPIO_InitStructure);

		/* Configure USART1 Tx (PA9) as alternate function push-pull */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_Init( GPIOA, &GPIO_InitStructure);

		USART_InitStructure.USART_BaudRate = 115200;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		USART_InitStructure.USART_StopBits = USART_StopBits_1;
		USART_InitStructure.USART_Parity = USART_Parity_No;
		USART_InitStructure.USART_HardwareFlowControl =
				USART_HardwareFlowControl_None;
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

		USART_Init( USART1, &USART_InitStructure);
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
		USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
		USART_ITConfig(USART1, USART_IT_TC, DISABLE);
		/* Enable the USART1 */
		USART_Cmd(USART1, ENABLE);
		break;
	case 1:
		/* Configure USART1 Rx (PA10) as input floating */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init( GPIOA, &GPIO_InitStructure);

		/* Configure USART1 Tx (PA9) as alternate function push-pull */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_Init( GPIOA, &GPIO_InitStructure);

		USART_InitStructure.USART_BaudRate = 115200;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		USART_InitStructure.USART_StopBits = USART_StopBits_1;
		USART_InitStructure.USART_Parity = USART_Parity_No;
		USART_InitStructure.USART_HardwareFlowControl =
				USART_HardwareFlowControl_None;
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

		USART_Init( USART2, &USART_InitStructure);
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
		USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
		USART_ITConfig(USART2, USART_IT_TC, DISABLE);
		/* Enable the USART2 */
		USART_Cmd(USART2, ENABLE);
		break;
	case 2:
		/* Configure USART1 Rx (PB11) as input floating */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init( GPIOB, &GPIO_InitStructure);

		/* Configure USART1 Tx (PB10) as alternate function push-pull */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_Init( GPIOB, &GPIO_InitStructure);

		USART_InitStructure.USART_BaudRate = 9600;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		USART_InitStructure.USART_StopBits = USART_StopBits_1;
		USART_InitStructure.USART_Parity = USART_Parity_No;
		USART_InitStructure.USART_HardwareFlowControl =
				USART_HardwareFlowControl_None;
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

		USART_Init( USART3, &USART_InitStructure);
		USART_ClearITPendingBit(USART3, USART_IT_RXNE);
		USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
		USART_ITConfig(USART3, USART_IT_TC, DISABLE);
		/* Enable the USART3 */
		USART_Cmd(USART3, ENABLE);
		break;
	case 3:
		/* Configure UART4  Rx (PC.11) as input floating */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOC, &GPIO_InitStructure);
		/* Configure  UART4 Tx (PC.10) as alternate function push-pull */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_Init(GPIOC, &GPIO_InitStructure);
		//	USART_DeInit(UART4);
		USART_InitStructure.USART_BaudRate = 9600;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		USART_InitStructure.USART_StopBits = USART_StopBits_1;
		USART_InitStructure.USART_Parity = USART_Parity_No;
		USART_InitStructure.USART_HardwareFlowControl =
				USART_HardwareFlowControl_None;
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

		USART_Init(UART4, &USART_InitStructure);

		/* Enable UART4 Receive and Transmit interrupts */
		USART_ClearITPendingBit(UART4, USART_IT_RXNE);
		USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
		USART_ITConfig(UART4, USART_IT_TC, DISABLE);

		/* Enable the USART4 */
		USART_Cmd(UART4, ENABLE);
		break;
	default:
		break;
	}
}

void UART_ReConfig(uint32_t baudrate, uint16_t len, uint16_t stopbit,
		uint16_t parity) {
	USART_InitTypeDef USART_InitStructure;

	/* UART4 configuration ------------------------------------------------------*/
	/* UART4 configured as follow:
	 - BaudRate = 360 baud
	 - Word Length = 8 Bits
	 - One Stop Bit
	 - No parity
	 - Hardware flow control disabled (RTS and CTS signals)
	 - Receive and transmit enabled
	 */
	USART_DeInit(UART4);
	USART_InitStructure.USART_BaudRate = baudrate;
	USART_InitStructure.USART_WordLength = len;
	USART_InitStructure.USART_StopBits = stopbit;
	USART_InitStructure.USART_Parity = parity;
	USART_InitStructure.USART_HardwareFlowControl =
			USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(UART4, &USART_InitStructure);

	/* Enable UART4 Receive and Transmit interrupts */
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
	USART_ITConfig(UART4, USART_IT_TC, DISABLE);

	/* Enable the USART4 */
	USART_Cmd(UART4, ENABLE);
}

/*******************************************************************************
 *    Function: Uart_Initialize
 *
 *  Parameters: Channel
 *     Returns: None
 * Description: Initialize UART specified by channel
 *******************************************************************************/
void Uart_Initialize(uint8_t chan) {
	if (chan >= UART_NUM_CHANNELS)
		return;  // Invalid channel!

	uart_chan[chan].rx_count = 0;    // Clear rx byte counter
	uart_chan[chan].rx_in = 0;       // Clear rx buffer input before write index
	uart_chan[chan].rx_out = 0;      // Clear rx buffer output before read index
	//uart_chan[chan].rx_func = NULL;  // Initialize callback off

	uart_chan[chan].tx_count = 0;    // Clear tx byte counter
	uart_chan[chan].tx_in = 0;       // Clear tx buffer input before write index
	uart_chan[chan].tx_out = 0;      // Clear tx buffer output before read index
	uart_chan[chan].tx_progress = false;   // Clear tx in progress flag

	// Initializations are channel specific
	switch (chan) {
	case 0:

		uart_chan[chan].rx_buf = uart0_rx_buf;
		uart_chan[chan].rx_size = UART0_RX_BUF_SIZE;
		uart_chan[chan].tx_buf = uart0_tx_buf;
		uart_chan[chan].tx_size = UART0_TX_BUF_SIZE;
		uart_initialize_hook(chan);   // Have to set up pins here
		Uart_InitIO(0);
		break;

	case 1:
		uart_chan[chan].rx_buf = uart1_rx_buf;
		uart_chan[chan].rx_size = UART1_RX_BUF_SIZE;
		uart_chan[chan].tx_buf = uart1_tx_buf;
		uart_chan[chan].tx_size = UART1_TX_BUF_SIZE;
		uart_initialize_hook(chan);   // Have to set up pins here
		Uart_InitIO(1);
		break;

	case 2:
		uart_chan[chan].rx_buf = uart2_rx_buf;
		uart_chan[chan].rx_size = UART2_RX_BUF_SIZE;
		uart_chan[chan].tx_buf = uart2_tx_buf;
		uart_chan[chan].tx_size = UART2_TX_BUF_SIZE;
		uart_initialize_hook(chan);   // Have to set up pins here
		Uart_InitIO(2);
		break;
	case 3:
		uart_chan[chan].rx_buf = uart3_rx_buf;
		uart_chan[chan].rx_size = UART3_RX_BUF_SIZE;
		uart_chan[chan].tx_buf = uart3_tx_buf;
		uart_chan[chan].tx_size = UART3_TX_BUF_SIZE;
		uart_initialize_hook(chan);   // Have to set up pins here
		Uart_InitIO(3);
		break;
	default:
		break;
	}
}

/*******************************************************************************
 *    Function: Uart_Get_Char
 *
 *  Parameters: Channel, Pointer to variable which can receive one byte of data
 *     Returns: true if data is available and written to pointer, else false
 * Description: Reads one byte from the UART receive buffer and writes it to a
 *              pointer provided by the caller. A value is also returned to
 *              indicate whether a byte was read.
 *******************************************************************************/
bool Uart_Get_Char(uint8_t chan, uint8_t* ptr) {
	bool ret = false;        // Return value. Assume buffer empty!

	// Error checking
	if (chan >= UART_NUM_CHANNELS)
		return false; // Invalid channel!
	if (!ptr)
		return false;    // Do not accept NULL pointers!

//	__disable_irq();

	if (uart_chan[chan].rx_count != 0)  // Rx buffer not empty
			{
		*ptr = uart_chan[chan].rx_buf[uart_chan[chan].rx_out++]; // Store read data
		if ((uart_chan[chan].rx_out) >= (uart_chan[chan].rx_size)) {
			uart_chan[chan].rx_out = 0; // Wrap index
		}
		uart_chan[chan].rx_count--;
		ret = true;
	}
//	__enable_irq();

	return (ret);
}

/*******************************************************************************
 *    Function: Uart_Put_Char
 *
 *  Parameters: Channel, Data to transmit
 *     Returns: true on success, false on failure
 * Description: Copy one byte to tx buffer
 *******************************************************************************/
bool Uart_Put_Char(uint8_t chan, uint8_t data) {
	bool ret = false;        // Return value. Assume buffer full!

	if (chan >= UART_NUM_CHANNELS)
		return false; // Invalid channel!

	/** Enter Critical Section can not restart during this time ***/
//	__disable_irq();

	// Tx buffer not full
	if (uart_chan[chan].tx_count < (uart_chan[chan].tx_size)) {
		uart_chan[chan].tx_buf[uart_chan[chan].tx_in] = data; // Copy byte to tx buffer
		uart_chan[chan].tx_count++;   // Increment tx buffer byte count
		uart_chan[chan].tx_in++;      // Increment tx buffer input index
		if ((uart_chan[chan].tx_in) >= (uart_chan[chan].tx_size)) {
			uart_chan[chan].tx_in = 0; // Wrap index
		}
		ret = true;
	}

	if (false == uart_chan[chan].tx_progress) // Send first byte. Interrupts do the rest.
			{
		uart_chan[chan].tx_progress = true;    // Flag tx in progress
		switch (chan) {
		case 0:
			USART_ClearITPendingBit(USART1, USART_IT_TC);
			USART_ITConfig(USART1, USART_IT_TC, ENABLE);
			break;
		case 1:
			USART_ClearITPendingBit(USART2, USART_IT_TC);
			USART_ITConfig(USART2, USART_IT_TC, ENABLE);
			break;
		case 2:
			USART_ClearITPendingBit(USART3, USART_IT_TC);
			USART_ITConfig(USART3, USART_IT_TC, ENABLE);
			break;
		case 3:
			USART_ClearITPendingBit(UART4, USART_IT_TC);
			USART_ITConfig(UART4, USART_IT_TC, ENABLE);
			break;
		}
		u_do_tx(chan);          // Send to hardware
	}

//	__enable_irq();
	return (ret);
}

/*******************************************************************************
 *    Function: u_do_tx
 *
 *  Parameters: Channel
 *     Returns: Nothing
 * Description: Transmit helper function. Takes one byte from transmit queue
 *              and sends it to the hardware. Provides common code for first
 *              byte transmission (before transmit interrupt is enabled) and
 *              successive byte transmission (from transmit interrupt).
 *******************************************************************************/
static void u_do_tx(uint8_t chan) {
	USART_TypeDef* tmp_USARTx;

	switch (chan) {
	case 0:
		tmp_USARTx = USART1;
		break;
	case 1:
		tmp_USARTx = USART2;
		break;
	case 2:
		tmp_USARTx = USART3;
		break;
	case 3:
		tmp_USARTx = UART4;
		break;
	default:
		return;
		break;
	}

	// Write to hardware transmit register
	USART_SendData(tmp_USARTx, uart_chan[chan].tx_buf[uart_chan[chan].tx_out++]);
	uart_chan[chan].tx_count--;      // Decrement tx buffer byte count
	if ((uart_chan[chan].tx_out) >= (uart_chan[chan].tx_size)) {
		uart_chan[chan].tx_out = 0; // Wrap index
	}     // Increment index
}

/*******************************************************************************
 *    Function: UART_TX_ISR
 *
 *  Parameters: Channel
 *     Returns: None
 * Description: UART transmit Interrupt Service Routine
 *******************************************************************************/
void UART_TX_ISR(uint8_t chan) {
	USART_TypeDef* tmp_USARTx;

	switch (chan) {
	case 0:
		tmp_USARTx = USART1;
		break;
	case 1:
		tmp_USARTx = USART2;
		break;
	case 2:
		tmp_USARTx = USART3;
		break;
	case 3:
		tmp_USARTx = UART4;
		break;
	default:
		return;
		break;
	}

	if (uart_chan[chan].tx_in != uart_chan[chan].tx_out) // Any bytes to send?
			{
		u_do_tx(chan);       // Send to hardware
	} else {
		uart_chan[chan].tx_progress = false;     // Disable transmit
		USART_ITConfig(tmp_USARTx, USART_IT_TC, DISABLE);
	}
}

/*******************************************************************************
 *    Function: UART_RX_ISR
 *
 *  Parameters: Channel
 *     Returns: None
 * Description: UART recieve Interrupt Service Routine
 *******************************************************************************/
void UART_RX_ISR(uint8_t chan) {
	volatile uint8_t data;
	volatile uint8_t err;
	USART_TypeDef* tmp_USARTx;

	if (chan >= UART_NUM_CHANNELS)
		return;  // Invalid channel!

	switch (chan) {
	case 0:
		tmp_USARTx = USART1;
		break;
	case 1:
		tmp_USARTx = USART2;
		break;
	case 2:
		tmp_USARTx = USART3;
		break;
	case 3:
		tmp_USARTx = UART4;
		break;
	default:
		break;
	}

	err = tmp_USARTx->SR;

	/* Read one byte from the receive data register */
	data = (uint16_t) (tmp_USARTx->DR & (uint16_t) 0x01FF);

	if (0 == ((UART_ERR_FRAME | UART_ERR_PARITY) & err)) {
		if (uart_chan[chan].rx_count != uart_chan[chan].rx_size) {
			uart_chan[chan].rx_buf[uart_chan[chan].rx_in++] = data; // Copy data to receive buffer
			if ((uart_chan[chan].rx_in) >= (uart_chan[chan].rx_size)) {
				uart_chan[chan].rx_in = 0; // Wrap index
			}
			uart_chan[chan].rx_count++;
		} else                                    // Rx buffer full
		{
			//TODO over flow...
		}

	}
}
/*******************************************************************************
 *    Function: UART_ERR_ISR
 *
 *  Parameters: Channel
 *     Returns: None
 * Description: UART ERR Interrupt Service Routine
 *******************************************************************************/
void UART_ERR_ISR(uint8_t chan) {
	volatile uint8_t data;
	volatile uint8_t err;
	USART_TypeDef* tmp_USARTx;

	switch (chan) {
	case 0:
		tmp_USARTx = USART1;
		break;
	case 1:
		tmp_USARTx = USART2;
		break;
	case 2:
		tmp_USARTx = USART3;
		break;
	case 3:
		tmp_USARTx = UART4;
		break;
	default:
		return;
		break;
	}

	//clear error
	err = tmp_USARTx->SR;
	data = (uint16_t) (tmp_USARTx->DR & (uint16_t) 0x01FF);
}

/******************************************************************************/
/* Function Definitions                                                       */
/******************************************************************************/

/*******************************************************************************
 *    Function: uart_initialize_hook
 *  Parameters: Channel
 *     Returns: None
 * Description: Project specific initializations for UART
 *******************************************************************************/
void uart_initialize_hook(uint8_t channel) {
	switch (channel) {
	case 0:
		break;
	case 1:
		break;
	case 2:
		break;
	case 3:
		break;
	default:
		return;
		break;
	}
}

/*******************************************************************************
 *    Function: UART_Transmit
 *  Parameters: Channel
 *              pointer to transmit data buffer
 *              number of bytes to send
 *     Returns: Number of bytes successfully sent/buffered
 * Description: This function is used only as legacy code for XM CBM diagnostics (not efficient)
 *              Driver task using UART driver should use Uart_Put_Char directly to queue
 *               bytes on transmit buffer and check return value to verify if byte to
 *               transmit is successfully sent/buffered. If not, task should sleep to allow
 *               Tx buffer to allow room for byte to be buffered.
 *              Tx Buffer size should be configured accordingly
 *******************************************************************************/
extern uint8_t UART_Transmit(uint8_t channel, const uint8_t* tx_buf,
		uint8_t bytes) {
	uint8_t ret = 0;
	int i;

	for (i = 0; i < bytes; i++) {
		if (Uart_Put_Char(channel, *(tx_buf + i))) {
			ret++;
		}
	}

	return (ret);
}

/*******************************************************************************
 *    Function: UART_Rx_Empty
 *  Parameters: Channel
 *     Returns: Nothing
 * Description: Reconfigures UART channel's pins to be I/O input (disables UART function)
 *******************************************************************************/
bool UART_Rx_Empty(uint8_t chan) {
	return ((0 == uart_chan[chan].rx_count)
			&& (uart_chan[chan].rx_in == uart_chan[chan].rx_out));
}
/*******************************************************************************
 *  Function: UART_Reset_Buf
 *
 *  Parameters: None
 *  Returns: None
 *  Description: Intialize the specified uart device
 *******************************************************************************/
void UART_Reset_Buf(uint8_t chan) {
	if (chan >= UART_NUM_CHANNELS)
		return;  // Invalid channel!

	if (NULL != &uart_chan[chan]) {
		uart_chan[chan].rx_count = 0; // Clear rx byte counter
		uart_chan[chan].rx_in = 0;    // Increment before write index
		uart_chan[chan].rx_out = 0;   // Increment before read index

		uart_chan[chan].tx_count = 0; // Clear tx byte counter
		uart_chan[chan].tx_in = 0;   // Clear tx buffer input before write index
		uart_chan[chan].tx_out = 0;  // Clear tx buffer output before read index
		uart_chan[chan].tx_progress = false;   // Clear tx in progress flag
	}
}

void USART1_IRQHandler(void)
{
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
        UART_RX_ISR(UART_DEBUG_CHANNEL);
    }
    else if(USART_GetITStatus(USART1, USART_IT_TC) != RESET)
    {
		USART_ClearITPendingBit(USART1, USART_IT_TC);
        UART_TX_ISR(UART_DEBUG_CHANNEL);
    }
}

void USART2_IRQHandler(void)
{
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
        UART_RX_ISR(UART_RESERVED_CHANNEL1);
    }
    else if(USART_GetITStatus(USART2, USART_IT_TC) != RESET)
    {
		USART_ClearITPendingBit(USART2, USART_IT_TC);
        UART_TX_ISR(UART_RESERVED_CHANNEL1);
    }
}

void USART3_IRQHandler(void)
{
    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
		USART_ClearITPendingBit(USART3, USART_IT_RXNE);
        UART_RX_ISR(UART_RESERVED_CHANNEL2);
    }
    else if(USART_GetITStatus(USART3, USART_IT_TC) != RESET)
    {
		USART_ClearITPendingBit(USART3, USART_IT_TC);
        UART_TX_ISR(UART_RESERVED_CHANNEL2);
    }
}

void UART4_IRQHandler(void)
{
    if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)
    {
		USART_ClearITPendingBit(UART4, USART_IT_RXNE);
        UART_RX_ISR(UART_MP3_CHANNEL);
    }
    else if(USART_GetITStatus(UART4, USART_IT_TC) != RESET)
    {
		USART_ClearITPendingBit(UART4, USART_IT_TC);
        UART_TX_ISR(UART_MP3_CHANNEL);
    }
}

/*=======================================================================================*\
 * File Revision History
 *=======================================================================================
 * ----------  ------   ---------------------------------------------
 *
 *
 \*=======================================================================================*/
