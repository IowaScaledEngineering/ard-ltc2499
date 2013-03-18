#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>


#include "mrbus-arduino.h"

// Common macros for handling 16 bit variables
// These aren't strictly part of MRBus, but are used through the code and need to be defined
#ifndef UINT16_HIGH_BYTE
#define UINT16_HIGH_BYTE(a)  ((a)>>8)
#endif 

#ifndef UINT16_LOW_BYTE
#define UINT16_LOW_BYTE(a)  ((a) & 0xFF)
#endif 

#ifndef min
#define min(a,b)  ((a)<(b)?(a):(b))
#endif

#ifndef max
#define max(a,b)  ((a)>(b)?(a):(b))
#endif

/* Variables used by MRBus code */
static volatile uint8_t mrbus_rx_input_buffer[MRBUS_BUFFER_SIZE];
static volatile uint8_t mrbus_rx_index;
static volatile uint8_t mrbus_tx_index;

/* Variables used by MRBus applications */
static volatile uint8_t mrbus_tx_buffer[MRBUS_BUFFER_SIZE];
static volatile uint8_t mrbus_state;
static uint8_t mrbus_activity;

#define MRBUS_QUEUE_DEPTH 4

typedef struct
{
	volatile uint8_t headIdx;
	volatile uint8_t tailIdx;
	volatile bool full;
	volatile MRBusPacket pktData[MRBUS_QUEUE_DEPTH];
} PacketBuffer;

static PacketBuffer rxQueue;
static PacketBuffer txQueue;

void packetBufferInitialize(PacketBuffer* r)
{
	r->headIdx = r->tailIdx = 0;
	r->full = false;
}

uint8_t packetBufferDepth(PacketBuffer* r)
{
	if(r->full)
		return(MRBUS_QUEUE_DEPTH);
	return((r->headIdx - r->tailIdx ) % MRBUS_QUEUE_DEPTH);
}

bool packetBufferPush(PacketBuffer* r, uint8_t* data, uint8_t dataLen)
{
	uint8_t* pktPtr;
	// If full, bail with a false
	if (r->full)
		return(false);

	dataLen = min(MRBUS_BUFFER_SIZE, dataLen);
	pktPtr = (uint8_t*)r->pktData[r->headIdx].pkt;
	memcpy(pktPtr, data, dataLen);
	memset(pktPtr+dataLen, 0, MRBUS_BUFFER_SIZE - dataLen);

	if( ++r->headIdx >= MRBUS_QUEUE_DEPTH )
		r->headIdx = 0;
	if (r->headIdx == r->tailIdx)
		r->full = true;
	return(true);
}

bool packetBufferPush(PacketBuffer* r, MRBusPacket* mrbPkt)
{
	// If full, bail with a false
	if (r->full)
		return(false);

	memcpy((uint8_t*)&(r->pktData[r->headIdx]), mrbPkt, sizeof(MRBusPacket));

	if( ++r->headIdx >= MRBUS_QUEUE_DEPTH )
		r->headIdx = 0;
	if (r->headIdx == r->tailIdx)
		r->full = true;
	return(true);
}

bool packetBufferPop(PacketBuffer* r, MRBusPacket* mrbPkt)
{
	if (0 == packetBufferDepth(r))
	{
		memset(mrbPkt, 0, sizeof(MRBusPacket));
		return(false);
	}
	memcpy(mrbPkt, (MRBusPacket*)&(r->pktData[r->tailIdx]), sizeof(MRBusPacket));
	if( ++r->tailIdx >= MRBUS_QUEUE_DEPTH )
		r->tailIdx = 0;
	r->full = false;
	return(true);
}

bool packetBufferPop(PacketBuffer* r, uint8_t* data, uint8_t dataLen, bool snoop)
{
	memset(data, 0, dataLen);
	if (0 == packetBufferDepth(r))
		return(false);

	memcpy(data, (uint8_t*)&(r->pktData[r->tailIdx].pkt), min(dataLen, r->pktData[r->tailIdx].pkt[MRBUS_PKT_LEN]));
	if (false == snoop)
	{
		if( ++r->tailIdx >= MRBUS_QUEUE_DEPTH )
			r->tailIdx = 0;
		r->full = false;
	}
	return(true);
}

bool packetBufferPop(PacketBuffer* r)
{
	if (0 == packetBufferDepth(r))
		return(false);
	if( ++r->tailIdx >= MRBUS_QUEUE_DEPTH )
		r->tailIdx = 0;
	r->full = false;
	return(true);
}

