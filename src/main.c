#include <genesis.h>

int main()
{
    VDP_drawText("Hello", 1, 1);
    while (TRUE) { VDP_waitVSync(); }
    return 0;
}
