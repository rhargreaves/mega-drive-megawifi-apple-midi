#include "mediator.h"
#include "mw/loop.h"
#include "mw/megawifi.h"
#include "mw/mpool.h"
#include "mw/util.h"
#include <genesis.h>

/// Length of the wflash buffer
#define MW_BUFLEN 1460

/// Maximum number of loop functions
#define MW_MAX_LOOP_FUNCS 2

/// Maximun number of loop timers
#define MW_MAX_LOOP_TIMERS 4

/// Command buffer
static char cmd_buf[MW_BUFLEN];

static mw_err associate_ap(struct loop_timer* t)
{
    mw_err err;

    // Join AP
    VDP_drawText("Associating to AP...", 1, 1);
    err = mw_ap_assoc(0);
    if (err != MW_ERR_NONE) {
        return err;
    }
    err = mw_ap_assoc_wait(MS_TO_FRAMES(30000));
    if (err != MW_ERR_NONE) {
        return err;
    }
    //mw_sleep(3 * 60);
    VDP_drawText("Done!", 22, 1);

    return MW_ERR_NONE;
}

static mw_err open_udp_socket(
    struct loop_timer* t, u8 ch, const char* dst_port, const char* src_port)
{
    enum mw_err err;

    err = mw_udp_set(ch, "127.0.0.1", dst_port, src_port);
    if (MW_ERR_NONE != err) {
        goto err;
    }

    return err;
err:
    VDP_drawText("UDP bind error", 1, 2);
    return err;
}

static mw_err wait_for_socket_open(struct loop_timer* t, u8 ch)
{
    enum mw_err err;

    err = mw_sock_conn_wait(ch, 60 * 60);
    if (MW_ERR_NONE != err) {
        goto err;
    }
    VDP_drawText("Socket open", 15, 2);

    return MW_ERR_NONE;
err:
    VDP_drawText("Failed to receive data", 1, 3);
    return err;
}

static mw_err display_ip_addr(struct loop_timer* t)
{
    mw_err err;

    struct mw_ip_cfg* ip_cfg;
    err = mw_ip_current(&ip_cfg);
    if (err != MW_ERR_NONE) {
        goto err;
    }
    char ip_str[16] = {};
    uint32_to_ip_str(ip_cfg->addr.addr, ip_str);
    VDP_drawText(ip_str, 1, 2);
    return err;

err:
    VDP_drawText("ERROR GETTING IP", 1, 2);
    return err;
}

static void print_error(mw_err err)
{
    char text[100];
    sprintf(text, "Error: %d", err);
    VDP_drawText(text, 1, 5);
}

static void udp_test(struct loop_timer* t)
{
    mw_err err;

    err = associate_ap(t);
    if (err != MW_ERR_NONE) {
        goto err;
    }
    err = display_ip_addr(t);
    if (err != MW_ERR_NONE) {
        goto err;
    }
    err = open_udp_socket(t, CH_CONTROL_PORT, "5004", "5006");
    if (err != MW_ERR_NONE) {
        goto err;
    }
    err = open_udp_socket(t, CH_MIDI_PORT, "5005", "5007");
    if (err != MW_ERR_NONE) {
        goto err;
    }
    err = wait_for_socket_open(t, CH_CONTROL_PORT);
    if (err != MW_ERR_NONE) {
        goto err;
    }
    err = wait_for_socket_open(t, CH_MIDI_PORT);
    if (err != MW_ERR_NONE) {
        goto err;
    }
    while (1) {
        err = mediator_receive();
        if (err != MW_ERR_NONE) {
            print_error(err);
        }
    }

err:
    print_error(err);
    loop_timer_del(t);
}

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
        VDP_drawText("MegaWiFi not found!", 1, 0);
    } else {
        // Megawifi found
        line[17] = ver_major + '0';
        line[19] = ver_minor + '0';
        VDP_drawText(line, 1, 0);

        // Configuration complete, run test function next frame
        t->timer_cb = udp_test;
        loop_timer_start(t, 1);
    }
}

static void idle_cb(struct loop_func* f)
{
    UNUSED_PARAM(f);
    mw_process();
}

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

int main()
{
    mp_init(0);
    main_loop_init();
    mw_init(cmd_buf, MW_BUFLEN);
    loop();
    return 0;
}