ISR(MRBUS_UART_DONE_INTERRUPT)
{
	// Transmit is complete: terminate
	MRBUS_PORT &= ~_BV(MRBUS_TXE);
	MRBUS_UART_SCR_B &= ~(_BV(MRBUS_TXCIE) | _BV(MRBUS_TXEN) );
	// Re-enable receive interrupt
	MRBUS_UART_SCR_B |= _BV(MRBUS_RXCIE);
	mrbus_state &= ~MRBUS_TX_BUF_ACTIVE;
}

ISR(MRBUS_UART_TX_INTERRUPT)
{
	// Transmit Routine
	if (mrbus_tx_index == 1)
	{
		// Set transmit complete interrupt after packet in underway
		MRBUS_UART_SCR_B |= _BV(MRBUS_TXCIE);
	}

	if ((mrbus_tx_index >= MRBUS_BUFFER_SIZE) || (mrbus_tx_index >= mrbus_tx_buffer[MRBUS_PKT_LEN]))
	{
		// Turn off transmit buffer interrupt
		// Wait for transmit complete interrupt to finish cleanup
		MRBUS_UART_SCR_B &= ~_BV(MRBUS_UART_UDRIE);
	}
	else
	{
		MRBUS_UART_DATA = mrbus_tx_buffer[mrbus_tx_index++];
	}
}


ISR(MRBUS_UART_RX_INTERRUPT)
{
	//Receive Routine
	mrbus_activity = MRBUS_ACTIVITY_RX;

	if (MRBUS_UART_SCR_A & MRBUS_RX_ERR_MASK)
	{
		// Handle framing errors - these are likely arbitration bytes
		mrbus_rx_index = MRBUS_UART_DATA;
		mrbus_rx_index = 0; // Reset receive buffer
	}
	else
	{
		mrbus_rx_input_buffer[mrbus_rx_index++] = MRBUS_UART_DATA;
		if ((mrbus_rx_index > 5) && ((mrbus_rx_index >= mrbus_rx_input_buffer[MRBUS_PKT_LEN]) || (mrbus_rx_index >= MRBUS_BUFFER_SIZE) ))
		{
			if (mrbus_rx_input_buffer[MRBUS_PKT_LEN] > MRBUS_BUFFER_SIZE) 
				mrbus_rx_input_buffer[MRBUS_PKT_LEN] = MRBUS_BUFFER_SIZE;

			if(false == packetBufferPush(&rxQueue, (uint8_t*)mrbus_rx_input_buffer, (uint8_t)mrbus_rx_input_buffer[MRBUS_PKT_LEN]))
				mrbus_state |=  MRBUS_RX_PKT_OVERFLOW;

			mrbus_activity = MRBUS_ACTIVITY_RX_COMPLETE;
		}

		// Prevent overflows of the rx_input buffer
		if (mrbus_rx_index >= MRBUS_BUFFER_SIZE)
			mrbus_rx_index = MRBUS_BUFFER_SIZE - 1;
	}
}

uint8_t MRBus::hasRxPackets()
{
	return(packetBufferDepth(&rxQueue));
}

uint8_t MRBus::hasTxPackets()
{
	return(packetBufferDepth(&txQueue));
}

void MRBus::setNodeAddress(uint8_t addr)
{
	this->nodeAddress = addr;
}

uint8_t MRBus::getNodeAddress()
{
	return(this->nodeAddress);
}

void MRBus::setNodePriority(uint8_t addr)
{
	this->nodeAddress = addr;
}

uint8_t MRBus::getNodePriority()
{
	return(this->nodeAddress);
}

bool MRBus::isTransmitting()
{
	return(0 != (mrbus_state & MRBUS_TX_BUF_ACTIVE));
}

const uint8_t MRBus_CRC16_HighTable[16] =
{
	0x00, 0xA0, 0xE0, 0x40, 0x60, 0xC0, 0x80, 0x20,
	0xC0, 0x60, 0x20, 0x80, 0xA0, 0x00, 0x40, 0xE0
};
const uint8_t MRBus_CRC16_LowTable[16] =
{
	0x00, 0x01, 0x03, 0x02, 0x07, 0x06, 0x04, 0x05,
	0x0E, 0x0F, 0x0D, 0x0C, 0x09, 0x08, 0x0A, 0x0B
};

