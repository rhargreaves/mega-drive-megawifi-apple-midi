#include "loop.h"
#include "megawifi.h"
#include "mpool.h"
#include "util.h"
#include <genesis.h>

/// Length of the wflash buffer
#define MW_BUFLEN 1460

/// Maximum number of loop functions
#define MW_MAX_LOOP_FUNCS 2

/// Maximun number of loop timers
#define MW_MAX_LOOP_TIMERS 4

/// Command buffer
static char cmd_buf[MW_BUFLEN];

/// MegaWiFi initialization
static void megawifi_init_cb(struct loop_timer* t)
{
    uint8_t ver_major = 0, ver_minor = 0;
    char* variant = NULL;
    enum mw_err err;
    char line[] = "MegaWiFi version X.Y";

    // Try detecting the module
    err = mw_detect(&ver_major, &ver_minor, &variant);

    if (MW_ERR_NONE != err) {
        // Megawifi not found
        VDP_drawText("MegaWiFi not found!", 1, 2);
    } else {
        // Megawifi found
        line[17] = ver_major + '0';
        line[19] = ver_minor + '0';
        VDP_drawText(line, 1, 3);
        // Configuration complete, run test function next frame
        //  t->timer_cb = run_test;
        loop_timer_start(t, 1);
    }
}

static void idle_cb(struct loop_func* f)
{
    UNUSED_PARAM(f);
    mw_process();
}

/// Loop run while idle
static void main_loop_init(void)
{
    // Run next frame, do not auto-reload
    static struct loop_timer frame_timer
        = { .timer_cb = megawifi_init_cb, .frames = 1 };
    static struct loop_func megawifi_loop = { .func_cb = idle_cb };

    loop_init(MW_MAX_LOOP_FUNCS, MW_MAX_LOOP_TIMERS);
    loop_timer_add(&frame_timer);
    loop_func_add(&megawifi_loop);
}

/// Global initialization
static void init(void)
{
    // Initialize memory pool
    mp_init(0);
    // Initialize game loop
    main_loop_init();
    // Initialize MegaWiFi
    mw_init(cmd_buf, MW_BUFLEN);
}

int main()
{
    init();

    VDP_drawText("Hello", 1, 1);

    loop();

    while (TRUE) { VDP_waitVSync(); }
    return 0;
}
