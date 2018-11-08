#pragma once
#include "../Project.h"

circular_buffer g_sample_buffer[3];
usb_8bit_buffer g_usb_8bit_buffer;

#pragma DATA_SECTION(data_sectionA, "ramgs16");
#ifndef _SINGLE_CH_MODE
#pragma DATA_SECTION(data_sectionB, "ramgs17");
#pragma DATA_SECTION(data_sectionC, "ramgs18");
#endif

// see below why those exist
int32_t data_sectionA[SAMPLE_BUFFER_LENGTH];

#ifndef _SINGLE_CH_MODE
int32_t data_sectionB[SAMPLE_BUFFER_LENGTH];
int32_t data_sectionC[SAMPLE_BUFFER_LENGTH];
#endif
uint8_t data_sectionD[USB_8BIT_BUFFER_SIZE+33];


void InitBuffers()
{
	cb_init(&g_sample_buffer[0], SAMPLE_BUFFER_LENGTH, data_sectionA);
#ifndef _SINGLE_CH_MODE
	cb_init(&g_sample_buffer[1], SAMPLE_BUFFER_LENGTH, data_sectionB);
	cb_init(&g_sample_buffer[2], SAMPLE_BUFFER_LENGTH, data_sectionC);
#endif

	usb_8bit_buf_init(&g_usb_8bit_buffer, USB_8BIT_BUFFER_SIZE, data_sectionD);
}

///////////////////////////////////////////////////////////////////////////////

void cb_init(circular_buffer *cb, size_t capacity, int32_t * data_section)
{
	cb->buffer = data_section;
	cb->sz = 4;
	cb->buffer_end = cb->buffer + capacity +1;
	cb->capacity = capacity;
	cb->count = 0;
	cb->head = cb->buffer;
	cb->tail = cb->buffer;
	cb->usb_tail = cb->buffer;
}

///////////////////////////////////////////////////////////////////////////////

void cb_flush(circular_buffer *cb)
{
	cb->count = 0;
	cb->head = cb->buffer;
	cb->tail = cb->buffer;
	cb->usb_tail = cb->buffer;
}

///////////////////////////////////////////////////////////////////////////////

void usb_8bit_buf_init(usb_8bit_buffer * buf, size_t capacity, uint8_t * data_section)
{
	buf->buffer = data_section;
	buf->count = capacity;
	buf->sz = 1;
}

///////////////////////////////////////////////////////////////////////////////
// add value to buffer
#ifdef _FLASH
	__attribute__((ramfunc))	// load the following function to ram for faster execution
#endif
void cb_push_back(circular_buffer * cb, const int32_t * item)
{
    if(cb->count == cb->capacity)
    	cb_pointer_increment(cb, &cb->tail);
    else
    	cb->count++;

    // add item and increase head by one
	memcpy(cb->head, item, sizeof(item));
	cb_pointer_increment(cb, &cb->head);

	// check if the other pointers have to be incremented as well
	if(cb->tail == cb->head)
		cb_pointer_increment(cb, &cb->tail);

	if(cb->usb_tail == cb->head)
		cb_pointer_increment(cb, &cb->usb_tail);

}

///////////////////////////////////////////////////////////////////////////////
// return the value in the buffer at position [index].
// Warning: this function does not check if the value has been initialized yet (index > count)
int32_t	cb_at(const circular_buffer * cb, int32_t index)
{
	if (cb->tail + index >= cb->buffer_end)
	{
		int32_t delta = cb->head - cb->buffer-1;
		return cb->buffer[index - delta];

	}
	else
	{
		return cb->tail[index-1];
	}
}

///////////////////////////////////////////////////////////////////////////////
// add an array to the buffer
#ifdef _FLASH
	__attribute__((ramfunc))	// load the following function to ram for faster execution
#endif
void cb_push_back_arr(circular_buffer * cb, int32_t * arr, int length)
{
	int i;
	for(i = 0; i<length; i++)
	{
		cb_push_back(cb, &arr[i]);
	}
}

///////////////////////////////////////////////////////////////////////////////
// convenience function to shift the pointer of a buffer by one and check for wrap around
#ifdef _FLASH
	__attribute__((ramfunc))	// load the following function to ram for faster execution
#endif
void cb_pointer_increment(circular_buffer *cb, int32_t ** p)
{
	(*p)++; // shift pointer by 1

	if((*p) >= cb->buffer_end-1)	// check for wrap-around
	{
		(*p) = cb->buffer;
	}
}

///////////////////////////////////////////////////////////////////////////////
// shifts cb_pointer p by a given amount of bytes and handles wrap around accordingly
#ifdef _FLASH
	__attribute__((ramfunc))	// load the following function to ram for faster execution
#endif
void cb_pointer_shift(circular_buffer *cb, int32_t ** p, int32 bytes)
{
	bytes /= 4;
	if((*p) + bytes < cb->buffer_end-1)
	{
		(*p) += bytes;
	}
	else
	{
		(*p) = cb->buffer + ( bytes- (cb->buffer_end - (*p) ));
	}
}

///////////////////////////////////////////////////////////////////////////////

#ifdef _FLASH
	__attribute__((ramfunc))	// load the following function to ram for faster execution
#endif
uint32_t cb_uploadtoUSB(circular_buffer * cb)
{
 	uint8_t flags = 0;


	// if there is nothing to transmit send empty header
	if(cb->usb_tail == cb->head)
	{
		const uint8_t header[2] = {0,0};
		USBBufferWrite(&g_sTxBuffer, header, 2);
		return 0;
	}

	// evaluate message length
	uint32_t ui32ByteCount;
	if(cb->head > cb->usb_tail)
	{
		ui32ByteCount = (cb->head - cb->usb_tail)*4;
	}
	else
	{
		ui32ByteCount = (cb->buffer_end - cb->usb_tail)*4;
		flags |= SPLIT_PACKET_FLAG;
	}


	if(ui32ByteCount > g_usb_8bit_buffer.count)
	{
		ui32ByteCount = g_usb_8bit_buffer.count;
		flags |= SPLIT_PACKET_FLAG;
	}


	// send and update
	uint32_t ui32BytesWritten = SendPackage(&g_usb_8bit_buffer, cb->usb_tail-1, ui32ByteCount, flags);
	cb_pointer_shift(cb, &cb->usb_tail, ui32BytesWritten);

	return ui32BytesWritten;
}

///////////////////////////////////////////////////////////////////////////////

// creates a header and the data package for the transfer
#ifdef _FLASH
	__attribute__((ramfunc))	// load the following function to ram for faster execution
#endif
uint32_t SendPackage(usb_8bit_buffer * buf, const int32_t * src, uint32_t byte_count, uint8_t flags)
{
	if(byte_count > buf->count)
		return 0;	// error

	// assemble header
	uint8_t header[2] = { 0,0 };
	header[0] = (uint8_t)(byte_count/4 >> 8);
	header[1] = (uint8_t)(byte_count/4);
	header[0] |= flags;

	// organize data into 8bit blocks - MSB first
	// we treat our 32 bit pointer as 8 bit and copy the contents into an 8 bit array. then we send it to the USB buffer

	int i;
	for(i = 0; i< (int)byte_count/4; i++)
	{
		buf->buffer[4*i]   = (src[i] >> 24);
		buf->buffer[4*i+1] = (src[i] >> 16);
		buf->buffer[4*i+2] = (src[i] >> 8);
		buf->buffer[4*i+3] = src[i];
	}
	// send package
	USBBufferWrite(&g_sTxBuffer, header, 2);
	return USBBufferWrite(&g_sTxBuffer, buf->buffer, byte_count);
}
