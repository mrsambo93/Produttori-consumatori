#include "produttoriconsumatori.h"

#define N        4
#define BUFFER_ERROR (msg_t *) NULL

pthread_mutex_t uso_b;
pthread_cond_t non_pieno;
pthread_cond_t non_vuoto;

//main non utilizzato
/*
int main() {
	srand(time(NULL));
	buffer_t* buffer = buffer_init(N);
	cond_init();
	pthread_t prod1;
	pthread_t prod2;
	pthread_t cons1;
	pthread_t cons2;
	pthread_create(&prod1, NULL, produttore_non_bloccante, (void*) buffer);
	pthread_create(&prod2, NULL, produttore_non_bloccante, (void*) buffer);
	pthread_create(&cons1, NULL, consumatore_non_bloccante, (void*) buffer);
	pthread_create(&cons2, NULL, consumatore_non_bloccante, (void*) buffer);
	pthread_join(prod1, NULL);
	pthread_join(prod2, NULL);
	pthread_join(cons1, NULL);
	pthread_join(cons2, NULL);
	msg_t* msg = msg_init("Ciao");
	msg_t* msg_w = put_bloccante(buffer, msg);
	msg_t* msg_r = get_bloccante(buffer);
	printf("Message write: %s\n", (char*) (msg_w->content));
	printf("D = %d\n", *buffer->D);
	printf("Message read: %s\n", (char*) (msg_r->content));
	printf("T = %d\n", *buffer->T);
	buffer_destroy(buffer);
	return 0;
}*/

// creazione di un buffer vuoto di dim. max nota
buffer_t* buffer_init(unsigned int maxsize) {
	buffer_t* buffer = (buffer_t*) malloc(sizeof(buffer_t));
	buffer->T = 0;
	buffer->D = 0;
	buffer->buff = (msg_t*) malloc(sizeof(msg_t) * maxsize);
	buffer->len = 0;
	buffer->maxsize = maxsize;
	return buffer;  
}

// deallocazione di un buffer
void buffer_destroy(buffer_t* buffer) {
	free(buffer->buff);
	free(buffer);
}

//creazione di un messaggio con contenuto content generico
msg_t* msg_init(void* content) {
	msg_t* new_msg = (msg_t*) malloc(sizeof(msg_t));
	new_msg->content = content;
	new_msg->msg_init = msg_init;
	new_msg->msg_destroy = msg_destroy;
	new_msg->msg_copy = msg_copy;
	return new_msg;
}

//deallocazione di un messaggio
void msg_destroy(msg_t* msg) {
	free(msg->content);
	free(msg);
}

//funzione che crea una copia del messaggio
msg_t* msg_copy(msg_t* msg) {
	return msg->msg_init(msg->content);
}

//inizializzazione di mutex e cndizioni
void cond_init() {
	pthread_mutex_init(&uso_b, NULL);
	pthread_cond_init(&non_pieno, NULL);
	pthread_cond_init(&non_vuoto, NULL);
}

//funzine che scrive in result una stringa di caratteri
//random di lunghezza size
void random_string(char* result, size_t size) {
	char charset[] = "0123456789"
                     "abcdefghijklmnopqrstuvwxyz"
                     "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    while (size-- > 0) {
        size_t index = (double) rand() / RAND_MAX * (sizeof charset - 1);
        *result++ = charset[index];
    }
    *result = '\0';
}

//generico produttore che crea stringhe randomiche
//e effettua inserimenti bloccanti 
void* produttore_bloccante(void* arg) {
	buffer_t* buffer = (buffer_t*) arg;
	while(1) {
		char* content = malloc(N);
		random_string(content,N);
		msg_t* temp = msg_init(content);
		put_bloccante(buffer, temp);
	}
	return NULL;
}

//generico consumatore che effettua estrazioni bloccanti 
void* consumatore_bloccante(void* arg) {
	buffer_t* buffer = (buffer_t*) arg;
	while(1) {
		get_bloccante(buffer);
	}
	return NULL;
}

//generico produttore che crea stringhe randomiche
//e effettua inserimenti non bloccanti
void* produttore_non_bloccante(void* arg) {
	buffer_t* buffer = (buffer_t*) arg;
	while(1) {
		char* content = malloc(N);
		random_string(content,N);
		msg_t* temp = msg_init(content);
		put_non_bloccante(buffer, temp);
	}
	return NULL;
}

