#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "test_log.c"
#include <cmocka.h>

#define log_test(test) cmocka_unit_test_setup(test, test_log_setup)

int main(void)
{
    // const struct CMUnitTest tests[] = {
    //     log_test(test_log_info_writes_to_log_buffer),

    // };

    // return cmocka_run_group_tests(tests, NULL, NULL);
}
