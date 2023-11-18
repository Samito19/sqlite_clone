#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include "table.h"

#define MAX_TABLE_PAGES 100

Table* open_db(const char* filename){
	Table* table = (Table*)malloc(sizeof(Table));
	Pager* pager= pager_open(filename);
	uint32_t num_rows = pager->file_length / ROW_SIZE;
	table->pager = pager;
	table->num_rows = num_rows;

	return table;
}	

void db_close(Table* table){
	Pager* pager = table->pager;
	uint32_t num_full_pages = table->num_rows / ROWS_PER_PAGE;

	for (uint32_t i = 0; i < num_full_pages; i++){
		if (pager->pages[i] == NULL){
			continue;
		}
		pager_flush(pager, i, PAGE_SIZE);
		free(pager->pages[i]);
		pager->pages[i] = NULL;
	}

	uint32_t num_additional_rows = table->num_rows % ROW_SIZE;
	if (num_additional_rows > 0){
		uint32_t page_num = num_full_pages;
		if (pager->pages[page_num] != NULL){
			pager_flush(pager, page_num, num_additional_rows * ROW_SIZE);
			free(pager->pages[page_num]);
			pager->pages[page_num] = NULL;
		}
	}

	int result = close(pager->file_descriptor);
	if (result == -1){
		printf("Error closing the db file !\n");
		exit(EXIT_FAILURE);
	}

	for (uint32_t i = 0; i < MAX_TABLE_PAGES; i ++){
		void* page = pager->pages[i];
		if (page){
			free(page);
			pager->pages[i] = NULL;
		}
	}
	
	free(pager);
	free(table);
}

/* This function allows us to find the slot of a row in a table by calculating the offset */
void* find_row_slot(Table* table, uint32_t row_num){
	/*Calculate the next non-full page available and store its memory address */
	uint32_t page_num = row_num / ROWS_PER_PAGE;
	void* page = get_page(table->pager, page_num);
	uint32_t row_offset = row_num % ROWS_PER_PAGE;
	uint32_t byte_offset = row_offset * ROW_SIZE;

	return page + byte_offset;
}

/* We use a pager to abstract the process of retrieving data. 
 * The virtual machine always assumes that the data it wants is in the memory.
 * If the row is not already cached in memory, the pager subtly take care of retrieving the data from the db file
 * and return */
Pager* pager_open(const char* filename){
	/* Attempt to open the file and retrived the file descriptor, if it does not exist create it */
	int fd = open(filename, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);
	if (fd == -1){
		printf("Unable to open the db file\n");
		exit(EXIT_FAILURE);
	}
	/* Calculate the length of the file in bytes */
	off_t file_length = lseek(fd, 0, SEEK_END);

	Pager* pager = (Pager*)malloc(sizeof(Pager));
	pager->file_descriptor = fd;
	pager->file_length = file_length;

	for (uint32_t i = 0; i < MAX_TABLE_PAGES; i++) {
		pager->pages[i] = NULL;
	}
	
	return pager;
}

void pager_flush(Pager* pager, uint32_t page_num, uint32_t size){
	if (pager->pages[page_num] == NULL){
		printf("Tried to flush a null page !\n");
		exit(EXIT_FAILURE);
	}

	off_t offset = lseek(pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET);
	if (offset == -1){
		printf("Error seeking in file !\n");
		exit(EXIT_FAILURE);
	}

	ssize_t bytes_written = write(pager->file_descriptor, pager->pages[page_num], size);
	if (bytes_written == -1){
		printf("Failed to write data to file !\n");
		exit(EXIT_FAILURE);
	}
}


void* get_page(Pager* pager, uint32_t page_num) {
	if (page_num > MAX_TABLE_PAGES) {
		printf("Tried to fetch page number out of bounds. %d > %d\n", page_num, MAX_TABLE_PAGES);
	    	exit(EXIT_FAILURE);
  	}

	if (pager->pages[page_num] == NULL) {
    		// Cache miss. Allocate memory and load from file.
		void* page = malloc(PAGE_SIZE);
    		uint32_t num_pages = pager->file_length / PAGE_SIZE;

    		// We might save a partial page at the end of the file
		if (pager->file_length % PAGE_SIZE) {
      			num_pages += 1;
    		}

    		if (page_num <= num_pages) {
	      		lseek(pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET);
		      	ssize_t bytes_read = read(pager->file_descriptor, page, PAGE_SIZE);
		      	if (bytes_read == -1) {
				printf("Error reading file: %d\n", errno);
				exit(EXIT_FAILURE);
		      	}
		}

    		pager->pages[page_num] = page;
  	}
	return pager->pages[page_num];
}