uint16_t MRBus::crc16Update(uint16_t crc, uint8_t a)
{
	uint8_t t;
	uint8_t i = 0;

	uint8_t W;
	uint8_t crc16_high = (crc >> 8) & 0xFF;
	uint8_t crc16_low = crc & 0xFF;

	while (i < 2)
	{
		if (i)
		{
			W = ((crc16_high << 4) & 0xF0) | ((crc16_high >> 4) & 0x0F);
			W = W ^ a;
			W = W & 0x0F;
			t = W;
		}
		else
		{
			W = crc16_high;
			W = W ^ a;
			W = W & 0xF0;
			t = W;
			t = ((t << 4) & 0xF0) | ((t >> 4) & 0x0F);
		}

		crc16_high = crc16_high << 4; 
		crc16_high |= (crc16_low >> 4);
		crc16_low = crc16_low << 4;

		crc16_high = crc16_high ^ MRBus_CRC16_HighTable[t];
		crc16_low = crc16_low ^ MRBus_CRC16_LowTable[t];

		i++;
	}

	return ( ((crc16_high << 8) & 0xFF00) + crc16_low );
}

bool MRBus::queueTransmitPacket(uint8_t* pkt, uint8_t len)
{
	uint16_t crc16Value = 0;
	uint8_t i;
	if(len < MRBUS_PKT_TYPE || len > MRBUS_BUFFER_SIZE)
		return(false);

	pkt[MRBUS_PKT_LEN] = len;
	pkt[MRBUS_PKT_SRC] = this->nodeAddress;

	// Calculate checksum
	for (i = 0; i < pkt[MRBUS_PKT_LEN]; i++)
	{
		if ((i != MRBUS_PKT_CRC_H) && (i != MRBUS_PKT_CRC_L))
		{
			crc16Value = MRBus::crc16Update(crc16Value, pkt[i]);
		}
	}
	pkt[MRBUS_PKT_CRC_L] = (crc16Value & 0xFF);
	pkt[MRBUS_PKT_CRC_H] = ((crc16Value >> 8) & 0xFF);

	return(packetBufferPush(&txQueue, pkt, len));
}

bool MRBus::queueTransmitPacket(MRBusPacket &mrbPkt)
{
	return(this->queueTransmitPacket(mrbPkt.pkt, mrbPkt.pkt[MRBUS_PKT_LEN]));
}

bool MRBus::getReceivedPacket(uint8_t* pkt, uint8_t len)
{
	return(packetBufferPop(&rxQueue, pkt, len, false));
}

bool MRBus::getReceivedPacket(MRBusPacket &mrbPkt)
{
	return(packetBufferPop(&rxQueue, &mrbPkt));
}

void MRBus::begin()
{
	MRBUS_DDR |= _BV(MRBUS_TXE);
	MRBUS_PORT &= ~_BV(MRBUS_TXE);

	MRBUS_DDR &= ~_BV(MRBUS_RX);
	MRBUS_DDR &= ~_BV(MRBUS_TX);

	this->nodeAddress = 0x03;
	this->nodePriority = 6;
	
	mrbus_tx_index = 0;
	mrbus_rx_index = 0;
	mrbus_activity = MRBUS_ACTIVITY_IDLE;
	this->nodeLoneliness = 6;
	mrbus_state = 0;

#undef BAUD
#define BAUD MRBUS_BAUD
#include <util/setbaud.h>

#if defined( MRBUS_AT90_UART )
	// FIXME - probably need more stuff here
	UBRR = (uint8_t)UBRRL_VALUE;

#elif defined( MRBUS_ATMEGA_USART_SIMPLE )
    MRBUS_UART_UBRR = UBRR_VALUE;
	MRBUS_UART_SCR_A = (USE_2X)?_BV(U2X):0;
	MRBUS_UART_SCR_B = 0;
	MRBUS_UART_SCR_C = _BV(URSEL) | _BV(UCSZ1) | _BV(UCSZ0);
	
#elif defined( MRBUS_ATMEGA_USART0_SIMPLE )
    MRBUS_UART_UBRR = UBRR_VALUE;
	MRBUS_UART_SCR_A = (USE_2X)?_BV(U2X0):0;
	MRBUS_UART_SCR_B = 0;
	MRBUS_UART_SCR_C = _BV(URSEL0) | _BV(UCSZ01) | _BV(UCSZ00);
	
#elif defined( MRBUS_ATMEGA_USART ) || defined ( MRBUS_ATMEGA_USART0 )
	MRBUS_UART_UBRR = UBRR_VALUE;
	MRBUS_UART_SCR_A = (USE_2X)?_BV(U2X0):0;
	MRBUS_UART_SCR_B = 0;
	MRBUS_UART_SCR_C = _BV(UCSZ01) | _BV(UCSZ00);

#elif defined( MRBUS_ATTINY_USART )
	// Top four bits are reserved and must always be zero - see ATtiny2313 datasheet
	// Also, H and L must be written independently, since they're non-adjacent registers
	// on the attiny parts
	MRBUS_UART_UBRRH = 0x0F & UBRRH_VALUE;
	MRBUS_UART_UBRRL = UBRRL_VALUE;
	MRBUS_UART_SCR_A = (USE_2X)?_BV(U2X):0;
	MRBUS_UART_SCR_B = 0;
	MRBUS_UART_SCR_C = _BV(UCSZ1) | _BV(UCSZ0);

#elif defined ( MRBUS_ATMEGA_USART1 )
	MRBUS_UART_UBRR = UBRR_VALUE;
	MRBUS_UART_SCR_A = (USE_2X)?_BV(U2X1):0;
	MRBUS_UART_SCR_B = 0;
	MRBUS_UART_SCR_C = _BV(UCSZ11) | _BV(UCSZ10);
#endif

#undef BAUD

	/* Enable USART receiver and transmitter and receive complete interrupt */
	MRBUS_UART_SCR_B = _BV(MRBUS_RXCIE) | _BV(MRBUS_RXEN) | _BV(MRBUS_TXEN);

}

