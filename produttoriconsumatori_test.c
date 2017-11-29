#include "produttoriconsumatori.c"
#include "CUnit/CUnit.h"
#include "CUnit/Basic.h"

#define BUFFER_ERROR (msg_t *) NULL

int init_suite(void) { 
	srand(time(NULL));
	return 0; 
}
int clean_suite(void) { return 0; }

void test_produzione_non_bloccante_buffer_pieno(void) {
	buffer_t* buffer_pieno = buffer_init(1);
	cond_init();
	int EXPECTED_MSG = 20;
	msg_t* exp_msg = msg_init(&EXPECTED_MSG);
	buffer_pieno->buff[0] = *exp_msg;
	++buffer_pieno->len;
	int MSG = 10;
	msg_t* new_msg = msg_init(&MSG);
	msg_t* msg = put_non_bloccante(buffer_pieno, new_msg);

	CU_ASSERT_EQUAL(msg, BUFFER_ERROR);
	int* content = (int*) buffer_pieno->buff[0].content;
	CU_ASSERT_EQUAL(*content, EXPECTED_MSG);
	CU_ASSERT_TRUE(buffer_pieno->len > 0);
	
	buffer_destroy(buffer_pieno);
}

void test_consumazione_bloccante_buffer_vuoto(void) {
	buffer_t* buffer_vuoto = buffer_init(1);
	cond_init();
	pthread_t consumatore;
	pthread_create(&consumatore, NULL, consumatore_bloccante_ug, (void*) buffer_vuoto);	
	int GO_MSG = 10;
	msg_t* msg = msg_init(&GO_MSG);
	put_bloccante(buffer_vuoto, msg);
	void* ret;
	pthread_join(consumatore, &ret);
	msg_t* letto = (msg_t*) ret;
	int* content = letto->content;

	CU_ASSERT_EQUAL(*content, GO_MSG);
	CU_ASSERT_TRUE(buffer_vuoto->len == 0);

	buffer_destroy(buffer_vuoto);
}

void test_cons_prod_buffer_unitario(void) {
	buffer_t* buffer = buffer_init(1);
	cond_init();
	pthread_t consumatore;
	pthread_t produttore;
	pthread_create(&consumatore, NULL, consumatore_bloccante_ug, (void*) buffer);
	pthread_create(&produttore, NULL, produttore_bloccante_ug, (void*) buffer);
	void* letto;
	void* scritto;
	pthread_join(consumatore, &letto);
	pthread_join(produttore, &scritto);
	msg_t* msg_c = (msg_t*) letto;
	msg_t* msg_p = (msg_t*) scritto;
	char* content_c = msg_c->content;
	char* content_p = msg_p->content;

	CU_ASSERT_STRING_EQUAL(content_c, content_p);
	CU_ASSERT_TRUE(buffer->len == 0);

	buffer_destroy(buffer);
}

void test_prod_cons_buffer_unitario(void) {
	buffer_t* buffer = buffer_init(1);
	cond_init();
	pthread_t produttore;
	pthread_t consumatore;
	pthread_create(&produttore, NULL, produttore_bloccante_ug, (void*) buffer);
	pthread_create(&consumatore, NULL, consumatore_bloccante_ug, (void*) buffer);
	void* scritto;
	void* letto;
	pthread_join(produttore, &scritto);
	pthread_join(consumatore, &letto);
	msg_t* msg_p = (msg_t*) scritto;
	msg_t* msg_c = (msg_t*) letto;
	char* content_p = msg_p->content;
	char* content_c = msg_c->content;

	CU_ASSERT_STRING_EQUAL(content_c, content_p);
	CU_ASSERT_TRUE(buffer->len == 0);

	buffer_destroy(buffer);
}

void test_produttori_buffer_unitario(void) {
	buffer_t* buffer = buffer_init(1);
	cond_init();
	pthread_t prod1;
	pthread_t prod2;
	pthread_create(&prod1, NULL, produttore_non_bloccante_ug, (void*) buffer);
	pthread_create(&prod2, NULL, produttore_non_bloccante_ug, (void*) buffer);
	void* scritto1;
	void* scritto2;
	pthread_join(prod1, &scritto1);
	pthread_join(prod2, &scritto2);
	msg_t* msg_p1 = (msg_t*) scritto1;
	msg_t* msg_p2 = (msg_t*) scritto2;

	CU_ASSERT_PTR_NOT_EQUAL(msg_p1, msg_p2);
	if (msg_p1 == BUFFER_ERROR || msg_p2 == BUFFER_ERROR) {
		CU_ASSERT(1);
	} else {
		CU_ASSERT(0);
	}

	buffer_destroy(buffer);
}

