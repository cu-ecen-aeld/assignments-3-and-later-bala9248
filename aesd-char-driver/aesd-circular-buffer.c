/**
 * @file aesd-circular-buffer.c
 * @brief Functions and data related to a circular buffer imlementation
 *
 * @author Dan Walkes
 * @date 2020-03-01
 * @copyright Copyright (c) 2020
 *
 */

#ifdef __KERNEL__
#include <linux/string.h>
#else
#include <string.h>
#include <stdio.h>
#endif

#include "aesd-circular-buffer.h"

#define TRUE  (1)
#define FALSE (0)
/**
 * @param buffer the buffer to search for corresponding offset.  Any necessary locking must be performed by caller.
 * @param char_offset the position to search for in the buffer list, describing the zero referenced
 *      character index if all buffer strings were concatenated end to end
 * @param entry_offset_byte_rtn is a pointer specifying a location to store the byte of the returned aesd_buffer_entry
 *      buffptr member corresponding to char_offset.  This value is only set when a matching char_offset is found
 *      in aesd_buffer. 
 * @return the struct aesd_buffer_entry structure representing the position described by char_offset, or
 * NULL if this position is not available in the buffer (not enough data is written).
 */
struct aesd_buffer_entry *aesd_circular_buffer_find_entry_offset_for_fpos(struct aesd_circular_buffer *buffer,
			size_t char_offset, size_t *entry_offset_byte_rtn ) {
	
	if ( (buffer->in_offs == buffer->out_offs && buffer->full == FALSE) || buffer == NULL)
		return NULL; //Buffer is empty
	
	uint8_t read_pos = buffer->out_offs;  //The position to start reading from
	int count = 0;
	
	bool flag = FALSE; //Flag to check if entry was found
	while (flag != TRUE) { 
		
		//Check to see if char_offset would be in the current buffer read (out) position
		if ( char_offset < buffer->entry[read_pos].size ) {
			*entry_offset_byte_rtn = char_offset; 
			return (&buffer->entry[read_pos]);
		}
		
		
		//If char_offset is not in the entry, must update offset and check next buffer element
		char_offset -= buffer->entry[read_pos].size;
		
		read_pos ++; //Increment and wrap around if needed
		if (read_pos == AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED)
			read_pos = 0;
			
		count++; //Increase the count to check if entry is present
		if (count >= AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED) 
			flag = TRUE;
	}
	
	return NULL;
}

/**
* Adds entry @param add_entry to @param buffer in the location specified in buffer->in_offs.
* If the buffer was already full, overwrites the oldest entry and advances buffer->out_offs to the
* new start location.
* Any necessary locking must be handled by the caller
* Any memory referenced in @param add_entry must be allocated by and/or must have a lifetime managed by the caller.
*/
void aesd_circular_buffer_add_entry(struct aesd_circular_buffer *buffer, const struct aesd_buffer_entry *add_entry)
{
	
	if(buffer == NULL)//Check to see if buffer is valid
		return;

	if (buffer->full == TRUE) //If buffer is full, update out position and discard oldest element
		buffer->out_offs ++;
	if (buffer->out_offs == AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED) //Wrap around if needed
		buffer->out_offs  = 0;
		
		
	buffer->entry[buffer->in_offs] = *add_entry; //Add entry to current in position
	buffer->in_offs ++;//Increment in position
	
	//Wrap around if needed
	if (buffer->in_offs == AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED)
		buffer->in_offs  = 0;

	//Check to see if buffer is full
	if (buffer->in_offs == buffer->out_offs)
		buffer->full = TRUE;		
}

/**
* Initializes the circular buffer described by @param buffer to an empty struct
*/
void aesd_circular_buffer_init(struct aesd_circular_buffer *buffer)
{
    memset(buffer,0,sizeof(struct aesd_circular_buffer));
}
