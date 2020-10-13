#pragma once
#include "applemidi.h"
#include "mw/megawifi.h"

mw_err mediator_receive(void);
void mediator_send(u8 ch, char* data, u16 len);
