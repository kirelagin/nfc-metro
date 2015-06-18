#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <memory.h>
#include <unistd.h>
#include <nfc/nfc.h>

#include "mifare.h"

nfc_device *pnd = NULL;


static void stop_polling(int sig)
{
    if (pnd) {
        nfc_abort_command(pnd);
    } else {
        exit(3);
    }
}


int main()
{
    int ret = 2;

    nfc_context *ctx = 0;

    const nfc_modulation nm = {
        .nmt = NMT_ISO14443A,
        .nbr = NBR_106,
    };

    uint8_t key[6] = {0x26, 0x97, 0x3e, 0xa7, 0x43, 0x21};
//    uint8_t key[6] = {0xd2, 0x70, 0x58, 0xc6, 0xe2, 0xc7};

    nfc_init(&ctx);
    if (!ctx) {
        printf("Could not initialise libnfc.\n");
        goto error;
    }

    pnd = nfc_open(ctx, NULL);
    if (!pnd) {
        printf("No NFC device found.\n");
        goto error;
    }

    if (nfc_initiator_init(pnd) != 0) {
        nfc_perror(pnd, "nfc_initiator_init");
        goto error;
    }
    if (nfc_device_set_property_bool(pnd, NP_ACTIVATE_FIELD, true) < 0) {
        nfc_perror(pnd, "activate field");
        goto error;
    }

    int res;
    nfc_target nt;
    signal(SIGINT, stop_polling);

    while (true) {
        if ((res = nfc_initiator_poll_target(pnd, &nm, 1, 0xFF, 2, &nt)) < 0) {
            nfc_perror(pnd, "poll");
            goto error;
        }
        if (res == 0) {
            printf("No target found.\n");
        } else {
            printf("===\nSAK: %02x\n", nt.nti.nai.btSak);
            if ((nt.nti.nai.btSak & 0x08) == 0) {
                printf("only Mifare Classic is supported\n");
                continue;
            }
            mifare_param mp;
            memcpy(mp.mpa.abtAuthUid, nt.nti.nai.abtUid + nt.nti.nai.szUidLen - 4, 4);
            memcpy(mp.mpa.abtKey, key, sizeof(mp.mpa.abtKey));
            if (!nfc_initiator_mifare_cmd(pnd, MC_AUTH_A, 8 * 4, &mp)) {
                printf("authentication failed\n");
                continue;
            }
            if (!nfc_initiator_mifare_cmd(pnd, MC_READ, 8 * 4, &mp)) {
                printf("0 read error\n");
                continue;
            }

            uint8_t year, month, day, hours, minutes, sid1, sid2, turn;
            unsigned char passp_s[7];
            uint32_t passp_n;

            year = mp.mpd.abtData[7];
            month = mp.mpd.abtData[8];
            day = mp.mpd.abtData[9];
            printf("Valid from   %02d.%02d.%02d ", day, month, year);
            year = mp.mpd.abtData[10];
            month = mp.mpd.abtData[11];
            day = mp.mpd.abtData[12];
            printf("to %02d.%02d.%02d\n", day, month, year);
            if (!nfc_initiator_mifare_cmd(pnd, MC_READ, 8 * 4 + 1, &mp)) {
                printf("1 read error\n");
                continue;
            }
            year = mp.mpd.abtData[0];
            month = mp.mpd.abtData[1];
            day = mp.mpd.abtData[2];
            passp_s[0] = mp.mpd.abtData[3];
            passp_s[1] = mp.mpd.abtData[4];
            passp_s[2] = mp.mpd.abtData[5];
            passp_s[3] = mp.mpd.abtData[6];
            passp_s[4] = mp.mpd.abtData[7];
            passp_s[5] = mp.mpd.abtData[8];
            passp_s[6] = '\0';
            passp_n  = mp.mpd.abtData[9]  << 8 * 0;
            passp_n += mp.mpd.abtData[10] << 8 * 1;
            passp_n += mp.mpd.abtData[11] << 8 * 2;
            passp_n += mp.mpd.abtData[12] << 8 * 3;
            printf("Purchased on %02d.%02d.%02d", day, month, year);
            printf(", passport [%s] %d\n", passp_s, passp_n);
            if (!nfc_initiator_mifare_cmd(pnd, MC_READ, 8 * 4 + 2, &mp)) {
                printf("2 read error\n");
                continue;
            }
            year = mp.mpd.abtData[1];
            month = mp.mpd.abtData[2];
            day = mp.mpd.abtData[3];
            hours = mp.mpd.abtData[4];
            minutes = mp.mpd.abtData[5];
            sid1 = mp.mpd.abtData[6];
            sid2 = mp.mpd.abtData[7];
            turn = mp.mpd.abtData[8];
            printf("Last seen:\n  %02d.%02d.%02d %02d:%02d\n  station (%02X %02X), turn %d\n\n", day, month, year, hours + 3, minutes, sid1, sid2, turn);

            //while (0 == nfc_initiator_target_is_present(pnd, &nt)) {}
            //nfc_perror(pnd, "nfc_initiator_target_is_present");
            sleep(1);
        }
    }

    nfc_close(pnd);
    nfc_exit(ctx);
    return 0;

error:
    if (pnd) {
      nfc_close(pnd);
    }
    if (ctx) {
      nfc_exit(ctx);
    }
    return 2;
}
