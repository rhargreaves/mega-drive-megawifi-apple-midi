#include <types.h>

#include <cmocka.h>

#define expect_midi_emit(mb)                                                   \
    {                                                                          \
        expect_value(__wrap_midi_emit, midiByte, mb);                          \
    }
