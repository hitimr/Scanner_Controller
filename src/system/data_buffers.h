#pragma once
#include "../Project.h"

#define OVERFLOW_FLAG 		0x80
#define SPLIT_PACKET_FLAG 	0x40

#ifdef _SINGLE_CH_MODE
#define SAMPLE_BUFFFER_CNT	1
#else
#define SAMPLE_BUFFFER_CNT	3
#endif



// prototypes for the data-buffers
// their memory location has been predefined in the linker command file
extern int32_t data_sectionA[];
#ifndef _SINGLE_CH_MODE
extern int32_t data_sectionB[];
extern int32_t data_sectionC[];
#endif
extern uint8_t data_sectionD[];


// A simple Buffer where the Measured Values are stored
// Initially I was trying to implement some special structure that also handles the 8bit-limitaiton of the USB interface
// However it turns out that a simple all-purpose circular buffer is a better choice and we handle USB seperatly..

typedef struct circular_buffer
{
	size_t 		capacity;			// maximum number of samples
	size_t  	count;				// hold number of 32bit samples
	size_t 		sz;
	int32_t *	buffer;				// points to the beginning of the data-array
	int32_t *	buffer_end;			// end of data buffer
	int32_t *	head;				// latest value in buffer
	int32_t *	tail;
	int32_t *	usb_tail;			// first value for USB transmission

}circular_buffer;


typedef struct usb_8bit_buffer
{
	uint8_t * 	buffer;
	size_t 		count;
	size_t 		sz;
}usb_8bit_buffer;


void 		cb_init(circular_buffer *, size_t, int32_t *);
void		usb_8bit_buf_init();
void		InitBuffers();
void 		cb_flush(circular_buffer * cb);
void 		cb_push_back(circular_buffer * cb, const int32_t * item);
int32_t		cb_at(const circular_buffer * cb, int32_t index);
void 		cb_pointer_increment(circular_buffer * cb, int32_t ** p);
void		cb_pointer_shift(circular_buffer *cb, int32_t ** p, int32 bytes);
void 		cb_push_back_arr(circular_buffer * cb, int32_t * arr, int length);
uint32_t	SendPackage(usb_8bit_buffer * buf, const int32_t * src, uint32_t byte_count, uint8_t flags);
uint32_t	cb_uploadtoUSB(circular_buffer * cb);




