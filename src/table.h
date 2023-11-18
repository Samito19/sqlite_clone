#ifndef TABLE_H
#define TABLE_H

#include <stdlib.h>
#include "serializer.h"

typedef struct {
	int file_descriptor;
	uint32_t file_length;
	void* pages[MAX_TABLE_PAGES];
} Pager;
	
typedef struct {
	uint32_t num_rows;
	Pager* pager;
} Table;

Table* open_db(const char*);
void db_close(Table*);
void* find_row_slot(Table*, uint32_t);
Pager* pager_open(const char*);
void pager_flush(Pager*, uint32_t, uint32_t);
void* get_page(Pager*, uint32_t);

#endif
