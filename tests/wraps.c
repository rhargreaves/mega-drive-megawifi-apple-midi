#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include "wraps.h"
#include <cmocka.h>

int _eflash;

void __wrap_VDP_drawText(const char* str, u16 x, u16 y)
{
}