#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#define EMPTY_NODE ""

#define CLIENT_NAME_SIZE 50
#define CLIENT_REAL_ADDR_SIZE 45
#define CLIENT_ADDR_V4_SIZE 17
#define CLIENT_ADDR_V6_SIZE 40
#define CLIENT_DATE_SIZE 20

struct node {

char name[CLIENT_NAME_SIZE];
char real_address[CLIENT_REAL_ADDR_SIZE];
char virtual_address_v4[CLIENT_ADDR_V4_SIZE];
char virtual_address_v6[CLIENT_ADDR_V6_SIZE];
unsigned int bytes_received;
unsigned int bytes_sent;
char connected_since[CLIENT_DATE_SIZE];

struct node *next_node;
};

struct hash_table {

        struct node *array;
        int capacity;
};

int init_hash_table(struct hash_table *ht, int capacity);

void free_hash_table(struct hash_table ht);

static void free_linked_list(struct node *head);

int hash_function(const char *name, int capacity);

int insert_client(struct hash_table *ht, const char *name, const char *real_address,
const char *addr_v4, const char *addr_v6, const char *bytes_received, const char *bytes_sent, const char *date);

struct node *get_client(struct hash_table *ht, const char *name);

void update_client(struct node *client, const char *real_address,
const char *addr_v4, const char *addr_v6, const char *bytes_received, const char *bytes_sent, const char *date);

int delete_client(struct hash_table *ht, const char *name);

static int delete_element_from_linked_list(struct node *head, const char *name);

static struct node *get_last_prev_linked_list_nodes(struct node *head, struct node **before_last_node);

struct hash_table_iterator {

        struct hash_table *ht;
        int current_index;
        struct node *current_node;
};

void init_hash_table_iterator(struct hash_table *ht, struct hash_table_iterator *it);

int hash_table_return_next(struct hash_table_iterator *it);

#endif