void test_consumatori_buffer_unitario(void) {
	buffer_t* buffer = buffer_init(1);
	cond_init();
	int EXPECTED_MSG = 20;
	msg_t* exp_msg = msg_init(&EXPECTED_MSG);
	put_non_bloccante(buffer, exp_msg);

	pthread_t cons1;
	pthread_t cons2;
	pthread_create(&cons1, NULL, consumatore_non_bloccante_ug, (void*) buffer);	
	pthread_create(&cons2, NULL, consumatore_non_bloccante_ug, (void*) buffer);
	void* letto1;
	void* letto2;
	pthread_join(cons1, &letto1);
	pthread_join(cons2, &letto2);
	msg_t* msg_c1 = (msg_t*) letto1;
	msg_t* msg_c2 = (msg_t*) letto2;

	CU_ASSERT_PTR_NOT_EQUAL(msg_c1, msg_c2);
	int* content;
	if(msg_c1 == BUFFER_ERROR) {
		content = msg_c2->content;
		CU_ASSERT_EQUAL(*content, EXPECTED_MSG);
	} else if(msg_c2 == BUFFER_ERROR) {
		content = msg_c1->content;
		CU_ASSERT_EQUAL(*content, EXPECTED_MSG);
	} else {
		CU_ASSERT(0);
	}

	buffer_destroy(buffer);
}

void test_produttori_consumatori_buffer_unitario(void) {
	buffer_t* buffer = buffer_init(1);
	cond_init();
	pthread_t prod1;
	pthread_t prod2;
	pthread_t cons1;
	pthread_t cons2;
	pthread_create(&prod1, NULL, produttore_bloccante_ug, (void*) buffer);
	pthread_create(&prod2, NULL, produttore_bloccante_ug, (void*) buffer);
	pthread_create(&cons1, NULL, consumatore_bloccante_ug, (void*) buffer);	
	pthread_create(&cons2, NULL, consumatore_bloccante_ug, (void*) buffer);
	void* scritto1;
	void* scritto2;
	void* letto1;
	void* letto2;
	pthread_join(prod1, &scritto1);
	pthread_join(prod2, &scritto2);
	pthread_join(cons1, &letto1);
	pthread_join(cons2, &letto2);
	msg_t* msg_p1 = (msg_t*) scritto1;
	msg_t* msg_p2 = (msg_t*) scritto2;
	msg_t* msg_c1 = (msg_t*) letto1;
	msg_t* msg_c2 = (msg_t*) letto2; 

	CU_ASSERT_PTR_NOT_NULL(msg_p1);
	CU_ASSERT_PTR_NOT_NULL(msg_p2);
	CU_ASSERT_PTR_NOT_NULL(msg_c1);
	CU_ASSERT_PTR_NOT_NULL(msg_c2);

	char* content_s1 = msg_p1->content;
	char* content_s2 = msg_p2->content;
	char* content_l1 = msg_c1->content;
	char* content_l2 = msg_c2->content; 

	CU_ASSERT_TRUE(buffer->len == 0);

	if(((strcmp(content_s1, content_l1) == 0) || (strcmp(content_s1, content_l2) == 0)) 
		&& ((strcmp(content_s2, content_l1) == 0) || (strcmp(content_s2, content_l2) == 0))) {
		CU_ASSERT(1);
	} else {
		CU_ASSERT(0);
	}

	buffer_destroy(buffer);
}

