#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>




typedef struct index_table index_table_t;


typedef struct index_element
{
	int data;
	index_table_t* next;
} index_element_t;


struct index_table
{
	index_element_t table[256];
};



typedef struct index
{
	index_table_t* root;
	int table_count;
	int word_count;
} index_t;

index_table_t* alloc_table ()
{
	index_table_t* t = malloc(sizeof(*t));
	if (!t) {
		printf("Can't alloc table!\n");
		return 0;
	}
	memset(t, 0, sizeof(*t));
	return t;
}

void init_index (index_t* index)
{
	index->root = alloc_table();
	if (index->root) {
		index->table_count = 1;
	} else {
		index->table_count = 0;
	}
	index->word_count = 0;
}


int new_table (index_t* index, index_element_t* element)
{
	index_table_t* t = alloc_table();
	if (t) {
		element->next = t;
		index->table_count++;
		return 1;
	}
	return 0;
}
int insert_table (index_t* index, index_table_t* table, uint8_t index_byte)
{
	return new_table(index, &(table->table[index_byte]));
}

void dbg_print_table (index_table_t* table)
{
	for (int i = 0; i < 256; i++) {
		printf("%03d: %d 0x%p\n", i, table->table[i].data, table->table[i].next);
	}
}

int insert_value (index_t* index, const char* str, int value)
{
	index_table_t* curr_table = index->root;
	int last_i = strlen(str) - 1;
	int i = 0;
	while(i < last_i) {
		if (curr_table->table[str[i]].next == 0) {
			if (!insert_table(index, curr_table, str[i])) {
				return 0;
			}
		}
		curr_table = curr_table->table[str[i]].next;
		i++;
	}
	if (!curr_table->table[str[i]].data) {
		curr_table->table[str[i]].data = value;
		index->word_count++;
		return 1;
	} else {
		printf("Duplicate!\n");
		return 0;
	}
}

void dbg_print_tree (index_table_t* table, const char* str)
{
	int len = strlen(str);
	char buffer[len+2];
	memcpy(buffer, str, strlen(str));
	buffer[len] = 0;
	buffer[len+1] = 0;

	for (int i = 0; i < 256; i++) {

		buffer[strlen(str)] = (char)i;

		if (table->table[i].data) {
			printf("(level %d, i=%d) \"%s\" = %d\n", len, i, buffer, table->table[i].data);
		}

		if (table->table[i].next) {
			dbg_print_tree(table->table[i].next, buffer);
		}
	}
}

void dbg_print_index (index_t* index)
{
	dbg_print_tree(index->root, "");
}

int index_lookup (index_t* index, const char* str)
{


}

void print_index_counters (index_t* index)
{
	printf("tables: %d, words: %d\n", index->table_count, index->word_count);
}

void dbg_insert (index_t* index, const char* str, int value)
{
	printf("\ninsert \"%s\"=%d\n", str, value);
	int result = insert_value(index, str, value);
	printf("result: %d\n", result);
	print_index_counters(index);
}

int main (int argc, const char* argv[])
{
	printf("\n");

	index_t index;

	init_index(&index);

	print_index_counters(&index);



	dbg_insert(&index, "a", 100);

	dbg_insert(&index, "a", 200);

	dbg_insert(&index, "x", 3);

	dbg_insert(&index, "B", 3);


	dbg_insert(&index, "2", 2);
	dbg_insert(&index, "28", 28);
	dbg_insert(&index, "29", 28);


	dbg_insert(&index, "qwhdererwtzxdhgdgh", 54876578);
	dbg_insert(&index, "qwhdererwtzxdhgdgd", 54876578);

	dbg_insert(&index, "qwhderer2tzxdhgdgh", 54876578);

	printf("\n");


	dbg_print_index(&index);






	printf("\n");
	return 0;
}

