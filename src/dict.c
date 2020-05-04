#include "dict.h"
#include <string.h>
#include <stdlib.h>
#include "stdio.h"

#define MAX_BUCKET 64
#define INITIAL_SIZE 16

typedef struct Node
{
    struct Node *next;
    char *key;
    char *value;
} Node;

typedef struct Bucket{
    struct Node *first;
    int size;
}Bucket;

typedef struct Dict
{
    struct Bucket *buckets;
    int num_buckets;
    int size;

} Dict;

void dict_rehash(Dict *d);

void dict_add_node(Dict *d, Node *node);

void node_free(Node *node);

Node *node_malloc(char *key, char*value);

void bucket_free(Bucket *bucket);

int str_hash(char *input);

Dict *dict_new(){
    //Initializes the dictionary
    Dict *d = malloc(sizeof(Dict));
    d->size = 0;
    d->num_buckets = INITIAL_SIZE;

    d->buckets = malloc(sizeof(Bucket)*d->num_buckets);
    memset(d->buckets, 0, sizeof(Bucket)*d->num_buckets); // Zeros the memory
    return d;
}

void dict_add(Dict *d, char *key, char *value){
    int hash = str_hash(key);
    int n = hash % d->num_buckets;

    // Replaces the values if the key already exists
    Node **curr = &(d->buckets[n].first);
    for(; NULL != *curr; curr = &(*curr)->next){
        if(0 == strcmp((*curr)->key, key)){
        	if((*curr)->value != value){
				free((*curr)->value);
				(*curr)->value = strdup(value);
        	}
            return;
        }
    }

    // If this insertion will result in overfilling a bucket
    if(d->buckets[n].size +1 >= MAX_BUCKET){
        dict_rehash(d);
    }

    // The key does not yet exist
    dict_add_node(d, node_malloc(key, value));
}

void dict_rehash(Dict *d){
    Dict d_new;
    d_new.num_buckets = d->num_buckets*2;
    d_new.buckets = malloc(sizeof(Bucket)*d_new.num_buckets);
	memset(d_new.buckets, 0, sizeof(Bucket)*d_new.num_buckets);
	d_new.size = 0;

    for(int i = 0; i < d->num_buckets; i++){
        Node *curr = d->buckets[i].first;
        while (NULL != curr){
            Node *next = curr->next;
            curr->next = NULL;
            dict_add_node(&d_new, curr);
            curr = next;
        }
    }

    free(d->buckets);
    d->buckets = d_new.buckets;
    d->num_buckets = d_new.num_buckets;
}

void dict_add_node(Dict *d, Node *node){
    int hash = str_hash(node->key);
    int n = hash % d->num_buckets;

    d->size++;
    d->buckets[n].size++;
    node->next = d->buckets[n].first;
    d->buckets[n].first = node;
}

char *dict_get(Dict *d, char *key){
    int hash = str_hash(key);
    int n = hash % d->num_buckets;

    Node *curr = d->buckets[n].first;
    while (curr != NULL ){
        if (0 == strcmp(curr->key, key)){
            return curr->value;
        }
        curr = curr->next;
    }
    return NULL;
}

int dict_contains(Dict *d, char *key){
    return dict_get(d, key) != NULL;
}

void dict_print_all(Dict *d){
	for(int i = 0; i < d->num_buckets; i++){
		Node *curr = d->buckets[i].first;
		while (curr != NULL){
			printf("%s=%s\n", curr->key, curr->value);
			curr = curr->next;
		}
	}
}

int dict_remove(Dict *d, char *key){
    int hash = str_hash(key);
    int n = hash % d->num_buckets;
    d->size--;

    Node **curr = &d->buckets[n].first; // = &d->buckets[n]
    Node *temp;
    for(;NULL != *curr;curr = &(*curr)->next){
        if(0 == strcmp((*curr)->key, key)){
            temp = *curr;
            *curr = (*curr)->next;
            node_free(temp);
            return 1;
        }
    }
    return 0;
}

void dict_free(Dict *d){
    for(int i = 0; i < d->num_buckets; i++){
        bucket_free(&d->buckets[i]);
    }
    free(d->buckets);
    free(d);
}

void node_free(Node *node){
    free(node->key);
    free(node->value);
    free(node);
}

Node *node_malloc(char *key, char*value){
    Node *node = malloc(sizeof(Node));
    node->key = strdup(key);// Allocates memory and then copies key to it
    node->value = strdup(value);
    return node;
}

void bucket_free(Bucket *bucket){
    while (NULL != bucket->first){
        Node *temp = bucket->first;
        bucket->first = bucket->first->next;
        node_free(temp);
    }
}

int str_hash(char *input){
    // Arbitrary prime starting point
    int hash = 11;

    // Multiply each char value by 29, an arbitrary prime
    while (*input != 0){
        hash = (*input)*29 + hash;
        input++;
    }
    return hash;
}