void test_produttori_consumatori_buffer(void) {
	buffer_t* buffer = buffer_init(2);
	cond_init();
	pthread_t prod1;
	pthread_t prod2;
	pthread_t cons1;
	pthread_t cons2;
	pthread_create(&prod1, NULL, produttore_non_bloccante_ug, (void*) buffer);
	pthread_create(&prod2, NULL, produttore_non_bloccante_ug, (void*) buffer);
	pthread_create(&cons1, NULL, consumatore_non_bloccante_ug, (void*) buffer);	
	pthread_create(&cons2, NULL, consumatore_non_bloccante_ug, (void*) buffer);
	void* scritto1;
	void* scritto2;
	void* letto1;
	void* letto2;
	pthread_join(prod1, &scritto1);
	pthread_join(prod2, &scritto2);
	pthread_join(cons1, &letto1);
	pthread_join(cons2, &letto2);
	msg_t* msg_p1 = (msg_t*) scritto1;
	msg_t* msg_p2 = (msg_t*) scritto2;
	msg_t* msg_c1 = (msg_t*) letto1;
	msg_t* msg_c2 = (msg_t*) letto2;

	CU_ASSERT_PTR_NOT_EQUAL(msg_p1, BUFFER_ERROR);
	CU_ASSERT_PTR_NOT_EQUAL(msg_p2, BUFFER_ERROR);
	CU_ASSERT_PTR_NOT_EQUAL(msg_c1, BUFFER_ERROR);
	CU_ASSERT_PTR_NOT_EQUAL(msg_c2, BUFFER_ERROR);

	char* content_s1 = msg_p1->content;
	char* content_s2 = msg_p2->content;
	char* content_l1 = msg_c1->content;
	char* content_l2 = msg_c2->content; 

	CU_ASSERT_TRUE(buffer->len == 0);

	if(((strcmp(content_s1, content_l1) == 0) || (strcmp(content_s1, content_l2) == 0)) 
		&& ((strcmp(content_s2, content_l1) == 0) || (strcmp(content_s2, content_l2) == 0))) {
		CU_ASSERT(1);
	} else {
		CU_ASSERT(0);
	}

	buffer_destroy(buffer);
}
 
int main() {
	CU_pSuite pSuite = NULL;
   /* initialize the CUnit test registry */
	if(CUE_SUCCESS != CU_initialize_registry())
		return CU_get_error();
   /* add a suite to the registry */
	pSuite = CU_add_suite("produttoriconsumatori_test_suite", init_suite, clean_suite);
	if(NULL == pSuite) {
   		CU_cleanup_registry();
   		return CU_get_error();
   	}
   	/* add the tests to the suite */
   	/* NOTE - ORDER IS IMPORTANT - MUST TEST fread() AFTER fprintf() */
   	if((NULL == CU_add_test(pSuite, "test di put_non_bloccante() con buffer unitario pieno", test_produzione_non_bloccante_buffer_pieno))
   		|| NULL == CU_add_test(pSuite, "test di 1 consumatore bloccante con buffer vuoto", test_consumazione_bloccante_buffer_vuoto)
   		|| NULL == CU_add_test(pSuite, "test di 1 produttore e 1 consumatore su buffer unitario, prima il consumatore", test_cons_prod_buffer_unitario)
   		|| NULL == CU_add_test(pSuite, "test di 1 produttore e 1 consumatore su buffer unitario, prima il produttore", test_prod_cons_buffer_unitario)
   		|| NULL == CU_add_test(pSuite, "test 2 produttori su buffer unitario", test_produttori_buffer_unitario)
   		|| NULL == CU_add_test(pSuite, "test 2 consumatori su buffer unitario", test_consumatori_buffer_unitario)
   		|| NULL == CU_add_test(pSuite, "test 2 produttori 2 consumatori su buffer unitario", test_produttori_consumatori_buffer_unitario)
   		|| NULL == CU_add_test(pSuite, "test 2 produttori 2 consumatori su buffer generico", test_produttori_consumatori_buffer)) {
 		CU_cleanup_registry();
 		return CU_get_error();
 	}
    // Run all tests using the basic interface
   	CU_basic_set_mode(CU_BRM_VERBOSE);
   	CU_basic_run_tests();
   	printf("\n");
   	CU_basic_show_failures(CU_get_failure_list());
   	printf("\n\n");
   	/*
   	// Run all tests using the automated interface
   	CU_automated_run_tests();
   	CU_list_tests_to_file();
   	// Run all tests using the console interface
   	CU_console_run_tests(); */
   	/* Clean up registry and return */
   	CU_cleanup_registry();
   	return CU_get_error();
}