uint8_t MRBus::mrbusArbBitSend(uint8_t bitval)
{
	uint8_t slice;
	uint8_t tmp = 0;

	cli();
	if (bitval)
		MRBUS_PORT &= ~_BV(MRBUS_TXE);
	else
		MRBUS_PORT |= _BV(MRBUS_TXE);

	for (slice = 0; slice < 10; slice++)
	{
		if (slice > 2)
		{
			if (MRBUS_PIN & _BV(MRBUS_RX)) tmp = 1;
			if (tmp ^ bitval)
			{
				MRBUS_PORT &= ~_BV(MRBUS_TXE);
				MRBUS_DDR &= ~_BV(MRBUS_TX);
				return(1);
			}

		}
		_delay_us(20);
	}
	return(0);
}

#define MRBUS_TRANSMIT_SUCCESS       0
#define MRBUS_TRANSMIT_FAIL_NO_PKT   1
#define MRBUS_TRANSMIT_FAIL_BUSY     10
#define MRBUS_TRANSMIT_FAIL_LEN      11
#define MRBUS_TRANSMIT_FAIL_RX_FULL  12

uint8_t MRBus::transmitBlocking()
{
	uint8_t retval = this->transmit();
	
	while(MRBUS_TRANSMIT_SUCCESS != retval && MRBUS_TRANSMIT_FAIL_NO_PKT != retval)
	{
		// We want to wait 20ms before we try a retransmit
		// Because MRBus has a minimum packet size of 6 bytes @ 57.6kbps,
		// need to check roughly every millisecond to see if we have a new packet
		// so that we don't miss things we're receiving while waiting to transmit
		uint8_t bus_countdown = 20;
		while (bus_countdown-- > 0)
		{
			wdt_reset();
			_delay_ms(1);
			if (MRBUS_QUEUE_DEPTH != packetBufferDepth(&rxQueue))
				return(MRBUS_TRANSMIT_FAIL_RX_FULL);
		}
		retval = this->transmit();
	}
	return(retval);
}

