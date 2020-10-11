#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "test_applemidi.c"
#include <cmocka.h>

#define applemidi_test(test) cmocka_unit_test_setup(test, test_applemidi_setup)

int main(void)
{
    const struct CMUnitTest tests[] = {
        applemidi_test(
            test_applemidi_parses_rtpmidi_packet_with_single_midi_event),
        applemidi_test(
            test_applemidi_parses_rtpmidi_packet_with_single_midi_event_long_header),
        applemidi_test(
            test_applemidi_parses_rtpmidi_packet_with_single_2_byte_midi_event)

    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
