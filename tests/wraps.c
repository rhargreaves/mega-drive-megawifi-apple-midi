#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "wraps.h"
#include <cmocka.h>

int _eflash;

void __wrap_VDP_drawText(const char* str, u16 x, u16 y)
{
}

void __wrap_midi_emit(u8 midiByte)
{
    check_expected(midiByte);
}

mw_err __wrap_mediator_recv_event(void)
{
    function_called();
    return mock_type(mw_err);
}

mw_err __wrap_mediator_send_packet(u8 ch, char* data, u16 len)
{
    check_expected(ch);
    check_expected(data);
    check_expected(len);
    return mock_type(mw_err);
}

void __wrap_SYS_die(char* err)
{
    print_error("%s", err);
}
