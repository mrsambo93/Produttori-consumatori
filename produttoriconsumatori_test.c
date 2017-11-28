#include "produttoriconsumatori.c"
#include "CUnit/CUnit.h"
#include "CUnit/Basic.h"

#define N 4
#define BUFFER_ERROR (msg_t *) NULL

int init_suite(void) { return 0; }
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
   		|| NULL == CU_add_test(pSuite, "test di 1 consumatore bloccante con buffer vuoto", test_consumazione_bloccante_buffer_vuoto)) {
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
