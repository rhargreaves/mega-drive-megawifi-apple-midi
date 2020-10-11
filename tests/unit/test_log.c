#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "unused.h"
#include <cmocka.h>
#include <stdio.h>

// extern void __real_log_init(void);

// static int test_log_setup(UNUSED void** state)
// {
//     __real_log_init();

//     return 0;
// }

// static void test_log_info_writes_to_log_buffer(UNUSED void** state)
// {
//     __real_log_info("Test Message %d", 1, 0);

//     Log* log = __real_log_dequeue();

//     assert_int_not_equal(log->msgLen, 0);
//     assert_memory_equal("Test Message 1", log->msg, 15);
// }