//generico consumatore che effettua estrazioni non bloccanti
void* consumatore_non_bloccante(void* arg) {
	buffer_t* buffer = (buffer_t*) arg;
	while(1) {
		get_non_bloccante(buffer);
	}
	return NULL;
}

//versione usa e getta del produttore bloccante (solo a scopo di test)
//ritorna il messaggio inserito
void* produttore_bloccante_ug(void* arg) {
	buffer_t* buffer = (buffer_t*) arg;
	char* content = malloc(N);
	random_string(content,N);
	msg_t* temp = msg_init(content);
	msg_t* msg = put_bloccante(buffer, temp);
	pthread_exit(msg);
}

//ersione usa e getta del consumatore bloccante (solo a scopo di test)
//ritorna il messaggio letto
void* consumatore_bloccante_ug(void* arg) {
	buffer_t* buffer = (buffer_t*) arg;
	msg_t* msg = get_bloccante(buffer);
	pthread_exit(msg);
}

//ersione usa e getta del produttore non bloccante (solo a scopo di test)
//ritorna il messaggio scritto o BUFFER_ERROR altrimenti
void* produttore_non_bloccante_ug(void* arg) {
	buffer_t* buffer = (buffer_t*) arg;
	char* content = malloc(N);
	random_string(content,N);
	msg_t* temp = msg_init(content);
	msg_t* msg = put_non_bloccante(buffer, temp);
	pthread_exit(msg);
}

//ersione usa e getta del consumatore non bloccante (solo a scopo di test)
//ritorna il messaggio letto o BUFFER_ERROR altrimenti
void* consumatore_non_bloccante_ug(void* arg) {
	buffer_t* buffer = (buffer_t*) arg;
	msg_t* msg = get_non_bloccante(buffer);
	pthread_exit(msg);
}

// inserimento bloccante: sospende se pieno, quindi
// effettua l’inserimento non appena si libera dello spazio
// restituisce il messaggio inserito; N.B.: msg!=null
msg_t* put_bloccante(buffer_t* buffer, msg_t* msg) {
	if(msg != NULL) {
		pthread_mutex_lock(&uso_b);
		while(buffer->len == buffer->maxsize)
			pthread_cond_wait(&non_pieno, &uso_b);
		int D = buffer->D;
		buffer->buff[D] = *msg;
		buffer->D = (D + 1) % buffer->maxsize;
		buffer->len += 1;
		pthread_cond_signal(&non_vuoto);
		pthread_mutex_unlock(&uso_b);
		return msg;
	}
	return NULL;
}

// estrazione bloccante: sospende se vuoto, quindi
// restituisce il valore estratto non appena disponibile
msg_t* get_bloccante(buffer_t* buffer) {
	pthread_mutex_lock(&uso_b);
	while(buffer->len == 0)
		pthread_cond_wait(&non_vuoto, &uso_b);
	int T = buffer->T;
	msg_t* msg = msg_copy(&buffer->buff[T]);
	buffer->T = (T + 1) % buffer->maxsize;
	buffer->len -= 1;
	pthread_cond_signal(&non_pieno);
	pthread_mutex_unlock(&uso_b);
	return msg;
}

// inserimento non bloccante: restituisce BUFFER_ERROR se pieno,
// altrimenti effettua l’inserimento e restituisce il messaggio
// inserito; N.B.: msg!=null
msg_t* put_non_bloccante(buffer_t* buffer, msg_t* msg) {
	if(msg != NULL) {
		pthread_mutex_lock(&uso_b);
		if(buffer->len == buffer->maxsize) {
			pthread_mutex_unlock(&uso_b);
			return BUFFER_ERROR;
		}
		int D = buffer->D;
		buffer->buff[D] = *msg;
		buffer->D = (D + 1) % buffer->maxsize;
		buffer->len += 1;
		pthread_mutex_unlock(&uso_b);
		return msg;
	}
	return NULL;
}

// estrazione non bloccante: restituisce BUFFER_ERROR se vuoto
// ed il valore estratto in caso contrario
msg_t* get_non_bloccante(buffer_t* buffer) {
	pthread_mutex_lock(&uso_b);
	if(buffer->len == 0) {
		pthread_mutex_unlock(&uso_b);
		return BUFFER_ERROR;
	}
	int T = buffer->T;
	msg_t* msg = msg_copy(&buffer->buff[T]);
	buffer->T = (T + 1) % buffer->maxsize;
	buffer->len -= 1;
	pthread_mutex_unlock(&uso_b);
	return msg;
}
