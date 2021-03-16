//
// Created by zelenova-yu on 20.12.2020.
//
#include "CUnit/CUnit.h"
#include "CUnit/Basic.h"
#include "stdio.h"
#include "test_handlers.h"
#include "test_socket.h"

static FILE* temp_file = NULL;

int init_suite1(void)
{
    if (NULL == (temp_file = fopen("temp.txt", "w+"))) {
        return -1;
    }
    else {
        return 0;
    }
}

int clean_suite1(void)
{
    if (0 != fclose(temp_file)) {
        return -1;
    }
    else {
        temp_file = NULL;
        return 0;
    }
}

int main()
{
    CU_pSuite pSuite = NULL;

    /* initialize the CUnit test registry */
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    /* add a suite to the registry */
    pSuite = CU_add_suite("Suite_1", init_suite1, clean_suite1);
    if (NULL == pSuite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    /*if (NULL == CU_add_test(pSuite, "test of automata()", test_correct_automata))
    {
        CU_cleanup_registry();
        return CU_get_error();
    }*/

    CU_add_test(pSuite, "test of automata", test_correct_automata);
    CU_add_test(pSuite, "test of correct HELO", test_correct_HELO);
    CU_add_test(pSuite, "test of incorrect HELO", test_incorrect_HELO);
    CU_add_test(pSuite, "test of correct EHLO", test_correct_EHLO);
    CU_add_test(pSuite, "test of incorrect EHLO", test_incorrect_EHLO);
    CU_add_test(pSuite, "test of correct MAIL", test_correct_MAIL);
    CU_add_test(pSuite, "test of incorrect MAIL", test_incorrect_MAIL);
    CU_add_test(pSuite, "test of correct RCPT", test_correct_RCPT);
    CU_add_test(pSuite, "test of incorrect RCPT", test_incorrect_RCPT);
    CU_add_test(pSuite, "test of correct DATA", test_correct_DATA);
    CU_add_test(pSuite, "test of incorrect DATA", test_incorrect_DATA);
    CU_add_test(pSuite, "test of correct TEXT", test_correct_TEXT);
    CU_add_test(pSuite, "test of correct QUIT", test_correct_QUIT);
    CU_add_test(pSuite, "test of correct RSET", test_correct_RSET);
    CU_add_test(pSuite, "test of correct ERROR", test_correct_ERROR);

    CU_add_test(pSuite, "test of correct connection", test_check_connection_correct);
    CU_add_test(pSuite, "test of correct greetings", test_check_greetings_correct);
    CU_add_test(pSuite, "test of correct send", test_check_send_correct);
    CU_add_test(pSuite, "test of correct receive", test_check_receive_correct);

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}

