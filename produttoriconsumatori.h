#ifndef PRODUTTORICONSUMATORI_H
#define PRODUTTORICONSUMATORI_H
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <time.h>

typedef struct msg {
 	void* content; // generico contenuto del messaggio
 	struct msg* (*msg_init)(void*); // creazione msg
 	void (*msg_destroy)(struct msg*); // deallocazione msg
  	struct msg* (*msg_copy)(struct msg*); // creazione/copia msg
} msg_t;

typedef struct buffer {
    int T; //indice di lettura
    int D; //indice di scrittura
    msg_t* buff; //buffer dei messaggi 
    size_t len; //numero di celle del buffer attualmente occupate
    size_t maxsize; //numero totale delle celle
} buffer_t;

buffer_t* buffer_init(unsigned int maxsize);

void buffer_destroy(buffer_t* buffer);

msg_t* msg_init(void* content);

void msg_destroy(msg_t* msg);

msg_t* msg_copy(msg_t* msg);

void cond_init();

msg_t* put_bloccante(buffer_t* buffer, msg_t* msg);

msg_t* put_non_bloccante(buffer_t* buffer, msg_t* msg);

msg_t* get_bloccante(buffer_t* buffer);

msg_t* get_non_bloccante(buffer_t* buffer);

void random_string(char*, size_t);

void* produttore_bloccante(void* arg);

void* consumatore_bloccante(void* arg);

void* produttore_non_bloccante(void* arg);

void* consumatore_non_bloccante(void* arg);

void* produttore_bloccante_ug(void* arg);

void* consumatore_bloccante_ug(void* arg);

void* produttore_non_bloccante_ug(void* arg);

void* consumatore_non_bloccante_ug(void* arg);
#endif