uint8_t MRBus::transmit()
{
	uint8_t status;
	uint8_t i;
	uint8_t address = this->nodeAddress;

	if (0 == packetBufferDepth(&txQueue))
		return(MRBUS_TRANSMIT_FAIL_NO_PKT);

	// Bail if we're currently sending something
	if (mrbus_state & MRBUS_TX_BUF_ACTIVE)
		return(MRBUS_TRANSMIT_FAIL_BUSY);

	if (false == packetBufferPop(&txQueue, (uint8_t*)mrbus_tx_buffer, sizeof(mrbus_tx_buffer), true))
		return(MRBUS_TRANSMIT_FAIL_NO_PKT);

	// If we have no packet length, or it's less than the header, just silently say we transmitted it
	// On the AVRs, if you don't have any packet length, it'll never clear up on the interrupt routine
	// and you'll get stuck in indefinite transmit busy
	if (mrbus_tx_buffer[MRBUS_PKT_LEN] < MRBUS_PKT_TYPE)
	{
		mrbus_state &= ~(MRBUS_TX_BUF_ACTIVE | MRBUS_TX_PKT_READY);
		packetBufferPop(&txQueue);
		return(MRBUS_TRANSMIT_FAIL_LEN);
	}
	
	/* Start 2ms wait for activity */
	mrbus_activity = MRBUS_ACTIVITY_IDLE;
	_delay_ms(2);

	/* Return if activity - we may have a packet to receive */
	/* Application is responsible for waiting 10ms or for successful receive */
	if (MRBUS_ACTIVITY_IDLE != mrbus_activity)
	{
		if (this->nodeLoneliness)
			this->nodeLoneliness--;
		return(MRBUS_TRANSMIT_FAIL_BUSY);
	}

	/* Now go into critical timing loop */
	/* Note that status is abused to calculate bus wait */
	status = ((this->nodeLoneliness + this->nodePriority) * 10) + (address & 0x0F);

	MRBUS_PORT &= ~_BV(MRBUS_TXE);  /* Clear driver enable to prevent transmitting */
	MRBUS_PORT &= ~_BV(MRBUS_TX);   /* Set the XMIT pin low */
	MRBUS_DDR |= _BV(MRBUS_TX);     /* Set as output */

	//  Disable transmitter
	cli();
	MRBUS_UART_SCR_B &= ~_BV(MRBUS_TXEN);
	sei();
	
	// Be sure to reset RX index - normally this is reset by other arbitration bytes causing
	//  framing errors, but if we're talking to ourselves, we're screwed because the RX
	//  side of the uart isn't on during arbitration sending
	mrbus_rx_index = 0;

	for (i = 0; i < 44; i++)
	{
		_delay_us(10);
		if (0 == (MRBUS_PIN & _BV(MRBUS_RX)))
		{
			MRBUS_DDR &= ~_BV(MRBUS_TX);
			if (this->nodeLoneliness)
				this->nodeLoneliness--;
			return(MRBUS_TRANSMIT_FAIL_BUSY);
		}
	}

	/* Now, wait calculated time from above */
	for (i = 0; i < status; i++)
	{
		_delay_us(10);
		if (0 == (MRBUS_PIN & _BV(MRBUS_RX)))
		{
			MRBUS_DDR &= ~_BV(MRBUS_TX);
			if (this->nodeLoneliness)
				this->nodeLoneliness--;
			return(MRBUS_TRANSMIT_FAIL_BUSY);
		}
	}

	/* Arbitration Sequence - 4800 bps */
	/* Start Bit */
	if (this->mrbusArbBitSend(0))
	{
		if (this->nodeLoneliness)
			this->nodeLoneliness--;
		return(MRBUS_TRANSMIT_FAIL_BUSY);
	}

	for (i = 0; i < 8; i++)
	{
		status = this->mrbusArbBitSend(address & 0x01);
		address >>= 1;

		if (status)
		{
			if (this->nodeLoneliness)
				this->nodeLoneliness--;
			return(MRBUS_TRANSMIT_FAIL_BUSY);
		}
	}

	/* Stop Bits */
	if (this->mrbusArbBitSend(1) || this->mrbusArbBitSend(1))
	{
		if (this->nodeLoneliness)
			this->nodeLoneliness--;
		return(MRBUS_TRANSMIT_FAIL_BUSY);
	}

	/* Set TX back to input */
	MRBUS_DDR &= ~_BV(MRBUS_TX);
	/* Enable transmitter since control over bus is assumed */
	MRBUS_UART_SCR_B |= _BV(MRBUS_TXEN);
	MRBUS_PORT |= _BV(MRBUS_TXE);

	mrbus_tx_index = 0;

	mrbus_state |= MRBUS_TX_BUF_ACTIVE;
	mrbus_state &= ~MRBUS_TX_PKT_READY;
	
	cli();
#ifdef MRBUS_DISABLE_LOOPBACK
	// Disable receive interrupt while transmitting
	MRBUS_UART_SCR_B &= ~_BV(MRBUS_RXCIE);
#endif
	// Enable transmit interrupt
	MRBUS_UART_SCR_B |= _BV(MRBUS_UART_UDRIE);
	sei();
	
	// Actually remove the packet from the queue, now that we're sure transmit is a go
	packetBufferPop(&txQueue);
	this->nodeLoneliness = 6;
	return(MRBUS_TRANSMIT_SUCCESS);
}
