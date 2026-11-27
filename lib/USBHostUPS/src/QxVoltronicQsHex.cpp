/*
 * esp32-nut
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Contains code and data derived from Network UPS Tools (https://networkupstools.org/),
 * GPL-2.0-or-later, modified for esp32-nut in 2026.
 *
 * drivers/nutdrv_qx_voltronic-qs-hex.c:
 *   Copyright (C) 2016 Daniele Pezzini <hyouko@gmail.com>
 */

#include "QxProtocols.h"
#include "QxBlazerCommon.h"
#include "QxEngine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Preprocess the raw binary QS\r answer into the ASCII hex/bitstring format:
 * Raw:  23 <2B> 20 <1B> 20 <2B> 20 <1B> 20 <1B> 20 <2B> 20 <3B> 20 <1B> 20 <1B> 20 <1B> 0D
 * Text: #XXXX XX XXXX XX XX XXXX XXXXXX XX XX BBBBBBBB
 */
static int qsHexPreprocessAnswer(QxEngine& qx, const QxItem& item, int len) {
    (void)item;
    char* ans = qx.answer();
    if (len < 27 || ans[0] != '#') return -1;

    const uint8_t* raw = reinterpret_cast<const uint8_t*>(ans);
    char decoded[64];
    int pos = 0;

    decoded[pos++] = '#';
    pos += snprintf(decoded + pos, sizeof(decoded) - pos, "%02x%02x ", raw[1], raw[2]);
    pos += snprintf(decoded + pos, sizeof(decoded) - pos, "%02x ", raw[4]);
    pos += snprintf(decoded + pos, sizeof(decoded) - pos, "%02x%02x ", raw[6], raw[7]);
    pos += snprintf(decoded + pos, sizeof(decoded) - pos, "%02x ", raw[9]);
    pos += snprintf(decoded + pos, sizeof(decoded) - pos, "%02x ", raw[11]);
    pos += snprintf(decoded + pos, sizeof(decoded) - pos, "%02x%02x ", raw[13], raw[14]);
    pos += snprintf(decoded + pos, sizeof(decoded) - pos, "%02x%02x%02x ", raw[16] & 0x1F, raw[17], raw[18]);
    pos += snprintf(decoded + pos, sizeof(decoded) - pos, "%02x ", raw[20]);
    pos += snprintf(decoded + pos, sizeof(decoded) - pos, "%02x ", raw[22]);

    uint8_t st = raw[24];
    for (int b = 7; b >= 0; b--) {
        decoded[pos++] = (st & (1U << b)) ? '1' : '0';
    }
    decoded[pos] = '\0';

    if ((size_t)pos >= QX_SMALLBUF) return -1;
    memcpy(ans, decoded, (size_t)pos + 1);
    return pos;
}

static int qsHexProcessHex(QxEngine& qx, const QxItem& item, char* value, size_t valuelen) {
    (void)qx;
    unsigned long raw = strtoul(value, nullptr, 16);
    double val = (double)raw;

    if (strcmp(item.info_type, "input.voltage") == 0 ||
        strcmp(item.info_type, "output.voltage") == 0) {
        val = (double)((raw >> 8) & 0xFF) * 2.0 + (double)(raw & 0xFF) * 0.1;
    } else if (strcmp(item.info_type, "input.frequency") == 0) {
        val = (double)raw / 10.0;
    } else if (strcmp(item.info_type, "battery.voltage") == 0) {
        val = (double)((raw >> 16) & 0xFF) + (double)(raw & 0xFFFF) / 10000.0;
    }

    snprintf(value, valuelen, item.dfl ? item.dfl : "%.1f", val);
    return 0;
}

static const QxItem VOLTRONIC_QS_HEX_QX2NUT[] = {
    // Decoded: #6f01 6c 6f01 6c 1a 5fe1 1312d0 e3 1e 00001000
    //          012345678901234567890123456789012345678901234567
    { "input.voltage",   0, nullptr, "QS\r", 27, '#', 1,  4,  "%.1f", 0, nullptr, qsHexPreprocessAnswer, qsHexProcessHex },
    { "output.voltage",  0, nullptr, "QS\r", 27, '#', 9,  12, "%.1f", 0, nullptr, qsHexPreprocessAnswer, qsHexProcessHex },
    { "ups.load",        0, nullptr, "QS\r", 27, '#', 17, 18, "%.0f", 0, nullptr, qsHexPreprocessAnswer, qsHexProcessHex },
    { "battery.voltage", 0, nullptr, "QS\r", 27, '#', 25, 30, "%.2f", 0, nullptr, qsHexPreprocessAnswer, qsHexProcessHex },
    { "ups.temperature", 0, nullptr, "QS\r", 27, '#', 32, 33, "%.1f", 0, nullptr, qsHexPreprocessAnswer, qsHexProcessHex },

    /* Status bits (positions 40..47 in decoded string) */
    { "ups.status",        0, nullptr, "QS\r", 27, '#', 40, 40, nullptr, QX_FLAG_QUICK_POLL, nullptr, qsHexPreprocessAnswer, blazerProcessStatusBits },
    { "ups.status",        0, nullptr, "QS\r", 27, '#', 41, 41, nullptr, QX_FLAG_QUICK_POLL, nullptr, qsHexPreprocessAnswer, blazerProcessStatusBits },
    { "ups.status",        0, nullptr, "QS\r", 27, '#', 42, 42, nullptr, QX_FLAG_QUICK_POLL, nullptr, qsHexPreprocessAnswer, blazerProcessStatusBits },
    { "ups.alarm",         0, nullptr, "QS\r", 27, '#', 43, 43, nullptr, 0,                  nullptr, qsHexPreprocessAnswer, blazerProcessStatusBits },
    { "ups.type",          0, nullptr, "QS\r", 27, '#', 44, 44, "%s",    QX_FLAG_STATIC,     nullptr, qsHexPreprocessAnswer, blazerProcessStatusBits },
    { "ups.status",        0, nullptr, "QS\r", 27, '#', 45, 45, nullptr, QX_FLAG_QUICK_POLL, nullptr, qsHexPreprocessAnswer, blazerProcessStatusBits },
    { "ups.status",        0, nullptr, "QS\r", 27, '#', 46, 46, nullptr, QX_FLAG_QUICK_POLL, nullptr, qsHexPreprocessAnswer, blazerProcessStatusBits },
    { "ups.beeper.status", 0, nullptr, "QS\r", 27, '#', 47, 47, "%s",    0,                  nullptr, qsHexPreprocessAnswer, blazerProcessStatusBits },

    /* End of structure. */
    { nullptr, 0, nullptr, nullptr, 0, 0, 0, 0, nullptr, 0, nullptr, nullptr, nullptr }
};

static bool qsHexClaim(QxEngine& qx) {
    return qx.claim("QS\r", 27, '#');
}

const QxSubdriver QX_VOLTRONIC_QS_HEX = {
    "Voltronic-QS-Hex 0.10",
    qsHexClaim,
    VOLTRONIC_QS_HEX_QX2NUT,
    blazerInitupsLight,
    nullptr,
    "ACK",
    nullptr,
};
