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

#define ERR_BASE 100
#define ERR_INVALID_APPLE_MIDI_SIGNATURE ERR_BASE;
#define ERR_UNEXPECTED_CHANNEL (ERR_BASE + 1)
#define ERR_APPLE_MIDI_EXCH_PKT_TOO_SMALL (ERR_BASE + 2)

/// Command buffer
static char cmd_buf[MW_BUFLEN];

#define NAME_LEN 16
#define UDP_PKT_LEN (16 + NAME_LEN)
#define UDP_PKT_BUFFER_LEN 64
#define APPLE_MIDI_EXCH_PKT_MIN_LEN 17

typedef enum mw_err mw_err;

union AppleMidiExchangePacket {
    u8 byte[UDP_PKT_LEN];
    struct {
        u16 signature;
        char command[2];
        u32 version;
        u32 initToken;
        u32 senderSSRC;
        char name[NAME_LEN];
    };
};

typedef union AppleMidiExchangePacket AppleMidiExchangePacket;

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

static mw_err open_tcp_socket(struct loop_timer* t)
{
    enum mw_err err;

    err = mw_tcp_bind(1, 5567);
    if (MW_ERR_NONE != err) {
        goto err;
    }

    return err;
err:
    VDP_drawText("TCP bind error", 1, 2);
    return err;
}

static mw_err open_udp_socket(struct loop_timer* t)
{
    enum mw_err err;

    err = mw_udp_set(1, "127.0.0.1", "5004", "5006");
    if (MW_ERR_NONE != err) {
        goto err;
    }

    return err;
err:
    VDP_drawText("UDP bind error", 1, 2);
    return err;
}

static mw_err wait_for_socket_open(struct loop_timer* t)
{
    enum mw_err err;

    err = mw_sock_conn_wait(1, 60 * 60);
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

#define APPLE_MIDI_SIGNATURE 0xFFFF

static mw_err receive_invitation(AppleMidiExchangePacket* invite)
{
    char buffer[UDP_PKT_BUFFER_LEN];
    s16 buf_length = sizeof(buffer);
    u8 ch;
    mw_err err = mw_recv_sync(&ch, buffer, &buf_length, 0);
    if (err != MW_ERR_NONE) {
        return err;
    }
    if (ch != 1) {
        return ERR_UNEXPECTED_CHANNEL;
    }
    if (buf_length < APPLE_MIDI_EXCH_PKT_MIN_LEN) {
        return ERR_APPLE_MIDI_EXCH_PKT_TOO_SMALL;
    }

    u8 index = 0;
    while (index < UDP_PKT_LEN) { invite->byte[index] = buffer[index++]; }

    if (invite->signature != APPLE_MIDI_SIGNATURE) {
        return ERR_INVALID_APPLE_MIDI_SIGNATURE;
    }

    return MW_ERR_NONE;
}

static mw_err send_invite_reply(AppleMidiExchangePacket* invite)
{
    const u32 MEGADRIVE_SSRC = 0x9E915150;

    AppleMidiExchangePacket inviteReply = { .signature = APPLE_MIDI_SIGNATURE,
        .command = "OK",
        .name = "MegaDrive",
        .initToken = invite->initToken,
        .senderSSRC = MEGADRIVE_SSRC,
        .version = 2 };

    char buffer[UDP_PKT_BUFFER_LEN];
    s16 buf_length = sizeof(buffer);
    u8 index = 0;
    while (index < UDP_PKT_LEN) { buffer[index] = inviteReply.byte[index++]; }
    u8 ch = 1;
    mw_err err = mw_send_sync(ch, buffer, index, 0);
    if (err != MW_ERR_NONE) {
        return err;
    }

    return MW_ERR_NONE;
}

static mw_err receive_data(struct loop_timer* t)
{
    mw_err err = MW_ERR_NONE;

    u8 lineNo = 3;
    while (err == MW_ERR_NONE) {
        char line[40];
        s16 buf_length = sizeof(line);
        u8 ch = 1;
        err = mw_recv_sync(&ch, line, &buf_length, 0);
        if (err != MW_ERR_NONE) {
            VDP_drawText("Timeout, no connection established", 1, 2);
            return err;
        }
        line[buf_length - 1] = '\0';
        // Data received
        char text[100] = {};
        sprintf(text, "Data: [%s] Len: %d", line, buf_length - 1);
        VDP_drawText(text, 1, lineNo++);
    }

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
    err = open_udp_socket(t);
    if (err != MW_ERR_NONE) {
        goto err;
    }
    err = wait_for_socket_open(t);
    if (err != MW_ERR_NONE) {
        goto err;
    }

    AppleMidiExchangePacket packet;
    err = receive_invitation(&packet);
    if (err != MW_ERR_NONE) {
        goto err;
    }
    char text[100];
    sprintf(text, "Name: %s", packet.name);
    VDP_drawText(text, 1, 10);

    err = send_invite_reply(&packet);
    if (err != MW_ERR_NONE) {
        goto err;
    }
    sprintf(text, "Invite sent");
    VDP_drawText(text, 1, 11);

    err = receive_data(t);
    if (err != MW_ERR_NONE) {
        goto err;
    }
    goto out;

err:
    print_error(err);

out:
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
