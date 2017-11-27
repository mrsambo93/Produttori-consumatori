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
    int T;
    int D;
    msg_t* buff;
    size_t len;
} buffer_t;

/* allocazione / deallocazione buffer */
// creazione di un buffer vuoto di dim. max nota
buffer_t* buffer_init(unsigned int maxsize);
// deallocazione di un buffer
void buffer_destroy(buffer_t* buffer);
/* operazioni sul buffer */

msg_t* msg_init(void* content);

void msg_destroy(msg_t* msg);

msg_t* msg_copy(msg_t* msg);

void cond_init();

// inserimento bloccante: sospende se pieno, quindi
// effettua l’inserimento non appena si libera dello spazio
// restituisce il messaggio inserito; N.B.: msg!=null
msg_t* put_bloccante(buffer_t* buffer, msg_t* msg);
// inserimento non bloccante: restituisce BUFFER_ERROR se pieno,
// altrimenti effettua l’inserimento e restituisce il messaggio
// inserito; N.B.: msg!=null
msg_t* put_non_bloccante(buffer_t* buffer, msg_t* msg);
// estrazione bloccante: sospende se vuoto, quindi
// restituisce il valore estratto non appena disponibile
msg_t* get_bloccante(buffer_t* buffer);
// estrazione non bloccante: restituisce BUFFER_ERROR se vuoto
// ed il valore estratto in caso contrario
msg_t* get_non_bloccante(buffer_t* buffer);

void random_string(char*, size_t);

void* produttore_bloccante(void* arg);

void* consumatore_bloccante(void* arg);

void* produttore_non_bloccante(void* arg);

void* consumatore_non_bloccante(void* arg);
#endif