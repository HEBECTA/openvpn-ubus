#include "hash_table.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <syslog.h>

int init_hash_table(struct hash_table *ht, int capacity){

        struct node *array = (struct node *) malloc(sizeof(struct node) * capacity);

        if ( array == NULL )
                return ENOMEM;

        for (int i = 0; i < capacity; ++i){

                strcpy(array[i].name, EMPTY_NODE);
                array[i].bytes_received = 0;
                array[i].bytes_sent = 0;
                array[i].next_node = NULL;
        }

        ht->array = array;
        ht->capacity = capacity;

        return 0;
}

void free_hash_table(struct hash_table ht){

        for ( int i = 0; i < ht.capacity; ++i){

                free_linked_list(&(ht.array[i]));
        }

        free(ht.array);
}

static void free_linked_list(struct node *head){

        struct node *iterator = head->next_node;
        struct node *prev_node = iterator;

        if (iterator == NULL )
                return;

        while ( iterator->next_node != NULL ){

                prev_node = iterator;
                iterator = iterator->next_node;
                free(prev_node);
        }

        free(iterator);
}

int hash_function(const char *name, int capacity){

        int index = 0;

        for (int i = 0; i < strlen(name); ++i)
                index += name[i];
        
        return index % capacity;
}

int insert_client(struct hash_table *ht, const char *name, const char *real_address,
const char *addr_v4, const char *addr_v6, const char *bytes_received, const char *bytes_sent, const char *date){

        int index = hash_function(name, ht->capacity);

        struct node *node = NULL;

        // if array element is empty, just copy data to it
        if ( strcmp(ht->array[index].name, EMPTY_NODE) == 0 )
                node = &(ht->array[index]);

        // if array element is not empty add new node to chain (linked list)
        else {

                node = (struct node *) malloc(sizeof(struct node));

                if ( node == NULL )
                        return ENOMEM;

                node->next_node = NULL;

                struct node *list_iterator = &(ht->array[index]);

                while ( list_iterator->next_node != NULL )
                        list_iterator = list_iterator->next_node;

                list_iterator->next_node = node;
        }

        strncpy(node->name, name, CLIENT_NAME_SIZE);
        node->name[CLIENT_NAME_SIZE-1] = '\0';

        strncpy(node->real_address, real_address, CLIENT_REAL_ADDR_SIZE);
        node->real_address[CLIENT_REAL_ADDR_SIZE-1] = '\0';

        strncpy(node->virtual_address_v4, addr_v4, CLIENT_ADDR_V4_SIZE);
        node->virtual_address_v4[CLIENT_ADDR_V4_SIZE-1] = '\0';

        strncpy(node->virtual_address_v6, addr_v6, CLIENT_ADDR_V6_SIZE);
        node->virtual_address_v6[CLIENT_ADDR_V6_SIZE-1] = '\0';

        node->bytes_received = (unsigned int) atoi(bytes_received);

        node->bytes_sent = (unsigned int) atoi(bytes_sent);

        strncpy(node->connected_since, date, CLIENT_DATE_SIZE);
        node->connected_since[CLIENT_DATE_SIZE-1] = '\0';

        return 0;
}

struct node *get_client(struct hash_table *ht, const char *name){

        int index = hash_function(name, ht->capacity);

        if ( strcmp(ht->array[index].name, EMPTY_NODE) == 0 )
                return NULL;

        struct node *iterator = &(ht->array[index]);

        while ( iterator != NULL ){

                if (strcmp(iterator->name, name) == 0)
                        return iterator;

                iterator = iterator->next_node;
        }

        return NULL;
}

