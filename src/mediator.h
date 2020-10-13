#pragma once
#include "applemidi.h"
#include "mw/megawifi.h"

mw_err mediator_recv_event(void);
void mediator_send_packet(u8 ch, char* data, u16 len);
