#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>

#define P        2
#define C        2
#define N        4
#define BUFFER_ERROR (msg_t *) NULL

typedef struct msg {
 	void* content; // generico contenuto del messaggio
 	struct msg* (*msg_init)(void*); // creazione msg
 	void (*msg_destroy)(struct msg*); // deallocazione msg
  	struct msg* (*msg_copy)(struct msg*); // creazione/copia msg
} msg_t;

typedef struct buffer {
    int* T;
    int* D;
    msg_t* buff;
    size_t k;
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

pthread_mutex_t uso_t;
pthread_mutex_t uso_d;
pthread_cond_t non_pieno;
pthread_cond_t non_vuoto;
int T = 0;
int D = 0;

int main() {
	buffer_t* buffer = buffer_init(N);
	cond_init();
	msg_t* msg = msg_init("Ciao");
	msg_t* msg_w = put_bloccante(buffer, msg);
	msg_t* msg_r = get_bloccante(buffer);
	printf("Message write: %s\n", (char*) (msg_w->content));
	printf("D = %d\n", *buffer->D);
	printf("Message read: %s\n", (char*) (msg_r->content));
	printf("T = %d\n", *buffer->T);
	get_bloccante(buffer);
	buffer_destroy(buffer);
	return 0;
}

buffer_t* buffer_init(unsigned int maxsize) {
	buffer_t* buffer = (buffer_t*) malloc(sizeof(buffer_t));
	buffer->T = &T;
	buffer->D = &D;
	buffer->buff = (msg_t*) malloc(sizeof(msg_t) * maxsize);
	return buffer;  
}

void buffer_destroy(buffer_t* buffer) {
	free(buffer->buff);
	free(buffer);
}

msg_t* msg_init(void* content) {
	msg_t* new_msg = (msg_t*) malloc(sizeof(msg_t));
	new_msg->content = content;
	new_msg->msg_init = msg_init;
	new_msg->msg_destroy = msg_destroy;
	new_msg->msg_copy = msg_copy;
	return new_msg;
}

void msg_destroy(msg_t* msg) {
	free(msg->content);
	free(msg);
}

msg_t* msg_copy(msg_t* msg) {
	return msg->msg_init(msg->content);
}

void cond_init() {
	pthread_mutex_init(&uso_t, NULL);
	pthread_mutex_init(&uso_d, NULL);
	pthread_cond_init(&non_pieno, NULL);
	pthread_cond_init(&non_vuoto, NULL);
}

msg_t* put_bloccante(buffer_t* buffer, msg_t* msg) {
	pthread_mutex_lock(&uso_d);
	while(buffer->k == N)
		pthread_cond_wait(&non_pieno, &uso_d);
	int D = *buffer->D;
	buffer->buff[D] = *msg;
	*buffer->D = (D + 1) % N;
	++buffer->k;
	pthread_cond_signal(&non_vuoto);
	pthread_mutex_unlock(&uso_d);
	return msg;
}

msg_t* get_bloccante(buffer_t* buffer) {
	pthread_mutex_lock(&uso_t);
	while(buffer->k == 0)
		pthread_cond_wait(&non_vuoto, &uso_t);
	int T = *buffer->T;
	msg_t* msg_r = &buffer->buff[T];
	*buffer->T = (T + 1) % N;
	--buffer->k;
	pthread_cond_signal(&non_pieno);
	pthread_mutex_unlock(&uso_d);
	return msg_r;
}