void update_client(struct node *client, const char *real_address,
const char *addr_v4, const char *addr_v6, const char *bytes_received, const char *bytes_sent, const char *date){

        strncpy(client->real_address, real_address, CLIENT_REAL_ADDR_SIZE);
        client->real_address[CLIENT_REAL_ADDR_SIZE-1] = '\0';

        strncpy(client->virtual_address_v4, addr_v4, CLIENT_ADDR_V4_SIZE);
        client->virtual_address_v4[CLIENT_ADDR_V4_SIZE-1] = '\0';

        strncpy(client->virtual_address_v6, addr_v6, CLIENT_ADDR_V6_SIZE);
        client->virtual_address_v6[CLIENT_ADDR_V6_SIZE-1] = '\0';

        client->bytes_received = (unsigned int) atoi(bytes_received);

        client->bytes_sent = (unsigned int) atoi(bytes_sent);

        strncpy(client->connected_since, date, CLIENT_DATE_SIZE);
        client->connected_since[CLIENT_DATE_SIZE-1] = '\0';
}

int delete_client(struct hash_table *ht, const char *name){

        int index = hash_function(name, ht->capacity);

        // element not found
        if ( strcmp(ht->array[index].name, EMPTY_NODE) == 0 )
                return ENODATA;

        // if first chain element is target, copy last element into first place
        if ( strcmp(ht->array[index].name, name) == 0 ){

                if ( ht->array[index].next_node == NULL)
                        strcpy(ht->array[index].name, EMPTY_NODE);

                else {

                        struct node *before_last_node = NULL;
                        struct node *last_node = get_last_prev_linked_list_nodes(&(ht->array[index]), &before_last_node);

                        strcpy(ht->array[index].name, last_node->name);
                        strcpy(ht->array[index].real_address, last_node->real_address);
                        strcpy(ht->array[index].virtual_address_v4, last_node->virtual_address_v4);
                        strcpy(ht->array[index].virtual_address_v6, last_node->virtual_address_v6);
                        ht->array[index].bytes_received = last_node->bytes_received;
                        ht->array[index].bytes_sent = last_node->bytes_sent;
                        strcpy(ht->array[index].connected_since, last_node->connected_since);

                        free(last_node);
                        if ( before_last_node != NULL )
                                before_last_node->next_node = NULL;
                }
          
                return 0;
        }

        return delete_element_from_linked_list(&(ht->array[index]), name);
}

static int delete_element_from_linked_list(struct node *head, const char *name){

        struct node *list_iterator = head;
        struct node *prev_node = list_iterator;
        struct node *next_node;

        // finds target element in the linked list
        while ( list_iterator->next_node != NULL && strcpy(list_iterator->name, name) != 0 ){

                prev_node = list_iterator;
                list_iterator = list_iterator->next_node;
        }

        // element not found
        if ( strcpy(list_iterator->name, name) != 0 )
                return ENODATA;

        // remove target element from linked list
        next_node = list_iterator->next_node;
        prev_node->next_node = next_node;
        free(list_iterator);

        return 0;
}

static struct node *get_last_prev_linked_list_nodes(struct node *head, struct node **before_last_node){

        while ( head->next_node != NULL ){

                *before_last_node = head;
                head =  head->next_node;
        }

        return head;
}

void init_hash_table_iterator(struct hash_table *ht, struct hash_table_iterator *it){

        it->ht = ht;
        it->current_index = 0;
        it->current_node = NULL;
}

int hash_table_return_next(struct hash_table_iterator *it){

        // find first element for empty iterator
        if ( it->current_node == NULL ) {

                int i = 0;
                for ( ; i < it->ht->capacity; ++i ){

                        it->current_node = &(it->ht->array[i]);

                        if ( strcmp(it->current_node->name, EMPTY_NODE) != 0){

                                it->current_index = i;

                                return 1;
                        }
                }

                return 0;
        }

        // find next element for none empty iterator
        else {

                // if chain is empty, find next element from array
                if ( it->current_node->next_node == NULL ){

                        int i = it->current_index + 1;

                        if ( i >= it->ht->capacity )
                                return 0;

                        it->current_node = &(it->ht->array[i]);

                        for ( ; i < it->ht->capacity; ++i ){

                                it->current_node = &(it->ht->array[i]);

                                if ( strcmp(it->current_node->name, EMPTY_NODE) != 0 ){

                                        it->current_index = i;

                                        return 1;
                                }
                        }
                               
                        return 0;
                }

                // get next element from chain
                it->current_node = (it->current_node->next_node);
        }

        return 1;
}
