#include <stdio.h>
#include <string.h>
#include "statements.h"
#include "table.h"
#include "serializer.h"

/* Helper function to extract the data from the input buffer and store as a statement structure */
PrepareStatementResult prepare_statement(InputBuffer* input_buffer, Statement* statement){
	if (strncmp(input_buffer->buffer, "insert", 6) == 0){
		return prepare_insert(input_buffer, statement);
	}
	else if (strcmp(input_buffer->buffer, "select") == 0){
		statement->type = STATEMENT_SELECT;
		return PREPARE_SUCCESS;
	} else {
		return PREPARE_UNRECOGNIZED_STATEMENT;
	}
}

/* This function uses strtok to retrieve tokens from an insert statement and also handles syntactic & buffer overflow errors */
PrepareStatementResult prepare_insert(InputBuffer* input_buffer, Statement* statement){
	char* keyword = strtok(input_buffer->buffer, " ");
	char* id_string = strtok(NULL, " ");
	char* username = strtok(NULL, " ");
	char* email = strtok(NULL, " ");

	if (id_string == NULL || username == NULL || email == NULL){
		return PREPARE_SYNTAX_ERROR;
	}
	
	if (strlen(username) > COLUMN_USERNAME_SIZE || strlen(email) > COLUMN_EMAIL_SIZE){
		return PREPARE_COLUMN_SIZE_EXCEEDED;
	}
	
	statement->type = STATEMENT_INSERT;

	Row* row_to_insert = &(statement->row_to_insert);
	row_to_insert->id = atoi(id_string);
	strcpy(row_to_insert->username, username);
	strcpy(row_to_insert->email, email);
	
	return PREPARE_SUCCESS;
}

ExecuteResult exec_statement(Statement* statement, Table* table){
	switch (statement->type){
		case (STATEMENT_INSERT):
			return exec_insert(statement, table);
			break;
		case (STATEMENT_SELECT):
			return exec_select(table);
			break;
		}
}

ExecuteResult exec_insert(Statement* statement, Table* table){
	if (table->num_rows >= MAX_TABLE_ROWS){
		return EXECUTE_TABLE_FULL;
	}

	/* Store the row in memory */
	Row* row_to_insert = &(statement->row_to_insert);
	serialize_row(row_to_insert, find_row_slot(table, table->num_rows));
	table->num_rows += 1;

	return EXECUTE_SUCCESS;
}

void print_row(Row* row){
	printf("(%d %s %s)\n", row->id, row->username, row->email);
}

ExecuteResult exec_select(Table* table){
	Row row;

	/* Retrieve and deserialize every row from memory */
	for (int row_num = 0; row_num < table->num_rows; row_num++){
		deserialize_row(find_row_slot(table, row_num), &row);
		print_row(&row);
	}

	if (table->num_rows == 0){
		printf("The table is empty.\n");
	}

	return EXECUTE_SUCCESS;
}
