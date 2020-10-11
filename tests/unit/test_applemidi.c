#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "unused.h"
#include <cmocka.h>
#include <stdio.h>

#include "applemidi.h"

static int test_applemidi_setup(UNUSED void** state)
{
    return 0;
}

static void test_applemidi_parses_rtpmidi_packet(UNUSED void** state)
{
    char data[1024] = { 0 };
    size_t len = sizeof(data);

    mw_err err = applemidi_process_midi_data(data, len);

    assert_int_equal(err, MW_ERR_NONE);
}
