#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "input_buffer.h"

InputBuffer* new_input_buffer(){
	InputBuffer* input_buffer = (InputBuffer*)malloc(sizeof(InputBuffer));
	input_buffer->buffer = NULL;
	input_buffer->b_length = 0;
	input_buffer->input_length = 0;

	return input_buffer;
}

/* Read input from the repl command line */
void read_input(InputBuffer* input_buffer){
	ssize_t bytes_read = getline(&(input_buffer->buffer), &(input_buffer->b_length), stdin);
	
	if (bytes_read <= 0){
		printf("Erorr reading input !\n");
		exit(EXIT_FAILURE);
	}

	input_buffer->input_length = bytes_read - 1;
	input_buffer->buffer[bytes_read -1] = '\0';
}

void close_input_buffer(InputBuffer* input_buffer){
	free(input_buffer->buffer);
	free(input_buffer);
}
