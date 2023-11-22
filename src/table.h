#ifndef TABLE_H
#define TABLE_H

#include <stdlib.h>
#include <stdbool.h>
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

typedef struct {
	Table* table;
	uint32_t row_num;
	bool end_of_table;
} Cursor;

Table* open_db(const char*);
void db_close(Table*);
Cursor* table_start(Table*);
Cursor* table_end(Table*);
void* cursor_value(Cursor*);
void cursor_advance(Cursor*);
Pager* pager_open(const char*);
void pager_flush(Pager*, uint32_t, uint32_t);
void* get_page(Pager*, uint32_t);

#endif
