/*
* Author: Christian Huitema
* Copyright (c) 2021, Private Octopus, Inc.
* All rights reserved.
*
* Permission to use, copy, modify, and distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL Private Octopus, Inc. BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <picoquic.h>
#include <picoquic_internal.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "fuzi_q.h"

/* This module holds a collection of QUIc frames, that can be inserted at
 * random positions in fuzzed frames.
 *
 * The first set of test frames is copied for picoquic tests.
 */

static uint8_t test_frame_type_padding[] = { 0, 0, 0 };

static uint8_t test_frame_type_padding_zero_byte[] = { 0x00 };

static uint8_t test_frame_type_padding_large[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 10 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 20 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 30 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 40 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 50 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 60 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 70 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 80 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 90 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 100 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 110 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 120 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 130 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 140 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 150 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 160 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 170 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 180 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 190 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  /* 200 */
};

static uint8_t test_frame_type_padding_5_bytes[] = { 0, 0, 0, 0, 0 };

static uint8_t test_frame_type_padding_7_bytes[] = { 0, 0, 0, 0, 0, 0, 0 };

static uint8_t test_frame_type_padding_13_bytes[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static uint8_t test_frame_padding_2_bytes[] = { 0x00, 0x00 };

static uint8_t test_frame_padding_10_bytes[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

static uint8_t test_frame_padding_50_bytes[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 10 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 20 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 30 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 40 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  /* 50 */
};

/* PADDING frame (type 0x00) followed by non-zero bytes that are part of the padding */
static uint8_t test_frame_padding_mixed_payload[] = {
    0x00,       /* Type: PADDING frame */
    0xFF,       /* Arbitrary byte 1 */
    0xAA,       /* Arbitrary byte 2 */
    0x55,       /* Arbitrary byte 3 */
    0xCC        /* Arbitrary byte 4 */
};

static uint8_t test_frame_type_reset_stream[] = {
    picoquic_frame_type_reset_stream,
    17,
    1,
    1
};

static uint8_t test_frame_type_reset_stream_high_error[] = {
    picoquic_frame_type_reset_stream,
    0x11,
    0xBF, 0xFF, 0xAA, 0xAA, /* Application Protocol Error Code: 0x3FFFAAAA */
    0x41, 0x00 /* Final Size: 0x100 */
};

static uint8_t test_frame_reset_stream_min_vals[] = {
    picoquic_frame_type_reset_stream, 0x00, 0x00, 0x00
};

static uint8_t test_frame_reset_stream_max_final_size[] = {
    picoquic_frame_type_reset_stream, 0x01, 0x00,
    0xBF, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 /* Varint for 0x3F00112233445566 */
};

static uint8_t test_frame_reset_stream_app_error_specific[] = {
    picoquic_frame_type_reset_stream, 0x02, 0x41, 0x00, 0x42, 0x00
};

/* New RESET_STREAM frame test cases */
/* Base Case */
static uint8_t test_reset_stream_base[] = {
    picoquic_frame_type_reset_stream, /* 0x04 */
    0x04,       /* Stream ID: 4 */
    0x41, 0x01, /* Application Protocol Error Code: 257 (0x101) */
    0x64        /* Final Size: 100 */
};

/* Stream ID Variations (ErrorCode=257, FinalSize=100) */
static uint8_t test_reset_stream_id_zero[] = {
    picoquic_frame_type_reset_stream, /* 0x04 */
    0x00,       /* Stream ID: 0 */
    0x41, 0x01, /* Application Protocol Error Code: 257 */
    0x64        /* Final Size: 100 */
};

static uint8_t test_reset_stream_id_large[] = {
    picoquic_frame_type_reset_stream, /* 0x04 */
    0x7F, 0xFF, /* Stream ID: 16383 (0x3FFF) */
    0x41, 0x01, /* Application Protocol Error Code: 257 */
    0x64        /* Final Size: 100 */
};

static uint8_t test_reset_stream_id_max_62bit[] = {
    picoquic_frame_type_reset_stream, /* 0x04 */
    0xBF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, /* Stream ID: 2^62-1 */
    0x41, 0x01, /* Application Protocol Error Code: 257 */
    0x64        /* Final Size: 100 */
};

/* Application Protocol Error Code Variations (StreamID=4, FinalSize=100) */
static uint8_t test_reset_stream_err_zero[] = {
    picoquic_frame_type_reset_stream, /* 0x04 */
    0x04,       /* Stream ID: 4 */
    0x00,       /* Application Protocol Error Code: 0 */
    0x64        /* Final Size: 100 */
};

static uint8_t test_reset_stream_err_transport_range_like[] = {
    picoquic_frame_type_reset_stream, /* 0x04 */
    0x04,       /* Stream ID: 4 */
    0x0A,       /* Application Protocol Error Code: 0x0A (like PROTOCOL_VIOLATION) */
    0x64        /* Final Size: 100 */
};

static uint8_t test_reset_stream_err_max_62bit[] = {
    picoquic_frame_type_reset_stream, /* 0x04 */
    0x04,       /* Stream ID: 4 */
    0xBF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, /* Application Protocol Error Code: 2^62-1 */
    0x64        /* Final Size: 100 */
};

/* Final Size Variations (StreamID=4, ErrorCode=257) */
static uint8_t test_reset_stream_final_size_zero[] = {
    picoquic_frame_type_reset_stream, /* 0x04 */
    0x04,       /* Stream ID: 4 */
    0x41, 0x01, /* Application Protocol Error Code: 257 */
    0x00        /* Final Size: 0 */
};

static uint8_t test_reset_stream_final_size_one[] = {
    picoquic_frame_type_reset_stream, /* 0x04 */
    0x04,       /* Stream ID: 4 */
    0x41, 0x01, /* Application Protocol Error Code: 257 */
    0x01        /* Final Size: 1 */
};

static uint8_t test_reset_stream_final_size_scenario_small[] = {
    picoquic_frame_type_reset_stream, /* 0x04 */
    0x04,       /* Stream ID: 4 */
    0x41, 0x01, /* Application Protocol Error Code: 257 */
    0x32        /* Final Size: 50 */
};

static uint8_t test_reset_stream_final_size_max_62bit[] = {
    picoquic_frame_type_reset_stream, /* 0x04 */
    0x04,       /* Stream ID: 4 */
    0x41, 0x01, /* Application Protocol Error Code: 257 */
    0xBF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF /* Final Size: 2^62-1 */
};

/* RESET_STREAM (0x04) - New test cases from plan */
static uint8_t test_frame_reset_stream_sid_zero[] = {
    picoquic_frame_type_reset_stream, 0x00, 0x00, 0x00
};
/* reset_stream_final_size_zero is covered by test_frame_reset_stream_app_error_specific if StreamID=1, Error=0, FinalSize=0 is needed.
   The provided test_frame_reset_stream_app_error_specific is {0x04, 0x01, 0x00, 0x01} (StreamID=1, ErrorCode=0, FinalSize=1).
   Let's create the exact requested one: StreamID=1, ErrorCode=0, FinalSize=0 */
static uint8_t test_frame_reset_stream_final_size_zero_explicit[] = { /* Renamed to avoid conflict if user meant a different existing one */
    picoquic_frame_type_reset_stream, 0x01, 0x00, 0x00
};
/* reset_stream_app_err_zero is {0x04, 0x01, 0x00, 0x01} (StreamID=1, ErrorCode=0, FinalSize=1) */
/* This is exactly test_frame_reset_stream_app_error_specific. No need for a new array. */

static uint8_t test_frame_reset_stream_all_large[] = {
    picoquic_frame_type_reset_stream,
    0x7F, 0xFF,       /* Stream ID: 16383 (0x3FFF) */
    0xBF, 0xFF, 0xFF, 0xFF, /* App Error Code: 1073741823 (0x3FFFFFFF) */
    0xBF, 0xFF, 0xFF, 0xFF  /* Final Size: 1073741823 (0x3FFFFFFF) */
};

static uint8_t test_frame_reset_stream_error_code_max[] = {
    picoquic_frame_type_reset_stream,
    0x01, /* Stream ID */
    0xBF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, /* Error Code: (1ULL << 62) - 1 */
    0x00 /* Final Size */
};

static uint8_t test_frame_reset_stream_final_size_max_new[] = {
    picoquic_frame_type_reset_stream,
    0x01, /* Stream ID */
    0x00, /* Error Code */
    0xBF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF /* Final Size: (1ULL << 62) - 1 */
};


static uint8_t test_type_connection_close[] = {
    picoquic_frame_type_connection_close,
    0x80, 0x00, 0xCF, 0xFF, 0,
    9,
    '1', '2', '3', '4', '5', '6', '7', '8', '9'
};

static uint8_t test_frame_connection_close_transport_long_reason[] = {
    picoquic_frame_type_connection_close, 0x1A, 0x00, 0x20,
    'T','h','i','s',' ','i','s',' ','a',' ','v','e','r','y',' ','l','o','n','g',' ','t','e','s','t',' ','r','e','a','s','o','n','.'
};

static uint8_t test_type_application_close[] = {
    picoquic_frame_type_application_close,
    0,
    0
};

static uint8_t test_type_application_close_reason[] = {
    picoquic_frame_type_application_close,
    0x44, 4,
    4,
    't', 'e', 's', 't'
};

static uint8_t test_frame_application_close_long_reason[] = {
    picoquic_frame_type_application_close, 0x2B, 0x1E,
    'A','n','o','t','h','e','r',' ','l','o','n','g',' ','a','p','p','l','i','c','a','t','i','o','n',' ','e','r','r','o','r','.'
};

static uint8_t test_frame_conn_close_no_reason[] = { 0x1c, 0x00, 0x00, 0x00 };

static uint8_t test_frame_conn_close_app_no_reason[] = { 0x1d, 0x00, 0x00 };

static uint8_t test_frame_conn_close_specific_transport_error[] = { 0x1c, 0x07, 0x15, 0x05, 'B','a','d','F','R' };

static uint8_t test_frame_connection_close_frame_encoding_error[] = {
    picoquic_frame_type_connection_close,       /* 0x1c */
    0x07,                                       /* FRAME_ENCODING_ERROR */
    0x00,                                       /* Offending Frame Type (e.g., PADDING) */
    0x00                                        /* Reason Phrase Length 0 */
};

static uint8_t test_frame_type_max_data[] = {
    picoquic_frame_type_max_data,
    0xC0, 0, 0x01, 0, 0, 0, 0, 0
};

static uint8_t test_frame_type_max_data_large[] = {
    picoquic_frame_type_max_data,
    0xBF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF
};

static uint8_t test_frame_max_data_zero[] = {
    picoquic_frame_type_max_data, 0x00
};

static uint8_t test_frame_max_data_small_value[] = {
    picoquic_frame_type_max_data, 0x44, 0x00 /* 1024 */
};

static uint8_t test_frame_type_max_stream_data[] = {
    picoquic_frame_type_max_stream_data,
    1,
    0x80, 0x01, 0, 0
};

static uint8_t test_frame_max_stream_data_zero[] = {
    picoquic_frame_type_max_stream_data, 0x02, 0x00
};

static uint8_t test_frame_type_max_streams_bidir[] = {
    picoquic_frame_type_max_streams_bidir,
    0x41, 0
};

static uint8_t test_frame_type_max_streams_unidir[] = {
    picoquic_frame_type_max_streams_unidir,
    0x41, 7
};

static uint8_t test_frame_type_max_streams_bidir_alt[] = {
    picoquic_frame_type_max_streams_bidir,
    0x42, 0x0A
};

static uint8_t test_frame_type_max_streams_bidir_zero[] = {
    picoquic_frame_type_max_streams_bidir,
    0x00
};

static uint8_t test_frame_max_streams_bidi_very_high[] = {
    picoquic_frame_type_max_streams_bidir, 0xBF, 0xFF, 0xFF, 0xFF
};

static uint8_t test_frame_type_max_streams_unidir_zero[] = {
    picoquic_frame_type_max_streams_unidir,
    0x00
};

static uint8_t test_frame_max_streams_uni_very_high[] = {
    picoquic_frame_type_max_streams_unidir, 0xBF, 0xFF, 0xFF, 0xFE
};

/* MAX_STREAMS (Unidirectional) frame with a very large stream limit (2^60) */
static uint8_t test_frame_max_streams_uni_at_limit[] = {
    0x13,       /* Type: MAX_STREAMS (Unidirectional) */
    0xC0, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00 /* Max Streams: 2^60 (Varint encoded) */
};

/* MAX_DATA (0x10) - New test cases from plan */
/* test_frame_max_data_val_zero is identical to existing test_frame_max_data_zero, so it's removed. */
static uint8_t test_frame_max_data_val_large[] = {
    picoquic_frame_type_max_data, 0xBF, 0xFF, 0xFF, 0xFF /* Max Data: 1073741823 (0x3FFFFFFF) */
};


static uint8_t test_frame_type_ping[] = {
    picoquic_frame_type_ping
};

/* Test Case: PING frame type encoded non-minimally.
 * Frame Type: PING (normally 0x01) encoded as a 2-byte varint (0x4001).
 */
static uint8_t test_frame_ping_long_encoding[] = {
    0x40, 0x01
};

static uint8_t test_frame_type_blocked[] = {
    picoquic_frame_type_data_blocked,
    0x80, 0x01, 0, 0
};

static uint8_t test_frame_type_data_blocked_large_offset[] = {
    picoquic_frame_type_data_blocked,
    0xBF, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE
};

static uint8_t test_frame_data_blocked_zero[] = {
    picoquic_frame_type_data_blocked, 0x00
};

static uint8_t test_frame_type_stream_blocked[] = {
    picoquic_frame_type_stream_data_blocked,
    0x80, 1, 0, 0,
    0x80, 0x02, 0, 0
};

static uint8_t test_frame_type_stream_data_blocked_large_limits[] = {
    picoquic_frame_type_stream_data_blocked,
    0xBA, 0x1B, 0x2C, 0x3D, /* Stream ID */
    0xBE, 0x4F, 0x5D, 0x6C  /* Stream Data Limit */
};

static uint8_t test_frame_stream_data_blocked_zero[] = {
    picoquic_frame_type_stream_data_blocked, 0x03, 0x00
};

static uint8_t test_frame_type_streams_blocked_bidir[] = {
    picoquic_frame_type_streams_blocked_bidir,
    0x41, 0
};

static uint8_t test_frame_streams_blocked_bidi_zero[] = {
    picoquic_frame_type_streams_blocked_bidir, 0x00
};

static uint8_t test_frame_type_streams_blocked_unidir[] = {
    picoquic_frame_type_streams_blocked_unidir,
    0x81, 2, 3, 4
};

static uint8_t test_frame_streams_blocked_uni_zero[] = {
    picoquic_frame_type_streams_blocked_unidir, 0x00
};

/* Test Case 1: STREAMS_BLOCKED (bidirectional) indicating a limit that isn't actually blocking.
 * Type: STREAMS_BLOCKED (bidirectional, 0x16)
 * Maximum Streams: 5
 * Scenario: Peer's actual limit is higher (e.g., 10).
 */
static uint8_t test_frame_streams_blocked_not_actually_blocked[] = {
    picoquic_frame_type_streams_blocked_bidir, /* 0x16 */
    0x05 /* Maximum Streams: 5 */
};

/* Test Case 2: STREAMS_BLOCKED (unidirectional) indicating a limit higher than the peer's actual limit.
 * Type: STREAMS_BLOCKED (unidirectional, 0x17)
 * Maximum Streams: 100
 * Scenario: Peer's actual limit is lower (e.g., 10).
 */
static uint8_t test_frame_streams_blocked_limit_too_high[] = {
    picoquic_frame_type_streams_blocked_unidir, /* 0x17 */
    0x64 /* Maximum Streams: 100 */
};

static uint8_t test_frame_type_new_connection_id[] = {
    picoquic_frame_type_new_connection_id,
    7,
    0,
    8,
    1, 2, 3, 4, 5, 6, 7, 8,
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
    0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF
};

static uint8_t test_frame_type_stop_sending[] = {
    picoquic_frame_type_stop_sending,
    17,
    0x17
};

static uint8_t test_frame_type_stop_sending_high_error[] = {
    picoquic_frame_type_stop_sending,
    0x12,
    0xBF, 0xFF, 0xBB, 0xBB /* Application Protocol Error Code: 0x3FFFBBBB */
};

static uint8_t test_frame_stop_sending_min_vals[] = {
    picoquic_frame_type_stop_sending, 0x00, 0x00
};

static uint8_t test_frame_stop_sending_app_error_specific[] = {
    picoquic_frame_type_stop_sending, 0x01, 0x41, 0x00
};

/* Test Case 1: STOP_SENDING for a stream already reset by the peer.
 * Type: STOP_SENDING (0x05)
 * Stream ID: 4
 * Application Protocol Error Code: 0x01 (generic app error)
 */
static uint8_t test_frame_stop_sending_for_peer_reset_stream[] = {
    picoquic_frame_type_stop_sending, /* 0x05 */
    0x04, /* Stream ID: 4 */
    0x01  /* Application Protocol Error Code: 0x01 */
};

/* Test Case 2: STOP_SENDING with a very large error code.
 * Type: STOP_SENDING (0x05)
 * Stream ID: 8
 * Application Protocol Error Code: 0x3FFFFFFFFFFFFFFF (max 8-byte varint)
 */
static uint8_t test_frame_stop_sending_large_error_code[] = {
    picoquic_frame_type_stop_sending, /* 0x05 */
    0x08, /* Stream ID: 8 */
    0xBF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF /* Error Code: 0x3FFFFFFFFFFFFFFF */
};

/* STOP_SENDING (0x05) - New test cases from plan */
static uint8_t test_frame_stop_sending_sid_err_zero[] = {
    picoquic_frame_type_stop_sending, 0x00, 0x00
};
static uint8_t test_frame_stop_sending_all_large[] = {
    picoquic_frame_type_stop_sending,
    0x7F, 0xFF,       /* Stream ID: 16383 (0x3FFF) */
    0xBF, 0xFF, 0xFF, 0xFF  /* App Error Code: 1073741823 (0x3FFFFFFF) */
};

/* New STOP_SENDING frame test cases */
/* Base Case */
static uint8_t test_stop_sending_base[] = {
    picoquic_frame_type_stop_sending, /* 0x05 */
    0x04,       /* Stream ID: 4 */
    0x41, 0x01  /* Application Protocol Error Code: 257 (0x101) */
};

/* Stream ID Variations (ErrorCode=257) */
static uint8_t test_stop_sending_id_zero[] = {
    picoquic_frame_type_stop_sending, /* 0x05 */
    0x00,       /* Stream ID: 0 */
    0x41, 0x01  /* Application Protocol Error Code: 257 */
};

static uint8_t test_stop_sending_id_large[] = {
    picoquic_frame_type_stop_sending, /* 0x05 */
    0x7F, 0xFF, /* Stream ID: 16383 (0x3FFF) */
    0x41, 0x01  /* Application Protocol Error Code: 257 */
};

static uint8_t test_stop_sending_id_max_62bit[] = {
    picoquic_frame_type_stop_sending, /* 0x05 */
    0xBF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, /* Stream ID: 2^62-1 */
    0x41, 0x01  /* Application Protocol Error Code: 257 */
};

static uint8_t test_stop_sending_id_recv_only_scenario[] = {
    picoquic_frame_type_stop_sending, /* 0x05 */
    0x02,       /* Stream ID: 2 (client-initiated uni) */
    0x41, 0x01  /* Application Protocol Error Code: 257 */
};

static uint8_t test_stop_sending_id_uncreated_sender_scenario[] = {
    picoquic_frame_type_stop_sending, /* 0x05 */
    0x0C,       /* Stream ID: 12 */
    0x41, 0x01  /* Application Protocol Error Code: 257 */
};

/* Application Protocol Error Code Variations (StreamID=4) */
static uint8_t test_stop_sending_err_zero[] = {
    picoquic_frame_type_stop_sending, /* 0x05 */
    0x04,       /* Stream ID: 4 */
    0x00        /* Application Protocol Error Code: 0 */
};

static uint8_t test_stop_sending_err_transport_range_like[] = {
    picoquic_frame_type_stop_sending, /* 0x05 */
    0x04,       /* Stream ID: 4 */
    0x0A        /* Application Protocol Error Code: 0x0A (like PROTOCOL_VIOLATION) */
};

static uint8_t test_stop_sending_err_max_62bit[] = {
    picoquic_frame_type_stop_sending, /* 0x05 */
    0x04,       /* Stream ID: 4 */
    0xBF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF /* Application Protocol Error Code: 2^62-1 */
};

static uint8_t test_frame_type_path_challenge[] = {
    picoquic_frame_type_path_challenge,
    1, 2, 3, 4, 5, 6, 7, 8
};

static uint8_t test_frame_type_path_challenge_alt_data[] = {
    picoquic_frame_type_path_challenge,
    0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x99, 0x88
};

static uint8_t test_frame_type_path_response[] = {
    picoquic_frame_type_path_response,
    1, 2, 3, 4, 5, 6, 7, 8
};

static uint8_t test_frame_type_path_response_alt_data[] = {
    picoquic_frame_type_path_response,
    0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF
};

static uint8_t test_frame_path_challenge_all_zeros[] = {
    picoquic_frame_type_path_challenge, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

static uint8_t test_frame_path_response_all_zeros[] = {
    picoquic_frame_type_path_response, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

static uint8_t test_frame_path_challenge_mixed_pattern[] = {
    picoquic_frame_type_path_challenge, 0xA5,0xA5,0xA5,0xA5,0xA5,0xA5,0xA5,0xA5
};

static uint8_t test_frame_path_response_mixed_pattern[] = {
    picoquic_frame_type_path_response, 0x5A,0x5A,0x5A,0x5A,0x5A,0x5A,0x5A,0x5A
};

static uint8_t test_frame_type_new_token[] = {
    picoquic_frame_type_new_token,
    17, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17
};

static uint8_t test_frame_new_token_long[] = {
    picoquic_frame_type_new_token, 0x40, 0x64,
    0xAB,0xAB,0xAB,0xAB,0xAB,0xAB,0xAB,0xAB,0xAB,0xAB, /* 10 */
    0xAB,0xAB,0xAB,0xAB,0xAB,0xAB,0xAB,0xAB,0xAB,0xAB, /* 20 */
    0xAB,0xAB,0xAB,0xAB,0xAB,0xAB,0xAB,0xAB,0xAB,0xAB, /* 30 */
    0xAB,0xAB,0xAB,0xAB,0xAB,0xAB,0xAB,0xAB,0xAB,0xAB, /* 40 */
    0xAB,0xAB,0xAB,0xAB,0xAB,0xAB,0xAB,0xAB,0xAB,0xAB, /* 50 */
    0xAB,0xAB,0xAB,0xAB,0xAB,0xAB,0xAB,0xAB,0xAB,0xAB, /* 60 */
    0xAB,0xAB,0xAB,0xAB,0xAB,0xAB,0xAB,0xAB,0xAB,0xAB, /* 70 */
    0xAB,0xAB,0xAB,0xAB,0xAB,0xAB,0xAB,0xAB,0xAB,0xAB, /* 80 */
    0xAB,0xAB,0xAB,0xAB,0xAB,0xAB,0xAB,0xAB,0xAB,0xAB, /* 90 */
    0xAB,0xAB,0xAB,0xAB,0xAB,0xAB,0xAB,0xAB,0xAB,0xAB  /* 100 */
};

static uint8_t test_frame_new_token_short[] = {
    picoquic_frame_type_new_token, 0x08, 0xCD,0xCD,0xCD,0xCD,0xCD,0xCD,0xCD,0xCD
};

static uint8_t test_frame_type_ack[] = {
    picoquic_frame_type_ack,
    0xC0, 0, 0, 1, 2, 3, 4, 5,
    0x44, 0,
    2,
    5,
    0, 0,
    5, 12
};

static uint8_t test_frame_ack_empty[] = {
    picoquic_frame_type_ack, 0x0A, 0x01, 0x00, 0x00
};

static uint8_t test_frame_ack_multiple_ranges[] = {
    picoquic_frame_type_ack, 0x20, 0x02, 0x03, 0x02,  0x01, 0x04,  0x03, 0x01,  0x05, 0x0A
};

static uint8_t test_frame_ack_large_delay[] = {
    picoquic_frame_type_ack, 0x05, 0x7F, 0xFF, /*0x3FFF encoded*/ 0x00, 0x01
};

static uint8_t test_frame_type_ack_ecn[] = {
    picoquic_frame_type_ack_ecn,
    0xC0, 0, 0, 1, 2, 3, 4, 5,
    0x44, 0,
    2,
    5,
    0, 0,
    5, 12,
    3, 0, 1
};

static uint8_t test_frame_ack_ecn_counts_high[] = {
    picoquic_frame_type_ack_ecn, 0x10, 0x01, 0x00, 0x00,  0x41, 0x00,  0x42, 0x00,  0x43, 0x00
};

/* ACK frame with Largest Ack = 50, ACK Delay = 10, 20 ACK ranges, each acking a single packet */
static uint8_t test_frame_ack_many_small_ranges[] = {
    0x02,       /* Type: ACK frame */
    0x32,       /* Largest Acknowledged: 50 */
    0x0A,       /* ACK Delay: 10 */
    0x13,       /* ACK Range Count: 19 (represents 20 ranges: First + 19 more) */
    0x00,       /* First ACK Range: 0 (acks packet 50) */
    /* 19 more ranges, each Gap=0, Range=0 */
    0x00, 0x00, /* Gap 0, Range 0 (acks 49) */
    0x00, 0x00, /* Gap 0, Range 0 (acks 48) */
    0x00, 0x00, /* Gap 0, Range 0 (acks 47) */
    0x00, 0x00, /* Gap 0, Range 0 (acks 46) */
    0x00, 0x00, /* Gap 0, Range 0 (acks 45) */
    0x00, 0x00, /* Gap 0, Range 0 (acks 44) */
    0x00, 0x00, /* Gap 0, Range 0 (acks 43) */
    0x00, 0x00, /* Gap 0, Range 0 (acks 42) */
    0x00, 0x00, /* Gap 0, Range 0 (acks 41) */
    0x00, 0x00, /* Gap 0, Range 0 (acks 40) */
    0x00, 0x00, /* Gap 0, Range 0 (acks 39) */
    0x00, 0x00, /* Gap 0, Range 0 (acks 38) */
    0x00, 0x00, /* Gap 0, Range 0 (acks 37) */
    0x00, 0x00, /* Gap 0, Range 0 (acks 36) */
    0x00, 0x00, /* Gap 0, Range 0 (acks 35) */
    0x00, 0x00, /* Gap 0, Range 0 (acks 34) */
    0x00, 0x00, /* Gap 0, Range 0 (acks 33) */
    0x00, 0x00, /* Gap 0, Range 0 (acks 32) */
    0x00, 0x00  /* Gap 0, Range 0 (acks 31) */
};

/* Test Case 1: Overlapping ACK Ranges.
 * Type: ACK (0x02)
 * Largest Acknowledged: 20 (0x14)
 * ACK Delay: 10 (0x0A)
 * ACK Range Count: 2 (0x02)
 * First ACK Range: 5 (0x05) (acks packets 15-20)
 * Second ACK Range:
 *   Gap: 1 (0x01) (previous smallest was 15. next largest acked by this range is 15 - 1 - 2 = 12)
 *   ACK Range Length: 4 (0x04) (acks packets 12 - 4 = 8 to 12). This range (8-12) overlaps with (15-20) due to how ranges are calculated.
 *   The test is to see if the parser correctly handles or flags this condition if it's considered invalid.
 *   Note: The example in the description seems to have a direct overlap logic,
 *   but QUIC ACK range processing means a gap reduces the next range's numbers.
 *   Let's define a case where the *resulting* packet numbers from two ranges would overlap if not processed carefully or if gaps lead to unexpected results.
 *   Largest Ack: 20. First Range: 5 (acks 15-20). Smallest_prev = 15.
 *   Gap: 1. Next_Largest_Acked_By_Range = Smallest_prev - Gap - 2 = 15 - 1 - 2 = 12.
 *   RangeLength: 7 (acks 12-7 = 5 to 12). Packets 5-12 and 15-20. No direct number overlap.
 *
 *   To create an overlap based on the problem description's intent (e.g. range 1 acks X to Y, range 2 acks W to Z, and they overlap):
 *   Largest Ack: 20 (0x14)
 *   ACK Delay: 10 (0x0A)
 *   ACK Range Count: 2 (0x02)
 *   First ACK Range: 5 (0x05) -> acks pkts (20-5) to 20 = 15 to 20. Smallest in this range is 15.
 *   Gap for 2nd range: 2 (0x02) -> next largest pkt in 2nd range = 15 - 2 - 2 = 11.
 *   ACK Range Length for 2nd range: 3 (0x03) -> acks pkts (11-3) to 11 = 8 to 11. No overlap.
 *
 *   Let's try to make the second range ACK numbers ALREADY covered by the first range.
 *   Largest Ack: 20. First Range: 5 (acks 15-20). Smallest in this range is 15.
 *   To make the next range ack something like 16,17:
 *   Next_Largest_Acked_By_Range = 17. Smallest_prev - Gap - 2 = 17.
 *   15 - Gap - 2 = 17  => 13 - Gap = 17 => Gap = -4. Not possible.
 *
 *   The definition of "overlapping" here might mean that the *sum* of (Gap + Range Length) for a subsequent block
 *   somehow encroaches into the space defined by a prior block, or that a block defines a range already covered.
 *   The standard processing implies ranges are ordered. The "First ACK Range" is for the highest packet numbers.
 *   Subsequent ranges are for lower packet numbers.
 *   A true overlap where e.g. range 1 covers 15-20 and range 2 covers 18-22 is not possible with the gap logic.
 *   Let's assume "overlapping" means a range that re-acknowledges a packet number that would have been
 *   covered by a previous (higher value) range if the ranges were strictly sequential and non-overlapping.
 *   This seems more like a test of complex gap arithmetic.
 *   The provided example `{ 0x02, 20, 10, 2, 5, 1, 4 };`
 *   Largest Ack = 20. Delay = 10. Range Count = 2.
 *   Range 1: Len = 5. Acks 15, 16, 17, 18, 20. Smallest = 15.
 *   Range 2: Gap = 1. Next Largest = 15 - 1 - 2 = 12. Len = 4. Acks 8, 9, 10, 11, 12.
 *   These ranges (15-20 and 8-12) are not overlapping.
 *
 *   Given the problem statement, the user likely intends a scenario that might be invalidly constructed.
 *   Let's stick to the user's example values directly, assuming it represents an edge case they want to test,
 *   even if it doesn't create a direct numerical overlap in the final acknowledged set due to standard processing.
 *   The term "overlapping" might be used loosely to mean "a complex interaction of ranges".
 */
static uint8_t test_frame_ack_overlapping_ranges[] = {
    0x02, /* Type: ACK */
    20,   /* Largest Acknowledged */
    10,   /* ACK Delay */
    2,    /* ACK Range Count */
    5,    /* First ACK Range Length (acks 15-20) */
    1,    /* Gap (next range starts relative to 15) */
    4     /* Second ACK Range Length (next largest is 15-1-2=12, acks 8-12) */
};

/* Test Case 2: ACK ranges that would imply ascending order or invalid gap.
 * Type: ACK (0x02)
 * Largest Acknowledged: 5
 * ACK Delay: 0
 * ACK Range Count: 2 (to have a "next" range)
 * First ACK Range: 2 (acks packets 3-5). Smallest in this range is 3.
 * Second ACK Range:
 *   Gap: 10 (This is the key part. Next largest ack in this range would be 3 - 10 - 2 = -9, which is invalid)
 *   ACK Range Length: 0 (minimal valid length for a range)
 */
static uint8_t test_frame_ack_ascending_ranges_invalid_gap[] = {
    0x02, /* Type: ACK */
    5,    /* Largest Acknowledged */
    0,    /* ACK Delay */
    2,    /* ACK Range Count */
    2,    /* First ACK Range (acks 3-5, smallest is 3) */
    10,   /* Gap (implies next largest is 3-10-2 = -9) */
    0     /* ACK Range Length for the second range */
};


/* Test Case 3: Invalid ACK Range Count (too large for the actual data provided).
 * Type: ACK (0x02)
 * Largest Acknowledged: 100 (0x64)
 * ACK Delay: 20 (0x14)
 * ACK Range Count: 200 (0xC8, varint encoded as 0x40, 0xC8 is wrong, it's 0x80 00 00 C8 for 4 bytes, or 0x40 C8 for 2 bytes if < 16383)
 *   Let's use 200, which is 0xC8. If it's a 1-byte varint, it's > 63, so it needs 2 bytes: 0x40 | (0xC8>>8) , 0xC8&0xFF -> 0x40, 0xC8.
 *   No, 200 is 11001000 in binary. It fits in 1 byte with 0 prefix: 0xc8.
 *   If Range Count is 200 (0xc8), it will be encoded as two bytes: 0x40 followed by 0xc8 is not correct.
 *   A varint for 200 is simply 0xC8 if it was <64.
 *   For 200: bits are 11001000. Two-byte encoding: 0x80 | (value >> 8), value & 0xFF.
 *   No, that's for values > 2^14.
 *   For 200: first byte is 0b01... (for 2-byte), so 0x40 + (200>>8) = 0x40. Second byte is 200&0xFF = 0xC8.
 *   So, 0x40, 0xC8 is correct for 200. The problem states 0x40, 0xc8 for 200.
 *   Actually, 200 in varint is: 0x80+128=200 -> 0x80+0x48 -> 0xC8. No, 200 = 128 + 72. So 0x80 | 0x48, 0x48.
 *   Let's re-check varint for 200 (0xC8):
 *   Since 200 > 63 and < 16383, it's a 2-byte varint.
 *   First byte: 0x40 | (200 >> 8) = 0x40 | 0 = 0x40.
 *   Second byte: 200 & 0xFF = 0xC8.
 *   So, 0x40, 0xC8 is correct for 200.
 * First ACK Range: 0 (0x00)
 * Provide only a few actual ranges, far less than 200.
 * The frame itself will be short, but the count implies many more.
 */
static uint8_t test_frame_ack_invalid_range_count[] = {
    0x02,       /* Type: ACK */
    100,        /* Largest Acknowledged */
    20,         /* ACK Delay */
    0x40, 0xC8, /* ACK Range Count: 200 (varint) */
    0,          /* First ACK Range Length */
    0,0,        /* Minimal Gap & Range */
    0,0,        /* Minimal Gap & Range */
    0,0         /* Minimal Gap & Range */
    /* Total frame length here is 1 + 1 + 1 + 2 + 1 + 2 + 2 + 2 = 12 bytes */
    /* but range count says 200 ranges. */
};

/* Test Case 4: Largest Acknowledged is smaller than the packet number implied by First ACK Range.
 * Type: ACK (0x02)
 * Largest Acknowledged: 5 (0x05)
 * ACK Delay: 0 (0x00)
 * ACK Range Count: 1 (0x01)
 * First ACK Range: 10 (0x0A) (implies packets (5-10) to 5, so -5 to 5. This is invalid.)
 */
static uint8_t test_frame_ack_largest_smaller_than_range[] = {
    0x02, /* Type: ACK */
    5,    /* Largest Acknowledged */
    0,    /* ACK Delay */
    1,    /* ACK Range Count */
    10    /* First ACK Range (length 10, implies acking below 0 if LargestAck is 5) */
};

/* ACK Delay variations */
static uint8_t test_ack_delay_zero[] = {
    0x02, /* Type: ACK */
    0x64, /* Largest Acknowledged: 100 */
    0x00, /* ACK Delay: 0 */
    0x01, /* ACK Range Count: 1 */
    0x00  /* First ACK Range: 0 */
};

static uint8_t test_ack_delay_effective_max_tp_val[] = {
    0x02, /* Type: ACK */
    0x64, /* Largest Acknowledged: 100 */
    0x87, 0xFF, /* ACK Delay: 2047 (max_ack_delay/8 with default_ack_exponent=3) */
    0x01, /* ACK Range Count: 1 */
    0x00  /* First ACK Range: 0 */
};

static uint8_t test_ack_delay_max_varint_val[] = {
    0x02, /* Type: ACK */
    0x64, /* Largest Acknowledged: 100 */
    0x7F, 0xFF, /* ACK Delay: 16383 (max 2-byte varint) */
    0x01, /* ACK Range Count: 1 */
    0x00  /* First ACK Range: 0 */
};

/* ACK Range Count variations */
static uint8_t test_ack_range_count_zero[] = {
    0x02, /* Type: ACK */
    0x64, /* Largest Acknowledged: 100 */
    0x0A, /* ACK Delay: 10 */
    0x00, /* ACK Range Count: 0 */
    0x00  /* First ACK Range: 0 */
};

static uint8_t test_ack_range_count_one[] = {
    0x02, /* Type: ACK */
    0x64, /* Largest Acknowledged: 100 */
    0x0A, /* ACK Delay: 10 */
    0x01, /* ACK Range Count: 1 */
    0x05, /* First ACK Range: 5 */
    0x00, /* Gap: 0 */
    0x00  /* ACK Range Length: 0 */
};

static uint8_t test_ack_range_count_many[] = {
    0x02, /* Type: ACK */
    0x64, /* Largest Acknowledged: 100 */
    0x0A, /* ACK Delay: 10 */
    0x3C, /* ACK Range Count: 60 */
    0x00, /* First ACK Range: 0 */
    0x00, 0x00, /* Gap 0, Len 0 */
    0x00, 0x00, /* Gap 0, Len 0 */
    0x00, 0x00, /* Gap 0, Len 0 */
    0x00, 0x00, /* Gap 0, Len 0 */
    0x00, 0x00  /* Gap 0, Len 0 */
};

/* First ACK Range variations */
static uint8_t test_ack_first_range_zero[] = {
    0x02, /* Type: ACK */
    0x64, /* Largest Acknowledged: 100 */
    0x0A, /* ACK Delay: 10 */
    0x01, /* ACK Range Count: 1 */
    0x00  /* First ACK Range: 0 */
};

static uint8_t test_ack_first_range_causes_negative_smallest[] = {
    0x02, /* Type: ACK */
    0x05, /* Largest Acknowledged: 5 */
    0x00, /* ACK Delay: 0 */
    0x01, /* ACK Range Count: 1 */
    0x0A  /* First ACK Range: 10 */
};

static uint8_t test_ack_first_range_covers_zero[] = {
    0x02, /* Type: ACK */
    0x05, /* Largest Acknowledged: 5 */
    0x00, /* ACK Delay: 0 */
    0x01, /* ACK Range Count: 1 */
    0x05  /* First ACK Range: 5 */
};

/* Gap variations (ACK Range Count >= 1) */
static uint8_t test_ack_gap_zero_len_zero[] = {
    0x02, /* Type: ACK */
    0x14, /* Largest Acknowledged: 20 */
    0x00, /* ACK Delay: 0 */
    0x01, /* ACK Range Count: 1 */
    0x00, /* First ACK Range: 0 */
    0x00, /* Gap: 0 */
    0x00  /* ACK Range Length: 0 */
};

static uint8_t test_ack_gap_causes_negative_next_largest[] = {
    0x02, /* Type: ACK */
    0x14, /* Largest Acknowledged: 20 */
    0x00, /* ACK Delay: 0 */
    0x01, /* ACK Range Count: 1 */
    0x05, /* First ACK Range: 5 (acks 15-20) */
    0x14, /* Gap: 20 (next largest = 15-20-2 = -7) */
    0x00  /* ACK Range Length: 0 */
};

/* ACK Range Length variations (ACK Range Count >= 1) */
static uint8_t test_ack_range_len_large[] = {
    0x02,       /* Type: ACK */
    0x85, 0xDC, /* Largest Acknowledged: 1500 */
    0x00,       /* ACK Delay: 0 */
    0x01,       /* ACK Range Count: 1 */
    0x00,       /* First ACK Range: 0 */
    0x00,       /* Gap: 0 */
    0x83, 0xE8  /* ACK Range Length: 1000 */
};

/* ECN Count variations (Type 0x03) */
static uint8_t test_ack_ecn_all_zero[] = {
    0x03, /* Type: ACK with ECN */
    0x64, /* Largest Acknowledged: 100 */
    0x0A, /* ACK Delay: 10 */
    0x01, /* ACK Range Count: 1 */
    0x00, /* First ACK Range: 0 */
    0x00, /* ECT0: 0 */
    0x00, /* ECT1: 0 */
    0x00  /* CE: 0 */
};

static uint8_t test_ack_ecn_one_each[] = {
    0x03, /* Type: ACK with ECN */
    0x64, /* Largest Acknowledged: 100 */
    0x0A, /* ACK Delay: 10 */
    0x01, /* ACK Range Count: 1 */
    0x00, /* First ACK Range: 0 */
    0x01, /* ECT0: 1 */
    0x01, /* ECT1: 1 */
    0x01  /* CE: 1 */
};

static uint8_t test_ack_ecn_large_counts[] = {
    0x03,       /* Type: ACK with ECN */
    0x64,       /* Largest Acknowledged: 100 */
    0x0A,       /* ACK Delay: 10 */
    0x01,       /* ACK Range Count: 1 */
    0x00,       /* First ACK Range: 0 */
    0x7F, 0xFF, /* ECT0: 16383 */
    0x7F, 0xFF, /* ECT1: 16383 */
    0x7F, 0xFF  /* CE: 16383 */
};

static uint8_t test_ack_ecn_sum_exceeds_largest_acked[] = {
    0x03, /* Type: ACK with ECN */
    0x0A, /* Largest Acknowledged: 10 */
    0x00, /* ACK Delay: 0 */
    0x01, /* ACK Range Count: 1 */
    0x00, /* First ACK Range: 0 */
    0x05, /* ECT0: 5 */
    0x05, /* ECT1: 5 */
    0x05  /* CE: 5 */
};

/* ACK frame with invalid Gap (10 - 20 - 2 = -12) */
static uint8_t test_frame_ack_invalid_gap_1[] = {
    picoquic_frame_type_ack, /* 0x02 */
    0x0A,       /* Largest Acknowledged: 10 */
    0x00,       /* ACK Delay: 0 */
    0x02,       /* ACK Range Count: 2 */
    0x00,       /* First ACK Range: 0 (acks packet 10) */
    0x14,       /* Gap: 20 */
    0x00        /* ACK Range Length for 2nd range: 0 */
};
static uint8_t ack_invalid_gap_1_specific[] = {
    picoquic_frame_type_ack, /* 0x02 */
    0x14,       /* Largest Acknowledged: 20 */
    0x00,       /* ACK Delay: 0 */
    0x02,       /* ACK Range Count: 2 */
    0x00,       /* First ACK Range: 0 (acks packet 20) Add commentMore actions */
    0x1E,       /* Gap: 30 */
    0x00        /* ACK Range Length for 2nd range: 0 */
};
static uint8_t test_frame_type_stream_range_min[] = {
    picoquic_frame_type_stream_range_min,
    1,
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
    0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF
};

static uint8_t test_frame_type_stream_range_max[] = {
    picoquic_frame_type_stream_range_min + 2 + 4,
    1,
    0x44, 0,
    0x10,
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
    0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF
};

static uint8_t test_frame_stream_no_offset_no_len_fin[] = { 0x09, 0x01, 'd','a','t','a' };
static uint8_t test_frame_stream_offset_no_len_no_fin[] = { 0x0C, 0x01, 0x40, 0x20, 'd','a','t','a' };
static uint8_t test_frame_stream_no_offset_len_no_fin[] = { 0x0A, 0x01, 0x04, 'd','a','t','a' };
static uint8_t test_frame_stream_all_bits_set[] = { 0x0F, 0x01, 0x40, 0x20, 0x04, 'd','a','t','a' };
static uint8_t test_frame_stream_zero_len_data[] = { 0x0A, 0x01, 0x00 };
static uint8_t test_frame_stream_max_offset_final[] = { 0x0D, 0x01, 0x52, 0x34, 'e','n','d' };

/* STREAM frame with OFF, LEN, FIN bits set, Stream ID 1, Offset 64, Length 0, No data */
static uint8_t test_frame_stream_off_len_empty_fin[] = {
    0x0F,       /* Type: OFF, LEN, FIN bits set */
    0x01,       /* Stream ID: 1 */
    0x40, 0x40, /* Offset: 64 (Varint encoded) */
    0x00        /* Length: 0 (Varint encoded) */
    /* No Stream Data */
};

/* Test Case 1: STREAM frame with FIN set and explicit length larger than data.
 * Type: 0x0B (FIN=1, OFF=0, LEN=1)
 * Stream ID: 0x04
 * Length: 2000 (Varint encoded as 0x47, 0xD0)
 * Stream Data: "test" (4 bytes)
 */
static uint8_t test_frame_stream_fin_too_long[] = {
    0x0B,       /* Type: FIN=1, LEN=1 */
    0x04,       /* Stream ID: 4 */
    0x47, 0xD0, /* Length: 2000 */
    't', 'e', 's', 't'
};

/* Test Case 2: First part of overlapping STREAM data.
 * Type: 0x0E (FIN=0, OFF=1, LEN=1)
 * Stream ID: 0x08
 * Offset: 10
 * Length: 5
 * Stream Data: "first"
 */
static uint8_t test_frame_stream_overlapping_data_part1[] = {
    0x0E,       /* Type: OFF=1, LEN=1 */
    0x08,       /* Stream ID: 8 */
    10,         /* Offset */
    5,          /* Length */
    'f', 'i', 'r', 's', 't'
};

/* Test Case 3: Second part of overlapping STREAM data.
 * Type: 0x0E (FIN=0, OFF=1, LEN=1)
 * Stream ID: 0x08 (same as part1)
 * Offset: 12 (overlaps with offset 10, length 5 from part1)
 * Length: 5
 * Stream Data: "SECON"
 */
static uint8_t test_frame_stream_overlapping_data_part2[] = {
    0x0E,       /* Type: OFF=1, LEN=1 */
    0x08,       /* Stream ID: 8 */
    12,         /* Offset */
    5,          /* Length */
    'S', 'E', 'C', 'O', 'N'
};

/* Part 1: Base test cases for all 8 STREAM types */
static uint8_t test_stream_0x08_off0_len0_fin0[] = {0x08, 0x04, 'h','e','l','l','o',' ','s','t','r','e','a','m'};
static uint8_t test_stream_0x09_off0_len0_fin1[] = {0x09, 0x04, 'h','e','l','l','o',' ','s','t','r','e','a','m'};
static uint8_t test_stream_0x0A_off0_len1_fin0[] = {0x0A, 0x04, 0x0C, 'h','e','l','l','o',' ','s','t','r','e','a','m'};
static uint8_t test_stream_0x0B_off0_len1_fin1[] = {0x0B, 0x04, 0x0C, 'h','e','l','l','o',' ','s','t','r','e','a','m'};
static uint8_t test_stream_0x0C_off1_len0_fin0[] = {0x0C, 0x04, 0x0A, 'h','e','l','l','o',' ','s','t','r','e','a','m'};
static uint8_t test_stream_0x0D_off1_len0_fin1[] = {0x0D, 0x04, 0x0A, 'h','e','l','l','o',' ','s','t','r','e','a','m'};
static uint8_t test_stream_0x0E_off1_len1_fin0[] = {0x0E, 0x04, 0x0A, 0x0C, 'h','e','l','l','o',' ','s','t','r','e','a','m'};
static uint8_t test_stream_0x0F_off1_len1_fin1[] = {0x0F, 0x04, 0x0A, 0x0C, 'h','e','l','l','o',' ','s','t','r','e','a','m'};

/* Part 2: Variations for STREAM type 0x0F (all bits set) */
static uint8_t test_stream_0x0F_id_zero[] = {0x0F, 0x00, 0x0A, 0x05, 'b','a','s','i','c'};
static uint8_t test_stream_0x0F_id_large[] = {0x0F, 0x7F, 0xFF, 0x0A, 0x05, 'b','a','s','i','c'};
static uint8_t test_stream_0x0F_id_max_62bit[] = {0x0F, 0xBF, 0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, 0x0A, 0x05, 'b','a','s','i','c'};

static uint8_t test_stream_0x0F_off_zero[] = {0x0F, 0x04, 0x00, 0x05, 'b','a','s','i','c'};
static uint8_t test_stream_0x0F_off_max_62bit[] = {0x0F, 0x04, 0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, 0x05, 'b','a','s','i','c'};
static uint8_t test_stream_0x0F_off_plus_len_exceeds_max[] = {0x0F, 0x04, 0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFD, 0x05, 'b','a','s','i','c'};

static uint8_t test_stream_0x0F_len_zero[] = {0x0F, 0x04, 0x0A, 0x00};
static uint8_t test_stream_0x0F_len_one[] = {0x0F, 0x04, 0x0A, 0x01, 'd'};
static uint8_t test_stream_0x0F_len_exceed_total_with_offset[] = {0x0F, 0x04, 0x0A, 0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFB, 'b','a','s','i','c'};

static uint8_t test_frame_type_crypto_hs[] = {
    picoquic_frame_type_crypto_hs,
    0,
    0x10,
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
    0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF
};

static uint8_t test_frame_type_crypto_hs_alt[] = {
    picoquic_frame_type_crypto_hs,
    0x40, 0x10, /* Offset */
    0x08, /* Length */
    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7 /* Crypto Data */
};

static uint8_t test_frame_crypto_zero_len[] = {
    picoquic_frame_type_crypto_hs, 0x00, 0x00
};

static uint8_t test_frame_crypto_large_offset[] = {
    picoquic_frame_type_crypto_hs, 0x50, 0x00, 0x05, 'd','u','m','m','y'
};

static uint8_t test_frame_crypto_fragment1[] = {
    picoquic_frame_type_crypto_hs, 0x00, 0x05, 'H','e','l','l','o'
};

static uint8_t test_frame_crypto_fragment2[] = {
    picoquic_frame_type_crypto_hs, 0x05, 0x05, 'W','o','r','l','d'
};

static uint8_t test_frame_type_retire_connection_id[] = {
    picoquic_frame_type_retire_connection_id,
    1
};

static uint8_t test_frame_retire_cid_seq_zero[] = {
    picoquic_frame_type_retire_connection_id, 0x00
};

static uint8_t test_frame_retire_cid_seq_high[] = {
    picoquic_frame_type_retire_connection_id, 0x0A
};

/* Test Case: RETIRE_CONNECTION_ID that refers to the CID currently in use.
 * Type: RETIRE_CONNECTION_ID (0x19)
 * Sequence Number: 0 (example, implies packet's DCID has sequence 0)
 */
static uint8_t test_frame_retire_cid_current_in_use[] = {
    picoquic_frame_type_retire_connection_id, /* 0x19 */
    0x00 /* Sequence Number: 0 */
};

static uint8_t test_frame_type_datagram[] = {
    picoquic_frame_type_datagram,
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
    0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF
};

static uint8_t test_frame_type_datagram_l[] = {
    picoquic_frame_type_datagram_l,
    0x10,
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
    0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF
};

static uint8_t test_frame_type_handshake_done[] = {
    picoquic_frame_type_handshake_done
};

static uint8_t test_frame_type_ack_frequency[] = {
    0x40, picoquic_frame_type_ack_frequency,
    17, 0x0A, 0x44, 0x20, 0x01
};

static uint8_t test_frame_type_time_stamp[] = {
    (uint8_t)(0x40 | (picoquic_frame_type_time_stamp >> 8)), (uint8_t)(picoquic_frame_type_time_stamp & 0xFF),
    0x44, 0
};

static uint8_t test_frame_type_path_abandon_0[] = {
    (uint8_t)(0x80 | (picoquic_frame_type_path_abandon >> 24)), (uint8_t)(picoquic_frame_type_path_abandon >> 16),
    (uint8_t)(picoquic_frame_type_path_abandon >> 8), (uint8_t)(picoquic_frame_type_path_abandon & 0xFF),
    0x01, /* Path 0 */
    0x00 /* No error */
};

static uint8_t test_frame_type_path_abandon_1[] = {
    (uint8_t)(0x80 | (picoquic_frame_type_path_abandon >> 24)), (uint8_t)(picoquic_frame_type_path_abandon >> 16),
    (uint8_t)(picoquic_frame_type_path_abandon >> 8), (uint8_t)(picoquic_frame_type_path_abandon & 0xFF),
    0x01,
    0x11 /* Some new error */
};

static uint8_t test_frame_type_path_backup[] = {
    (uint8_t)(0x80 | (picoquic_frame_type_path_backup >> 24)), (uint8_t)(picoquic_frame_type_path_backup >> 16),
    (uint8_t)(picoquic_frame_type_path_backup >> 8), (uint8_t)(picoquic_frame_type_path_backup & 0xFF),
    0x00, /* Path 0 */
    0x0F, /* Sequence = 0x0F */
};

static uint8_t test_frame_type_path_available[] = {
    (uint8_t)(0x80 | (picoquic_frame_type_path_available >> 24)), (uint8_t)(picoquic_frame_type_path_available >> 16),
    (uint8_t)(picoquic_frame_type_path_available >> 8), (uint8_t)(picoquic_frame_type_path_available & 0xFF),
    0x00, /* Path 0 */
    0x0F, /* Sequence = 0x0F */
};

static uint8_t test_frame_type_path_blocked[] = {
    (uint8_t)(0x80 | (picoquic_frame_type_paths_blocked >> 24)), (uint8_t)(picoquic_frame_type_paths_blocked >> 16),
    (uint8_t)(picoquic_frame_type_paths_blocked >> 8), (uint8_t)(picoquic_frame_type_paths_blocked & 0xFF),
    0x11, /* max paths = 17 */
};

static uint8_t test_frame_type_bdp[] = {
    (uint8_t)(0x80 | (picoquic_frame_type_bdp >> 24)), (uint8_t)(picoquic_frame_type_bdp >> 16),
    (uint8_t)(picoquic_frame_type_bdp >> 8), (uint8_t)(picoquic_frame_type_bdp & 0xFF),
    0x01, 0x02, 0x03,
    0x04, 0x0A, 0x0, 0x0, 0x01
};

static uint8_t test_frame_type_bad_reset_stream_offset[] = {
    picoquic_frame_type_reset_stream,
    17,
    1,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

static uint8_t test_frame_type_bad_reset_stream[] = {
    picoquic_frame_type_reset_stream,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    1,
    1
};

static uint8_t test_type_bad_connection_close[] = {
    picoquic_frame_type_connection_close,
    0x80, 0x00, 0xCF, 0xFF, 0,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    '1', '2', '3', '4', '5', '6', '7', '8', '9'
};


static uint8_t test_type_bad_application_close[] = {
    picoquic_frame_type_application_close,
    0x44, 4,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    't', 'e', 's', 't'
};

static uint8_t test_frame_type_bad_max_stream_stream[] = {
    picoquic_frame_type_max_stream_data,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x80, 0x01, 0, 0
};

static uint8_t test_frame_type_max_bad_streams_bidir[] = {
    picoquic_frame_type_max_streams_bidir,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

static uint8_t test_frame_type_bad_max_streams_unidir[] = {
    picoquic_frame_type_max_streams_unidir,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

static uint8_t test_frame_type_bad_new_cid_length[] = {
    picoquic_frame_type_new_connection_id,
    7,
    0,
    0x3F,
    1, 2, 3, 4, 5, 6, 7, 8,
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
    0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF
};

static uint8_t test_frame_type_bad_new_cid_retire[] = {
    picoquic_frame_type_new_connection_id,
    7,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    8,
    1, 2, 3, 4, 5, 6, 7, 8,
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
    0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF
};

static uint8_t test_frame_type_bad_stop_sending[] = {
    picoquic_frame_type_stop_sending,
    19,
    0x17
};

static uint8_t test_frame_type_bad_new_token[] = {
    picoquic_frame_type_new_token,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17
};

static uint8_t test_frame_type_bad_ack_range[] = {
    picoquic_frame_type_ack,
    0xC0, 0, 0, 1, 2, 3, 4, 5,
    0x44, 0,
    2,
    5,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0,
    5, 12
};

static uint8_t test_frame_type_bad_ack_gaps[] = {
    picoquic_frame_type_ack,
    0xC0, 0, 0, 1, 2, 3, 4, 5,
    0x44, 0,
    2,
    5,
    0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    5, 12
};

static uint8_t test_frame_type_bad_ack_blocks[] = {
    picoquic_frame_type_ack_ecn,
    0xC0, 0, 0, 1, 2, 3, 4, 5,
    0x44, 0,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    5,
    0, 0,
    5, 12,
    3, 0, 1
};

static uint8_t test_frame_type_bad_crypto_hs[] = {
    picoquic_frame_type_crypto_hs,
    0,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
    0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF
};

static uint8_t test_frame_type_bad_datagram[] = {
    picoquic_frame_type_datagram_l,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
    0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF
};

static uint8_t test_frame_type_new_connection_id_alt[] = {
    picoquic_frame_type_new_connection_id,
    0x0A, /* Sequence Number */
    0x03, /* Retire Prior To */
    8,    /* Length */
    8, 7, 6, 5, 4, 3, 2, 1, /* Connection ID */
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7,
    0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF /* Stateless Reset Token */
};

static uint8_t test_frame_new_cid_retire_high[] = {
    picoquic_frame_type_new_connection_id, 0x0B, 0x0B, 8,
    0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,
    0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF
};

static uint8_t test_frame_new_cid_short_id[] = {
    picoquic_frame_type_new_connection_id, 0x0C, 0x0A, 4,
    0xAA,0xBB,0xCC,0xDD,
    0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF
};

static uint8_t test_frame_new_cid_long_id[] = {
    picoquic_frame_type_new_connection_id, 0x0D, 0x0B, 20,
    0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,0x13,0x14,
    0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF
};

/* NEW_CONNECTION_ID frame with Sequence Number 0, Retire Prior To 0 */
static uint8_t test_frame_new_cid_seq_much_lower[] = {
    0x18,       /* Type: NEW_CONNECTION_ID */
    0x00,       /* Sequence Number: 0 */
    0x00,       /* Retire Prior To: 0 */
    0x08,       /* Length: 8 */
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, /* Connection ID */
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, /* Stateless Reset Token */
    0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF
};

/* Test Case 1: NEW_CONNECTION_ID frame with Retire Prior To > Sequence Number.
 * Type: NEW_CONNECTION_ID (0x18)
 * Sequence Number: 5
 * Retire Prior To: 10 (invalid as it's > Sequence Number)
 * Length: 8
 * Connection ID: 0x01...0x08
 * Stateless Reset Token: 0xA0...0xAF (16 bytes)
 */
static uint8_t test_frame_new_cid_retire_prior_to_seq_num_mismatch[] = {
    picoquic_frame_type_new_connection_id,
    5,    /* Sequence Number */
    10,   /* Retire Prior To */
    8,    /* Length */
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, /* Connection ID */
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, /* Stateless Reset Token (first 8 bytes) */
    0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF  /* Stateless Reset Token (last 8 bytes) */
};

/* Test Case 2: NEW_CONNECTION_ID frame with invalid Connection ID Length (0).
 * Type: NEW_CONNECTION_ID (0x18)
 * Sequence Number: 6
 * Retire Prior To: 1
 * Length: 0 (invalid)
 * Connection ID: (empty)
 * Stateless Reset Token: 0xB0...0xBF (16 bytes)
 */
static uint8_t test_frame_new_cid_invalid_length[] = {
    picoquic_frame_type_new_connection_id,
    6,    /* Sequence Number */
    1,    /* Retire Prior To */
    0,    /* Length */
    /* No Connection ID */
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, /* Stateless Reset Token (first 8 bytes) */
    0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF  /* Stateless Reset Token (last 8 bytes) */
};

/* Test Case 3: NEW_CONNECTION_ID frame with Connection ID Length > 20.
 * Type: NEW_CONNECTION_ID (0x18)
 * Sequence Number: 7
 * Retire Prior To: 2
 * Length: 21 (invalid for RFC 9000, max is 20)
 * Connection ID: 0xC0...0xD4 (21 bytes)
 * Stateless Reset Token: 0xE0...0xEF (16 bytes)
 */
static uint8_t test_frame_new_cid_length_too_long_for_rfc[] = {
    picoquic_frame_type_new_connection_id,
    7,    /* Sequence Number */
    2,    /* Retire Prior To */
    21,   /* Length */
    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, /* Connection ID (21 bytes) */
    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, /* Stateless Reset Token (first 8 bytes) */
    0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF  /* Stateless Reset Token (last 8 bytes) */
};

/* Test Case: NEW_CONNECTION_ID that would exceed active_connection_id_limit.
 * Type: NEW_CONNECTION_ID (0x18)
 * Sequence Number: 5
 * Retire Prior To: 0
 * Length: 8
 * Connection ID: 8x 0xAA
 * Stateless Reset Token: 16x 0xBB
 */
static uint8_t test_frame_new_cid_exceed_limit_no_retire[] = {
    picoquic_frame_type_new_connection_id, /* 0x18 */
    0x05, /* Sequence Number */
    0x00, /* Retire Prior To */
    8,    /* Length */
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, /* Connection ID */
    0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, /* Stateless Reset Token (first 8) */
    0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB  /* Stateless Reset Token (last 8) */
};

static uint8_t test_frame_stream_hang[] = {
    0x01, 0x00, 0x0D, 0xFF, 0xFF, 0xFF, 0x01, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

static uint8_t test_frame_type_path_abandon_bad_0[] = {
    (uint8_t)(0x80 | (picoquic_frame_type_path_abandon >> 24)), (uint8_t)(picoquic_frame_type_path_abandon >> 16),
    (uint8_t)(picoquic_frame_type_path_abandon >> 8), (uint8_t)(picoquic_frame_type_path_abandon & 0xFF),
    0x00, /* type 0 */
    /* 0x01, missing type */
    0x00, /* No error */
    0x00 /* No phrase */
};

static uint8_t test_frame_type_path_abandon_bad_1[] = {
    (uint8_t)(0x80 | (picoquic_frame_type_path_abandon >> 24)),
    (uint8_t)(picoquic_frame_type_path_abandon >> 16),
    (uint8_t)(picoquic_frame_type_path_abandon >> 8),
    (uint8_t)(picoquic_frame_type_path_abandon & 0xFF),
    0x01, /* type 1 */
    0x01,
    0x11, /* Some new error */
    0x4f,
    0xff, /* bad length */
    (uint8_t)'b',
    (uint8_t)'a',
    (uint8_t)'d',
};

static uint8_t test_frame_type_path_abandon_bad_2[] = {
    (uint8_t)(0x80 | (picoquic_frame_type_path_abandon >> 24)), (uint8_t)(picoquic_frame_type_path_abandon >> 16),
    (uint8_t)(picoquic_frame_type_path_abandon >> 8), (uint8_t)(picoquic_frame_type_path_abandon & 0xFF),
    0x03, /* unknown type */
    0x00, /* No error */
    0x00 /* No phrase */
};


static uint8_t test_frame_type_bdp_bad[] = {
    (uint8_t)(0x80 | (picoquic_frame_type_bdp >> 24)), (uint8_t)(picoquic_frame_type_bdp >> 16),
    (uint8_t)(picoquic_frame_type_bdp >> 8), (uint8_t)(picoquic_frame_type_bdp & 0xFF),
    0x01, 0x02, 0x04
};

static uint8_t test_frame_type_bdp_bad_addr[] = {
    (uint8_t)(0x80 | (picoquic_frame_type_bdp >> 24)), (uint8_t)(picoquic_frame_type_bdp >> 16),
    (uint8_t)(picoquic_frame_type_bdp >> 8), (uint8_t)(picoquic_frame_type_bdp & 0xFF),
    0x01, 0x02, 0x04, 0x05, 1, 2, 3, 4, 5
};

static uint8_t test_frame_type_bdp_bad_length[] = {
    (uint8_t)(0x80 | (picoquic_frame_type_bdp >> 24)), (uint8_t)(picoquic_frame_type_bdp >> 16),
    (uint8_t)(picoquic_frame_type_bdp >> 8), (uint8_t)(picoquic_frame_type_bdp & 0xFF),
    0x08, 0x02, 0x04, 0x8F, 0xFF, 0xFF, 0xFF, 1, 2, 3, 4
};

/* New ACK frame test cases */

/* Test Case 1: Excessive ACK Delay */
/* Type: picoquic_frame_type_ack
 * Largest Acknowledged: 100 (0x64)
 * ACK Delay: Max varint (0x3FFFFFFFFFFFFFFF) -> Encoded as 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
 * ACK Range Count: 1 (0x01)
 * First ACK Range: 10 (0x0A)
 */
static uint8_t test_frame_ack_excessive_ack_delay[] = {
    picoquic_frame_type_ack,
    0x64, /* Largest Acknowledged: 100 */
    0xC0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, /* ACK Delay: 0x3FFFFFFFFFFFFFFF (Varint encoded) */
    0x01, /* ACK Range Count: 1 */
    0x0A  /* First ACK Range: 10 */
};

/* Test Case 2: First ACK Range Too Large */
/* Type: picoquic_frame_type_ack
 * Largest Acknowledged: 50 (0x32)
 * ACK Delay: 1000 (0x03E8) -> Encoded as 0x43, 0xE8
 * ACK Range Count: 1 (0x01)
 * First ACK Range: 60 (0x3C) (larger than Largest Acknowledged)
 */
static uint8_t test_frame_ack_first_range_too_large[] = {
    picoquic_frame_type_ack,
    0x32, /* Largest Acknowledged: 50 */
    0x43, 0xE8, /* ACK Delay: 1000 */
    0x01, /* ACK Range Count: 1 */
    0x3C  /* First ACK Range: 60 */
};

/* Test Case 3: Too Many ACK Ranges */
/* Type: picoquic_frame_type_ack
 * Largest Acknowledged: 1000 (0x03E8) -> Encoded as 0x43, 0xE8
 * ACK Delay: 100 (0x64)
 * ACK Range Count: 60 (0x3C)
 * Ranges: First ACK Range = 0 (ack 1 packet: 1000), then 59 * (Gap=0, Range=0)
 */
static uint8_t test_frame_ack_too_many_ranges[] = {
    picoquic_frame_type_ack,
    0x43, 0xE8, /* Largest Acknowledged: 1000 */
    0x64,       /* ACK Delay: 100 */
    0x3C,       /* ACK Range Count: 60 */
    0x00,       /* First ACK Range: 0 (packet 1000) */
    /* 59 more ranges: Gap=0, Range=0 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 10 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 20 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 30 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 40 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 50 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00        /* 59 */
};

/* Test Case 4: ECN ECT0 Count Too Large */
/* Type: picoquic_frame_type_ack_ecn
 * Largest Acknowledged: 10 (0x0A)
 * ACK Delay: 100 (0x64)
 * ACK Range Count: 1 (0x01)
 * First ACK Range: 5 (0x05) (acks 6-10)
 * ECT0: 100 (0x64) (larger than largest acknowledged)
 * ECT1: 0 (0x00)
 * CE: 0 (0x00)
 */
static uint8_t test_frame_ack_ecn_ect0_too_large[] = {
    picoquic_frame_type_ack_ecn,
    0x0A,       /* Largest Acknowledged: 10 */
    0x64,       /* ACK Delay: 100 */
    0x01,       /* ACK Range Count: 1 */
    0x05,       /* First ACK Range: 5 */
    0x64,       /* ECT0: 100 */
    0x00,       /* ECT1: 0 */
    0x00        /* CE: 0 */
};

/* New STREAM frame test cases */

/* Test Case 1: test_frame_stream_len_beyond_packet */
/* Type: 0x0A (LEN bit set)
 * Stream ID: 0x04
 * Length: 0x10000 (65536) -> Encoded as 0x80, 0x01, 0x00, 0x00
 * Stream Data: "testdata" (8 bytes)
 */
static uint8_t test_frame_stream_len_beyond_packet[] = {
    picoquic_frame_type_stream_range_min | 0x02, /* Type 0x0A */
    0x04,       /* Stream ID: 4 */
    0x80, 0x01, 0x00, 0x00, /* Length: 65536 */
    't', 'e', 's', 't', 'd', 'a', 't', 'a'
};

/* Test Case 2: test_frame_stream_zero_len_with_data */
/* Type: 0x0A (LEN bit set)
 * Stream ID: 0x04
 * Length: 0
 * Stream Data: "somedata" (8 bytes)
 */
static uint8_t test_frame_stream_zero_len_with_data[] = {
    picoquic_frame_type_stream_range_min | 0x02, /* Type 0x0A */
    0x04,       /* Stream ID: 4 */
    0x00,       /* Length: 0 */
    's', 'o', 'm', 'e', 'd', 'a', 't', 'a'
};

/* Test Case 3: test_frame_stream_len_shorter_than_data */
/* Type: 0x0A (LEN bit set)
 * Stream ID: 0x04
 * Length: 4
 * Stream Data: "longertestdata" (14 bytes)
 */
static uint8_t test_frame_stream_len_shorter_than_data[] = {
    picoquic_frame_type_stream_range_min | 0x02, /* Type 0x0A */
    0x04,       /* Stream ID: 4 */
    0x04,       /* Length: 4 */
    'l', 'o', 'n', 'g', 'e', 'r', 't', 'e', 's', 't', 'd', 'a', 't', 'a'
};

/* Test Case 4: test_frame_stream_len_longer_than_data */
/* Type: 0x0A (LEN bit set)
 * Stream ID: 0x04
 * Length: 20 (0x14)
 * Stream Data: "shortdata" (9 bytes)
 */
static uint8_t test_frame_stream_len_longer_than_data[] = {
    picoquic_frame_type_stream_range_min | 0x02, /* Type 0x0A */
    0x04,       /* Stream ID: 4 */
    0x14,       /* Length: 20 */
    's', 'h', 'o', 'r', 't', 'd', 'a', 't', 'a'
};

/* Test Case 5: test_frame_stream_max_offset_max_len */
/* Type: 0x0E (OFF bit, LEN bit set)
 * Stream ID: 0x04
 * Offset: Max varint (0x3FFFFFFFFFFFFFFF) -> Encoded as 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
 * Length: Max varint (0x3FFFFFFFFFFFFFFF) -> Encoded as 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
 * Stream Data: "tiny" (4 bytes)
 */
static uint8_t test_frame_stream_max_offset_max_len[] = {
    picoquic_frame_type_stream_range_min | 0x04 | 0x02, /* Type 0x0E */
    0x04,       /* Stream ID: 4 */
    0xC0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, /* Offset: 0x3FFFFFFFFFFFFFFF */
    0xC0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, /* Length: 0x3FFFFFFFFFFFFFFF */
    't', 'i', 'n', 'y'
};

/* New MAX_DATA, MAX_STREAM_DATA, MAX_STREAMS frame test cases */

/* Test Case 1: test_frame_max_data_extremely_large */
/* Type: picoquic_frame_type_max_data (0x10)
 * Maximum Data: 0x3FFFFFFFFFFFFFFF (Varint encoded as 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF)
 */
static uint8_t test_frame_max_data_extremely_large[] = {
    picoquic_frame_type_max_data,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

/* Test Case 2: test_frame_max_stream_data_extremely_large */
/* Type: picoquic_frame_type_max_stream_data (0x11)
 * Stream ID: 0x04
 * Maximum Stream Data: 0x3FFFFFFFFFFFFFFF (Varint encoded as 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF)
 */
static uint8_t test_frame_max_stream_data_extremely_large[] = {
    picoquic_frame_type_max_stream_data,
    0x04, /* Stream ID */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

/* Test Case 3: test_frame_max_streams_bidir_extremely_large */
/* Type: picoquic_frame_type_max_streams_bidir (0x12)
 * Maximum Streams: 0x3FFFFFFFFFFFFFFF (Varint encoded as 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF)
 */
static uint8_t test_frame_max_streams_bidir_extremely_large[] = {
    picoquic_frame_type_max_streams_bidir,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

/* Test Case 4: test_frame_max_streams_unidir_extremely_large */
/* Type: picoquic_frame_type_max_streams_unidir (0x13)
 * Maximum Streams: 0x3FFFFFFFFFFFFFFF (Varint encoded as 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF)
 */
static uint8_t test_frame_max_streams_unidir_extremely_large[] = {
    picoquic_frame_type_max_streams_unidir,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

/* Test Case for MAX_DATA in a post-close scenario.
 * Type: MAX_DATA (0x10)
 * Maximum Data: 200000 (Varint encoded as 0x80, 0x03, 0x0D, 0x40)
 */
static uint8_t test_frame_max_data_after_close_scenario[] = {
    picoquic_frame_type_max_data,
    0x80, 0x03, 0x0D, 0x40 /* 200000 */
};

/* Test Case for MAX_STREAM_DATA on a reset stream.
 * Type: MAX_STREAM_DATA (0x11)
 * Stream ID: 4
 * Maximum Stream Data: 10000 (Varint encoded as 0x80, 0x00, 0x27, 0x10)
 */
static uint8_t test_frame_max_stream_data_for_reset_stream_scenario[] = {
    picoquic_frame_type_max_stream_data,
    0x04,       /* Stream ID: 4 */
    0x80, 0x00, 0x27, 0x10 /* 10000 */
};

/* New CONNECTION_CLOSE and APPLICATION_CLOSE frame test cases */

/* Test Case 1: test_frame_connection_close_reason_len_too_large */
/* Type: picoquic_frame_type_connection_close (0x1c)
 * Error Code: 0x01 (INTERNAL_ERROR)
 * Frame Type: 0x00 (Padding, chosen as an example)
 * Reason Phrase Length: 2000 (Varint encoded as 0x47, 0xD0)
 * Reason Phrase: "short actual phrase" (19 bytes)
 */
static uint8_t test_frame_connection_close_reason_len_too_large[] = {
    picoquic_frame_type_connection_close,
    0x01,       /* Error Code: INTERNAL_ERROR */
    0x00,       /* Frame Type: PADDING_FRAME */
    0x47, 0xD0, /* Reason Phrase Length: 2000 */
    's', 'h', 'o', 'r', 't', ' ', 'a', 'c', 't', 'u', 'a', 'l', ' ', 'p', 'h', 'r', 'a', 's', 'e'
};

/* Test Case 2: test_frame_application_close_reason_len_too_large */
/* Type: picoquic_frame_type_application_close (0x1d)
 * Error Code: 0x0101 (Application specific, encoded as 0x41, 0x01)
 * Reason Phrase Length: 2000 (Varint encoded as 0x47, 0xD0)
 * Reason Phrase: "short actual phrase" (19 bytes)
 */
static uint8_t test_frame_application_close_reason_len_too_large[] = {
    picoquic_frame_type_application_close,
    0x41, 0x01, /* Error Code: 0x0101 */
    0x47, 0xD0, /* Reason Phrase Length: 2000 */
    's', 'h', 'o', 'r', 't', ' ', 'a', 'c', 't', 'u', 'a', 'l', ' ', 'p', 'h', 'r', 'a', 's', 'e'
};

/* Test Case 3: test_frame_connection_close_reason_len_shorter */
/* Type: picoquic_frame_type_connection_close (0x1c)
 * Error Code: 0x01
 * Frame Type: 0x00
 * Reason Phrase Length: 5 (Varint encoded as 0x05)
 * Reason Phrase: "this is much longer than five" (29 bytes)
 */
static uint8_t test_frame_connection_close_reason_len_shorter[] = {
    picoquic_frame_type_connection_close,
    0x01,       /* Error Code: INTERNAL_ERROR */
    0x00,       /* Frame Type: PADDING_FRAME */
    0x05,       /* Reason Phrase Length: 5 */
    't', 'h', 'i', 's', ' ', 'i', 's', ' ', 'm', 'u', 'c', 'h', ' ', 'l', 'o', 'n', 'g', 'e', 'r', ' ', 't', 'h', 'a', 'n', ' ', 'f', 'i', 'v', 'e'
};

/* Test Case 4: test_frame_application_close_reason_len_shorter */
/* Type: picoquic_frame_type_application_close (0x1d)
 * Error Code: 0x0101 (encoded as 0x41, 0x01)
 * Reason Phrase Length: 5 (Varint encoded as 0x05)
 * Reason Phrase: "this is much longer than five" (29 bytes)
 */
static uint8_t test_frame_application_close_reason_len_shorter[] = {
    picoquic_frame_type_application_close,
    0x41, 0x01, /* Error Code: 0x0101 */
    0x05,       /* Reason Phrase Length: 5 */
    't', 'h', 'i', 's', ' ', 'i', 's', ' ', 'm', 'u', 'c', 'h', ' ', 'l', 'o', 'n', 'g', 'e', 'r', ' ', 't', 'h', 'a', 'n', ' ', 'f', 'i', 'v', 'e'
};

/* Test Case 5: test_frame_connection_close_reason_len_longer */
/* Type: picoquic_frame_type_connection_close (0x1c)
 * Error Code: 0x01
 * Frame Type: 0x00
 * Reason Phrase Length: 30 (Varint encoded as 0x1E)
 * Reason Phrase: "short" (5 bytes)
 */
static uint8_t test_frame_connection_close_reason_len_longer[] = {
    picoquic_frame_type_connection_close,
    0x01,       /* Error Code: INTERNAL_ERROR */
    0x00,       /* Frame Type: PADDING_FRAME */
    0x1E,       /* Reason Phrase Length: 30 */
    's', 'h', 'o', 'r', 't'
};

/* Test Case 6: test_frame_application_close_reason_len_longer */
/* Type: picoquic_frame_type_application_close (0x1d)
 * Error Code: 0x0101 (encoded as 0x41, 0x01)
 * Reason Phrase Length: 30 (Varint encoded as 0x1E)
 * Reason Phrase: "short" (5 bytes)
 */
static uint8_t test_frame_application_close_reason_len_longer[] = {
    picoquic_frame_type_application_close,
    0x41, 0x01, /* Error Code: 0x0101 */
    0x1E,       /* Reason Phrase Length: 30 */
    's', 'h', 'o', 'r', 't'
};

/* Test Case 1: CONNECTION_CLOSE (transport error) with an invalid inner Frame Type
 * for the packet type it might be placed in (e.g. STREAM frame in Initial).
 * Type: CONNECTION_CLOSE (transport error, 0x1c)
 * Error Code: 0x0a (PROTOCOL_VIOLATION)
 * Frame Type: 0x08 (STREAM frame type)
 * Reason Phrase Length: 4
 * Reason Phrase: "test"
 */
static uint8_t test_frame_connection_close_invalid_inner_frame_type[] = {
    picoquic_frame_type_connection_close, /* 0x1c */
    0x0a,       /* Error Code: PROTOCOL_VIOLATION */
    0x08,       /* Frame Type: STREAM (example of an invalid type in certain contexts) */
    0x04,       /* Reason Phrase Length: 4 */
    't', 'e', 's', 't'
};

/* Test Case 2: CONNECTION_CLOSE (application error) with a non-UTF-8 reason phrase.
 * Type: CONNECTION_CLOSE (application error, 0x1d)
 * Error Code: 0x0101 (application error, varint 0x4101)
 * Reason Phrase Length: 4
 * Reason Phrase: { 0xC3, 0x28, 0xA0, 0xA1 } (invalid UTF-8)
 */
static uint8_t test_frame_connection_close_reason_non_utf8[] = {
    picoquic_frame_type_application_close, /* 0x1d */
    0x41, 0x01, /* Error Code: 0x0101 */
    0x04,       /* Reason Phrase Length: 4 */
    0xC3, 0x28, 0xA0, 0xA1 /* Invalid UTF-8 sequence */
};

/* New NEW_CONNECTION_ID frame test cases */

/* Test Case 1: test_frame_new_cid_retire_prior_to_greater */
/* Type: picoquic_frame_type_new_connection_id (0x18)
 * Sequence Number: 5 (0x05)
 * Retire Prior To: 10 (0x0A)
 * Length: 8 (0x08)
 * Connection ID: 0x01, ..., 0x08
 * Stateless Reset Token: 0xA0, ..., 0xAF
 */
static uint8_t test_frame_new_cid_retire_prior_to_greater[] = {
    picoquic_frame_type_new_connection_id,
    0x05, /* Sequence Number */
    0x0A, /* Retire Prior To */
    0x08, /* Length */
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, /* Connection ID */
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, /* Stateless Reset Token */
    0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF
};

/* Test Case 2: test_frame_new_cid_zero_length */
/* Type: picoquic_frame_type_new_connection_id (0x18)
 * Sequence Number: 7 (0x07)
 * Retire Prior To: 2 (0x02)
 * Length: 0 (0x00)
 * Connection ID: (empty)
 * Stateless Reset Token: 0xB0, ..., 0xBF
 */
static uint8_t test_frame_new_cid_zero_length[] = {
    picoquic_frame_type_new_connection_id,
    0x07, /* Sequence Number */
    0x02, /* Retire Prior To */
    0x00, /* Length */
    /* No Connection ID */
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, /* Stateless Reset Token */
    0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF
};

/* Test Case 3: test_frame_new_cid_length_too_large */
/* Type: picoquic_frame_type_new_connection_id (0x18)
 * Sequence Number: 8 (0x08)
 * Retire Prior To: 3 (0x03)
 * Length: 21 (0x15)
 * Connection ID: 0xC0, ..., 0xD4 (21 bytes)
 * Stateless Reset Token: 0xE0, ..., 0xEF
 */
static uint8_t test_frame_new_cid_length_too_large[] = {
    picoquic_frame_type_new_connection_id,
    0x08, /* Sequence Number */
    0x03, /* Retire Prior To */
    0x15, /* Length (21) */
    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, /* Connection ID (21 bytes) */
    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, /* Stateless Reset Token */
    0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF
};

/* New NEW_TOKEN frame test cases */

/* Test Case 1: test_frame_new_token_zero_length */
/* Type: picoquic_frame_type_new_token (0x07)
 * Token Length: 0 (Varint encoded as 0x00)
 * Token: (empty)
 */
static uint8_t test_frame_new_token_zero_length[] = {
    picoquic_frame_type_new_token,
    0x00  /* Token Length: 0 */
    /* No Token data */
};

/* Test Case 2: test_frame_new_token_length_too_large */
/* Type: picoquic_frame_type_new_token (0x07)
 * Token Length: 2000 (Varint encoded as 0x47, 0xD0)
 * Token: "shortactualtoken" (16 bytes)
 */
static uint8_t test_frame_new_token_length_too_large[] = {
    picoquic_frame_type_new_token,
    0x47, 0xD0, /* Token Length: 2000 */
    's', 'h', 'o', 'r', 't', 'a', 'c', 't', 'u', 'a', 'l', 't', 'o', 'k', 'e', 'n'
};

/* Test Case 3: test_frame_new_token_length_shorter_than_data */
/* Type: picoquic_frame_type_new_token (0x07)
 * Token Length: 5 (Varint encoded as 0x05)
 * Token: "thisisalongertokenvalue" (25 bytes)
 */
static uint8_t test_frame_new_token_length_shorter_than_data[] = {
    picoquic_frame_type_new_token,
    0x05,       /* Token Length: 5 */
    't', 'h', 'i', 's', 'i', 's', 'a', 'l', 'o', 'n', 'g', 'e', 'r', 't', 'o', 'k', 'e', 'n', 'v', 'a', 'l', 'u', 'e'
};

/* Test Case 4: test_frame_new_token_length_longer_than_data */
/* Type: picoquic_frame_type_new_token (0x07)
 * Token Length: 30 (Varint encoded as 0x1E)
 * Token: "shorttoken" (10 bytes)
 */
static uint8_t test_frame_new_token_length_longer_than_data[] = {
    picoquic_frame_type_new_token,
    0x1E,       /* Token Length: 30 */
    's', 'h', 'o', 'r', 't', 't', 'o', 'k', 'e', 'n'
};

/* New CRYPTO frame test cases */

/* Test Case 1: test_frame_crypto_len_beyond_packet */
/* Type: picoquic_frame_type_crypto_hs (0x06)
 * Offset: 0 (Varint encoded as 0x00)
 * Length: 65536 (Varint encoded as 0x80, 0x01, 0x00, 0x00)
 * Crypto Data: "testcryptodata" (14 bytes)
 */
static uint8_t test_frame_crypto_len_beyond_packet[] = {
    picoquic_frame_type_crypto_hs,
    0x00,       /* Offset: 0 */
    0x80, 0x01, 0x00, 0x00, /* Length: 65536 */
    't', 'e', 's', 't', 'c', 'r', 'y', 'p', 't', 'o', 'd', 'a', 't', 'a'
};

/* Test Case 2: test_frame_crypto_zero_len_with_data */
/* Type: picoquic_frame_type_crypto_hs (0x06)
 * Offset: 0 (Varint encoded as 0x00)
 * Length: 0 (Varint encoded as 0x00)
 * Crypto Data: "actualdata" (10 bytes)
 */
static uint8_t test_frame_crypto_zero_len_with_data[] = {
    picoquic_frame_type_crypto_hs,
    0x00,       /* Offset: 0 */
    0x00,       /* Length: 0 */
    'a', 'c', 't', 'u', 'a', 'l', 'd', 'a', 't', 'a'
};

/* Test Case 3: test_frame_crypto_len_shorter_than_data */
/* Type: picoquic_frame_type_crypto_hs (0x06)
 * Offset: 0 (Varint encoded as 0x00)
 * Length: 5 (Varint encoded as 0x05)
 * Crypto Data: "muchlongercryptodata" (20 bytes)
 */
static uint8_t test_frame_crypto_len_shorter_than_data[] = {
    picoquic_frame_type_crypto_hs,
    0x00,       /* Offset: 0 */
    0x05,       /* Length: 5 */
    'm', 'u', 'c', 'h', 'l', 'o', 'n', 'g', 'e', 'r', 'c', 'r', 'y', 'p', 't', 'o', 'd', 'a', 't', 'a'
};

/* Test Case 4: test_frame_crypto_len_longer_than_data */
/* Type: picoquic_frame_type_crypto_hs (0x06)
 * Offset: 0 (Varint encoded as 0x00)
 * Length: 30 (Varint encoded as 0x1E)
 * Crypto Data: "shortcrypto" (11 bytes)
 */
static uint8_t test_frame_crypto_len_longer_than_data[] = {
    picoquic_frame_type_crypto_hs,
    0x00,       /* Offset: 0 */
    0x1E,       /* Length: 30 */
    's', 'h', 'o', 'r', 't', 'c', 'r', 'y', 'p', 't', 'o'
};

/* Test Case 5: test_frame_crypto_max_offset_max_len */
/* Type: picoquic_frame_type_crypto_hs (0x06)
 * Offset: 0x3FFFFFFFFFFFFFFF (Varint encoded as 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF)
 * Length: 0x3FFFFFFFFFFFFFFF (Varint encoded as 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF)
 * Crypto Data: "tiny" (4 bytes)
 */
static uint8_t test_frame_crypto_max_offset_max_len[] = {
    picoquic_frame_type_crypto_hs,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, /* Offset */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, /* Length */
    't', 'i', 'n', 'y'
};

/* New fuzzy varint test cases */

/* Test Case 1: test_frame_max_data_non_minimal_varint */
/* Type: picoquic_frame_type_max_data (0x10)
 * Maximum Data: Value 48 (0x30) encoded non-minimally as a 2-byte varint (0x40, 0x30).
 * Minimal encoding would be 0x30.
 */
static uint8_t test_frame_max_data_non_minimal_varint[] = {
    picoquic_frame_type_max_data,
    0x40, 0x30  /* Non-minimal encoding of 48 */
};

/* Test Case 2: test_frame_reset_stream_invalid_9_byte_varint */
/* Type: picoquic_frame_type_reset_stream (0x04)
 * Stream ID: Invalid 9-byte varint. First byte 0xC0 suggests an 8-byte encoding,
 *            but is followed by 8 more bytes, making the varint itself 9 bytes long.
 * Application Protocol Error Code: 0 (0x00)
 * Final Size: 0 (0x00)
 */
static uint8_t test_frame_reset_stream_invalid_9_byte_varint[] = {
    picoquic_frame_type_reset_stream,
    0xC0, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, /* Stream ID (9-byte varint) */
    0x00,       /* Application Protocol Error Code: 0 */
    0x00        /* Final Size: 0 */
};

/* Test Case 3: test_frame_stop_sending_non_minimal_error_code */
/* Type: picoquic_frame_type_stop_sending (0x05)
 * Stream ID: 1 (0x01)
 * Application Protocol Error Code: Value 0 encoded non-minimally as 2-byte varint (0x40, 0x00).
 * Minimal encoding would be 0x00.
 */
static uint8_t test_frame_stop_sending_non_minimal_error_code[] = {
    picoquic_frame_type_stop_sending,
    0x01,       /* Stream ID: 1 */
    0x40, 0x00  /* Non-minimal encoding of error code 0 */
};

/* Test Case 1 (Varint): MAX_STREAMS (bidirectional) with non-minimal varint for stream count.
 * Type: MAX_STREAMS (bidirectional, 0x12)
 * Maximum Streams: Value 10 (normally 0x0A) encoded as a 2-byte varint (0x40, 0x0A).
 */
static uint8_t test_frame_max_streams_non_minimal_varint[] = {
    picoquic_frame_type_max_streams_bidir, /* 0x12 */
    0x40, 0x0A  /* Non-minimal encoding of 10 */
};

/* Test Case 2 (Varint): CRYPTO frame with a non-minimal large varint for offset.
 * Type: CRYPTO (0x06)
 * Offset: Value 1 encoded as an 8-byte varint (0xC000000000000001).
 * Length: 5
 * Crypto Data: "hello"
 */
static uint8_t test_frame_crypto_offset_non_minimal_large_varint[] = {
    picoquic_frame_type_crypto_hs, /* 0x06 */
    0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, /* Non-minimal 8-byte encoding of 1 */
    0x05,       /* Length: 5 */
    'h', 'e', 'l', 'l', 'o'
};

/* New static test cases for less common frame variations */

/* Test Case 1: test_frame_retire_cid_seq_much_higher */
/* Type: picoquic_frame_type_retire_connection_id (0x19)
 * Sequence Number: 10000 (Varint encoded as 0x67, 0x10)
 */
static uint8_t test_frame_retire_cid_seq_much_higher[] = {
    picoquic_frame_type_retire_connection_id,
    0x67, 0x10  /* Sequence Number: 10000 */
};

/* Test Case 2: test_frame_datagram_len_shorter_than_data */
/* Type: picoquic_frame_type_datagram_l (0x31)
 * Length: 5 (Varint encoded as 0x05)
 * Datagram Data: "thisislongdatagramdata" (24 bytes)
 */
static uint8_t test_frame_datagram_len_shorter_than_data[] = {
    picoquic_frame_type_datagram_l,
    0x05,       /* Length: 5 */
    't', 'h', 'i', 's', 'i', 's', 'l', 'o', 'n', 'g', 'd', 'a', 't', 'a', 'g', 'r', 'a', 'm', 'd', 'a', 't', 'a'
};

/* Test Case 3: test_frame_datagram_len_longer_than_data */
/* Type: picoquic_frame_type_datagram_l (0x31)
 * Length: 20 (Varint encoded as 0x14)
 * Datagram Data: "shortdata" (9 bytes)
 */
static uint8_t test_frame_datagram_len_longer_than_data[] = {
    picoquic_frame_type_datagram_l,
    0x14,       /* Length: 20 */
    's', 'h', 'o', 'r', 't', 'd', 'a', 't', 'a'
};

/* Test Case 4: test_frame_datagram_zero_len_with_data */
/* Type: picoquic_frame_type_datagram_l (0x31)
 * Length: 0 (Varint encoded as 0x00)
 * Datagram Data: "actualdatagramdata" (18 bytes)
 */
static uint8_t test_frame_datagram_zero_len_with_data[] = {
    picoquic_frame_type_datagram_l,
    0x00,       /* Length: 0 */
    'a', 'c', 't', 'u', 'a', 'l', 'd', 'a', 't', 'a', 'g', 'r', 'a', 'm', 'd', 'a', 't', 'a'
};

/* DATAGRAM frame with LEN bit set, Length is 0, and no data. */
static uint8_t test_frame_datagram_with_len_empty[] = {
    0x31, /* Type: DATAGRAM, LEN=1 */
    0x00  /* Length: 0 */
};

/* DATAGRAM frame with LEN bit set, Length is 4 encoded non-canonically as 2 bytes. */
static uint8_t test_frame_datagram_len_non_canon[] = {
    0x31,       /* Type: DATAGRAM, LEN=1 */
    0x40, 0x04, /* Length: 4 (2-byte varint) */
    0x64, 0x61, 0x74, 0x61 /* Data: "data" */
};

/* DATAGRAM frame with LEN bit set, moderately large length, and sample data. */
static uint8_t test_frame_datagram_very_large[] = {
    0x31,       /* Type: DATAGRAM, LEN=1 */
    0x40, 0xFA, /* Length: 250 (varint) */
    'l', 'a', 'r', 'g', 'e', '_', 'd', 'a', 't', 'a', 'g', 'r', 'a', 'm', '_', 't', 'e', 's', 't', '_', 'd', 'a', 't', 'a'
};

/* Non-Canonical Variable-Length Integers */
static uint8_t test_frame_stream_long_varint_stream_id_2byte[] = {
    0x08,       /* Type: STREAM */
    0x40, 0x05, /* Stream ID: 5 (2-byte varint) */
    't', 'e', 's', 't'
};

static uint8_t test_frame_stream_long_varint_stream_id_4byte[] = {
    0x08,       /* Type: STREAM */
    0x80, 0x00, 0x00, 0x05, /* Stream ID: 5 (4-byte varint) */
    't', 'e', 's', 't'
};

static uint8_t test_frame_stream_long_varint_offset_2byte[] = {
    0x0C,       /* Type: STREAM, OFF bit */
    0x01,       /* Stream ID: 1 */
    0x40, 0x0A, /* Offset: 10 (2-byte varint) */
    't', 'e', 's', 't'
};

static uint8_t test_frame_stream_long_varint_offset_4byte[] = {
    0x0C,       /* Type: STREAM, OFF bit */
    0x01,       /* Stream ID: 1 */
    0x80, 0x00, 0x00, 0x0A, /* Offset: 10 (4-byte varint) */
    't', 'e', 's', 't'
};

static uint8_t test_frame_stream_long_varint_length_2byte[] = {
    0x0A,       /* Type: STREAM, LEN bit */
    0x01,       /* Stream ID: 1 */
    0x40, 0x04, /* Length: 4 (2-byte varint) */
    't', 'e', 's', 't'
};

static uint8_t test_frame_stream_long_varint_length_4byte[] = {
    0x0A,       /* Type: STREAM, LEN bit */
    0x01,       /* Stream ID: 1 */
    0x80, 0x00, 0x00, 0x04, /* Length: 4 (4-byte varint) */
    't', 'e', 's', 't'
};

static uint8_t test_frame_max_data_long_varint_2byte[] = {
    picoquic_frame_type_max_data,
    0x44, 0x00  /* Maximum Data: 1024 (0x400) (2-byte varint) */
};

static uint8_t test_frame_max_data_long_varint_4byte[] = {
    picoquic_frame_type_max_data,
    0x80, 0x00, 0x04, 0x00  /* Maximum Data: 1024 (0x400) (4-byte varint) */
};

static uint8_t test_frame_ack_long_varint_largest_acked_2byte[] = {
    picoquic_frame_type_ack,
    0x40, 0x14, /* Largest Acknowledged: 20 (2-byte varint) */
    0x00,       /* ACK Delay: 0 */
    0x01,       /* ACK Range Count: 1 */
    0x00        /* First ACK Range: 0 */
};

static uint8_t test_frame_ack_long_varint_largest_acked_4byte[] = {
    picoquic_frame_type_ack,
    0x80, 0x00, 0x00, 0x14, /* Largest Acknowledged: 20 (4-byte varint) */
    0x00,       /* ACK Delay: 0 */
    0x01,       /* ACK Range Count: 1 */
    0x00        /* First ACK Range: 0 */
};

static uint8_t test_frame_crypto_long_varint_offset_2byte[] = {
    picoquic_frame_type_crypto_hs,
    0x40, 0x0A, /* Offset: 10 (2-byte varint) */
    0x04,       /* Length: 4 */
    't', 'e', 's', 't'
};

static uint8_t test_frame_crypto_long_varint_offset_4byte[] = {
    picoquic_frame_type_crypto_hs,
    0x80, 0x00, 0x00, 0x0A, /* Offset: 10 (4-byte varint) */
    0x04,       /* Length: 4 */
    't', 'e', 's', 't'
};

/* STREAM SID: Non-Canonical Varints */
static uint8_t test_stream_sid_0_nc2[] = {0x08, 0x40, 0x00, 'S','I','D','n','c'};
static uint8_t test_stream_sid_0_nc4[] = {0x08, 0x80, 0x00, 0x00, 0x00, 'S','I','D','n','c'};
static uint8_t test_stream_sid_0_nc8[] = {0x08, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 'S','I','D','n','c'};
static uint8_t test_stream_sid_1_nc2[] = {0x08, 0x40, 0x01, 'S','I','D','n','c'};
static uint8_t test_stream_sid_1_nc4[] = {0x08, 0x80, 0x00, 0x00, 0x01, 'S','I','D','n','c'};
static uint8_t test_stream_sid_1_nc8[] = {0x08, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 'S','I','D','n','c'};
static uint8_t test_stream_sid_5_nc2[] = {0x08, 0x40, 0x05, 'S','I','D','n','c'};
static uint8_t test_stream_sid_5_nc4[] = {0x08, 0x80, 0x00, 0x00, 0x05, 'S','I','D','n','c'};
static uint8_t test_stream_sid_5_nc8[] = {0x08, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 'S','I','D','n','c'};

/* STREAM Offset: Non-Canonical Varints */
static uint8_t test_stream_off_0_nc2[] = {0x0C, 0x01, 0x40, 0x00, 'O','F','F','n','c'};
static uint8_t test_stream_off_0_nc4[] = {0x0C, 0x01, 0x80, 0x00, 0x00, 0x00, 'O','F','F','n','c'};
static uint8_t test_stream_off_0_nc8[] = {0x0C, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 'O','F','F','n','c'};
static uint8_t test_stream_off_1_nc2[] = {0x0C, 0x01, 0x40, 0x01, 'O','F','F','n','c'};
static uint8_t test_stream_off_1_nc4[] = {0x0C, 0x01, 0x80, 0x00, 0x00, 0x01, 'O','F','F','n','c'};
static uint8_t test_stream_off_1_nc8[] = {0x0C, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 'O','F','F','n','c'};
static uint8_t test_stream_off_5_nc2[] = {0x0C, 0x01, 0x40, 0x05, 'O','F','F','n','c'};
static uint8_t test_stream_off_5_nc4[] = {0x0C, 0x01, 0x80, 0x00, 0x00, 0x05, 'O','F','F','n','c'};
static uint8_t test_stream_off_5_nc8[] = {0x0C, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 'O','F','F','n','c'};

/* STREAM Length: Non-Canonical Varints */
static uint8_t test_stream_len_0_nc2[] = {0x0A, 0x01, 0x40, 0x00};
static uint8_t test_stream_len_0_nc4[] = {0x0A, 0x01, 0x80, 0x00, 0x00, 0x00};
static uint8_t test_stream_len_0_nc8[] = {0x0A, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t test_stream_len_1_nc2[] = {0x0A, 0x01, 0x40, 0x01, 'L'};
static uint8_t test_stream_len_1_nc4[] = {0x0A, 0x01, 0x80, 0x00, 0x00, 0x01, 'L'};
static uint8_t test_stream_len_1_nc8[] = {0x0A, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 'L'};
static uint8_t test_stream_len_4_nc2[] = {0x0A, 0x01, 0x40, 0x04, 'L','E','N','n'};
static uint8_t test_stream_len_4_nc4[] = {0x0A, 0x01, 0x80, 0x00, 0x00, 0x04, 'L','E','N','n'};
static uint8_t test_stream_len_4_nc8[] = {0x0A, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 'L','E','N','n'};

/* ACK Largest Acknowledged: Non-Canonical Varints */
static uint8_t test_ack_largest_ack_0_nc2[] = {0x02, 0x40, 0x00, 0x00, 0x01, 0x00};
static uint8_t test_ack_largest_ack_0_nc4[] = {0x02, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00};
static uint8_t test_ack_largest_ack_0_nc8[] = {0x02, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00};
static uint8_t test_ack_largest_ack_1_nc2[] = {0x02, 0x40, 0x01, 0x00, 0x01, 0x00};
static uint8_t test_ack_largest_ack_1_nc4[] = {0x02, 0x80, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00};
static uint8_t test_ack_largest_ack_1_nc8[] = {0x02, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00};
static uint8_t test_ack_largest_ack_5_nc2[] = {0x02, 0x40, 0x05, 0x00, 0x01, 0x00};
static uint8_t test_ack_largest_ack_5_nc4[] = {0x02, 0x80, 0x00, 0x00, 0x05, 0x00, 0x01, 0x00};
static uint8_t test_ack_largest_ack_5_nc8[] = {0x02, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x01, 0x00};

/* ACK Delay: Non-Canonical Varints */
static uint8_t test_ack_delay_0_nc2[] = {0x02, 0x0A, 0x40, 0x00, 0x01, 0x00};
static uint8_t test_ack_delay_0_nc4[] = {0x02, 0x0A, 0x80, 0x00, 0x00, 0x00, 0x01, 0x00};
static uint8_t test_ack_delay_0_nc8[] = {0x02, 0x0A, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00};
static uint8_t test_ack_delay_1_nc2[] = {0x02, 0x0A, 0x40, 0x01, 0x01, 0x00};
static uint8_t test_ack_delay_1_nc4[] = {0x02, 0x0A, 0x80, 0x00, 0x00, 0x01, 0x01, 0x00};
static uint8_t test_ack_delay_1_nc8[] = {0x02, 0x0A, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00};
static uint8_t test_ack_delay_5_nc2[] = {0x02, 0x0A, 0x40, 0x05, 0x01, 0x00};
static uint8_t test_ack_delay_5_nc4[] = {0x02, 0x0A, 0x80, 0x00, 0x00, 0x05, 0x01, 0x00};
static uint8_t test_ack_delay_5_nc8[] = {0x02, 0x0A, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x01, 0x00};

/* ACK Range Count: Non-Canonical Varints */
static uint8_t test_ack_range_count_1_nc2[] = {0x02, 0x0A, 0x00, 0x40, 0x01, 0x00};
static uint8_t test_ack_range_count_1_nc4[] = {0x02, 0x0A, 0x00, 0x80, 0x00, 0x00, 0x01, 0x00};
static uint8_t test_ack_range_count_1_nc8[] = {0x02, 0x0A, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00};
static uint8_t test_ack_range_count_2_nc2[] = {0x02, 0x0A, 0x00, 0x40, 0x02, 0x00, 0x00, 0x00};
static uint8_t test_ack_range_count_2_nc4[] = {0x02, 0x0A, 0x00, 0x80, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00};
static uint8_t test_ack_range_count_2_nc8[] = {0x02, 0x0A, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00};

/* ACK First ACK Range: Non-Canonical Varints */
static uint8_t test_ack_first_range_0_nc2[] = {0x02, 0x0A, 0x00, 0x01, 0x40, 0x00};
static uint8_t test_ack_first_range_0_nc4[] = {0x02, 0x0A, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00};
static uint8_t test_ack_first_range_0_nc8[] = {0x02, 0x0A, 0x00, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t test_ack_first_range_1_nc2[] = {0x02, 0x0A, 0x00, 0x01, 0x40, 0x01};
static uint8_t test_ack_first_range_1_nc4[] = {0x02, 0x0A, 0x00, 0x01, 0x80, 0x00, 0x00, 0x01};
static uint8_t test_ack_first_range_1_nc8[] = {0x02, 0x0A, 0x00, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
static uint8_t test_ack_first_range_5_nc2[] = {0x02, 0x0A, 0x00, 0x01, 0x40, 0x05};
static uint8_t test_ack_first_range_5_nc4[] = {0x02, 0x0A, 0x00, 0x01, 0x80, 0x00, 0x00, 0x05};
static uint8_t test_ack_first_range_5_nc8[] = {0x02, 0x0A, 0x00, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05};

/* ACK Gap: Non-Canonical Varints */
static uint8_t test_ack_gap_0_nc2[] = {0x02, 0x14, 0x00, 0x02, 0x01, 0x40, 0x00, 0x00};
static uint8_t test_ack_gap_0_nc4[] = {0x02, 0x14, 0x00, 0x02, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00};
static uint8_t test_ack_gap_0_nc8[] = {0x02, 0x14, 0x00, 0x02, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t test_ack_gap_1_nc2[] = {0x02, 0x14, 0x00, 0x02, 0x01, 0x40, 0x01, 0x00};
static uint8_t test_ack_gap_1_nc4[] = {0x02, 0x14, 0x00, 0x02, 0x01, 0x80, 0x00, 0x00, 0x01, 0x00};
static uint8_t test_ack_gap_1_nc8[] = {0x02, 0x14, 0x00, 0x02, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00};
static uint8_t test_ack_gap_2_nc2[] = {0x02, 0x14, 0x00, 0x02, 0x01, 0x40, 0x02, 0x00};
static uint8_t test_ack_gap_2_nc4[] = {0x02, 0x14, 0x00, 0x02, 0x01, 0x80, 0x00, 0x00, 0x02, 0x00};
static uint8_t test_ack_gap_2_nc8[] = {0x02, 0x14, 0x00, 0x02, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00};

/* ACK Range Length: Non-Canonical Varints */
static uint8_t test_ack_range_len_0_nc2[] = {0x02, 0x14, 0x00, 0x02, 0x01, 0x01, 0x40, 0x00};
static uint8_t test_ack_range_len_0_nc4[] = {0x02, 0x14, 0x00, 0x02, 0x01, 0x01, 0x80, 0x00, 0x00, 0x00};
static uint8_t test_ack_range_len_0_nc8[] = {0x02, 0x14, 0x00, 0x02, 0x01, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t test_ack_range_len_1_nc2[] = {0x02, 0x14, 0x00, 0x02, 0x01, 0x01, 0x40, 0x01};
static uint8_t test_ack_range_len_1_nc4[] = {0x02, 0x14, 0x00, 0x02, 0x01, 0x01, 0x80, 0x00, 0x00, 0x01};
static uint8_t test_ack_range_len_1_nc8[] = {0x02, 0x14, 0x00, 0x02, 0x01, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
static uint8_t test_ack_range_len_5_nc2[] = {0x02, 0x14, 0x00, 0x02, 0x01, 0x01, 0x40, 0x05};
static uint8_t test_ack_range_len_5_nc4[] = {0x02, 0x14, 0x00, 0x02, 0x01, 0x01, 0x80, 0x00, 0x00, 0x05};
static uint8_t test_ack_range_len_5_nc8[] = {0x02, 0x14, 0x00, 0x02, 0x01, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05};

/* RESET_STREAM Stream ID: Non-Canonical Varints */
static uint8_t test_reset_stream_sid_0_nc2[] = {0x04, 0x40, 0x00, 0x00, 0x00};
static uint8_t test_reset_stream_sid_0_nc4[] = {0x04, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t test_reset_stream_sid_0_nc8[] = {0x04, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t test_reset_stream_sid_1_nc2[] = {0x04, 0x40, 0x01, 0x00, 0x00};
static uint8_t test_reset_stream_sid_1_nc4[] = {0x04, 0x80, 0x00, 0x00, 0x01, 0x00, 0x00};
static uint8_t test_reset_stream_sid_1_nc8[] = {0x04, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00};
static uint8_t test_reset_stream_sid_5_nc2[] = {0x04, 0x40, 0x05, 0x00, 0x00};
static uint8_t test_reset_stream_sid_5_nc4[] = {0x04, 0x80, 0x00, 0x00, 0x05, 0x00, 0x00};
static uint8_t test_reset_stream_sid_5_nc8[] = {0x04, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00};

/* RESET_STREAM App Error Code: Non-Canonical Varints */
static uint8_t test_reset_stream_err_0_nc2[] = {0x04, 0x01, 0x40, 0x00, 0x00};
static uint8_t test_reset_stream_err_0_nc4[] = {0x04, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00};
static uint8_t test_reset_stream_err_0_nc8[] = {0x04, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t test_reset_stream_err_1_nc2[] = {0x04, 0x01, 0x40, 0x01, 0x00};
static uint8_t test_reset_stream_err_1_nc4[] = {0x04, 0x01, 0x80, 0x00, 0x00, 0x01, 0x00};
static uint8_t test_reset_stream_err_1_nc8[] = {0x04, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00};
static uint8_t test_reset_stream_err_5_nc2[] = {0x04, 0x01, 0x40, 0x05, 0x00};
static uint8_t test_reset_stream_err_5_nc4[] = {0x04, 0x01, 0x80, 0x00, 0x00, 0x05, 0x00};
static uint8_t test_reset_stream_err_5_nc8[] = {0x04, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00};

/* RESET_STREAM Final Size: Non-Canonical Varints */
static uint8_t test_reset_stream_final_0_nc2[] = {0x04, 0x01, 0x00, 0x40, 0x00};
static uint8_t test_reset_stream_final_0_nc4[] = {0x04, 0x01, 0x00, 0x80, 0x00, 0x00, 0x00};
static uint8_t test_reset_stream_final_0_nc8[] = {0x04, 0x01, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t test_reset_stream_final_1_nc2[] = {0x04, 0x01, 0x00, 0x40, 0x01};
static uint8_t test_reset_stream_final_1_nc4[] = {0x04, 0x01, 0x00, 0x80, 0x00, 0x00, 0x01};
static uint8_t test_reset_stream_final_1_nc8[] = {0x04, 0x01, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
static uint8_t test_reset_stream_final_5_nc2[] = {0x04, 0x01, 0x00, 0x40, 0x05};
static uint8_t test_reset_stream_final_5_nc4[] = {0x04, 0x01, 0x00, 0x80, 0x00, 0x00, 0x05};
static uint8_t test_reset_stream_final_5_nc8[] = {0x04, 0x01, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05};

/* STOP_SENDING Stream ID: Non-Canonical Varints */
static uint8_t test_stop_sending_sid_0_nc2[] = {0x05, 0x40, 0x00, 0x00};
static uint8_t test_stop_sending_sid_0_nc4[] = {0x05, 0x80, 0x00, 0x00, 0x00, 0x00};
static uint8_t test_stop_sending_sid_0_nc8[] = {0x05, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t test_stop_sending_sid_1_nc2[] = {0x05, 0x40, 0x01, 0x00};
static uint8_t test_stop_sending_sid_1_nc4[] = {0x05, 0x80, 0x00, 0x00, 0x01, 0x00};
static uint8_t test_stop_sending_sid_1_nc8[] = {0x05, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00};
static uint8_t test_stop_sending_sid_5_nc2[] = {0x05, 0x40, 0x05, 0x00};
static uint8_t test_stop_sending_sid_5_nc4[] = {0x05, 0x80, 0x00, 0x00, 0x05, 0x00};
static uint8_t test_stop_sending_sid_5_nc8[] = {0x05, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00};

/* STOP_SENDING App Error Code: Non-Canonical Varints */
static uint8_t test_stop_sending_err_0_nc2[] = {0x05, 0x01, 0x40, 0x00};
static uint8_t test_stop_sending_err_0_nc4[] = {0x05, 0x01, 0x80, 0x00, 0x00, 0x00};
static uint8_t test_stop_sending_err_0_nc8[] = {0x05, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t test_stop_sending_err_1_nc2[] = {0x05, 0x01, 0x40, 0x01};
static uint8_t test_stop_sending_err_1_nc4[] = {0x05, 0x01, 0x80, 0x00, 0x00, 0x01};
static uint8_t test_stop_sending_err_1_nc8[] = {0x05, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
static uint8_t test_stop_sending_err_5_nc2[] = {0x05, 0x01, 0x40, 0x05};
static uint8_t test_stop_sending_err_5_nc4[] = {0x05, 0x01, 0x80, 0x00, 0x00, 0x05};
static uint8_t test_stop_sending_err_5_nc8[] = {0x05, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05};

/* MAX_DATA Maximum Data: Non-Canonical Varints */
static uint8_t test_max_data_0_nc2[] = {0x10, 0x40, 0x00};
static uint8_t test_max_data_0_nc4[] = {0x10, 0x80, 0x00, 0x00, 0x00};
static uint8_t test_max_data_0_nc8[] = {0x10, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t test_max_data_1_nc2[] = {0x10, 0x40, 0x01};
static uint8_t test_max_data_1_nc4[] = {0x10, 0x80, 0x00, 0x00, 0x01};
static uint8_t test_max_data_1_nc8[] = {0x10, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
static uint8_t test_max_data_10_nc2[] = {0x10, 0x40, 0x0A};
static uint8_t test_max_data_10_nc4[] = {0x10, 0x80, 0x00, 0x00, 0x0A};
static uint8_t test_max_data_10_nc8[] = {0x10, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A};

/* MAX_STREAM_DATA Stream ID: Non-Canonical Varints */
static uint8_t test_max_sdata_sid_0_nc2[] = {0x11, 0x40, 0x00, 0x64};
static uint8_t test_max_sdata_sid_0_nc4[] = {0x11, 0x80, 0x00, 0x00, 0x00, 0x64};
static uint8_t test_max_sdata_sid_0_nc8[] = {0x11, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64};
static uint8_t test_max_sdata_sid_1_nc2[] = {0x11, 0x40, 0x01, 0x64};
static uint8_t test_max_sdata_sid_1_nc4[] = {0x11, 0x80, 0x00, 0x00, 0x01, 0x64};
static uint8_t test_max_sdata_sid_1_nc8[] = {0x11, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x64};
static uint8_t test_max_sdata_sid_5_nc2[] = {0x11, 0x40, 0x05, 0x64};
static uint8_t test_max_sdata_sid_5_nc4[] = {0x11, 0x80, 0x00, 0x00, 0x05, 0x64};
static uint8_t test_max_sdata_sid_5_nc8[] = {0x11, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x64};

/* MAX_STREAM_DATA Max Value: Non-Canonical Varints */
static uint8_t test_max_sdata_val_0_nc2[] = {0x11, 0x01, 0x40, 0x00};
static uint8_t test_max_sdata_val_0_nc4[] = {0x11, 0x01, 0x80, 0x00, 0x00, 0x00};
static uint8_t test_max_sdata_val_0_nc8[] = {0x11, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t test_max_sdata_val_1_nc2[] = {0x11, 0x01, 0x40, 0x01};
static uint8_t test_max_sdata_val_1_nc4[] = {0x11, 0x01, 0x80, 0x00, 0x00, 0x01};
static uint8_t test_max_sdata_val_1_nc8[] = {0x11, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
static uint8_t test_max_sdata_val_10_nc2[] = {0x11, 0x01, 0x40, 0x0A};
static uint8_t test_max_sdata_val_10_nc4[] = {0x11, 0x01, 0x80, 0x00, 0x00, 0x0A};
static uint8_t test_max_sdata_val_10_nc8[] = {0x11, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A};

/* MAX_STREAMS (Bidi): Non-Canonical Varints */
static uint8_t test_max_streams_bidi_0_nc2[] = {0x12, 0x40, 0x00};
static uint8_t test_max_streams_bidi_0_nc4[] = {0x12, 0x80, 0x00, 0x00, 0x00};
static uint8_t test_max_streams_bidi_0_nc8[] = {0x12, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t test_max_streams_bidi_1_nc2[] = {0x12, 0x40, 0x01};
static uint8_t test_max_streams_bidi_1_nc4[] = {0x12, 0x80, 0x00, 0x00, 0x01};
static uint8_t test_max_streams_bidi_1_nc8[] = {0x12, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
static uint8_t test_max_streams_bidi_5_nc2[] = {0x12, 0x40, 0x05};
static uint8_t test_max_streams_bidi_5_nc4[] = {0x12, 0x80, 0x00, 0x00, 0x05};
static uint8_t test_max_streams_bidi_5_nc8[] = {0x12, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05};

/* MAX_STREAMS (Uni): Non-Canonical Varints */
static uint8_t test_max_streams_uni_0_nc2[] = {0x13, 0x40, 0x00};
static uint8_t test_max_streams_uni_0_nc4[] = {0x13, 0x80, 0x00, 0x00, 0x00};
static uint8_t test_max_streams_uni_0_nc8[] = {0x13, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t test_max_streams_uni_1_nc2[] = {0x13, 0x40, 0x01};
static uint8_t test_max_streams_uni_1_nc4[] = {0x13, 0x80, 0x00, 0x00, 0x01};
static uint8_t test_max_streams_uni_1_nc8[] = {0x13, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
static uint8_t test_max_streams_uni_5_nc2[] = {0x13, 0x40, 0x05};
static uint8_t test_max_streams_uni_5_nc4[] = {0x13, 0x80, 0x00, 0x00, 0x05};
static uint8_t test_max_streams_uni_5_nc8[] = {0x13, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05};


/* DATA_BLOCKED Maximum Data: Non-Canonical Varints */
static uint8_t test_data_blocked_0_nc2[] = {0x14, 0x40, 0x00};
static uint8_t test_data_blocked_0_nc4[] = {0x14, 0x80, 0x00, 0x00, 0x00};
static uint8_t test_data_blocked_0_nc8[] = {0x14, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t test_data_blocked_1_nc2[] = {0x14, 0x40, 0x01};
static uint8_t test_data_blocked_1_nc4[] = {0x14, 0x80, 0x00, 0x00, 0x01};
static uint8_t test_data_blocked_1_nc8[] = {0x14, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
static uint8_t test_data_blocked_10_nc2[] = {0x14, 0x40, 0x0A};
static uint8_t test_data_blocked_10_nc4[] = {0x14, 0x80, 0x00, 0x00, 0x0A};
static uint8_t test_data_blocked_10_nc8[] = {0x14, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A};

/* STREAM_DATA_BLOCKED Stream ID: Non-Canonical Varints */
static uint8_t test_sdata_blocked_sid_0_nc2[] = {0x15, 0x40, 0x00, 0x64};
static uint8_t test_sdata_blocked_sid_0_nc4[] = {0x15, 0x80, 0x00, 0x00, 0x00, 0x64};
static uint8_t test_sdata_blocked_sid_0_nc8[] = {0x15, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64};
static uint8_t test_sdata_blocked_sid_1_nc2[] = {0x15, 0x40, 0x01, 0x64};
static uint8_t test_sdata_blocked_sid_1_nc4[] = {0x15, 0x80, 0x00, 0x00, 0x01, 0x64};
static uint8_t test_sdata_blocked_sid_1_nc8[] = {0x15, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x64};
static uint8_t test_sdata_blocked_sid_5_nc2[] = {0x15, 0x40, 0x05, 0x64};
static uint8_t test_sdata_blocked_sid_5_nc4[] = {0x15, 0x80, 0x00, 0x00, 0x05, 0x64};
static uint8_t test_sdata_blocked_sid_5_nc8[] = {0x15, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x64};

/* STREAM_DATA_BLOCKED Stream Data Limit: Non-Canonical Varints */
static uint8_t test_sdata_blocked_limit_0_nc2[] = {0x15, 0x01, 0x40, 0x00};
static uint8_t test_sdata_blocked_limit_0_nc4[] = {0x15, 0x01, 0x80, 0x00, 0x00, 0x00};
static uint8_t test_sdata_blocked_limit_0_nc8[] = {0x15, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t test_sdata_blocked_limit_1_nc2[] = {0x15, 0x01, 0x40, 0x01};
static uint8_t test_sdata_blocked_limit_1_nc4[] = {0x15, 0x01, 0x80, 0x00, 0x00, 0x01};
static uint8_t test_sdata_blocked_limit_1_nc8[] = {0x15, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
static uint8_t test_sdata_blocked_limit_10_nc2[] = {0x15, 0x01, 0x40, 0x0A};
static uint8_t test_sdata_blocked_limit_10_nc4[] = {0x15, 0x01, 0x80, 0x00, 0x00, 0x0A};
static uint8_t test_sdata_blocked_limit_10_nc8[] = {0x15, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A};

/* STREAMS_BLOCKED (Bidi) Maximum Streams: Non-Canonical Varints */
static uint8_t test_streams_blocked_bidi_0_nc2[] = {0x16, 0x40, 0x00};
static uint8_t test_streams_blocked_bidi_0_nc4[] = {0x16, 0x80, 0x00, 0x00, 0x00};
static uint8_t test_streams_blocked_bidi_0_nc8[] = {0x16, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t test_streams_blocked_bidi_1_nc2[] = {0x16, 0x40, 0x01};
static uint8_t test_streams_blocked_bidi_1_nc4[] = {0x16, 0x80, 0x00, 0x00, 0x01};
static uint8_t test_streams_blocked_bidi_1_nc8[] = {0x16, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
static uint8_t test_streams_blocked_bidi_5_nc2[] = {0x16, 0x40, 0x05};
static uint8_t test_streams_blocked_bidi_5_nc4[] = {0x16, 0x80, 0x00, 0x00, 0x05};
static uint8_t test_streams_blocked_bidi_5_nc8[] = {0x16, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05};

/* STREAMS_BLOCKED (Uni) Maximum Streams: Non-Canonical Varints */
static uint8_t test_streams_blocked_uni_0_nc2[] = {0x17, 0x40, 0x00};
static uint8_t test_streams_blocked_uni_0_nc4[] = {0x17, 0x80, 0x00, 0x00, 0x00};
static uint8_t test_streams_blocked_uni_0_nc8[] = {0x17, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8_t test_streams_blocked_uni_1_nc2[] = {0x17, 0x40, 0x01};
static uint8_t test_streams_blocked_uni_1_nc4[] = {0x17, 0x80, 0x00, 0x00, 0x01};
static uint8_t test_streams_blocked_uni_1_nc8[] = {0x17, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
static uint8_t test_streams_blocked_uni_5_nc2[] = {0x17, 0x40, 0x05};
static uint8_t test_streams_blocked_uni_5_nc4[] = {0x17, 0x80, 0x00, 0x00, 0x05};
static uint8_t test_streams_blocked_uni_5_nc8[] = {0x17, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05};

/* Aggressive Padding / PMTU Probing Mimics */
static uint8_t test_frame_ping_padded_to_1200[1200] = {0x01}; /* PING + 1199 PADDING */
static uint8_t test_frame_ping_padded_to_1500[1500] = {0x01}; /* PING + 1499 PADDING */

/* ACK Frame Stress Tests */
static uint8_t test_frame_ack_very_many_small_ranges[] = {
    0x02,       /* Type: ACK */
    0x40, 0xC8, /* Largest Acknowledged: 200 */
    0x00,       /* ACK Delay: 0 */
    0x32,       /* ACK Range Count: 50 */
    0x00,       /* First ACK Range: 0 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 10 ranges */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 20 ranges */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 30 ranges */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 40 ranges */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00        /* 49 ranges */
};

static uint8_t test_frame_ack_alternating_large_small_gaps[] = {
    0x02,       /* Type: ACK */
    0x64,       /* Largest Acknowledged: 100 */
    0x00,       /* ACK Delay: 0 */
    0x04,       /* ACK Range Count: 4 */
    0x00,       /* First ACK Range: 0 (acks 100) */
    0x30,       /* Gap: 48 */
    0x00,       /* ACK Range Length: 0 (acks 50) */
    0x00,       /* Gap: 0 */
    0x00,       /* ACK Range Length: 0 (acks 48) */
    0x14,       /* Gap: 20 */
    0x00        /* ACK Range Length: 0 (acks 26) */
};

/* Unusual but Valid Header Flags/Values (Frames) */
static uint8_t test_frame_stream_id_almost_max[] = {
    0x08,       /* Type: STREAM */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, /* Stream ID: 2^62-1 */
    'm', 'a', 'x', 'S'
};

static uint8_t test_frame_stream_offset_almost_max[] = {
    0x0C,       /* Type: STREAM, OFF bit */
    0x01,       /* Stream ID: 1 */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, /* Offset: 2^62-1 */
    'm', 'a', 'x', 'O'
};

/* PATH_CHALLENGE / PATH_RESPONSE Variants */
static uint8_t test_frame_path_challenge_alt_pattern[] = {
    0x1a, 0xA5,0x5A,0xA5,0x5A,0xA5,0x5A,0xA5,0x5A
};
static uint8_t test_frame_path_response_alt_pattern[] = {
    0x1b, 0x5A,0xA5,0x5A,0xA5,0x5A,0xA5,0x5A,0xA5
};

/* NEW_TOKEN Frame Variants */
static uint8_t test_frame_new_token_max_plausible_len[3 + 256] = {
    0x07, 0x41, 0x00, /* Token Length 256 */
    /* Followed by 256 bytes of 0xAA */
};
static uint8_t test_frame_new_token_min_len[] = {
    0x07, 0x01, 0xBB
};

/* CONNECTION_CLOSE Frame Variants */
static uint8_t test_frame_connection_close_max_reason_len[5 + 1000] = {
    0x1c, 0x00, 0x00, 0x43, 0xE8, /* Error Code 0, Frame Type 0, Reason Length 1000 */
    /* Followed by 1000 bytes of 'A' (0x41) */
};
static uint8_t test_frame_connection_close_app_max_reason_len[4 + 1000] = {
    0x1d, 0x00, 0x43, 0xE8, /* Error Code 0, Reason Length 1000 */
    /* Followed by 1000 bytes of 'B' (0x42) */
};

/* RETIRE_CONNECTION_ID Variants */
static uint8_t test_frame_retire_cid_high_seq[] = {
    0x19, 0x80, 0x3B, 0x9A, 0xCA, 0x00 /* Sequence Number 1,000,000,000 */
};

/* MAX_STREAMS Variants (Absolute Max) */
static uint8_t test_frame_max_streams_bidi_abs_max[] = {
    0x12, 0xC0,0x00,0x00,0x00,0x10,0x00,0x00,0x00 /* Max Streams 2^60 */
};
static uint8_t test_frame_max_streams_uni_abs_max[] = {
    0x13, 0xC0,0x00,0x00,0x00,0x10,0x00,0x00,0x00 /* Max Streams 2^60 */
};

/* Additional STREAM Frame Variants */
static uint8_t test_frame_stream_off_len_fin_empty[] = {
    0x0F,       /* Type: STREAM, OFF, LEN, FIN bits */
    0x01,       /* Stream ID: 1 */
    0x64,       /* Offset: 100 (1-byte varint) */
    0x00        /* Length: 0 */
};

static uint8_t test_frame_stream_off_no_len_fin[] = {
    0x0D,       /* Type: STREAM, OFF, FIN bits */
    0x02,       /* Stream ID: 2 */
    0x40, 0xC8, /* Offset: 200 (2-byte varint) */
    'f', 'i', 'n'
};

static uint8_t test_frame_stream_no_off_len_fin_empty[] = {
    0x0B,       /* Type: STREAM, LEN, FIN bits */
    0x03,       /* Stream ID: 3 */
    0x00        /* Length: 0 */
};

static uint8_t test_frame_stream_just_fin_at_zero[] = {
    0x09,       /* Type: STREAM, FIN bit */
    0x04        /* Stream ID: 4 */
};

/* Zero-Length Data Frames with Max Varint Encoding for Fields */
static uint8_t test_frame_data_blocked_max_varint_offset[] = {
    0x14,       /* Type: DATA_BLOCKED */
    0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00 /* Max Data: 1024 (8-byte varint) */
};

static uint8_t test_frame_stream_data_blocked_max_varint_fields[] = {
    0x15,       /* Type: STREAM_DATA_BLOCKED */
    0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, /* Stream ID: 1 (8-byte varint) */
    0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00  /* Max Stream Data: 1024 (8-byte varint) */
};

static uint8_t test_frame_streams_blocked_bidi_max_varint_limit[] = {
    0x16,       /* Type: STREAMS_BLOCKED (bidirectional) */
    0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A  /* Max Streams: 10 (8-byte varint) */
};

static uint8_t test_frame_streams_blocked_uni_max_varint_limit[] = {
    0x17,       /* Type: STREAMS_BLOCKED (unidirectional) */
    0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A  /* Max Streams: 10 (8-byte varint) */
};

/* CRYPTO Frame Edge Cases */
static uint8_t test_frame_crypto_zero_len_large_offset[] = {
    0x06,       /* Type: CRYPTO */
    0x80, 0x01, 0x00, 0x00, /* Offset: 65536 (4-byte varint) */
    0x00        /* Length: 0 */
};

/* Non-Canonical Field Encodings (RFC 9000) */
/* DATA_BLOCKED Frame Variations (Type 0x14): */
static uint8_t test_frame_data_blocked_val_non_canon_2byte[] = {picoquic_frame_type_data_blocked, 0x40, 0x64};
/* STREAM_DATA_BLOCKED Frame Variations (Type 0x15): */
static uint8_t test_frame_sdb_sid_non_canon_4byte[] = {picoquic_frame_type_stream_data_blocked, 0x80, 0x00, 0x00, 0x01, 0x41, 0x00};
static uint8_t test_frame_sdb_val_non_canon_8byte[] = {picoquic_frame_type_stream_data_blocked, 0x01, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64};
/* STREAMS_BLOCKED Frame Variations (Bidirectional - Type 0x16): */
static uint8_t test_frame_streams_blocked_bidi_non_canon_8byte[] = {picoquic_frame_type_streams_blocked_bidir, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A};
/* STREAMS_BLOCKED Frame Variations (Unidirectional - Type 0x17): */
static uint8_t test_frame_streams_blocked_uni_non_canon_2byte[] = {picoquic_frame_type_streams_blocked_unidir, 0x40, 0x64};
/* NEW_CONNECTION_ID Frame Variations (Type 0x18): */
static uint8_t test_frame_ncid_seq_non_canon_2byte[] = {picoquic_frame_type_new_connection_id, 0x40, 0x01, 0x00, 0x08, 0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA, 0xBB,0xBB,0xBB,0xBB,0xBB,0xBB,0xBB,0xBB,0xBB,0xBB,0xBB,0xBB,0xBB,0xBB,0xBB,0xBB};
static uint8_t test_frame_ncid_ret_non_canon_4byte[] = {picoquic_frame_type_new_connection_id, 0x01, 0x80, 0x00, 0x00, 0x00, 0x08, 0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA, 0xBB,0xBB,0xBB,0xBB,0xBB,0xBB,0xBB,0xBB,0xBB,0xBB,0xBB,0xBB,0xBB,0xBB,0xBB,0xBB};
/* RETIRE_CONNECTION_ID Frame Variations (Type 0x19): */
static uint8_t test_frame_retire_cid_seq_non_canon_4byte[] = {picoquic_frame_type_retire_connection_id, 0x80, 0x00, 0x00, 0x01};
/* CONNECTION_CLOSE Frame Variations (Transport Error - Type 0x1c): */
static uint8_t test_frame_conn_close_ec_non_canon[] = {picoquic_frame_type_connection_close, 0x40, 0x01, 0x00, 0x00};
/* test_frame_conn_close_ft_non_canon is already defined as {0x1c, 0x00, 0x40, 0x08, 0x00} */
static uint8_t test_frame_conn_close_rlen_non_canon[] = {picoquic_frame_type_connection_close, 0x00, 0x00, 0x40, 0x04, 't', 'e', 's', 't'};
/* CONNECTION_CLOSE Frame Variations (Application Error - Type 0x1d): */
static uint8_t test_frame_conn_close_app_ec_non_canon[] = {picoquic_frame_type_application_close, 0x80, 0x00, 0x01, 0x01, 0x00};
static uint8_t test_frame_conn_close_app_rlen_non_canon_2byte[] = {picoquic_frame_type_application_close, 0x00, 0x40, 0x05, 't', 'e', 's', 't', '!'};

/* CRYPTO Frame Variations (Type 0x06) */
static uint8_t test_frame_crypto_offset_non_canon_4byte[] = {picoquic_frame_type_crypto_hs, 0x80, 0x00, 0x00, 0x0A, 0x04, 't', 'e', 's', 't'};
static uint8_t test_frame_crypto_len_non_canon_4byte[] = {picoquic_frame_type_crypto_hs, 0x0A, 0x80, 0x00, 0x00, 0x04, 't', 'e', 's', 't'};
/* NEW_TOKEN Frame Variations (Type 0x07) */
static uint8_t test_frame_new_token_len_non_canon_4byte[] = {picoquic_frame_type_new_token, 0x80, 0x00, 0x00, 0x08, '1', '2', '3', '4', '5', '6', '7', '8'};
/* MAX_DATA Frame Variations (Type 0x10) */
static uint8_t test_frame_max_data_non_canon_8byte[] = {picoquic_frame_type_max_data, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00};
/* MAX_STREAM_DATA Frame Variations (Type 0x11) */
static uint8_t test_frame_max_stream_data_sid_non_canon_2byte[] = {picoquic_frame_type_max_stream_data, 0x40, 0x01, 0x44, 0x00};
static uint8_t test_frame_max_stream_data_val_non_canon_4byte[] = {picoquic_frame_type_max_stream_data, 0x01, 0x80, 0x00, 0x01, 0x00};
/* MAX_STREAMS Frame Variations (Bidirectional - Type 0x12) */
static uint8_t test_frame_max_streams_bidi_non_canon_2byte[] = {picoquic_frame_type_max_streams_bidir, 0x40, 0x0A};
static uint8_t test_frame_max_streams_bidi_non_canon_8byte[] = {picoquic_frame_type_max_streams_bidir, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05};
/* MAX_STREAMS Frame Variations (Unidirectional - Type 0x13) */
static uint8_t test_frame_max_streams_uni_non_canon_4byte[] = {picoquic_frame_type_max_streams_unidir, 0x80, 0x00, 0x00, 0x64};

/* ACK, RESET_STREAM, STOP_SENDING Frame Variations (RFC 9000) */
/* ACK Frame Variations (Type 0x03 for ECN) */
static uint8_t test_frame_ack_ecn_ect0_large[] = {0x03, 0x0A, 0x00, 0x01, 0x00, 0x3F, 0x00, 0x00};
static uint8_t test_frame_ack_ecn_ect1_large[] = {0x03, 0x0A, 0x00, 0x01, 0x00, 0x00, 0x7F, 0xFF, 0x00};
static uint8_t test_frame_ack_ecn_ce_large[] = {0x03, 0x0A, 0x00, 0x01, 0x00, 0x00, 0x00, 0xBF, 0xFF, 0xFF, 0xFF};
static uint8_t test_frame_ack_ecn_all_large[] = {0x03, 0x0A, 0x00, 0x01, 0x00, 0x3F, 0x3F, 0x3F};
static uint8_t test_frame_ack_delay_non_canon[] = {picoquic_frame_type_ack, 0x05, 0x40, 0x0A, 0x01, 0x00};
static uint8_t test_frame_ack_range_count_non_canon[] = {picoquic_frame_type_ack, 0x05, 0x00, 0x40, 0x01, 0x00};
static uint8_t test_frame_ack_first_ack_range_non_canon[] = {picoquic_frame_type_ack, 0x05, 0x00, 0x01, 0x40, 0x00};
static uint8_t test_frame_ack_gap_non_canon[] = {picoquic_frame_type_ack, 0x14, 0x00, 0x02, 0x01, 0x40, 0x01, 0x01};

/* RESET_STREAM Frame Variations (Type 0x04) */
static uint8_t test_frame_reset_stream_app_err_non_canon[] = {picoquic_frame_type_reset_stream, 0x01, 0x80, 0x00, 0x01, 0x01, 0x64};
static uint8_t test_frame_reset_stream_final_size_non_canon_8byte[] = {picoquic_frame_type_reset_stream, 0x01, 0x00, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xE8};

/* STOP_SENDING Frame Variations (Type 0x05) */
static uint8_t test_frame_stop_sending_app_err_non_canon[] = {picoquic_frame_type_stop_sending, 0x01, 0x80, 0x00, 0x01, 0x01};

/* STREAM Frame Variations (RFC 9000, Section 19.8) */
/* Type 0x08 (OFF=0, LEN=0, FIN=0) */
static uint8_t test_stream_0x08_minimal[] = {0x08, 0x01};
static uint8_t test_stream_0x08_sid_non_canon[] = {0x08, 0x40, 0x01, 'h', 'i'};
static uint8_t test_stream_0x08_data_long[] = {0x08, 0x02, 'l', 'o', 'n', 'g', 's', 't', 'r', 'e', 'a', 'm', 'd', 'a', 't', 'a'};
/* Type 0x09 (OFF=0, LEN=0, FIN=1) */
static uint8_t test_stream_0x09_minimal[] = {0x09, 0x01};
static uint8_t test_stream_0x09_sid_non_canon[] = {0x09, 0x40, 0x01, 'f', 'i', 'n'};
/* Type 0x0A (OFF=0, LEN=1, FIN=0) */
static uint8_t test_stream_0x0A_len_zero_no_data[] = {0x0A, 0x01, 0x00};
static uint8_t test_stream_0x0A_len_zero_with_data[] = {0x0A, 0x02, 0x00, 'e', 'x', 't', 'r', 'a'};
static uint8_t test_stream_0x0A_len_small[] = {0x0A, 0x03, 0x01, 'd'};
static uint8_t test_stream_0x0A_len_large[] = {0x0A, 0x04, 0x40, 0xC8, 's', 'o', 'm', 'e', 'd', 'a', 't', 'a'};
static uint8_t test_stream_0x0A_sid_non_canon[] = {0x0A, 0x40, 0x01, 0x04, 't', 'e', 's', 't'};
static uint8_t test_stream_0x0A_len_non_canon[] = {0x0A, 0x02, 0x40, 0x04, 't', 'e', 's', 't'};
/* Type 0x0B (OFF=0, LEN=1, FIN=1) */
static uint8_t test_stream_0x0B_len_zero_no_data_fin[] = {0x0B, 0x01, 0x00};
static uint8_t test_stream_0x0B_len_non_canon_fin[] = {0x0B, 0x02, 0x40, 0x03, 'e', 'n', 'd'};
/* Type 0x0C (OFF=1, LEN=0, FIN=0) */
static uint8_t test_stream_0x0C_offset_zero[] = {0x0C, 0x01, 0x00, 'd', 'a', 't', 'a'};
static uint8_t test_stream_0x0C_offset_large[] = {0x0C, 0x02, 0x40, 0xC8, 'd', 'a', 't', 'a'};
static uint8_t test_stream_0x0C_sid_non_canon[] = {0x0C, 0x40, 0x01, 0x0A, 'o', 'f', 'f'};
static uint8_t test_stream_0x0C_offset_non_canon[] = {0x0C, 0x02, 0x40, 0x0A, 'o', 'f', 'f'};
/* Type 0x0D (OFF=1, LEN=0, FIN=1) */
static uint8_t test_stream_0x0D_offset_zero_fin[] = {0x0D, 0x01, 0x00, 'l', 'a', 's', 't'};
static uint8_t test_stream_0x0D_offset_non_canon_fin[] = {0x0D, 0x02, 0x40, 0x05, 'f', 'i', 'n', 'a', 'l'};
/* Type 0x0E (OFF=1, LEN=1, FIN=0) */
static uint8_t test_stream_0x0E_all_fields_present[] = {0x0E, 0x01, 0x0A, 0x04, 'd', 'a', 't', 'a'};
static uint8_t test_stream_0x0E_all_non_canon[] = {0x0E, 0x40, 0x01, 0x40, 0x0A, 0x40, 0x04, 't', 'e', 's', 't'};
/* Type 0x0F (OFF=1, LEN=1, FIN=1) */
static uint8_t test_stream_0x0F_all_fields_fin[] = {0x0F, 0x01, 0x0A, 0x07, 't', 'h', 'e', ' ', 'e', 'n', 'd'};
static uint8_t test_stream_0x0F_all_non_canon_fin[] = {0x0F, 0x40, 0x01, 0x40, 0x0A, 0x40, 0x04, 'd', 'o', 'n', 'e'};

/* Application Protocol Payloads (HTTP/3, DoQ) */
/* HTTP/3 Frame Payloads */
static uint8_t test_h3_frame_data_payload[] = {0x00, 0x04, 0x74, 0x65, 0x73, 0x74}; /* Type 0x00 (DATA), Length 4, "test" */
static uint8_t test_h3_frame_headers_payload_simple[] = {0x01, 0x01, 0x99}; /* Type 0x01 (HEADERS), Length 1, QPACK :method: GET */
static uint8_t test_h3_frame_settings_payload_empty[] = {0x04, 0x00}; /* Type 0x04 (SETTINGS), Length 0 */
static uint8_t test_h3_frame_settings_payload_one_setting[] = {0x04, 0x03, 0x06, 0x44, 0x00}; /* Type 0x04 (SETTINGS), Len 3, ID 0x06, Val 1024 (varint 0x4400) */
static uint8_t test_h3_frame_goaway_payload[] = {0x07, 0x01, 0x00}; /* Type 0x07 (GOAWAY), Length 1, ID 0 */
static uint8_t test_h3_frame_max_push_id_payload[] = {0x0D, 0x01, 0x0A}; /* Type 0x0D (MAX_PUSH_ID), Length 1, ID 10 */
static uint8_t test_h3_frame_cancel_push_payload[] = {0x03, 0x01, 0x03}; /* Type 0x03 (CANCEL_PUSH), Length 1, ID 3 */
static uint8_t test_h3_frame_push_promise_payload_simple[] = {0x05, 0x02, 0x01, 0x99}; /* Type 0x05 (PUSH_PROMISE), Len 2, PushID 1, QPACK :method: GET */

/* HTTP/3 ORIGIN frame draft-ietf-httpbis-origin-frame-00 */
static uint8_t test_frame_h3_origin_val_0x0c[] = { 0x0c };

/* HTTP/3 PRIORITY_UPDATE frame RFC9218 */
static uint8_t test_frame_h3_priority_update_val_0xf0700[] = { 0x80, 0x0F, 0x07, 0x00 };

/* HTTP/3 ORIGIN Frame Payload */
static uint8_t test_h3_frame_origin_payload[] = {
    0x0c, 0x14, 0x00, 0x12, 'h', 't', 't', 'p', ':', '/', '/', 'o', 'r', 'i', 'g', 'i', 'n', '.', 't', 'e', 's', 't'
};

/* HTTP/3 PRIORITY_UPDATE Frame Payloads */
static uint8_t test_h3_frame_priority_update_request_payload[] = {
    0x80, 0x0F, 0x07, 0x00, 0x06, 0x04, 0x75, 0x3d, 0x33, 0x2c, 0x69
};

static uint8_t test_h3_frame_priority_update_placeholder_payload[] = {
    0x80, 0x0F, 0x07, 0x01, 0x04, 0x02, 0x75, 0x3d, 0x35
};
/* H3_DATA Frame Variations */
static uint8_t test_h3_frame_data_empty[] = {0x00, 0x00};
static uint8_t test_h3_frame_data_len_non_canon[] = {0x00, 0x40, 0x04, 't', 'e', 's', 't'};
/* H3_SETTINGS Frame Variations */
static uint8_t test_h3_settings_max_field_section_size_zero[] = {0x04, 0x02, 0x06, 0x00};
static uint8_t test_h3_settings_max_field_section_size_large[] = {0x04, 0x03, 0x06, 0x7F, 0xFF};
static uint8_t test_h3_settings_multiple[] = {0x04, 0x06, 0x06, 0x44, 0x00, 0x21, 0x41, 0x00};
static uint8_t test_h3_settings_id_non_canon[] = {0x04, 0x03, 0x40, 0x06, 0x00};
static uint8_t test_h3_settings_val_non_canon[] = {0x04, 0x03, 0x06, 0x40, 0x0A};
/* H3_GOAWAY Frame Variations */
static uint8_t test_h3_goaway_max_id[] = {0x07, 0x01, 0x3F};
static uint8_t test_h3_goaway_id_non_canon[] = {0x07, 0x02, 0x40, 0x00};
/* H3_MAX_PUSH_ID Frame Variations */
static uint8_t test_h3_max_push_id_zero[] = {0x0D, 0x01, 0x00};
static uint8_t test_h3_max_push_id_non_canon[] = {0x0D, 0x02, 0x40, 0x0A};
/* H3_CANCEL_PUSH Frame Variations */
static uint8_t test_h3_cancel_push_max_id[] = {0x03, 0x02, 0x7F, 0xFF};
static uint8_t test_h3_cancel_push_id_non_canon[] = {0x03, 0x02, 0x40, 0x03};

/* DoQ Message Payload */
static uint8_t test_doq_dns_query_payload[] = {
    0x00, 0x1d, /* Length prefix for DNS message (29 bytes) */
    /* Start of DNS query */
    0x00, 0x00, /* Transaction ID */
    0x01, 0x00, /* Flags: Standard query, Recursion Desired */
    0x00, 0x01, /* Questions: 1 */
    0x00, 0x00, /* Answer RRs: 0 */
    0x00, 0x00, /* Authority RRs: 0 */
    0x00, 0x00, /* Additional RRs: 0 */
    /* Query: example.com A IN */
    0x07, 'e', 'x', 'a', 'm', 'p', 'l', 'e', /* example */
    0x03, 'c', 'o', 'm',                   /* com */
    0x00,                                  /*   (root) */
    0x00, 0x01,                            /* Type: A */
    0x00, 0x01                             /* Class: IN */
};

/* RFC 9113 (HTTP/2) Frame Types */
static uint8_t test_frame_h2_data_val_0x0[] = { 0x00 };
static uint8_t test_frame_h2_headers_val_0x1[] = { 0x01 };
static uint8_t test_frame_h2_priority_val_0x2[] = { 0x02 };
static uint8_t test_frame_h2_rst_stream_val_0x3[] = { 0x03 };
static uint8_t test_frame_h2_settings_val_0x4[] = { 0x04 };
static uint8_t test_frame_h2_push_promise_val_0x5[] = { 0x05 };
static uint8_t test_frame_h2_ping_val_0x6[] = { 0x06 };
static uint8_t test_frame_h2_goaway_val_0x7[] = { 0x07 };
static uint8_t test_frame_h2_window_update_val_0x8[] = { 0x08 };
static uint8_t test_frame_h2_continuation_val_0x9[] = { 0x09 };
static uint8_t test_frame_h2_altsvc_val_0xa[] = { 0x0a };

/* RFC 6455 (WebSocket) Frame Types */
static uint8_t test_frame_ws_continuation_val_0x0[] = { 0x00, 0x00 }; /* FIN=0, RSV1=0, RSV2=0, RSV3=0, Opcode=0x0 (Continuation), Mask=0, Payload length=0 */
static uint8_t test_frame_ws_text_val_0x1[] = { 0x81, 0x00 };         /* FIN=1, RSV1=0, RSV2=0, RSV3=0, Opcode=0x1 (Text), Mask=0, Payload length=0 */
static uint8_t test_frame_ws_binary_val_0x2[] = { 0x82, 0x00 };        /* FIN=1, RSV1=0, RSV2=0, RSV3=0, Opcode=0x2 (Binary), Mask=0, Payload length=0 */
static uint8_t test_frame_ws_connection_close_val_0x8[] = { 0x88, 0x00 }; /* FIN=1, RSV1=0, RSV2=0, RSV3=0, Opcode=0x8 (Connection Close), Mask=0, Payload length=0 */
static uint8_t test_frame_ws_ping_val_0x9[] = { 0x89, 0x00 };          /* FIN=1, RSV1=0, RSV2=0, RSV3=0, Opcode=0x9 (Ping), Mask=0, Payload length=0 */
static uint8_t test_frame_ws_pong_val_0xa[] = { 0x8A, 0x00 };          /* FIN=1, RSV1=0, RSV2=0, RSV3=0, Opcode=0xA (Pong), Mask=0, Payload length=0 */

/* Test Case: NEW_TOKEN frame with an empty token. */
/* Expected: Client treats as FRAME_ENCODING_ERROR (RFC 19.7). */
static uint8_t test_frame_new_token_empty_token[] = {
    picoquic_frame_type_new_token, /* Type 0x07 */
    0x00                           /* Token Length: 0 */
};

/* Test Case: STREAM frame type (0x08) encoded non-minimally as 2 bytes (0x4008). */
/* Expected: Peer MAY treat as PROTOCOL_VIOLATION (RFC 12.4). */
static uint8_t test_frame_stream_type_long_encoding[] = {
    0x40, 0x08, /* Frame Type STREAM (0x08) as 2-byte varint */
    0x01,       /* Stream ID: 1 */
    't', 'e', 's', 't'
};

/* Test Case: ACK frame type (0x02) encoded non-minimally as 2 bytes (0x4002). */
/* Expected: Peer MAY treat as PROTOCOL_VIOLATION (RFC 12.4). */
static uint8_t test_frame_ack_type_long_encoding[] = {
    0x40, 0x02, /* Frame Type ACK (0x02) as 2-byte varint */
    0x00,       /* Largest Acknowledged: 0 */
    0x00,       /* ACK Delay: 0 */
    0x01,       /* ACK Range Count: 1 */
    0x00        /* First ACK Range: 0 */
};

/* Test Case: RESET_STREAM frame type (0x04) encoded non-minimally as 2 bytes (0x4004). */
/* Expected: Peer MAY treat as PROTOCOL_VIOLATION (RFC 12.4). */
static uint8_t test_frame_reset_stream_type_long_encoding[] = {
    0x40, 0x04, /* Frame Type RESET_STREAM (0x04) as 2-byte varint */
    0x01,       /* Stream ID: 1 */
    0x00,       /* Application Error Code: 0 */
    0x00        /* Final Size: 0 */
};

/* Test Case: MAX_STREAMS (bidirectional) with Maximum Streams = 2^60 + 1 (invalid) */
/* Expected: FRAME_ENCODING_ERROR (RFC 19.11). */
static uint8_t test_frame_max_streams_bidi_just_over_limit[] = {
    picoquic_frame_type_max_streams_bidir, /* Type 0x12 */
    0xC0, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x01 /* Max Streams: (1ULL<<60) + 1 */
};

/* Test Case: MAX_STREAMS (unidirectional) with Maximum Streams = 2^60 + 1 (invalid) */
/* Expected: FRAME_ENCODING_ERROR (RFC 19.11). */
static uint8_t test_frame_max_streams_uni_just_over_limit[] = {
    picoquic_frame_type_max_streams_unidir, /* Type 0x13 */
    0xC0, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x01 /* Max Streams: (1ULL<<60) + 1 */
};

/* Test Case: Client sends STOP_SENDING for a server-initiated unidirectional stream (receive-only for client) */
/* Expected: Server treats as STREAM_STATE_ERROR (RFC 19.5 / Sec 3). */
/* To be injected when fuzzer acts as client. Stream ID 3 is server-initiated uni. */
static uint8_t test_client_sends_stop_sending_for_server_uni_stream[] = {
    picoquic_frame_type_stop_sending, /* Type 0x05 */
    0x03,                             /* Stream ID: 3 (server-initiated uni) */
    0x00                              /* Application Error Code: 0 */
};

/* Test Case: Server sends STOP_SENDING for a client-initiated unidirectional stream (receive-only for server) */
/* Expected: Client treats as STREAM_STATE_ERROR (RFC 19.5 / Sec 3). */
/* To be injected when fuzzer acts as server. Stream ID 2 is client-initiated uni. */
static uint8_t test_server_sends_stop_sending_for_client_uni_stream[] = {
    picoquic_frame_type_stop_sending, /* Type 0x05 */
    0x02,                             /* Stream ID: 2 (client-initiated uni) */
    0x00                              /* Application Error Code: 0 */
};

/* Test Case: Client sends MAX_STREAM_DATA for a client-initiated unidirectional stream (send-only for client) */
/* Expected: Server treats as STREAM_STATE_ERROR (RFC 19.10 / Sec 3). */
/* To be injected when fuzzer acts as client. Stream ID 2 is client-initiated uni. */
static uint8_t test_client_sends_max_stream_data_for_client_uni_stream[] = {
    picoquic_frame_type_max_stream_data, /* Type 0x11 */
    0x02,                                /* Stream ID: 2 */
    0x41, 0x00                           /* Max Stream Data: 256 */
};

/* Test Case: ACK frame in 1-RTT space acknowledging packet numbers typical of Initial/Handshake space. */
/* Expected: Peer should ignore these ACK ranges as they don't pertain to 1-RTT packets. */
/* (Indirect test for RFC 19.3, 12.5 PNS isolation for ACKs) */
static uint8_t test_frame_ack_cross_pns_low_pkns[] = {
    picoquic_frame_type_ack, /* Type 0x02 */
    0x02,                   /* Largest Acknowledged: 2 */
    0x00,                   /* ACK Delay: 0 */
    0x01,                   /* ACK Range Count: 1 */
    0x02                    /* First ACK Range: 2 (acks packets 0, 1, 2) */
};

/* Test Case: NEW_CONNECTION_ID frame. */
/* Context: To be sent to a peer that is configured to use/expect zero-length CIDs. */
/* Expected: Peer treats as PROTOCOL_VIOLATION (RFC 19.15). */
static uint8_t test_frame_new_cid_to_zero_len_peer[] = {
    picoquic_frame_type_new_connection_id, /* Type 0x18 */
    0x05,                                  /* Sequence Number: 5 */
    0x01,                                  /* Retire Prior To: 1 */
    0x08,                                  /* Length: 8 */
    0xAA,0xBB,0xCC,0xDD,0xEE,0xFF,0x00,0x11, /* Connection ID */
    0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88, /* Stateless Reset Token */
    0x99,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF,0x00
};

/* Test Case: RETIRE_CONNECTION_ID frame. */
/* Context: To be sent to a peer that has provided a zero-length source CID. */
/* Expected: Peer treats as PROTOCOL_VIOLATION (RFC 19.16). */
static uint8_t test_frame_retire_cid_to_zero_len_provider[] = {
    picoquic_frame_type_retire_connection_id, /* Type 0x19 */
    0x01                                      /* Sequence Number: 1 (to retire) */
};

/* Test Case: RESET_STREAM with Stream ID encoded non-canonically (value 1 as 2 bytes). */
/* Expected: Peer should process normally. (RFC 16) */
static uint8_t test_frame_reset_stream_sid_non_canon[] = {
    picoquic_frame_type_reset_stream, /* Type 0x04 */
    0x40, 0x01,                       /* Stream ID: 1 (2-byte varint) */
    0x00,                             /* Application Protocol Error Code: 0 */
    0x00                              /* Final Size: 0 */
};

/* Test Case: RESET_STREAM with App Error Code encoded non-canonically (value 1 as 2 bytes). */
/* Expected: Peer should process normally. (RFC 16) */
static uint8_t test_frame_reset_stream_err_non_canon[] = {
    picoquic_frame_type_reset_stream, /* Type 0x04 */
    0x01,                             /* Stream ID: 1 */
    0x40, 0x01,                       /* Application Protocol Error Code: 1 (2-byte varint) */
    0x00                              /* Final Size: 0 */
};

/* Test Case: RESET_STREAM with Final Size encoded non-canonically (value 1 as 2 bytes). */
/* Expected: Peer should process normally. (RFC 16) */
static uint8_t test_frame_reset_stream_final_non_canon[] = {
    picoquic_frame_type_reset_stream, /* Type 0x04 */
    0x01,                             /* Stream ID: 1 */
    0x00,                             /* Application Protocol Error Code: 0 */
    0x40, 0x01                        /* Final Size: 1 (2-byte varint) */
};

/* Test Case: STOP_SENDING with Stream ID encoded non-canonically (value 1 as 2 bytes). */
/* Expected: Peer should process normally. (RFC 16) */
static uint8_t test_frame_stop_sending_sid_non_canon[] = {
    picoquic_frame_type_stop_sending, /* Type 0x05 */
    0x40, 0x01,                       /* Stream ID: 1 (2-byte varint) */
    0x00                              /* Application Protocol Error Code: 0 */
};

/* Test Case: STOP_SENDING with App Error Code encoded non-canonically (value 1 as 2 bytes). */
/* Expected: Peer should process normally. (RFC 16) */
static uint8_t test_frame_stop_sending_err_non_canon[] = {
    picoquic_frame_type_stop_sending, /* Type 0x05 */
    0x01,                             /* Stream ID: 1 */
    0x40, 0x01                        /* Application Protocol Error Code: 1 (2-byte varint) */
};

/* Test Case: CRYPTO frame with Offset encoded non-canonically (value 10 as 2 bytes). */
/* Expected: Peer should process normally. (RFC 16) */
static uint8_t test_frame_crypto_offset_small_non_canon[] = {
    picoquic_frame_type_crypto_hs, /* Type 0x06 */
    0x40, 0x0A,                    /* Offset: 10 (2-byte varint) */
    0x04,                          /* Length: 4 */
    'd','a','t','a'
};

/* Test Case: CRYPTO frame with Length encoded non-canonically (value 4 as 2 bytes). */
/* Expected: Peer should process normally. (RFC 16) */
static uint8_t test_frame_crypto_len_small_non_canon[] = {
    picoquic_frame_type_crypto_hs, /* Type 0x06 */
    0x0A,                          /* Offset: 10 */
    0x40, 0x04,                    /* Length: 4 (2-byte varint) */
    'd','a','t','a'
};

/* Test Case: NEW_TOKEN frame with Token Length encoded non-canonically (value 16 as 2 bytes). */
/* Expected: Peer should process normally. (RFC 16) */
static uint8_t test_frame_new_token_len_non_canon[] = {
    picoquic_frame_type_new_token, /* Type 0x07 */
    0x40, 0x10,                    /* Token Length: 16 (2-byte varint) */
    '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f' /* Token data */
};

/* Test Case: PADDING frame (single byte). */
/* Expected: Peer should process normally. (RFC 19.1) */
static uint8_t test_frame_padding_single[] = {
    picoquic_frame_type_padding /* Type 0x00 */
};

/* Test Case: ACK frame with Range Count = 0 and a non-zero First ACK Range. */
/* Expected: Peer should process normally. (RFC 19.3) */
static uint8_t test_frame_ack_range_count_zero_first_range_set[] = {
    picoquic_frame_type_ack, /* Type 0x02 */
    0x0A,                    /* Largest Acknowledged: 10 */
    0x00,                    /* ACK Delay: 0 */
    0x00,                    /* ACK Range Count: 0 */
    0x05                     /* First ACK Range: 5 (acks packets 6-10) */
};

/* Test Case: ACK frame with an ACK Delay that might cause overflow if not handled carefully with ack_delay_exponent. */
/* Assuming default ack_delay_exponent = 3. Max ACK Delay field val is (2^62-1).
   A large val like 2^24 (0x01000000) for ACK Delay field, when multiplied by 2^3, is 2^27 microseconds.
   If ack_delay_exponent was, e.g., 20, then 2^24 * 2^20 = 2^44, still okay for u64 RTT.
   Let's use a value that is itself large, but not max varint.
   0x80, 0x01, 0x00, 0x00 -> 65536. Shifted by 3 = 524288 us = ~0.5 sec.
   Shifted by 20 = 65536 * 2^20 = 2^16 * 2^20 = 2^36 us = ~19 hours. This is large. */
/* Expected: Peer calculates RTT correctly or clamps delay. (RFC 19.3) */
static uint8_t test_frame_ack_delay_potentially_large_calc[] = {
    picoquic_frame_type_ack,       /* Type 0x02 */
    0x0A,                          /* Largest Acknowledged: 10 */
    0x80, 0x01, 0x00, 0x00,        /* ACK Delay: 65536 (raw value) */
    0x01,                          /* ACK Range Count: 1 */
    0x00                           /* First ACK Range: 0 */
};

/* Test Case: ACK frame acknowledging packet 0, with First ACK Range = 0. */
/* Expected: Peer should process normally. (RFC 19.3) */
static uint8_t test_frame_ack_largest_zero_first_zero[] = {
    picoquic_frame_type_ack, /* Type 0x02 */
    0x00,                    /* Largest Acknowledged: 0 */
    0x00,                    /* ACK Delay: 0 */
    0x01,                    /* ACK Range Count: 1 */
    0x00                     /* First ACK Range: 0 (acks packet 0) */
};

/* Test Case: ACK frame with ECN ECT0 count encoded non-minimally. */
/* Expected: Peer should process normally. (RFC 16) */
static uint8_t test_frame_ack_ecn_non_minimal_ect0[] = {
    picoquic_frame_type_ack_ecn, /* Type 0x03 */
    0x0A,                        /* Largest Acknowledged: 10 */
    0x00,                        /* ACK Delay: 0 */
    0x01,                        /* ACK Range Count: 1 */
    0x00,                        /* First ACK Range: 0 */
    0x40, 0x01,                  /* ECT0 Count: 1 (2-byte varint) */
    0x00,                        /* ECT1 Count: 0 */
    0x00                         /* ECN-CE Count: 0 */
};

/* Test Case: STREAM from Client on a server-initiated bidirectional Stream ID (e.g., 1). */
/* Expected: Server treats as STREAM_STATE_ERROR. (RFC 3.2, 19.8) */
static uint8_t test_stream_client_sends_server_bidi_stream[] = {
    0x0F,       /* Type: All bits (OFF,LEN,FIN) set for generic data */
    0x01,       /* Stream ID: 1 (Server-initiated bidi) */
    0x00,       /* Offset: 0 */
    0x04,       /* Length: 4 */
    't','e','s','t'
};

/* Test Case: STREAM from Client on a server-initiated unidirectional Stream ID (e.g., 3). */
/* Expected: Server treats as STREAM_STATE_ERROR. (RFC 3.2, 19.8) */
static uint8_t test_stream_client_sends_server_uni_stream[] = {
    0x0F,       /* Type: All bits set */
    0x03,       /* Stream ID: 3 (Server-initiated uni) */
    0x00,       /* Offset: 0 */
    0x04,       /* Length: 4 */
    't','e','s','t'
};

/* Test Case: STREAM from Server on a client-initiated bidirectional Stream ID (e.g., 0). */
/* Expected: Client treats as STREAM_STATE_ERROR. (RFC 3.2, 19.8) */
static uint8_t test_stream_server_sends_client_bidi_stream[] = {
    0x0F,       /* Type: All bits set */
    0x00,       /* Stream ID: 0 (Client-initiated bidi) */
    0x00,       /* Offset: 0 */
    0x04,       /* Length: 4 */
    't','e','s','t'
};

/* Test Case: STREAM from Server on a client-initiated unidirectional Stream ID (e.g., 2). */
/* Expected: Client treats as STREAM_STATE_ERROR. (RFC 3.2, 19.8) */
static uint8_t test_stream_server_sends_client_uni_stream[] = {
    0x0F,       /* Type: All bits set */
    0x02,       /* Stream ID: 2 (Client-initiated uni) */
    0x00,       /* Offset: 0 */
    0x04,       /* Length: 4 */
    't','e','s','t'
};

/* Test Case: STREAM with LEN=1, Length field = 0, but data is present. */
/* Expected: Parser should take Length field; trailing data might be another frame or error. (RFC 19.8) */
static uint8_t test_stream_explicit_len_zero_with_data[] = {
    0x0A,       /* Type: OFF=0, LEN=1, FIN=0 */
    0x01,       /* Stream ID: 1 */
    0x00,       /* Length: 0 */
    'd','a','t','a' /* This data should ideally be parsed as a separate frame or cause error */
};

/* Test Case: STREAM with only FIN bit set (Type 0x09), no data, implicit length, zero offset. */
/* Expected: Valid empty stream with FIN. (RFC 19.8) */
static uint8_t test_stream_fin_only_implicit_len_zero_offset[] = {
    0x09,       /* Type: OFF=0, LEN=0, FIN=1 */
    0x04        /* Stream ID: 4 */
    /* Data implicitly to end of packet, which is 0 here if this is the only frame. */
};

/* Test Case: STREAM with FIN=1, LEN=1, Length field = 0, but trailing data present. */
/* Expected: Stream ends at offset + 0. Trailing data is next frame or error. (RFC 19.8) */
static uint8_t test_stream_fin_len_zero_with_trailing_data[] = {
    0x0B,        /* Type: OFF=0, LEN=1, FIN=1 */
    0x01,        /* Stream ID: 1 */
    0x00,        /* Length: 0 */
    0x01         /* A PING frame as trailing data, for example */
};

/* Test Case: MAX_DATA with value 10 encoded non-canonically as 8-byte varint. */
/* Expected: Peer should process normally. (RFC 16) */
static uint8_t test_frame_max_data_long_varint_8byte_small[] = {
    picoquic_frame_type_max_data, /* Type 0x10 */
    0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A /* Maximum Data: 10 (8-byte varint) */
};

/* MAX_STREAM_DATA Variations */
/* Test Case: MAX_STREAM_DATA with Stream ID 0 and a non-zero Max Data value. */
/* Expected: Peer should process normally (Stream 0 is a valid bidi stream). (RFC 19.10) */
static uint8_t test_frame_max_stream_data_id_zero_val_set[] = {
    picoquic_frame_type_max_stream_data, /* Type 0x11 */
    0x00,                                /* Stream ID: 0 */
    0x41, 0x00                           /* Maximum Stream Data: 256 */
};

/* Test Case: MAX_STREAM_DATA with Stream ID encoded non-canonically (value 1 as 2 bytes). */
/* Expected: Peer should process normally. (RFC 16) */
static uint8_t test_frame_max_stream_data_sid_non_canon[] = {
    picoquic_frame_type_max_stream_data, /* Type 0x11 */
    0x40, 0x01,                          /* Stream ID: 1 (2-byte varint) */
    0x41, 0x00                           /* Maximum Stream Data: 256 */
};

/* Test Case: MAX_STREAM_DATA with Max Stream Data encoded non-canonically (value 10 as 2 bytes). */
/* Expected: Peer should process normally. (RFC 16) */
static uint8_t test_frame_max_stream_data_val_non_canon[] = {
    picoquic_frame_type_max_stream_data, /* Type 0x11 */
    0x01,                                /* Stream ID: 1 */
    0x40, 0x0A                           /* Maximum Stream Data: 10 (2-byte varint) */
};


/* MAX_STREAMS Variations */
/* Test Case: MAX_STREAMS (bidirectional) with Maximum Streams = 2^50 (large valid value). */
/* Expected: Peer should process normally. (RFC 19.11) */
static uint8_t test_frame_max_streams_bidi_val_2_pow_50[] = {
    picoquic_frame_type_max_streams_bidir, /* Type 0x12 */
    0xC0, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00 /* Max Streams: 2^50 */
};

/* Test Case: MAX_STREAMS (bidirectional) with small value 5 encoded non-canonically as 8-byte varint. */
/* Expected: Peer should process normally. (RFC 16) */
static uint8_t test_frame_max_streams_bidi_small_non_canon8[] = {
    picoquic_frame_type_max_streams_bidir, /* Type 0x12 */
    0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05 /* Maximum Streams: 5 (8-byte varint) */
};

/* DATA_BLOCKED Variations */
/* Test Case: DATA_BLOCKED with Maximum Data encoded non-canonically (value 10 as 2 bytes). */
/* Expected: Peer should process normally. (RFC 16) */
static uint8_t test_frame_data_blocked_non_canon2[] = {
    picoquic_frame_type_data_blocked, /* Type 0x14 */
    0x40, 0x0A                        /* Maximum Data: 10 (2-byte varint) */
};

/* Test Case: DATA_BLOCKED with Maximum Data encoded non-canonically (value 10 as 4 bytes). */
/* Expected: Peer should process normally. (RFC 16) */
static uint8_t test_frame_data_blocked_non_canon4[] = {
    picoquic_frame_type_data_blocked, /* Type 0x14 */
    0x80, 0x00, 0x00, 0x0A            /* Maximum Data: 10 (4-byte varint) */
};

/* Test Case: STREAM_DATA_BLOCKED with Stream ID non-canonically encoded (value 1 as 2 bytes). */
/* Expected: Peer should process normally. (RFC 16) */
static uint8_t test_frame_sdb_sid_non_canon[] = {
    picoquic_frame_type_stream_data_blocked, /* Type 0x15 */
    0x40, 0x01,                              /* Stream ID: 1 (2-byte varint) */
    0x41, 0x00                               /* Maximum Stream Data: 256 */
};

/* Test Case: STREAM_DATA_BLOCKED with Maximum Stream Data non-canonically encoded (value 10 as 2 bytes). */
/* Expected: Peer should process normally. (RFC 16) */
static uint8_t test_frame_sdb_val_non_canon[] = {
    picoquic_frame_type_stream_data_blocked, /* Type 0x15 */
    0x01,                                    /* Stream ID: 1 */
    0x40, 0x0A                               /* Maximum Stream Data: 10 (2-byte varint) */
};

/* Test Case: STREAM_DATA_BLOCKED with Stream ID 0. */
/* Expected: Peer should process normally for bidirectional stream 0. (RFC 19.13) */
static uint8_t test_frame_sdb_sid_zero[] = {
    picoquic_frame_type_stream_data_blocked, /* Type 0x15 */
    0x00,                                    /* Stream ID: 0 */
    0x41, 0x00                               /* Maximum Stream Data: 256 */
};

/* Test Case: STREAMS_BLOCKED (Bidirectional) with Maximum Streams = 2^60 (valid max). */
/* Expected: Peer should process normally. (RFC 19.14) */
static uint8_t test_frame_streams_blocked_bidi_at_limit[] = {
    picoquic_frame_type_streams_blocked_bidir, /* Type 0x16 */
    0xC0, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00 /* Max Streams: 2^60 */
};

/* Test Case: STREAMS_BLOCKED (Unidirectional) with Maximum Streams = 2^60 (valid max). */
/* Expected: Peer should process normally. (RFC 19.14) */
static uint8_t test_frame_streams_blocked_uni_at_limit[] = {
    picoquic_frame_type_streams_blocked_unidir, /* Type 0x17 */
    0xC0, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00 /* Max Streams: 2^60 */
};

/* Test Case: STREAMS_BLOCKED (Bidirectional) with Max Streams non-canonically encoded (value 10 as 2 bytes). */
/* Expected: Peer should process normally. (RFC 16) */
static uint8_t test_frame_streams_blocked_bidi_non_canon2[] = {
    picoquic_frame_type_streams_blocked_bidir, /* Type 0x16 */
    0x40, 0x0A                                 /* Maximum Streams: 10 (2-byte varint) */
};

/* Test Case: STREAMS_BLOCKED (Unidirectional) with Max Streams non-canonically encoded (value 10 as 4 bytes). */
/* Expected: Peer should process normally. (RFC 16) */
static uint8_t test_frame_streams_blocked_uni_non_canon4[] = {
    picoquic_frame_type_streams_blocked_unidir, /* Type 0x17 */
    0x80, 0x00, 0x00, 0x0A                     /* Maximum Streams: 10 (4-byte varint) */
};

/* Test Case: NEW_CONNECTION_ID with Sequence Number non-canonically encoded (value 1 as 2 bytes). */
/* Expected: Peer should process normally. (RFC 16) */
static uint8_t test_frame_ncid_seq_non_canon[] = {
    picoquic_frame_type_new_connection_id, /* Type 0x18 */
    0x40, 0x01,                            /* Sequence Number: 1 (2-byte varint) */
    0x00,                                  /* Retire Prior To: 0 */
    0x08,                                  /* Length: 8 */
    0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08, /* Connection ID */
    0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7, /* Stateless Reset Token */
    0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF
};

/* Test Case: NEW_CONNECTION_ID with Retire Prior To non-canonically encoded (value 0 as 2 bytes). */
/* Expected: Peer should process normally. (RFC 16) */
static uint8_t test_frame_ncid_ret_non_canon[] = {
    picoquic_frame_type_new_connection_id, /* Type 0x18 */
    0x01,                                  /* Sequence Number: 1 */
    0x40, 0x00,                            /* Retire Prior To: 0 (2-byte varint) */
    0x08,                                  /* Length: 8 */
    0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08, /* Connection ID */
    0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7, /* Stateless Reset Token */
    0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF
};
/* Specific ACK frame with an invalid gap of 1 */
/* Largest Ack: 5, Delay:0, RangeCount:1, FirstRangeLen:0 (acks 5), Gap:1 (skips 4), NextRangeLen:0 (acks 3) */
static uint8_t test_frame_ack_invalid_gap_1_specific_val[] = {0x02, 0x05, 0x00, 0x01, 0x00, 0x01, 0x00};

/* Frame Sequences for test_frame_sequence */
static uint8_t sequence_stream_ping_padding_val[] = {
    0x0A, 0x01, 0x05, 'h', 'e', 'l', 'l', 'o', /* STREAM ID 1, len 5, "hello" */
    0x01,                                     /* PING */
    0x00, 0x00, 0x00                          /* PADDING x3 */
};
static uint8_t sequence_max_data_max_stream_data_val[] = {
    0x10, 0x44, 0x00,                         /* MAX_DATA 1024 Add commentMore actions */
    0x11, 0x01, 0x42, 0x00                    /* MAX_STREAM_DATA Stream 1, 512 */
};

/* Error condition test frames */
static uint8_t error_stream_client_on_server_uni_val[] = { /* Client sends on server-initiated uni stream (ID 3) */
    0x09, 0x03, 'd', 'a', 't', 'a'         /* STREAM ID 3, FIN, "data" */
};
static uint8_t error_stream_len_shorter_val[] = { /* STREAM frame, LEN bit, Length=2, Data="test" (4 bytes) */
    0x0A, 0x04, 0x02, 't', 'e', 's', 't'
};
/* Test Case: NEW_CONNECTION_ID with minimum Connection ID Length (1). */
/* Expected: Peer should process normally. (RFC 19.15) */
static uint8_t test_frame_ncid_cid_len_min[] = {
    picoquic_frame_type_new_connection_id, /* Type 0x18 */
    0x02,                                  /* Sequence Number: 2 */
    0x00,                                  /* Retire Prior To: 0 */
    0x01,                                  /* Length: 1 */
    0xBB,                                  /* Connection ID (1 byte) */
    0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7, /* Stateless Reset Token */
    0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF
};

#define FUZI_Q_ITEM(n, x) \
    {                        \
        n, x, sizeof(x),     \
    }

/* Test Case: RETIRE_CONNECTION_ID with Sequence Number encoded non-canonically (value 1 as 2 bytes). */
/* Expected: Peer should process normally. (RFC 16) */
static uint8_t test_frame_retire_cid_seq_non_canon[] = {
    0x19,       /* Type: RETIRE_CONNECTION_ID */
    0x40, 0x01  /* Sequence Number: 1 (2-byte varint) */
};

/* Test Case: CONNECTION_CLOSE (transport error) with a reserved error code. */
/* Error Code 0x100 (QUIC_TLS_HANDSHAKE_FAILED) -> varint 0x4100. Frame Type PADDING (0x00). Empty reason. */
static uint8_t test_frame_conn_close_reserved_err[] = {
    picoquic_frame_type_connection_close, /* Type 0x1c */
    0x41, 0x00, /* Error Code: 0x100 (varint) */
    0x00,       /* Frame Type: PADDING */
    0x00        /* Reason Phrase Length: 0 */
};

/* Test Case: CONNECTION_CLOSE (transport error) with Frame Type encoded non-canonically. */
/* Error Code 0. Frame Type STREAM (0x08) as 2-byte varint (0x40, 0x08). Empty reason. */
static uint8_t test_frame_conn_close_ft_non_canon[] = {
    picoquic_frame_type_connection_close, /* Type 0x1c */
    0x00,       /* Error Code: 0 */
    0x40, 0x08, /* Frame Type: STREAM (0x08) as 2-byte varint */
    0x00        /* Reason Phrase Length: 0 */
};

static uint8_t sequence_stream_ping_padding[] = {
    0x0A, 0x01, 0x04, 't', 'e', 's', 't', /* STREAM frame */
    0x01,                               /* PING frame */
    0x00, 0x00, 0x00                    /* PADDING frame (3 bytes) */
};

static uint8_t sequence_max_data_max_stream_data[] = {
    0x10, 0x44, 0x00, /* MAX_DATA frame (Type 0x10, Value 1024) */
    0x11, 0x01, 0x44, 0x00 /* MAX_STREAM_DATA frame (Type 0x11, Stream 1, Value 1024) */
};

/* Test Case: CONNECTION_CLOSE (application error) with Reason Phrase Length non-canonically encoded. */
/* Error Code 0. Reason Phrase Length 5 as 2-byte varint (0x40, 0x05). Reason "test!". */
static uint8_t test_frame_conn_close_app_rlen_non_canon[] = {
    picoquic_frame_type_application_close, /* Type 0x1d */
    0x00,       /* Error Code: 0 */
    0x40, 0x05, /* Reason Phrase Length: 5 (2-byte varint) */
    't', 'e', 's', 't', '!'
};

/* Test Case: HANDSHAKE_DONE with Frame Type encoded non-canonically. */
/* Frame Type HANDSHAKE_DONE (0x1e) as 2-byte varint (0x40, 0x1e). */
static uint8_t test_frame_hsd_type_non_canon[] = {
    0x40, 0x1e  /* Frame Type: HANDSHAKE_DONE (0x1e) as 2-byte varint */
};

/* Proposed New STREAM Variants - RFC 9000, Section 19.8, 4.5 */

/* Test STREAM frame with LEN=1, FIN=1, OFF=0, explicit non-zero Length, but NO actual Stream Data.
 * Type 0x0B (OFF=0, LEN=1, FIN=1), Stream ID 1, Length 5. Packet ends.
 * Final size should be 5.
 */
static uint8_t test_stream_len_set_explicit_length_no_data_fin[] = {
    0x0B,       /* Type: OFF=0, LEN=1, FIN=1 */
    0x01,       /* Stream ID: 1 */
    0x05        /* Length: 5 */
    /* Packet is truncated here; no stream data provided. */
};

/* Test STREAM frame with OFF=1, LEN=1, FIN=1, where offset + length is very close to 2^62-1.
 * Type 0x0F, Stream ID 2, Offset ((1ULL<<62)-10), Length 5, Data "hello".
 * Final size should be (2^62)-10 + 5 = (2^62)-5.
 */
static uint8_t test_stream_off_len_fin_offset_plus_length_almost_max[] = {
    0x0F,        /* Type: OFF=1, LEN=1, FIN=1 */
    0x02,        /* Stream ID: 2 */
    /* Offset: (1ULL<<62)-10. Encoded as 8-byte varint.
       (2^62)-10 = 0x3FFFFFFFFFFFFFF6
       Prefix 0b11 -> 8 bytes.
       Value: 0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF6
    */
    0xBF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF6,
    0x05,        /* Length: 5 */
    'h', 'e', 'l', 'l', 'o' /* Stream Data */
};

/* Proposed New ACK Variant - RFC 9000, Section 19.3, 19.3.1, 19.3.2 */

/* Test ACK frame with ECN (Type 0x03), ACK Range Count = 0, but First ACK Range is set.
 * Largest Ack: 0x10, Delay: 0, Range Count: 0, First ACK Range: 0x05.
 * ECN Counts: ECT0=1, ECT1=1, CE=1.
 */
static uint8_t test_ack_ecn_range_count_zero_first_range_set_with_counts[] = {
    0x03,       /* Type: ACK with ECN */
    0x10,       /* Largest Acknowledged: 16 */
    0x00,       /* ACK Delay: 0 */
    0x00,       /* ACK Range Count: 0 */
    0x05,       /* First ACK Range: 5 (acks packets 11-16) */
    0x01,       /* ECT0 Count: 1 */
    0x01,       /* ECT1 Count: 1 */
    0x01        /* ECN-CE Count: 1 */
};

/* Proposed New CONNECTION_CLOSE Variant - RFC 9000, Section 19.19 */

/* Test CONNECTION_CLOSE (transport) with minimal fields.
 * Type 0x1c, Error Code INTERNAL_ERROR (0x01), Frame Type PADDING (0x00), Reason Phrase Length 0.
 */
static uint8_t test_connection_close_transport_min_fields[] = {
    0x1c,       /* Type: CONNECTION_CLOSE (transport) */
    0x01,       /* Error Code: INTERNAL_ERROR (0x01) */
    0x00,       /* Frame Type: PADDING (0x00) */
    0x00        /* Reason Phrase Length: 0 */
};

/* Proposed New MAX_STREAM_DATA Variant - RFC 9000, Section 19.10 */

/* Test MAX_STREAM_DATA with Stream ID and Max Stream Data at max varint values.
 * Type 0x11, Stream ID (2^62)-1, Max Stream Data (2^62)-1.
 */
static uint8_t test_max_stream_data_id_max_val_max[] = {
    0x11,       /* Type: MAX_STREAM_DATA */
    /* Stream ID: (1ULL<<62)-1 = 0x3FFFFFFFFFFFFFFF. Encoded as 8-byte varint. */
    0xBF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    /* Maximum Stream Data: (1ULL<<62)-1 = 0x3FFFFFFFFFFFFFFF. Encoded as 8-byte varint. */
    0xBF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

/* --- Batch 1 of New Edge Case Test Variants --- */

/* RFC 9000, Sec 19.8, 4.5 - STREAM (Type 0x0D: OFF=1,LEN=0,FIN=1) with max offset and 1 byte of data.
 * Offset is (2^62-1), implicit length is 1. Final size = offset + 1, which exceeds 2^62-1.
 * Expected: FINAL_SIZE_ERROR by receiver.
 */
static uint8_t test_stream_implicit_len_max_offset_with_data[] = {
    0x0D,       /* Type: OFF=1, LEN=0, FIN=1 */
    0x01,       /* Stream ID: 1 */
    /* Offset: (1ULL<<62)-1 = 0x3FFFFFFFFFFFFFFF. Encoded as 8-byte varint. */
    0xBF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    'X'         /* Stream Data: 1 byte */
};

/* RFC 9000, Sec 19.3 - ACK frame Type 0x02 (no ECN) but with trailing ECN-like data.
 * Parser should correctly identify end of ACK frame based on its fields and either
 * ignore trailing data or flag an error if it tries to parse beyond necessary.
 */
static uint8_t test_ack_type02_with_trailing_ecn_like_data[] = {
    0x02,       /* Type: ACK (no ECN bit) */
    0x10,       /* Largest Acknowledged: 16 */
    0x01,       /* ACK Delay: 1 (raw value) */
    0x01,       /* ACK Range Count: 1 */
    0x02,       /* First ACK Range: 2 (acks packets 14-16) */
    0x03,       /* Gap: 3 (unacked 10-12) */
    0x04,       /* ACK Range Length: 4 (acks packets 6-9) */
    /* Trailing ECN-like data, should be ignored or cause error if parsed as part of ACK */
    0x40, 0x01, /* ECT0 Count: 1 (varint) */
    0x40, 0x01, /* ECT1 Count: 1 (varint) */
    0x40, 0x01  /* ECN-CE Count: 1 (varint) */
};

/* RFC 9000, Sec 19.15 - NEW_CONNECTION_ID frame with CID truncated.
 * Length field for CID is 8, but only 4 bytes of CID data provided before packet ends.
 * Expected: FRAME_ENCODING_ERROR by receiver.
 */
static uint8_t test_new_connection_id_truncated_cid[] = {
    0x18,       /* Type: NEW_CONNECTION_ID */
    0x01,       /* Sequence Number: 1 */
    0x00,       /* Retire Prior To: 0 */
    0x08,       /* Length of Connection ID: 8 */
    0xAA, 0xBB, 0xCC, 0xDD /* Connection ID data (only 4 bytes) */
    /* Packet ends here, Stateless Reset Token is missing */
};

/* RFC 9000, Sec 19.15 - NEW_CONNECTION_ID frame with Stateless Reset Token truncated.
 * CID is fully provided (8 bytes), but token is only partially provided (8 of 16 bytes).
 * Expected: FRAME_ENCODING_ERROR by receiver.
 */
static uint8_t test_new_connection_id_truncated_token[] = {
    0x18,       /* Type: NEW_CONNECTION_ID */
    0x02,       /* Sequence Number: 2 */
    0x01,       /* Retire Prior To: 1 */
    0x08,       /* Length of Connection ID: 8 */
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, /* Connection ID data */
    /* Stateless Reset Token (partially provided) */
    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7
    /* Packet ends here, 8 bytes of token missing */
};

/* RFC 9000, Sec 19.15 - NEW_CONNECTION_ID frame with CID data longer than Length field.
 * Length field for CID is 4, but 6 bytes of CID data provided.
 * Parser should use Length field to find start of token.
 */
static uint8_t test_new_connection_id_cid_overrun_length_field[] = {
    0x18,       /* Type: NEW_CONNECTION_ID */
    0x03,       /* Sequence Number: 3 */
    0x00,       /* Retire Prior To: 0 */
    0x04,       /* Length of Connection ID: 4 */
    /* Actual Connection ID data (6 bytes) */
    0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
    /* Stateless Reset Token (16 bytes) */
    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
    0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF
};

/* --- Batch 2 of New Edge Case Test Variants (Flow Control) --- */

/* RFC 9000, Sec 19.12 - DATA_BLOCKED frame with Maximum Data at max varint value.
 * Maximum Data: (1ULL<<62)-1 = 0x3FFFFFFFFFFFFFFF
 */
static uint8_t test_data_blocked_max_value[] = {
    0x14,       /* Type: DATA_BLOCKED */
    /* Maximum Data: (1ULL<<62)-1. Encoded as 8-byte varint. */
    0xBF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

/* RFC 9000, Sec 19.13 - STREAM_DATA_BLOCKED frame with Stream ID and Max Stream Data at max varint values.
 * Stream ID: (1ULL<<62)-1
 * Maximum Stream Data: (1ULL<<62)-1
 */
static uint8_t test_stream_data_blocked_max_id_max_value[] = {
    0x15,       /* Type: STREAM_DATA_BLOCKED */
    /* Stream ID: (1ULL<<62)-1. Encoded as 8-byte varint. */
    0xBF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    /* Maximum Stream Data: (1ULL<<62)-1. Encoded as 8-byte varint. */
    0xBF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

/* --- Batch 3 of New Edge Case Test Variants (Stream Limit Frames) --- */

/* RFC 9000, Sec 19.14 - STREAMS_BLOCKED (bidirectional) with Maximum Streams over limit.
 * Maximum Streams: (1ULL<<60) + 1.
 * Expected: FRAME_ENCODING_ERROR or STREAM_LIMIT_ERROR by receiver.
 */
static uint8_t test_streams_blocked_bidi_over_limit[] = {
    0x16,       /* Type: STREAMS_BLOCKED (bidirectional) */
    /* Maximum Streams: (1ULL<<60) + 1. Varint encoded: */
    /* (1ULL<<60) is 0x1000000000000000. (1ULL<<60)+1 is 0x1000000000000001 */
    /* For 8-byte varint, first byte is (value >> 56) | 0xC0 */
    /* (0x1000000000000001 >> 56) & 0x3F = 0x10 & 0x3F = 0x10. */
    /* First byte: 0xC0 | 0x10 = 0xD0. */
    /* Remaining 7 bytes: 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01. */
    0xD0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
};

/* RFC 9000, Sec 19.14 - STREAMS_BLOCKED (unidirectional) with Maximum Streams over limit.
 * Maximum Streams: (1ULL<<60) + 1.
 * Expected: FRAME_ENCODING_ERROR or STREAM_LIMIT_ERROR by receiver.
 */
static uint8_t test_streams_blocked_uni_over_limit[] = {
    0x17,       /* Type: STREAMS_BLOCKED (unidirectional) */
    /* Maximum Streams: (1ULL<<60) + 1. Varint encoded (same as above): */
    0xD0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
};

/* --- Batch 5 of New Edge Case Test Variants (Other Control Frames) --- */

/* RFC 9000, Sec 19.5 - STOP_SENDING with max Stream ID and max App Error Code.
 * Stream ID: (1ULL<<62)-1
 * App Error Code: (1ULL<<62)-1
 */
static uint8_t test_stop_sending_max_id_max_error[] = {
    0x05,       /* Type: STOP_SENDING */
    /* Stream ID: (1ULL<<62)-1. Varint encoded: */
    0xBF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    /* Application Protocol Error Code: (1ULL<<62)-1. Varint encoded: */
    0xBF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

/* RFC 9000, Sec 19.16 - RETIRE_CONNECTION_ID with max Sequence Number.
 * Sequence Number: (1ULL<<62)-1
 */
static uint8_t test_retire_connection_id_max_sequence[] = {
    0x19,       /* Type: RETIRE_CONNECTION_ID */
    /* Sequence Number: (1ULL<<62)-1. Varint encoded: */
    0xBF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

/* RFC 9000, Sec 19.7 - NEW_TOKEN with Token Length > 0, but no token data (truncated).
 * Token Length: 5
 * Expected: FRAME_ENCODING_ERROR by receiver.
 */
static uint8_t test_new_token_len_gt_zero_no_token_data_truncated[] = {
    0x07,       /* Type: NEW_TOKEN */
    0x05        /* Token Length: 5 */
    /* Packet ends here, no token data */
};

/* RFC 9000, Sec 19.4 - RESET_STREAM with Stream ID, App Error Code, and Final Size all max.
 * Stream ID: (1ULL<<62)-1
 * App Error Code: (1ULL<<62)-1
 * Final Size: (1ULL<<62)-1
 * Expected: Likely FINAL_SIZE_ERROR by receiver.
 */
static uint8_t test_reset_stream_all_fields_max_value[] = {
    0x04,       /* Type: RESET_STREAM */
    /* Stream ID: (1ULL<<62)-1. Varint encoded: */
    0xBF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    /* Application Protocol Error Code: (1ULL<<62)-1. Varint encoded: */
    0xBF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    /* Final Size: (1ULL<<62)-1. Varint encoded: */
    0xBF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

/* --- Batch 4 of New Edge Case Test Variants (Path Validation Frames) --- */

/* RFC 9000, Sec 19.17 - PATH_CHALLENGE frame with Data field all ones. */
static uint8_t test_path_challenge_all_ones[] = {
    0x1a,       /* Type: PATH_CHALLENGE */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF /* Data */
};

/* RFC 9000, Sec 19.18 - PATH_RESPONSE frame with Data field as 0xAA pattern. */
static uint8_t test_path_response_alt_bits_AA[] = {
    0x1b,       /* Type: PATH_RESPONSE */
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA /* Data */
};

/* RFC 9000, Sec 19.17 - PATH_CHALLENGE frame, truncated after 4 data bytes.
 * Expected: FRAME_ENCODING_ERROR by receiver.
 */
static uint8_t test_path_challenge_truncated_4bytes[] = {
    0x1a,       /* Type: PATH_CHALLENGE */
    0xDE, 0xAD, 0xBE, 0xEF /* Data (4 of 8 bytes) */
    /* Packet ends here */
};

/* RFC 9000, Sec 19.18 - PATH_RESPONSE frame, truncated after type (0 data bytes).
 * Expected: FRAME_ENCODING_ERROR by receiver.
 */
static uint8_t test_path_response_truncated_0bytes[] = {
    0x1b       /* Type: PATH_RESPONSE */
    /* Packet ends here */
};

/* --- Batch 6 of New Edge Case Test Variants (CRYPTO, DATAGRAM, PADDING) --- */

/* RFC 9000, Sec 19.6 - CRYPTO frame with Length > 0, but no data (truncated after Length).
 * Offset 0, Length 5. Expected: FRAME_ENCODING_ERROR.
 */
static uint8_t test_crypto_len_gt_zero_no_data_truncated[] = {
    0x06,       /* Type: CRYPTO */
    0x00,       /* Offset: 0 */
    0x05        /* Length: 5 */
    /* Packet ends here, no crypto data */
};

/* RFC 9221, Sec 4 - DATAGRAM frame (Type 0x30, no length) empty (truncated after type). */
static uint8_t test_datagram_type0x30_empty_truncated[] = {
    0x30        /* Type: DATAGRAM (no length) */
    /* Packet ends here */
};

/* RFC 9221, Sec 4 - DATAGRAM frame (Type 0x30, no length) with one byte of data. */
static uint8_t test_datagram_type0x30_one_byte[] = {
    0x30,       /* Type: DATAGRAM (no length) */
    0xAB        /* Datagram Data: 1 byte */
};

/* RFC 9221, Sec 4 - DATAGRAM frame (Type 0x31, with length) with max Length field, minimal data.
 * Length: (1ULL<<62)-1. Actual data: 1 byte (truncated).
 * Expected: FRAME_ENCODING_ERROR.
 */
static uint8_t test_datagram_type0x31_maxlength_field_min_data[] = {
    0x31,       /* Type: DATAGRAM (with length) */
    /* Length: (1ULL<<62)-1. Varint encoded (0xBF, 0xFF, ..., 0xFF) */
    0xBF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xCD        /* Datagram Data: 1 byte */
    /* Packet ends, far short of declared length */
};

/* RFC 9221, Sec 4 - DATAGRAM frame (Type 0x31, with length) with Length > 0, but no data (truncated after Length).
 * Length: 5. Expected: FRAME_ENCODING_ERROR.
 */
static uint8_t test_datagram_type0x31_len_gt_zero_no_data_truncated[] = {
    0x31,       /* Type: DATAGRAM (with length) */
    0x05        /* Length: 5 */
    /* Packet ends here, no datagram data */
};

/* RFC 9000, Sec 19.1, 12.4 - PADDING frame type (0x00) non-canonically encoded (2 bytes).
 * Expected: PROTOCOL_VIOLATION (optional).
 */
static uint8_t test_padding_type_non_canonical_2byte[] = {
    0x40, 0x00  /* PADDING type 0x00 encoded as 2-byte varint */
};

/* START OF JULES ADDED FRAMES (BATCHES 1-8) */

/* --- Batch 1: Unknown or Unassigned Frame Types --- */
// QUIC frame type 0x20 (in private use range, but could be unassigned for this impl)
static uint8_t test_frame_quic_unknown_0x20[] = { 0x20 };
// QUIC frame type 0x3F (max 1-byte varint, likely unassigned) with short payload
static uint8_t test_frame_quic_unknown_0x3f_payload[] = { 0x3F, 0x01, 0x02, 0x03 };
// QUIC frame type 0x402A (2-byte varint, greased pattern, likely unassigned)
static uint8_t test_frame_quic_unknown_greased_0x402a[] = { 0x40, 0x2A };
// HTTP/3 reserved frame type 0x02 (from RFC9114 Table 2) with empty payload
static uint8_t test_frame_h3_reserved_0x02[] = { 0x02, 0x00 };
// HTTP/3 reserved frame type 0x06 (from RFC9114 Table 2) with empty payload
static uint8_t test_frame_h3_reserved_0x06[] = { 0x06, 0x00 };
// HTTP/3 frame type 0x21 (unassigned in RFC9114, but in reserved block 0x21-0x3F for extensions)
static uint8_t test_frame_h3_unassigned_extension_0x21[] = { 0x21, 0x00 };

/* --- Batch 1: Malformed Frame Lengths --- */
// STREAM frame (0x0a: OFF=0,LEN=1,FIN=0), Len=0, but data is present
static uint8_t test_frame_quic_stream_len0_with_data[] = { 0x0a, 0x01, 0x00, 'd', 'a', 't', 'a' };
// STREAM frame (0x0a), Len=100, but data is "short" (5 bytes)
static uint8_t test_frame_quic_stream_len_gt_data[] = { 0x0a, 0x02, 0x64, 's', 'h', 'o', 'r', 't' };
// STREAM frame (0x0a), Len=2, but data is "longerdata" (10 bytes)
static uint8_t test_frame_quic_stream_len_lt_data[] = { 0x0a, 0x03, 0x02, 'l', 'o', 'n', 'g', 'e', 'r', 'd', 'a', 't', 'a' };
// CRYPTO frame (0x06), Len=0, but data is present
static uint8_t test_frame_quic_crypto_len0_with_data[] = { 0x06, 0x00, 0x00, 'c', 'r', 'y', 'p', 't' };
// NEW_TOKEN frame (0x07), Len=10, but token is "short" (5 bytes)
static uint8_t test_frame_quic_new_token_len_gt_data[] = { 0x07, 0x0A, 's', 'h', 'o', 'r', 't'};
// CONNECTION_CLOSE (0x1c), ReasonLen=10, but reason is "err" (3 bytes)
static uint8_t test_frame_quic_conn_close_reason_len_gt_data[] = { 0x1c, 0x01, 0x00, 0x0A, 'e', 'r', 'r' };

/* --- Batch 1: Invalid Frame Field Values --- */
// MAX_STREAMS Bidi (0x12), value 0 (disallowing any new bidi streams by peer)
static uint8_t test_frame_quic_max_streams_bidi_value0[] = { 0x12, 0x00 };
// STOP_SENDING (0x05) for Stream 1, with a very large error code (2^62-1)
static uint8_t test_frame_quic_stop_sending_large_error[] = { 0x05, 0x01, 0xBF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
// MAX_DATA (0x10) with value 0
static uint8_t test_frame_quic_max_data_value0_b1[] = { 0x10, 0x00 }; // Renamed to avoid conflict
// ACK frame (0x02) with LargestAck=0, Delay=0, 1 Range, RangeLen=0 (acks only pkt 0)
static uint8_t test_frame_quic_ack_largest0_delay0_1range0_b1[] = { 0x02, 0x00, 0x00, 0x01, 0x00 }; // Renamed
// NEW_CONNECTION_ID (0x18) with RetirePriorTo > SequenceNumber
static uint8_t test_frame_quic_ncid_retire_gt_seq_b1[] = { 0x18, 0x02, 0x05, 0x08, 0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8, 0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,0xF0 }; // Renamed

/* --- Batch 2: More Invalid Frame Field Values --- */
// MAX_DATA (0x10) with value 0 - covered by test_frame_quic_max_data_value0_b1, keep one.
static uint8_t test_frame_quic_max_data_value0[] = { 0x10, 0x00 };
// ACK frame (0x02) with LargestAck=0, Delay=0, 1 Range, RangeLen=0 (acks only pkt 0) - covered by test_frame_quic_ack_largest0_delay0_1range0_b1, keep one.
static uint8_t test_frame_quic_ack_largest0_delay0_1range0[] = { 0x02, 0x00, 0x00, 0x01, 0x00 };
// ACK frame (0x02) with ACK Range Count = 0, but First ACK Range is set (valid per RFC 19.3)
static uint8_t test_frame_quic_ack_range_count0_first_range_set[] = { 0x02, 0x0A, 0x00, 0x00, 0x05 };
// H3 SETTINGS frame (type 0x04) with an unknown Setting ID (e.g. 0x7FFF, a large 2-byte varint) and value 0.
static uint8_t test_frame_h3_settings_unknown_id[] = { 0x04, 0x03, 0x7F, 0xFF, 0x00 };
// H3 SETTINGS frame (type 0x04) setting MAX_FIELD_SECTION_SIZE (ID 0x06) to 0.
static uint8_t test_frame_h3_settings_max_field_section_size0[] = { 0x04, 0x02, 0x06, 0x00 };
// MAX_STREAM_DATA (0x11) for Stream 1, with Max Stream Data = 0.
static uint8_t test_frame_quic_max_stream_data_value0[] = { 0x11, 0x01, 0x00 };
// CONNECTION_CLOSE (type 0x1c) with a reserved error code (e.g., 0x1A = PATH_CHALLENGE type)
static uint8_t test_frame_quic_conn_close_reserved_error[] = { 0x1c, 0x1A, 0x00, 0x00 }; // Error 0x1A, Frame Type PADDING, ReasonLen 0
// NEW_TOKEN (0x07) with Token Length = 0 (invalid according to RFC 19.7)
static uint8_t test_frame_quic_new_token_zero_len_invalid[] = { 0x07, 0x00 };

/* --- Batch 2: Padding Fuzzing --- */
// PADDING frame (0x00) making up an entire large packet (e.g. 70 bytes of PADDING frames)
static uint8_t test_frame_quic_padding_excessive_70bytes[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 10 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 20 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 30 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 40 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 50 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 60 */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  /* 70 */
};
// PADDING frame followed by non-zero bytes (already covered by test_frame_padding_mixed_payload)
// PADDING frame at the end of a packet that's not full (valid, but fuzzer might try this context)
// A single PING frame followed by many PADDING frames (e.g. to reach MTU)
static uint8_t test_frame_quic_ping_then_many_padding[60]; // Initialized in code

/* --- Batch 2: Stream ID Fuzzing (static part) --- */
// STREAM frame (0x0a) for Stream ID 0 (client-initiated bidi)
static uint8_t test_frame_quic_stream_id0[] = { 0x0a, 0x00, 0x04, 't', 'e', 's', 't' };
// MAX_STREAM_DATA (0x11) for a server-initiated unidirectional stream ID (e.g. ID 3 from client perspective)
static uint8_t test_frame_quic_max_stream_data_server_uni[] = { 0x11, 0x03, 0x41, 0x00 }; /* Max data 256 */
// RESET_STREAM (0x04) for a client-initiated unidirectional stream ID (e.g. ID 2 from server perspective)
static uint8_t test_frame_quic_reset_stream_client_uni[] = { 0x04, 0x02, 0x00, 0x00 }; /* Error 0, Final Size 0 */
// STOP_SENDING (0x05) for a stream ID that is too large (exceeds peer's MAX_STREAMS limit if known, or just a large number)
static uint8_t test_frame_quic_stop_sending_large_stream_id[] = { 0x05, 0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE, 0x00 }; // Stream ID 2^62-2, Error 0.
// STREAM frame (0x08) using a server-initiated stream ID from a client (e.g. ID 1)
static uint8_t test_frame_quic_stream_client_uses_server_id[] = { 0x08, 0x01, 'b', 'a', 'd' };


/* --- Batch 3: User Prioritized Frames (Part 1 - DATAGRAM) --- */
// DATAGRAM type 0x30 (no length) but includes a varint that looks like a length, then data
static uint8_t test_frame_datagram_type30_with_len_data_error[] = { 0x30, 0x04, 'd','a','t','a' };
// DATAGRAM type 0x31 (with length) but is truncated before the length field
static uint8_t test_frame_datagram_type31_missing_len_error[] = { 0x31 };
// DATAGRAM type 0x31 (with length), length is 0, but data is present
static uint8_t test_frame_datagram_type31_len_zero_with_data_error[] = { 0x31, 0x00, 'd','a','t','a' };
// DATAGRAM type 0x31, length is huge, data is small (truncated content)
static uint8_t test_frame_datagram_type31_len_huge_data_small[] = { 0x31, 0x80, 0x01, 0x00, 0x00, 't', 'i', 'n', 'y' }; /* Length 65536 */
// DATAGRAM type 0x30 (no length) but packet is empty after type (valid empty datagram)
static uint8_t test_frame_datagram_type30_empty_valid[] = { 0x30 };
// DATAGRAM type 0x31 (with length), length is 0, no data (valid empty datagram)
static uint8_t test_frame_datagram_type31_len0_empty_valid[] = { 0x31, 0x00 };

/* --- Batch 3: User Prioritized Frames (Part 1 - H3 SETTINGS) --- */
// H3 SETTINGS (type 0x04) with far too many ID/Value pairs (exceeds reasonable frame length)
// Length is 254 (0xFE), followed by 127 pairs of (ID=1, Value=1)
static uint8_t test_h3_settings_excessive_pairs[] = {
    0x04, 0xFE,
    0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, /* 8 pairs */
    0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, /* 16 pairs */
    0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, /* 24 pairs */
    0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, /* 32 pairs */
    0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, /* 40 pairs */
    0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, /* 48 pairs */
    0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, /* 56 pairs */
    0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, /* 64 pairs */
    0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, /* 72 pairs */
    0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, /* 80 pairs */
    0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, /* 88 pairs */
    0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, /* 96 pairs */
    0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, /* 104 pairs */
    0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, /* 112 pairs */
    0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, /* 120 pairs */
    0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, 0x01,0x01, /* 127 pairs */
};
// H3 SETTINGS frame with an unknown Setting ID (e.g., 0x7FFF)
static uint8_t test_h3_settings_unknown_id_b3[] = { 0x04, 0x03, 0x7F, 0xFF, 0x00 }; // Renamed
// H3 SETTINGS with duplicate setting ID
static uint8_t test_h3_settings_duplicate_id[] = { 0x04, 0x04, 0x01, 0x0A, 0x01, 0x0B }; // ID 1=10, ID 1=11
// H3 SETTINGS with a setting ID that requires a specific value range, but value is outside (e.g., hypothetical ID 0x20 requires value < 100, sent 1000)
static uint8_t test_h3_settings_invalid_value_for_id[] = { 0x04, 0x03, 0x20, 0x83, 0xE8 }; // ID 0x20, Value 1000


 /* --- Batch 3: User Prioritized Frames (Part 2 - H3 ORIGIN & QUIC STREAM) --- */
 // H3 ORIGIN frame (type 0x0c) sent when ORIGIN extension was not negotiated
 static uint8_t test_h3_origin_unnegotiated[] = { 0x0c, 0x14, 0x00, 0x12, 'h', 't', 't', 'p', ':', '/', '/', 'o', 'r', 'i', 'g', 'i', 'n', '.', 't', 'e', 's', 't' };
// H3 ORIGIN frame with multiple Origin entries (invalid, only one allowed per RFC draft-ietf-httpbis-origin-frame)
static uint8_t test_h3_origin_multiple_entries[] = { 0x0c, 0x2A, 0x00, 0x12, 'h','t','t','p',':','/','/','o','1','.','t','e','s','t', 0x00, 0x12, 'h','t','t','p',':','/','/','o','2','.','t','e','s','t' };
// H3 ORIGIN frame with empty Origin-Entry (e.g. Length 0 for the ASCII Origin value)
static uint8_t test_h3_origin_empty_entry[] = { 0x0c, 0x02, 0x00, 0x00 }; // Two Origin-Entries, both zero length
 // QUIC STREAM frame with LEN bit set (e.g. 0x0A) but Length field is missing (truncated)
 static uint8_t test_stream_len_bit_no_len_field[] = { 0x0A, 0x01 }; // Stream ID 1
 // QUIC STREAM frame with OFF bit set (e.g. 0x0C) but Offset field is missing (truncated)
 static uint8_t test_stream_off_bit_no_off_field[] = { 0x0C, 0x01 }; // Stream ID 1
 // QUIC STREAM frame with LEN and FIN set (e.g. 0x0B), Length is 0, but data is present after frame.
 static uint8_t test_stream_len_fin_zero_len_with_data[] = { 0x0B, 0x01, 0x00, 'e','x','t','r','a' };
// QUIC STREAM frame, type 0x08 (no OFF, no LEN, no FIN), but packet ends immediately (empty stream data)
static uint8_t test_stream_type08_empty_implicit_len[] = { 0x08, 0x04 }; // Stream ID 4
// QUIC STREAM frame, type 0x0C (OFF, no LEN, no FIN), Offset present, but packet ends (empty stream data)
static uint8_t test_stream_type0C_offset_empty_implicit_len[] = { 0x0C, 0x04, 0x40, 0x10 }; // Stream ID 4, Offset 16


 /* --- Batch 3: User Prioritized Frames (Part 3 - QUIC STREAM type range & WebSocket) --- */
 // Frame type just below STREAM range (e.g. 0x07 NEW_TOKEN) but formatted like a STREAM frame
 static uint8_t test_frame_type_stream_range_just_below[] = {0x07, 0x01, 0x00, 0x04, 'd','a','t','a'}; // ID 1, Offset 0, Len 4
 // Frame type PADDING (0x00) formatted like a STREAM frame (should be invalid)
 static uint8_t test_frame_type_padding_as_stream[] = {0x00, 0x01, 0x00, 0x04, 'd','a','t','a'};
 // Frame type at lower bound of STREAM (0x08)
 static uint8_t test_frame_type_stream_range_lower_bound[] = {0x08, 0x01, 'd','a','t','a'}; // Implicit off 0, implicit len
 // Frame type at upper bound of STREAM (0x0F)
 static uint8_t test_frame_type_stream_range_upper_bound[] = {0x0F, 0x01, 0x00, 0x04, 'd','a','t','a'}; // ID 1, Offset 0, Len 4, FIN
 // Frame type just above STREAM range (e.g. 0x10 MAX_DATA) but formatted like a STREAM frame
 static uint8_t test_frame_type_stream_range_just_above[] = {0x10, 0x01, 0x00, 0x04, 'd','a','t','a'};
 // WebSocket Control Frame (e.g. PING 0x89) with FIN bit = 0 (invalid for control frames)
 static uint8_t test_ws_control_frame_fin_zero_invalid[] = { 0x09, 0x00 }; // PING, FIN=0, len=0
 // WebSocket Text Frame (0x81 FIN=1) but RSV1 bit set (invalid if not negotiated)
 static uint8_t test_ws_text_frame_rsv1_set_invalid[] = { 0xC1, 0x04, 't','e','s','t'}; // FIN=1, RSV1=1, Opcode=Text, len=4
 // WebSocket Text Frame (0x01 FIN=0), then another Text Frame (0x01 FIN=0) instead of Continuation (0x00)
 static uint8_t test_ws_text_fin0_then_text_continuation_part1[] = { 0x01, 0x04, 'p','a','r','t' }; // Text, FIN=0, "part"
 static uint8_t test_ws_text_fin0_then_text_continuation_part2_invalid[] = { 0x01, 0x03, 't','w','o' }; // Text, FIN=0, "two" (invalid sequence)
// WebSocket frame with payload length 126, but length field indicates more data than available
static uint8_t test_ws_len126_data_truncated[] = { 0x81, 0x7E, 0x00, 0xFA, 's', 'h', 'o', 'r', 't' }; // Text, FIN=1, actual len 250, but only "short" provided
// WebSocket frame with payload length 127 (extended 64-bit), but length field indicates more data than available
static uint8_t test_ws_len127_data_truncated[] = { 0x81, 0x7F, 0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00, 's', 'm', 'a', 'l', 'l' }; // Text, FIN=1, actual len 65536

 /* --- Batch 4: More Static Frames (as per plan before user specified Batch 8) --- */
 // QUIC Unknown Frame Type (high value, e.g., 0x7FEE DEAD BEEF - uses max 2-byte varint for type)
 // QUIC Unknown Frame Type (high value, e.g., 0x7FEE DEAD BEEF - uses max 2-byte varint for type)


static uint8_t test_frame_quic_unknown_frame_high_value[] = { 0x7F, 0xEE, 0xDE, 0xAD, 0xBE, 0xEF };
// H3 Reserved Frame Type 0x08 (from RFC9114 Table 2) with empty payload
static uint8_t test_frame_h3_reserved_frame_0x08[] = { 0x08, 0x00 };
// H3 Unassigned Type (e.g., 0x4040 - 2-byte varint) with empty payload
static uint8_t test_frame_h3_unassigned_type_0x4040[] = { 0x40, 0x40, 0x00 };
// WebSocket Reserved Control Opcode (e.g. 0x8B - FIN + Opcode 0xB)
static uint8_t test_frame_ws_reserved_control_0x0B[] = { 0x8B, 0x00 };
// WebSocket Reserved Non-Control Opcode (e.g. 0x83 - FIN + Opcode 0x3)
static uint8_t test_frame_ws_reserved_non_control_0x03[] = { 0x83, 0x00 };
// H3 HEADERS frame (type 0x01) with incomplete QPACK data (e.g. length mismatch or truncated instruction)
static uint8_t test_frame_h3_headers_incomplete_qpack[] = { 0x01, 0x05, 0x00, 0x00 }; // Len 5, but only 2 bytes of QPACK placeholder
// WebSocket PING frame (0x89) with payload length > 125 (invalid for PING)
static uint8_t test_frame_ws_ping_payload_gt_125[] = { 0x89, 0x7E, 0x00, 0x7E, 0xFF }; // PING, FIN=1, len=126, one byte of payload
// QUIC MAX_STREAMS Uni (0x13), value 0
static uint8_t test_frame_quic_max_streams_uni_value0[] = { 0x13, 0x00 };
// QUIC NEW_CONNECTION_ID (0x18) with a very short token (e.g., 8 bytes instead of 16)
static uint8_t test_frame_quic_ncid_short_token[] = { 0x18, 0x01, 0x00, 0x08, 0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8, 0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8 }; // Token is only 8 bytes
// QUIC NEW_CONNECTION_ID (0x18) with CID length 0 (invalid)
static uint8_t test_frame_quic_ncid_zero_len_cid[] = { 0x18, 0x02, 0x00, 0x00, 0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,0xD0 };
// QUIC PATH_CHALLENGE (0x1a) with data all zeros
static uint8_t test_frame_quic_path_challenge_all_zero_data_b4[] = { 0x1a, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 }; // Renamed
// QUIC PATH_RESPONSE (0x1b) with data mismatching a typical challenge
static uint8_t test_frame_quic_path_response_mismatch_data_b4[] = { 0x1b, 0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88 }; // Renamed

 /* --- Batch 5: Further Static Frames (as per plan before user specified Batch 8) --- */
// QUIC Unknown Frame Type (greased 0x2A type from RFC9000 Sec 15) with some payload
static uint8_t test_frame_quic_unknown_frame_grease_0x2A[] = { 0x2A, 0x01, 0x02, 0x03, 0x04 };
// H3 Reserved Frame Type 0x09 (from RFC9114 Table 2) with empty payload
static uint8_t test_frame_h3_reserved_frame_0x09[] = { 0x09, 0x00 };
// WebSocket Control Frame Opcode 0x0C (invalid) with FIN=1
static uint8_t test_frame_ws_control_frame_0x0C_invalid[] = { 0x8C, 0x00 };
// QUIC CRYPTO frame (0x06) with length > actual crypto data in packet
static uint8_t test_frame_quic_crypto_len_gt_data_b5[] = { 0x06, 0x00, 0x64, 0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A }; // Renamed, Len 100, Data 10 bytes
// H3 PUSH_PROMISE frame (type 0x05) with incomplete payload (e.g. missing Push ID or header block)
static uint8_t test_frame_h3_push_promise_incomplete_payload[] = { 0x05, 0x0A, 0x01, 0x00, 0x00 }; // Len 10, Push ID 1, then truncated QPACK
// QUIC RETIRE_CONNECTION_ID (0x19) with a very large sequence number (2^62-1)
static uint8_t test_frame_quic_retire_connection_id_large_seq[] = { 0x19, 0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF };
// H3 GOAWAY frame (type 0x07) with a very large Stream/Request ID (2^62-1)
static uint8_t test_frame_h3_goaway_large_id[] = { 0x07, 0x08, 0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF }; // Len 8 for ID
// QUIC NEW_CONNECTION_ID (0x18) with RetirePriorTo > SequenceNumber (duplicate of _b1 for systematic inclusion)
static uint8_t test_frame_quic_ncid_retire_gt_seq_b5[] = { 0x18, 0x02, 0x05, 0x08, 0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8, 0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,0xF0 }; // Renamed
// QUIC PATH_CHALLENGE (0x1a) frame truncated (empty data)
static uint8_t test_frame_quic_path_challenge_empty[] = { 0x1a };
// QUIC PATH_RESPONSE (0x1b) frame truncated (empty data)
static uint8_t test_frame_quic_path_response_empty[] = { 0x1b };
// QUIC ACK frame (0x02) with ACK Delay encoded using max varint (8 bytes)
static uint8_t test_frame_quic_ack_delay_max_varint[] = { 0x02, 0x0A, 0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x01, 0x01, 0x00 }; // LargestAck 10, Delay 1 (8-byte), RangeCount 1, FirstRange 0
// QUIC STREAM frame (0x0F, all bits set) with StreamID, Offset, Length all max varint values
static uint8_t test_frame_quic_stream_all_fields_max_varint[] = {
    0x0F,                                                               // Type
    0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,                            // Stream ID (2^62-1)
    0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,                            // Offset (2^62-1)
    0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFD,                            // Length (2^62-3 to avoid total overflow with data)
    'd', 'a', 't', 'a'                                                  // Data (4 bytes)
};
// H3 DATA frame (type 0x00) with length 0, but payload present
static uint8_t test_frame_h3_data_len0_with_payload[] = { 0x00, 0x00, 0x01, 0x02, 0x03 };
// WebSocket Close frame (0x88) with invalid close code (e.g. 0)
static uint8_t test_frame_ws_close_invalid_code[] = { 0x88, 0x02, 0x00, 0x00 }; // FIN=1, Opcode=Close, Len=2, Code=0 (invalid)

 /* --- Batch 8: Combined Set (original Batch 6/7 + 4 new from user) --- */
 // H2 WINDOW_UPDATE frame (type 0x08) with 0 increment. Len 4, StreamID 0, Inc 0.
// H2 WINDOW_UPDATE frame (type 0x08) with 0 increment. Len 4, StreamID 0, Inc 0.
static uint8_t test_frame_h2_window_update_increment0_b7[] = { 0x00, 0x00, 0x04, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // This is not a QUIC frame, but H2 frame payload. Assuming it's wrapped in QUIC STREAM.
// QUIC CONNECTION_CLOSE (type 0x1c) with an application error code (e.g., 0x0101 from app space)
static uint8_t test_frame_quic_conn_close_transport_app_err_code_b7[] = { 0x1c, 0x41, 0x01, 0x08, 0x00 }; // Error 0x101 (like app), Frame Type STREAM(0x08), ReasonLen 0
// H3 MAX_PUSH_ID (type 0x0D) with value 0
static uint8_t test_frame_h3_max_push_id_value0_b7[] = { 0x0D, 0x01, 0x00 }; // Len 1, ID 0
// QUIC NEW_CONNECTION_ID (0x18) with CID length > 20 (e.g. 21)
static uint8_t test_frame_quic_ncid_cid_len_gt_pico_max_b7[] = { 0x18, 0x04, 0x00, 21, 0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC, 0xBB,0xBB,0xBB,0xBB,0xBB,0xBB,0xBB,0xBB,0xBB,0xBB,0xBB,0xBB,0xBB,0xBB,0xBB,0xBB };
// WebSocket Text Frame with RSV2 bit set (invalid if not negotiated)
static uint8_t test_frame_ws_text_rsv2_set[] = { 0xA1, 0x04, 't','e','s','t' }; // FIN=1, RSV2=1, Opcode=Text
// WebSocket Text Frame with RSV3 bit set (invalid if not negotiated)
static uint8_t test_frame_ws_text_rsv3_set[] = { 0x91, 0x04, 't','e','s','t' }; // FIN=1, RSV3=1, Opcode=Text
// QUIC ACK frame type 0x02 (no ECN) but with ECN count fields present (malformed)
static uint8_t test_frame_quic_ack_non_ecn_with_ecn_counts_b7[] = { 0x02, 0x0A, 0x00, 0x01, 0x00, 0x01, 0x01, 0x01 }; // LargestAck 10, Delay 0, RangeCount 1, FirstRange 0, then ECT0=1, ECT1=1, CE=1
// QUIC Greased Frame Type (e.g. 0x5BEE from RFC9000 Sec 15 pattern 0x?a?a?a?a + 0x21, here 0x5BEE = 42*N+27 for N=1450, not exactly greased but using a high value for test)
static uint8_t test_frame_quic_greased_type_0x5BEE_with_payload[] = { 0x5B, 0xEE, 0x01, 0x02, 0x03, 0x04 }; // Type 0x5BEE, then payload
// H3 Reserved Frame Type encoded with 4-byte varint (e.g. 0x80000020)
static uint8_t test_frame_h3_reserved_type_4byte_varint[] = { 0x80, 0x00, 0x00, 0x20, 0x00 }; // Type 0x20 (hypothetical reserved), Len 0
// WebSocket Continuation Frame (opcode 0x00) with FIN=1 (valid, but can be fuzzed for interaction)
static uint8_t test_frame_ws_continuation_fin1_with_payload[] = { 0x80, 0x04, 'c','o','n','t' };
// QUIC NEW_CONNECTION_ID with a very large RetirePriorTo value but small SequenceNumber
static uint8_t test_frame_quic_ncid_large_retire_small_seq[] = { 0x18, 0x05, 0xBF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, 0x08, 0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8, 0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F,0xA0 };
// QUIC frame type 0x21 (RFC 9000 reserved for extensions, potentially unhandled)
static uint8_t test_frame_quic_extension_0x21[] = { 0x21 };
// H3 frame type 0x2F (RFC 9114 reserved for extensions) with empty payload
static uint8_t test_frame_h3_extension_0x2F[] = { 0x2F, 0x00 };
// QUIC ACK frame with first ACK range having length 0, and then a Gap of 0, then another range of 0. (Ack Pkt N, N-1)
static uint8_t test_frame_quic_ack_double_zero_range[] = { 0x02, 0x0A, 0x00, 0x02, 0x00, 0x00, 0x00 };
// WebSocket frame with RSV1, RSV2, and RSV3 all set (highly unlikely to be negotiated)
static uint8_t test_frame_ws_all_rsv_set[] = { 0xF1, 0x04, 'd', 'a', 't', 'a' }; // FIN=1, RSV1-3=1, Opcode=Text

 /* END OF JULES ADDED FRAMES */

/* Additional QUIC Negative Test Cases for Comprehensive Fuzzing */

// CRYPTO frame with offset exceeding limits  
static uint8_t test_frame_quic_crypto_offset_max[] = { 0x06, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x04, 't', 'e', 's', 't' };

// STREAM frame with invalid final size (smaller than offset)
static uint8_t test_frame_quic_stream_invalid_final_size[] = { 0x0f, 0x01, 0x0A, 0x05, 't', 'e', 's', 't' };

// ACK frame with packet number overflow
static uint8_t test_frame_quic_ack_pkt_overflow[] = { 0x02, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x01, 0x00 };

// HANDSHAKE_DONE in wrong context
static uint8_t test_frame_quic_handshake_done_invalid[] = { 0x1e };

// Multiple HANDSHAKE_DONE frames
static uint8_t test_frame_quic_multiple_handshake_done[] = { 0x1e, 0x1e };

// STREAM frame with maximum stream ID
static uint8_t test_frame_quic_stream_id_maximum[] = { 0x08, 0xBF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 'd', 'a', 't', 'a' };

// PING frame in wrong packet type context
static uint8_t test_frame_quic_ping_invalid_context[] = { 0x01 };

// CONNECTION_CLOSE with maximum error code
static uint8_t test_frame_quic_conn_close_max_err[] = { 0x1c, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00 };

// RESET_STREAM after FIN violation
static uint8_t test_frame_quic_reset_after_fin_violation[] = { 0x04, 0x01, 0x00, 0x04 };

// Unknown frame types for stress testing
static uint8_t test_frame_quic_unknown_type_0x40[] = { 0x40, 0x00 };
static uint8_t test_frame_quic_unknown_type_0x41[] = { 0x41, 0x01, 0x02 };
static uint8_t test_frame_quic_unknown_type_0x42[] = { 0x42, 0x03, 0x04, 0x05 };

// PATH_RESPONSE with incorrect data
static uint8_t test_frame_quic_path_response_incorrect[] = { 0x1b, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77 };

// STREAM frame exceeding flow control
static uint8_t test_frame_quic_stream_flow_violation[] = { 0x0a, 0x01, 0xFF, 0xFF, 'd','a','t','a' };

// CRYPTO frame in 0-RTT packet (invalid)
static uint8_t test_frame_quic_crypto_0rtt_invalid[] = { 0x06, 0x00, 0x04, 't', 'e', 's', 't' };

// ACK frame in 0-RTT packet (invalid)
static uint8_t test_frame_quic_ack_0rtt_invalid[] = { 0x02, 0x01, 0x00, 0x01, 0x00 };

// NEW_CONNECTION_ID flooding (rapid sequence numbers)
static uint8_t test_frame_quic_ncid_flood_seq1[] = { 0x18, 0x01, 0x00, 0x08, 0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01, 0xB1,0xB1,0xB1,0xB1,0xB1,0xB1,0xB1,0xB1,0xB1,0xB1,0xB1,0xB1,0xB1,0xB1,0xB1,0xB1 };
static uint8_t test_frame_quic_ncid_flood_seq2[] = { 0x18, 0x02, 0x00, 0x08, 0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02, 0xB2,0xB2,0xB2,0xB2,0xB2,0xB2,0xB2,0xB2,0xB2,0xB2,0xB2,0xB2,0xB2,0xB2,0xB2,0xB2 };

// MAX_STREAMS rapid changes for DoS testing
static uint8_t test_frame_quic_max_streams_rapid1[] = { 0x12, 0x64 }; // 100 streams
static uint8_t test_frame_quic_max_streams_rapid2[] = { 0x12, 0x32 }; // 50 streams

// STREAM frame with zero-length data but LEN bit set
static uint8_t test_frame_quic_stream_zero_len_explicit[] = { 0x0a, 0x01, 0x00 };

// PATH_CHALLENGE replay attack simulation
static uint8_t test_frame_quic_path_challenge_replay[] = { 0x1a, 0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE };

// MAX_STREAM_DATA with decreasing value
static uint8_t test_frame_quic_max_stream_data_decrease[] = { 0x11, 0x01, 0x32 };

// STREAMS_BLOCKED with invalid higher limit
static uint8_t test_frame_quic_streams_blocked_invalid_limit[] = { 0x16, 0xC8 };

// Frame with invalid varint encoding
static uint8_t test_frame_quic_invalid_varint[] = { 0xFF, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 };

// ACK frame with malformed ack range
static uint8_t test_frame_quic_ack_malformed[] = { 0x02, 0x0A, 0x00, 0x01, 0xFF, 0xFF, 0xFF, 0xFF };

// Multiple PATH_CHALLENGE frames in single packet
static uint8_t test_frame_quic_multiple_path_challenge[] = { 
    0x1a, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x1a, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18
};

// STREAM fragments for memory pressure testing
static uint8_t test_frame_quic_stream_fragment1[] = { 0x0a, 0x01, 0x01, 'A' };
static uint8_t test_frame_quic_stream_fragment2[] = { 0x0c, 0x01, 0x01, 0x01, 'B' };

// ACK frame with many ranges for memory pressure
static uint8_t test_frame_quic_ack_many_ranges[] = { 
    0x02, 0x64, 0x00, 0x05, // Largest 100, delay 0, 5 ranges
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01
};

// NEW_CONNECTION_ID with duplicate sequence number
static uint8_t test_frame_quic_ncid_duplicate[] = { 0x18, 0x01, 0x00, 0x08, 0xDD,0xDD,0xDD,0xDD,0xDD,0xDD,0xDD,0xDD, 0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE,0xEE };

// Connection ID at maximum length (20 bytes)
static uint8_t test_frame_quic_ncid_max_len[] = { 
    0x18, 0x05, 0x00, 20, 
    0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF,0x00,0x01,0x02,0x03,0x04,
    0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,0xB0
};

// Reset flood simulation
static uint8_t test_frame_quic_reset_flood1[] = { 0x04, 0x05, 0x00, 0x00 };
static uint8_t test_frame_quic_reset_flood2[] = { 0x04, 0x09, 0x00, 0x00 };

// Buffer overflow attempts
static uint8_t test_frame_quic_crypto_overflow[] = { 0x06, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x01 };
static uint8_t test_frame_quic_new_token_overflow[] = { 0x07, 0xFF, 0xFF, 0x01 };

// ACK with suspicious timing patterns
static uint8_t test_frame_quic_ack_timing_suspicious[] = { 0x02, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00 };

// Additional missing test cases referenced in FUZI_Q_ITEM array
static uint8_t test_quic_conn_close_missing_error[] = { 0x1c }; // CONNECTION_CLOSE with missing error code
static uint8_t test_quic_ack_bad_range[] = { 0x02, 0x05, 0x00, 0x01, 0x10 }; // ACK with invalid range
static uint8_t test_quic_reset_zero_error[] = { 0x04, 0x01, 0x00, 0x00 }; // RESET_STREAM with error 0
static uint8_t test_quic_crypto_big_offset[] = { 0x06, 0xBF, 0xFF, 0xFF, 0xFF, 0x04, 't', 'e', 's', 't' }; // CRYPTO with large offset
static uint8_t test_quic_new_token_empty[] = { 0x07, 0x00 }; // NEW_TOKEN with zero length
static uint8_t test_quic_stream_id_zero[] = { 0x08, 0x00, 'z', 'e', 'r', 'o' }; // STREAM with ID 0
static uint8_t test_quic_max_data_zero[] = { 0x10, 0x00 }; // MAX_DATA with value 0
static uint8_t test_quic_max_streams_huge[] = { 0x12, 0xFF, 0xFF, 0xFF, 0xFF }; // MAX_STREAMS with huge value
static uint8_t test_quic_ncid_bad_seq[] = { 0x18, 0xFF, 0x00, 0x08, 0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA,0xAA, 0xBB,0xBB,0xBB,0xBB,0xBB,0xBB,0xBB,0xBB,0xBB,0xBB,0xBB,0xBB,0xBB,0xBB,0xBB,0xBB }; // NEW_CONNECTION_ID with bad sequence
static uint8_t test_quic_retire_seq_zero[] = { 0x19, 0x00 }; // RETIRE_CONNECTION_ID with sequence 0
static uint8_t test_quic_path_challenge_predictable[] = { 0x1a, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 }; // PATH_CHALLENGE with predictable data
static uint8_t test_quic_reserved_frame_type[] = { 0x1f }; // Reserved frame type
static uint8_t test_quic_stream_len_mismatch[] = { 0x0a, 0x01, 0x05, 'x', 'y' }; // STREAM with length mismatch
static uint8_t test_quic_ack_future[] = { 0x02, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x01, 0x00 }; // ACK for future packet
static uint8_t test_quic_datagram_bad_len[] = { 0x30, 0x10, 'x' }; // DATAGRAM with bad length
static uint8_t test_quic_stream_noncanon_varint[] = { 0x08, 0x40, 0x01, 't', 'e', 's', 't' }; // STREAM with non-canonical varint
static uint8_t test_quic_conn_close_bad_frame_ref[] = { 0x1c, 0x01, 0xFF, 0x00 }; // CONNECTION_CLOSE with bad frame reference

 /* RFC 9204 (QPACK Instructions) Placeholders */
/* Encoder Instructions */
static uint8_t test_qpack_enc_set_dynamic_table_capacity[] = {0x20}; /* Placeholder: Set Dynamic Table Capacity (e.g., 001xxxxx) */
static uint8_t test_qpack_enc_insert_with_name_ref[] = {0x80}; /* Placeholder: Insert With Name Reference (Indexed Name, e.g., 1xxxxxxx) */
static uint8_t test_qpack_enc_insert_without_name_ref[] = {0x40}; /* Placeholder: Insert Without Name Reference (e.g., 0100xxxx) */
static uint8_t test_qpack_enc_duplicate[] = {0x00}; /* Placeholder: Duplicate (e.g., 000xxxxx) */

/* Decoder Instructions (as per prompt, acknowledging potential mismatch with RFC for "Set Dynamic Table Capacity") */
static uint8_t test_qpack_dec_header_block_ack[] = {0x80}; /* Placeholder: Section Acknowledgment (Decoder, e.g., 1xxxxxxx) */
static uint8_t test_qpack_dec_stream_cancellation[] = {0x40}; /* Placeholder: Stream Cancellation (Decoder, e.g., 01xxxxxx) */
static uint8_t test_qpack_dec_insert_count_increment[] = {0x01}; /* Placeholder: Insert Count Increment (Decoder, e.g., 00xxxxxx) */
static uint8_t test_qpack_dec_set_dynamic_table_capacity[] = {0x20}; /* Placeholder: Set Dynamic Table Capacity (Encoder Instruction pattern 001xxxxx) */

// HTTP/2 Frame Types (RFC 9113) - Placeholders
static uint8_t test_h2_frame_type_data[] = {0x00}; /* H2 DATA frame type */
static uint8_t test_h2_frame_type_headers[] = {0x01}; /* H2 HEADERS frame type */
static uint8_t test_h2_frame_type_priority[] = {0x02}; /* H2 PRIORITY frame type */
static uint8_t test_h2_frame_type_rst_stream[] = {0x03}; /* H2 RST_STREAM frame type */
static uint8_t test_h2_frame_type_settings[] = {0x04}; /* H2 SETTINGS frame type */
static uint8_t test_h2_frame_type_push_promise[] = {0x05}; /* H2 PUSH_PROMISE frame type */
static uint8_t test_h2_frame_type_ping[] = {0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; /* H2 PING frame type (type + 8 bytes opaque data) */
static uint8_t test_h2_frame_type_goaway[] = {0x07}; /* H2 GOAWAY frame type */
static uint8_t test_h2_frame_type_window_update[] = {0x08}; /* H2 WINDOW_UPDATE frame type */
static uint8_t test_h2_frame_type_continuation[] = {0x09}; /* H2 CONTINUATION frame type */

// HPACK Representations/Instructions (RFC 7541) - Placeholders
static uint8_t test_hpack_indexed_header_field[] = {0x80}; /* HPACK Indexed Header Field (pattern 1xxxxxxx) */
static uint8_t test_hpack_literal_inc_indexing[] = {0x40}; /* HPACK Literal Header Field with Incremental Indexing (pattern 01xxxxxx) */
static uint8_t test_hpack_literal_no_indexing[] = {0x00}; /* HPACK Literal Header Field without Indexing (pattern 0000xxxx) */
static uint8_t test_hpack_literal_never_indexed[] = {0x10}; /* HPACK Literal Header Field Never Indexed (pattern 0001xxxx) */
static uint8_t test_hpack_dynamic_table_size_update[] = {0x20}; /* HPACK Dynamic Table Size Update (pattern 001xxxxx) */

// HTTP Alternative Services (RFC 7838) - H2 Frame Type Placeholder
static uint8_t test_h2_frame_type_altsvc[] = {0x0a}; /* H2 ALTSVC frame type (0xa) */

// WebSocket Frame Types (RFC 6455) - Placeholders (minimal frames, FIN=1, Mask=0, PayloadLen=0)
static uint8_t test_ws_frame_continuation[] = {0x80, 0x00}; /* WebSocket Continuation Frame (FIN + Opcode 0x0) */
static uint8_t test_ws_frame_text[] = {0x81, 0x00};         /* WebSocket Text Frame (FIN + Opcode 0x1) */
static uint8_t test_ws_frame_binary[] = {0x82, 0x00};        /* WebSocket Binary Frame (FIN + Opcode 0x2) */
static uint8_t test_ws_frame_connection_close[] = {0x88, 0x00}; /* WebSocket Connection Close Frame (FIN + Opcode 0x8) */
static uint8_t test_ws_frame_ping[] = {0x89, 0x00};          /* WebSocket Ping Frame (FIN + Opcode 0x9) */
static uint8_t test_ws_frame_pong[] = {0x8a, 0x00};          /* WebSocket Pong Frame (FIN + Opcode 0xA) */

fuzi_q_frames_t fuzi_q_frame_list[] = {
    FUZI_Q_ITEM("padding", test_frame_type_padding),
    FUZI_Q_ITEM("padding_zero_byte", test_frame_type_padding_zero_byte),
    FUZI_Q_ITEM("padding_large", test_frame_type_padding_large),
    FUZI_Q_ITEM("padding_2_bytes", test_frame_padding_2_bytes),
    FUZI_Q_ITEM("padding_5_bytes", test_frame_type_padding_5_bytes),
    FUZI_Q_ITEM("padding_7_bytes", test_frame_type_padding_7_bytes),
    FUZI_Q_ITEM("padding_10_bytes", test_frame_padding_10_bytes),
    FUZI_Q_ITEM("padding_13_bytes", test_frame_type_padding_13_bytes),
    FUZI_Q_ITEM("padding_50_bytes", test_frame_padding_50_bytes),
    FUZI_Q_ITEM("reset_stream", test_frame_type_reset_stream),
    FUZI_Q_ITEM("reset_stream_high_error", test_frame_type_reset_stream_high_error),
    FUZI_Q_ITEM("reset_stream_min_vals", test_frame_reset_stream_min_vals),
    FUZI_Q_ITEM("reset_stream_max_final_size", test_frame_reset_stream_max_final_size),
    FUZI_Q_ITEM("reset_stream_app_error_specific", test_frame_reset_stream_app_error_specific), /* This is {0x04, 0x01, 0x00, 0x01} */
    FUZI_Q_ITEM("reset_stream_sid_zero", test_frame_reset_stream_sid_zero), /* New */
    FUZI_Q_ITEM("reset_stream_final_size_zero_explicit", test_frame_reset_stream_final_size_zero_explicit), /* New (StreamID=1, Err=0, FinalSize=0) */
    FUZI_Q_ITEM("reset_stream_all_large", test_frame_reset_stream_all_large), /* New */
    FUZI_Q_ITEM("reset_stream_error_code_max", test_frame_reset_stream_error_code_max),
    FUZI_Q_ITEM("reset_stream_final_size_max_new", test_frame_reset_stream_final_size_max_new),
    FUZI_Q_ITEM("connection_close", test_type_connection_close),
    FUZI_Q_ITEM("connection_close_transport_long_reason", test_frame_connection_close_transport_long_reason),
    FUZI_Q_ITEM("application_close", test_type_application_close),
    FUZI_Q_ITEM("application_close", test_type_application_close_reason),
    FUZI_Q_ITEM("application_close_long_reason", test_frame_application_close_long_reason),
    FUZI_Q_ITEM("conn_close_no_reason", test_frame_conn_close_no_reason),
    FUZI_Q_ITEM("conn_close_app_no_reason", test_frame_conn_close_app_no_reason),
    FUZI_Q_ITEM("conn_close_specific_transport_error", test_frame_conn_close_specific_transport_error),
    FUZI_Q_ITEM("max_data", test_frame_type_max_data),
    FUZI_Q_ITEM("max_data_large", test_frame_type_max_data_large),
    FUZI_Q_ITEM("max_data_zero", test_frame_max_data_zero),
    FUZI_Q_ITEM("max_data_small_value", test_frame_max_data_small_value),
    FUZI_Q_ITEM("max_data_val_large", test_frame_max_data_val_large), /* New */
    FUZI_Q_ITEM("max_stream_data", test_frame_type_max_stream_data),
    FUZI_Q_ITEM("max_stream_data_zero", test_frame_max_stream_data_zero),
    FUZI_Q_ITEM("max_streams_bidir", test_frame_type_max_streams_bidir),
    FUZI_Q_ITEM("max_streams_unidir", test_frame_type_max_streams_unidir),
    FUZI_Q_ITEM("max_streams_bidir_alt", test_frame_type_max_streams_bidir_alt),
    FUZI_Q_ITEM("max_streams_bidir_zero", test_frame_type_max_streams_bidir_zero),
    FUZI_Q_ITEM("max_streams_bidi_very_high", test_frame_max_streams_bidi_very_high),
    FUZI_Q_ITEM("max_streams_unidir_zero", test_frame_type_max_streams_unidir_zero),
    FUZI_Q_ITEM("max_streams_uni_very_high", test_frame_max_streams_uni_very_high),
    FUZI_Q_ITEM("ping", test_frame_type_ping),
    FUZI_Q_ITEM("blocked", test_frame_type_blocked),
    FUZI_Q_ITEM("data_blocked_large_offset", test_frame_type_data_blocked_large_offset),
    FUZI_Q_ITEM("data_blocked_zero", test_frame_data_blocked_zero),
    FUZI_Q_ITEM("stream_data_blocked", test_frame_type_stream_blocked),
    FUZI_Q_ITEM("stream_data_blocked_large_limits", test_frame_type_stream_data_blocked_large_limits),
    FUZI_Q_ITEM("stream_data_blocked_zero", test_frame_stream_data_blocked_zero),
    FUZI_Q_ITEM("streams_blocked_bidir", test_frame_type_streams_blocked_bidir),
    FUZI_Q_ITEM("streams_blocked_bidi_zero", test_frame_streams_blocked_bidi_zero),
    FUZI_Q_ITEM("streams_blocked_unidir", test_frame_type_streams_blocked_unidir),
    FUZI_Q_ITEM("streams_blocked_uni_zero", test_frame_streams_blocked_uni_zero),
    FUZI_Q_ITEM("new_connection_id", test_frame_type_new_connection_id),
    FUZI_Q_ITEM("new_connection_id_alt", test_frame_type_new_connection_id_alt),
    FUZI_Q_ITEM("new_cid_retire_high", test_frame_new_cid_retire_high),
    FUZI_Q_ITEM("new_cid_short_id", test_frame_new_cid_short_id),
    FUZI_Q_ITEM("new_cid_long_id", test_frame_new_cid_long_id),
    FUZI_Q_ITEM("stop_sending", test_frame_type_stop_sending),
    FUZI_Q_ITEM("stop_sending_high_error", test_frame_type_stop_sending_high_error),
    FUZI_Q_ITEM("stop_sending_min_vals", test_frame_stop_sending_min_vals),
    FUZI_Q_ITEM("stop_sending_app_error_specific", test_frame_stop_sending_app_error_specific),
    FUZI_Q_ITEM("stop_sending_sid_err_zero", test_frame_stop_sending_sid_err_zero), /* New */
    FUZI_Q_ITEM("stop_sending_all_large", test_frame_stop_sending_all_large), /* New */
    FUZI_Q_ITEM("challenge", test_frame_type_path_challenge),
    FUZI_Q_ITEM("path_challenge_alt_data", test_frame_type_path_challenge_alt_data),
    FUZI_Q_ITEM("response", test_frame_type_path_response),
    FUZI_Q_ITEM("path_response_alt_data", test_frame_type_path_response_alt_data),
    FUZI_Q_ITEM("path_challenge_all_zeros", test_frame_path_challenge_all_zeros),
    FUZI_Q_ITEM("path_response_all_zeros", test_frame_path_response_all_zeros),
    FUZI_Q_ITEM("path_challenge_mixed_pattern", test_frame_path_challenge_mixed_pattern),
    FUZI_Q_ITEM("path_response_mixed_pattern", test_frame_path_response_mixed_pattern),
    FUZI_Q_ITEM("new_token", test_frame_type_new_token),
    FUZI_Q_ITEM("new_token_long", test_frame_new_token_long),
    FUZI_Q_ITEM("new_token_short", test_frame_new_token_short),
    FUZI_Q_ITEM("ack", test_frame_type_ack),
    FUZI_Q_ITEM("ack_empty", test_frame_ack_empty),
    FUZI_Q_ITEM("ack_multiple_ranges", test_frame_ack_multiple_ranges),
    FUZI_Q_ITEM("ack_large_delay", test_frame_ack_large_delay),
    FUZI_Q_ITEM("ack_ecn", test_frame_type_ack_ecn),
    FUZI_Q_ITEM("ack_ecn_counts_high", test_frame_ack_ecn_counts_high),
    FUZI_Q_ITEM("stream_min", test_frame_type_stream_range_min),
    FUZI_Q_ITEM("stream_max", test_frame_type_stream_range_max),
    FUZI_Q_ITEM("stream_no_offset_no_len_fin", test_frame_stream_no_offset_no_len_fin),
    FUZI_Q_ITEM("stream_offset_no_len_no_fin", test_frame_stream_offset_no_len_no_fin),
    FUZI_Q_ITEM("stream_no_offset_len_no_fin", test_frame_stream_no_offset_len_no_fin),
    FUZI_Q_ITEM("stream_all_bits_set", test_frame_stream_all_bits_set),
    FUZI_Q_ITEM("stream_zero_len_data", test_frame_stream_zero_len_data),
    FUZI_Q_ITEM("stream_max_offset_final", test_frame_stream_max_offset_final),
    FUZI_Q_ITEM("crypto_hs", test_frame_type_crypto_hs),
    FUZI_Q_ITEM("crypto_hs_alt", test_frame_type_crypto_hs_alt),
    FUZI_Q_ITEM("crypto_zero_len", test_frame_crypto_zero_len),
    FUZI_Q_ITEM("crypto_large_offset", test_frame_crypto_large_offset),
    FUZI_Q_ITEM("crypto_fragment1", test_frame_crypto_fragment1),
    FUZI_Q_ITEM("crypto_fragment2", test_frame_crypto_fragment2),
    FUZI_Q_ITEM("retire_connection_id", test_frame_type_retire_connection_id),
    FUZI_Q_ITEM("retire_cid_seq_zero", test_frame_retire_cid_seq_zero),
    FUZI_Q_ITEM("retire_cid_seq_high", test_frame_retire_cid_seq_high),
    FUZI_Q_ITEM("datagram", test_frame_type_datagram),
    FUZI_Q_ITEM("datagram_l", test_frame_type_datagram_l),
    FUZI_Q_ITEM("handshake_done", test_frame_type_handshake_done),
    FUZI_Q_ITEM("ack_frequency", test_frame_type_ack_frequency),
    FUZI_Q_ITEM("time_stamp", test_frame_type_time_stamp),
    FUZI_Q_ITEM("path_abandon_0", test_frame_type_path_abandon_0),
    FUZI_Q_ITEM("path_abandon_1", test_frame_type_path_abandon_1),
    FUZI_Q_ITEM("path_backup", test_frame_type_path_backup),
    FUZI_Q_ITEM("path_available", test_frame_type_path_available),
    FUZI_Q_ITEM("path_backup", test_frame_type_path_backup),
    FUZI_Q_ITEM("path_blocked", test_frame_type_path_blocked),
    FUZI_Q_ITEM("bdp", test_frame_type_bdp),
    FUZI_Q_ITEM("bad_reset_stream_offset", test_frame_type_bad_reset_stream_offset),
    FUZI_Q_ITEM("bad_reset_stream", test_frame_type_bad_reset_stream),
    FUZI_Q_ITEM("bad_connection_close", test_type_bad_connection_close),
    FUZI_Q_ITEM("bad_application_close", test_type_bad_application_close),
    FUZI_Q_ITEM("bad_max_stream_stream", test_frame_type_bad_max_stream_stream),
    FUZI_Q_ITEM("bad_max_streams_bidir", test_frame_type_max_bad_streams_bidir),
    FUZI_Q_ITEM("bad_max_streams_unidir", test_frame_type_bad_max_streams_unidir),
    FUZI_Q_ITEM("bad_new_connection_id_length", test_frame_type_bad_new_cid_length),
    FUZI_Q_ITEM("bad_new_connection_id_retire", test_frame_type_bad_new_cid_retire),
    FUZI_Q_ITEM("bad_stop_sending", test_frame_type_bad_stop_sending),
    FUZI_Q_ITEM("bad_new_token", test_frame_type_bad_new_token),
    FUZI_Q_ITEM("bad_ack_range", test_frame_type_bad_ack_range),
    FUZI_Q_ITEM("bad_ack_gaps", test_frame_type_bad_ack_gaps),
    FUZI_Q_ITEM("bad_ack_blocks", test_frame_type_bad_ack_blocks),
    FUZI_Q_ITEM("bad_crypto_hs", test_frame_type_bad_crypto_hs),
    FUZI_Q_ITEM("bad_datagram", test_frame_type_bad_datagram),
    FUZI_Q_ITEM("stream_hang", test_frame_stream_hang),
    FUZI_Q_ITEM("bad_abandon_0", test_frame_type_path_abandon_bad_0),
    FUZI_Q_ITEM("bad_abandon_1", test_frame_type_path_abandon_bad_1),
    FUZI_Q_ITEM("bad_abandon_2", test_frame_type_path_abandon_bad_2),
    FUZI_Q_ITEM("bad_bdp", test_frame_type_bdp_bad),
    FUZI_Q_ITEM("bad_bdp", test_frame_type_bdp_bad_addr),
    FUZI_Q_ITEM("bad_bdp", test_frame_type_bdp_bad_length),
    /* New ACK frame test items */
    FUZI_Q_ITEM("ack_excessive_ack_delay", test_frame_ack_excessive_ack_delay),
    FUZI_Q_ITEM("ack_first_range_too_large", test_frame_ack_first_range_too_large),
    FUZI_Q_ITEM("ack_too_many_ranges", test_frame_ack_too_many_ranges),
    FUZI_Q_ITEM("ack_ecn_ect0_too_large", test_frame_ack_ecn_ect0_too_large),
    /* New STREAM frame test items */
    FUZI_Q_ITEM("stream_len_beyond_packet", test_frame_stream_len_beyond_packet),
    FUZI_Q_ITEM("stream_zero_len_with_data", test_frame_stream_zero_len_with_data),
    FUZI_Q_ITEM("stream_len_shorter_than_data", test_frame_stream_len_shorter_than_data),
    FUZI_Q_ITEM("stream_len_longer_than_data", test_frame_stream_len_longer_than_data),
    FUZI_Q_ITEM("stream_max_offset_max_len", test_frame_stream_max_offset_max_len),
    /* New MAX_DATA, MAX_STREAM_DATA, MAX_STREAMS frame test items */
    FUZI_Q_ITEM("max_data_extremely_large", test_frame_max_data_extremely_large),
    FUZI_Q_ITEM("max_stream_data_extremely_large", test_frame_max_stream_data_extremely_large),
    FUZI_Q_ITEM("max_streams_bidir_extremely_large", test_frame_max_streams_bidir_extremely_large),
    FUZI_Q_ITEM("max_streams_unidir_extremely_large", test_frame_max_streams_unidir_extremely_large),
    /* New CONNECTION_CLOSE and APPLICATION_CLOSE frame test items */
    FUZI_Q_ITEM("connection_close_reason_len_too_large", test_frame_connection_close_reason_len_too_large),
    FUZI_Q_ITEM("application_close_reason_len_too_large", test_frame_application_close_reason_len_too_large),
    FUZI_Q_ITEM("connection_close_reason_len_shorter", test_frame_connection_close_reason_len_shorter),
    FUZI_Q_ITEM("application_close_reason_len_shorter", test_frame_application_close_reason_len_shorter),
    FUZI_Q_ITEM("connection_close_reason_len_longer", test_frame_connection_close_reason_len_longer),
    FUZI_Q_ITEM("application_close_reason_len_longer", test_frame_application_close_reason_len_longer),
    /* New NEW_CONNECTION_ID frame test items */
    FUZI_Q_ITEM("new_cid_retire_prior_to_greater", test_frame_new_cid_retire_prior_to_greater),
    FUZI_Q_ITEM("new_cid_zero_length", test_frame_new_cid_zero_length),
    FUZI_Q_ITEM("new_cid_length_too_large", test_frame_new_cid_length_too_large),
    /* New NEW_TOKEN frame test items */
    FUZI_Q_ITEM("new_token_zero_length", test_frame_new_token_zero_length),
    FUZI_Q_ITEM("new_token_length_too_large", test_frame_new_token_length_too_large),
    FUZI_Q_ITEM("new_token_length_shorter_than_data", test_frame_new_token_length_shorter_than_data),
    FUZI_Q_ITEM("new_token_length_longer_than_data", test_frame_new_token_length_longer_than_data),
    /* New CRYPTO frame test items */
    FUZI_Q_ITEM("crypto_len_beyond_packet", test_frame_crypto_len_beyond_packet),
    FUZI_Q_ITEM("crypto_zero_len_with_data", test_frame_crypto_zero_len_with_data),
    FUZI_Q_ITEM("crypto_len_shorter_than_data", test_frame_crypto_len_shorter_than_data),
    FUZI_Q_ITEM("crypto_len_longer_than_data", test_frame_crypto_len_longer_than_data),
    FUZI_Q_ITEM("crypto_max_offset_max_len", test_frame_crypto_max_offset_max_len),
    /* User added STREAM frame test items */
    FUZI_Q_ITEM("stream_fin_too_long", test_frame_stream_fin_too_long),
    FUZI_Q_ITEM("stream_overlapping_data_part1", test_frame_stream_overlapping_data_part1),
    FUZI_Q_ITEM("stream_overlapping_data_part2", test_frame_stream_overlapping_data_part2),
    /* New fuzzy varint test items */
    FUZI_Q_ITEM("max_data_non_minimal_varint", test_frame_max_data_non_minimal_varint),
    FUZI_Q_ITEM("reset_stream_invalid_9_byte_varint", test_frame_reset_stream_invalid_9_byte_varint),
    FUZI_Q_ITEM("stop_sending_non_minimal_error_code", test_frame_stop_sending_non_minimal_error_code),
    /* User added ACK frame test items */
    FUZI_Q_ITEM("ack_overlapping_ranges", test_frame_ack_overlapping_ranges),
    FUZI_Q_ITEM("ack_ascending_ranges_invalid_gap", test_frame_ack_ascending_ranges_invalid_gap),
    FUZI_Q_ITEM("ack_invalid_range_count", test_frame_ack_invalid_range_count),
    FUZI_Q_ITEM("ack_largest_smaller_than_range", test_frame_ack_largest_smaller_than_range),
    /* New static test cases for less common frame variations */
    FUZI_Q_ITEM("retire_cid_seq_much_higher", test_frame_retire_cid_seq_much_higher),
    FUZI_Q_ITEM("datagram_len_shorter_than_data", test_frame_datagram_len_shorter_than_data),
    FUZI_Q_ITEM("datagram_len_longer_than_data", test_frame_datagram_len_longer_than_data),
    FUZI_Q_ITEM("datagram_zero_len_with_data", test_frame_datagram_zero_len_with_data),

    /* User added test frames from current plan (steps 1-6) */
    FUZI_Q_ITEM("max_data_after_close_scenario", test_frame_max_data_after_close_scenario),
    FUZI_Q_ITEM("max_stream_data_for_reset_stream_scenario", test_frame_max_stream_data_for_reset_stream_scenario),
    FUZI_Q_ITEM("streams_blocked_not_actually_blocked", test_frame_streams_blocked_not_actually_blocked),
    FUZI_Q_ITEM("streams_blocked_limit_too_high", test_frame_streams_blocked_limit_too_high),
    FUZI_Q_ITEM("stop_sending_for_peer_reset_stream", test_frame_stop_sending_for_peer_reset_stream),
    FUZI_Q_ITEM("stop_sending_large_error_code", test_frame_stop_sending_large_error_code),
    FUZI_Q_ITEM("retire_cid_current_in_use", test_frame_retire_cid_current_in_use),
    FUZI_Q_ITEM("new_cid_exceed_limit_no_retire", test_frame_new_cid_exceed_limit_no_retire),
    FUZI_Q_ITEM("connection_close_invalid_inner_frame_type", test_frame_connection_close_invalid_inner_frame_type),
    FUZI_Q_ITEM("connection_close_reason_non_utf8", test_frame_connection_close_reason_non_utf8),
    FUZI_Q_ITEM("ping_long_encoding", test_frame_ping_long_encoding),

    /* User added NEW_CONNECTION_ID frame test items (specific names) */
    FUZI_Q_ITEM("new_cid_retire_prior_to_seq_num_mismatch", test_frame_new_cid_retire_prior_to_seq_num_mismatch),
    FUZI_Q_ITEM("new_cid_invalid_length", test_frame_new_cid_invalid_length),
    FUZI_Q_ITEM("new_cid_length_too_long_for_rfc", test_frame_new_cid_length_too_long_for_rfc),
    /* User added Varint encoding frame test items */
    FUZI_Q_ITEM("max_streams_non_minimal_varint", test_frame_max_streams_non_minimal_varint),
    FUZI_Q_ITEM("crypto_offset_non_minimal_large_varint", test_frame_crypto_offset_non_minimal_large_varint),

    /* New test frames based on RFC 9000 review */
    FUZI_Q_ITEM("stream_off_len_empty_fin", test_frame_stream_off_len_empty_fin),
    FUZI_Q_ITEM("ack_many_small_ranges", test_frame_ack_many_small_ranges),
    FUZI_Q_ITEM("new_cid_seq_much_lower", test_frame_new_cid_seq_much_lower),
    FUZI_Q_ITEM("padding_mixed_payload", test_frame_padding_mixed_payload),
    FUZI_Q_ITEM("max_streams_uni_at_limit", test_frame_max_streams_uni_at_limit),

    /* Comprehensive Fuzzing Pass 1 Test Cases */
    /* ACK frames (16 cases) */
    FUZI_Q_ITEM("test_ack_delay_zero", test_ack_delay_zero),
    FUZI_Q_ITEM("test_ack_delay_effective_max_tp_val", test_ack_delay_effective_max_tp_val),
    FUZI_Q_ITEM("test_ack_delay_max_varint_val", test_ack_delay_max_varint_val),
    FUZI_Q_ITEM("test_ack_range_count_zero", test_ack_range_count_zero),
    FUZI_Q_ITEM("test_ack_range_count_one", test_ack_range_count_one),
    FUZI_Q_ITEM("test_ack_range_count_many", test_ack_range_count_many),
    FUZI_Q_ITEM("test_ack_first_range_zero", test_ack_first_range_zero),
    FUZI_Q_ITEM("test_ack_first_range_causes_negative_smallest", test_ack_first_range_causes_negative_smallest),
    FUZI_Q_ITEM("test_ack_first_range_covers_zero", test_ack_first_range_covers_zero),
    FUZI_Q_ITEM("test_ack_gap_zero_len_zero", test_ack_gap_zero_len_zero),
    FUZI_Q_ITEM("test_ack_gap_causes_negative_next_largest", test_ack_gap_causes_negative_next_largest),
    FUZI_Q_ITEM("test_ack_range_len_large", test_ack_range_len_large),
    FUZI_Q_ITEM("test_ack_ecn_all_zero", test_ack_ecn_all_zero),
    FUZI_Q_ITEM("test_ack_ecn_one_each", test_ack_ecn_one_each),
    FUZI_Q_ITEM("test_ack_ecn_large_counts", test_ack_ecn_large_counts),
    FUZI_Q_ITEM("test_ack_ecn_sum_exceeds_largest_acked", test_ack_ecn_sum_exceeds_largest_acked),
    /* STREAM frames (17 cases) */
    FUZI_Q_ITEM("test_stream_0x08_off0_len0_fin0", test_stream_0x08_off0_len0_fin0),
    FUZI_Q_ITEM("test_stream_0x09_off0_len0_fin1", test_stream_0x09_off0_len0_fin1),
    FUZI_Q_ITEM("test_stream_0x0A_off0_len1_fin0", test_stream_0x0A_off0_len1_fin0),
    FUZI_Q_ITEM("test_stream_0x0B_off0_len1_fin1", test_stream_0x0B_off0_len1_fin1),
    FUZI_Q_ITEM("test_stream_0x0C_off1_len0_fin0", test_stream_0x0C_off1_len0_fin0),
    FUZI_Q_ITEM("test_stream_0x0D_off1_len0_fin1", test_stream_0x0D_off1_len0_fin1),
    FUZI_Q_ITEM("test_stream_0x0E_off1_len1_fin0", test_stream_0x0E_off1_len1_fin0),
    FUZI_Q_ITEM("test_stream_0x0F_off1_len1_fin1", test_stream_0x0F_off1_len1_fin1),
    FUZI_Q_ITEM("test_stream_0x0F_id_zero", test_stream_0x0F_id_zero),
    FUZI_Q_ITEM("test_stream_0x0F_id_large", test_stream_0x0F_id_large),
    FUZI_Q_ITEM("test_stream_0x0F_id_max_62bit", test_stream_0x0F_id_max_62bit),
    FUZI_Q_ITEM("test_stream_0x0F_off_zero", test_stream_0x0F_off_zero),
    FUZI_Q_ITEM("test_stream_0x0F_off_max_62bit", test_stream_0x0F_off_max_62bit),
    FUZI_Q_ITEM("test_stream_0x0F_off_plus_len_exceeds_max", test_stream_0x0F_off_plus_len_exceeds_max),
    FUZI_Q_ITEM("test_stream_0x0F_len_zero", test_stream_0x0F_len_zero),
    FUZI_Q_ITEM("test_stream_0x0F_len_one", test_stream_0x0F_len_one),
    FUZI_Q_ITEM("test_stream_0x0F_len_exceed_total_with_offset", test_stream_0x0F_len_exceed_total_with_offset),
    /* RESET_STREAM frames (11 cases) */
    FUZI_Q_ITEM("test_reset_stream_base", test_reset_stream_base),
    FUZI_Q_ITEM("test_reset_stream_id_zero", test_reset_stream_id_zero),
    FUZI_Q_ITEM("test_reset_stream_id_large", test_reset_stream_id_large),
    FUZI_Q_ITEM("test_reset_stream_id_max_62bit", test_reset_stream_id_max_62bit),
    FUZI_Q_ITEM("test_reset_stream_err_zero", test_reset_stream_err_zero),
    FUZI_Q_ITEM("test_reset_stream_err_transport_range_like", test_reset_stream_err_transport_range_like),
    FUZI_Q_ITEM("test_reset_stream_err_max_62bit", test_reset_stream_err_max_62bit),
    FUZI_Q_ITEM("test_reset_stream_final_size_zero", test_reset_stream_final_size_zero),
    FUZI_Q_ITEM("test_reset_stream_final_size_one", test_reset_stream_final_size_one),
    FUZI_Q_ITEM("test_reset_stream_final_size_scenario_small", test_reset_stream_final_size_scenario_small),
    FUZI_Q_ITEM("test_reset_stream_final_size_max_62bit", test_reset_stream_final_size_max_62bit),
    /* STOP_SENDING frames (9 cases) */
    FUZI_Q_ITEM("test_stop_sending_base", test_stop_sending_base),
    FUZI_Q_ITEM("test_stop_sending_id_zero", test_stop_sending_id_zero),
    FUZI_Q_ITEM("test_stop_sending_id_large", test_stop_sending_id_large),
    FUZI_Q_ITEM("test_stop_sending_id_max_62bit", test_stop_sending_id_max_62bit),
    FUZI_Q_ITEM("test_stop_sending_id_recv_only_scenario", test_stop_sending_id_recv_only_scenario),
    FUZI_Q_ITEM("test_stop_sending_id_uncreated_sender_scenario", test_stop_sending_id_uncreated_sender_scenario),
    FUZI_Q_ITEM("test_stop_sending_err_zero", test_stop_sending_err_zero),
    FUZI_Q_ITEM("test_stop_sending_err_transport_range_like", test_stop_sending_err_transport_range_like),
    FUZI_Q_ITEM("test_stop_sending_err_max_62bit", test_stop_sending_err_max_62bit),
    /* Non-Canonical Variable-Length Integers */
    FUZI_Q_ITEM("stream_long_varint_stream_id_2byte", test_frame_stream_long_varint_stream_id_2byte),
    FUZI_Q_ITEM("stream_long_varint_stream_id_4byte", test_frame_stream_long_varint_stream_id_4byte),
    FUZI_Q_ITEM("stream_long_varint_offset_2byte", test_frame_stream_long_varint_offset_2byte),
    FUZI_Q_ITEM("stream_long_varint_offset_4byte", test_frame_stream_long_varint_offset_4byte),
    FUZI_Q_ITEM("stream_long_varint_length_2byte", test_frame_stream_long_varint_length_2byte),
    FUZI_Q_ITEM("stream_long_varint_length_4byte", test_frame_stream_long_varint_length_4byte),
    FUZI_Q_ITEM("max_data_long_varint_2byte", test_frame_max_data_long_varint_2byte),
    FUZI_Q_ITEM("max_data_long_varint_4byte", test_frame_max_data_long_varint_4byte),
    FUZI_Q_ITEM("ack_long_varint_largest_acked_2byte", test_frame_ack_long_varint_largest_acked_2byte),
    FUZI_Q_ITEM("ack_long_varint_largest_acked_4byte", test_frame_ack_long_varint_largest_acked_4byte),
    FUZI_Q_ITEM("crypto_long_varint_offset_2byte", test_frame_crypto_long_varint_offset_2byte),
    FUZI_Q_ITEM("crypto_long_varint_offset_4byte", test_frame_crypto_long_varint_offset_4byte),
    /* STREAM SID: Non-Canonical Varints */
    FUZI_Q_ITEM("stream_sid_0_nc2", test_stream_sid_0_nc2),
    FUZI_Q_ITEM("stream_sid_0_nc4", test_stream_sid_0_nc4),
    FUZI_Q_ITEM("stream_sid_0_nc8", test_stream_sid_0_nc8),
    FUZI_Q_ITEM("stream_sid_1_nc2", test_stream_sid_1_nc2),
    FUZI_Q_ITEM("stream_sid_1_nc4", test_stream_sid_1_nc4),
    FUZI_Q_ITEM("stream_sid_1_nc8", test_stream_sid_1_nc8),
    FUZI_Q_ITEM("stream_sid_5_nc2", test_stream_sid_5_nc2),
    FUZI_Q_ITEM("stream_sid_5_nc4", test_stream_sid_5_nc4),
    FUZI_Q_ITEM("stream_sid_5_nc8", test_stream_sid_5_nc8),
    /* STREAM Offset: Non-Canonical Varints */
    FUZI_Q_ITEM("stream_off_0_nc2", test_stream_off_0_nc2),
    FUZI_Q_ITEM("stream_off_0_nc4", test_stream_off_0_nc4),
    FUZI_Q_ITEM("stream_off_0_nc8", test_stream_off_0_nc8),
    FUZI_Q_ITEM("stream_off_1_nc2", test_stream_off_1_nc2),
    FUZI_Q_ITEM("stream_off_1_nc4", test_stream_off_1_nc4),
    FUZI_Q_ITEM("stream_off_1_nc8", test_stream_off_1_nc8),
    FUZI_Q_ITEM("stream_off_5_nc2", test_stream_off_5_nc2),
    FUZI_Q_ITEM("stream_off_5_nc4", test_stream_off_5_nc4),
    FUZI_Q_ITEM("stream_off_5_nc8", test_stream_off_5_nc8),
    /* STREAM Length: Non-Canonical Varints */
    FUZI_Q_ITEM("stream_len_0_nc2", test_stream_len_0_nc2),
    FUZI_Q_ITEM("stream_len_0_nc4", test_stream_len_0_nc4),
    FUZI_Q_ITEM("stream_len_0_nc8", test_stream_len_0_nc8),
    FUZI_Q_ITEM("stream_len_1_nc2", test_stream_len_1_nc2),
    FUZI_Q_ITEM("stream_len_1_nc4", test_stream_len_1_nc4),
    FUZI_Q_ITEM("stream_len_1_nc8", test_stream_len_1_nc8),
    FUZI_Q_ITEM("stream_len_4_nc2", test_stream_len_4_nc2),
    FUZI_Q_ITEM("stream_len_4_nc4", test_stream_len_4_nc4),
    FUZI_Q_ITEM("stream_len_4_nc8", test_stream_len_4_nc8),
    /* ACK Largest Acknowledged: Non-Canonical Varints */
    FUZI_Q_ITEM("ack_largest_ack_0_nc2", test_ack_largest_ack_0_nc2),
    FUZI_Q_ITEM("ack_largest_ack_0_nc4", test_ack_largest_ack_0_nc4),
    FUZI_Q_ITEM("ack_largest_ack_0_nc8", test_ack_largest_ack_0_nc8),
    FUZI_Q_ITEM("ack_largest_ack_1_nc2", test_ack_largest_ack_1_nc2),
    FUZI_Q_ITEM("ack_largest_ack_1_nc4", test_ack_largest_ack_1_nc4),
    FUZI_Q_ITEM("ack_largest_ack_1_nc8", test_ack_largest_ack_1_nc8),
    FUZI_Q_ITEM("ack_largest_ack_5_nc2", test_ack_largest_ack_5_nc2),
    FUZI_Q_ITEM("ack_largest_ack_5_nc4", test_ack_largest_ack_5_nc4),
    FUZI_Q_ITEM("ack_largest_ack_5_nc8", test_ack_largest_ack_5_nc8),
    /* ACK Delay: Non-Canonical Varints */
    FUZI_Q_ITEM("ack_delay_0_nc2", test_ack_delay_0_nc2),
    FUZI_Q_ITEM("ack_delay_0_nc4", test_ack_delay_0_nc4),
    FUZI_Q_ITEM("ack_delay_0_nc8", test_ack_delay_0_nc8),
    FUZI_Q_ITEM("ack_delay_1_nc2", test_ack_delay_1_nc2),
    FUZI_Q_ITEM("ack_delay_1_nc4", test_ack_delay_1_nc4),
    FUZI_Q_ITEM("ack_delay_1_nc8", test_ack_delay_1_nc8),
    FUZI_Q_ITEM("ack_delay_5_nc2", test_ack_delay_5_nc2),
    FUZI_Q_ITEM("ack_delay_5_nc4", test_ack_delay_5_nc4),
    FUZI_Q_ITEM("ack_delay_5_nc8", test_ack_delay_5_nc8),
    /* ACK Range Count: Non-Canonical Varints */
    FUZI_Q_ITEM("ack_range_count_1_nc2", test_ack_range_count_1_nc2),
    FUZI_Q_ITEM("ack_range_count_1_nc4", test_ack_range_count_1_nc4),
    FUZI_Q_ITEM("ack_range_count_1_nc8", test_ack_range_count_1_nc8),
    FUZI_Q_ITEM("ack_range_count_2_nc2", test_ack_range_count_2_nc2),
    FUZI_Q_ITEM("ack_range_count_2_nc4", test_ack_range_count_2_nc4),
    FUZI_Q_ITEM("ack_range_count_2_nc8", test_ack_range_count_2_nc8),
    /* ACK First ACK Range: Non-Canonical Varints */
    FUZI_Q_ITEM("ack_first_range_0_nc2", test_ack_first_range_0_nc2),
    FUZI_Q_ITEM("ack_first_range_0_nc4", test_ack_first_range_0_nc4),
    FUZI_Q_ITEM("ack_first_range_0_nc8", test_ack_first_range_0_nc8),
    FUZI_Q_ITEM("ack_first_range_1_nc2", test_ack_first_range_1_nc2),
    FUZI_Q_ITEM("ack_first_range_1_nc4", test_ack_first_range_1_nc4),
    FUZI_Q_ITEM("ack_first_range_1_nc8", test_ack_first_range_1_nc8),
    FUZI_Q_ITEM("ack_first_range_5_nc2", test_ack_first_range_5_nc2),
    FUZI_Q_ITEM("ack_first_range_5_nc4", test_ack_first_range_5_nc4),
    FUZI_Q_ITEM("ack_first_range_5_nc8", test_ack_first_range_5_nc8),
    /* ACK Gap: Non-Canonical Varints */
    FUZI_Q_ITEM("ack_gap_0_nc2", test_ack_gap_0_nc2),
    FUZI_Q_ITEM("ack_gap_0_nc4", test_ack_gap_0_nc4),
    FUZI_Q_ITEM("ack_gap_0_nc8", test_ack_gap_0_nc8),
    FUZI_Q_ITEM("ack_gap_1_nc2", test_ack_gap_1_nc2),
    FUZI_Q_ITEM("ack_gap_1_nc4", test_ack_gap_1_nc4),
    FUZI_Q_ITEM("ack_gap_1_nc8", test_ack_gap_1_nc8),
    FUZI_Q_ITEM("ack_gap_2_nc2", test_ack_gap_2_nc2),
    FUZI_Q_ITEM("ack_gap_2_nc4", test_ack_gap_2_nc4),
    FUZI_Q_ITEM("ack_gap_2_nc8", test_ack_gap_2_nc8),
    /* ACK Range Length: Non-Canonical Varints */
    FUZI_Q_ITEM("ack_range_len_0_nc2", test_ack_range_len_0_nc2),
    FUZI_Q_ITEM("ack_range_len_0_nc4", test_ack_range_len_0_nc4),
    FUZI_Q_ITEM("ack_range_len_0_nc8", test_ack_range_len_0_nc8),
    FUZI_Q_ITEM("ack_range_len_1_nc2", test_ack_range_len_1_nc2),
    FUZI_Q_ITEM("ack_range_len_1_nc4", test_ack_range_len_1_nc4),
    FUZI_Q_ITEM("ack_range_len_1_nc8", test_ack_range_len_1_nc8),
    FUZI_Q_ITEM("ack_range_len_5_nc2", test_ack_range_len_5_nc2),
    FUZI_Q_ITEM("ack_range_len_5_nc4", test_ack_range_len_5_nc4),
    FUZI_Q_ITEM("ack_range_len_5_nc8", test_ack_range_len_5_nc8),
    /* RESET_STREAM Stream ID: Non-Canonical Varints */
    FUZI_Q_ITEM("reset_stream_sid_0_nc2", test_reset_stream_sid_0_nc2),
    FUZI_Q_ITEM("reset_stream_sid_0_nc4", test_reset_stream_sid_0_nc4),
    FUZI_Q_ITEM("reset_stream_sid_0_nc8", test_reset_stream_sid_0_nc8),
    FUZI_Q_ITEM("reset_stream_sid_1_nc2", test_reset_stream_sid_1_nc2),
    FUZI_Q_ITEM("reset_stream_sid_1_nc4", test_reset_stream_sid_1_nc4),
    FUZI_Q_ITEM("reset_stream_sid_1_nc8", test_reset_stream_sid_1_nc8),
    FUZI_Q_ITEM("reset_stream_sid_5_nc2", test_reset_stream_sid_5_nc2),
    FUZI_Q_ITEM("reset_stream_sid_5_nc4", test_reset_stream_sid_5_nc4),
    FUZI_Q_ITEM("reset_stream_sid_5_nc8", test_reset_stream_sid_5_nc8),
    /* RESET_STREAM App Error Code: Non-Canonical Varints */
    FUZI_Q_ITEM("reset_stream_err_0_nc2", test_reset_stream_err_0_nc2),
    FUZI_Q_ITEM("reset_stream_err_0_nc4", test_reset_stream_err_0_nc4),
    FUZI_Q_ITEM("reset_stream_err_0_nc8", test_reset_stream_err_0_nc8),
    FUZI_Q_ITEM("reset_stream_err_1_nc2", test_reset_stream_err_1_nc2),
    FUZI_Q_ITEM("reset_stream_err_1_nc4", test_reset_stream_err_1_nc4),
    FUZI_Q_ITEM("reset_stream_err_1_nc8", test_reset_stream_err_1_nc8),
    FUZI_Q_ITEM("reset_stream_err_5_nc2", test_reset_stream_err_5_nc2),
    FUZI_Q_ITEM("reset_stream_err_5_nc4", test_reset_stream_err_5_nc4),
    FUZI_Q_ITEM("reset_stream_err_5_nc8", test_reset_stream_err_5_nc8),
    /* RESET_STREAM Final Size: Non-Canonical Varints */
    FUZI_Q_ITEM("reset_stream_final_0_nc2", test_reset_stream_final_0_nc2),
    FUZI_Q_ITEM("reset_stream_final_0_nc4", test_reset_stream_final_0_nc4),
    FUZI_Q_ITEM("reset_stream_final_0_nc8", test_reset_stream_final_0_nc8),
    FUZI_Q_ITEM("reset_stream_final_1_nc2", test_reset_stream_final_1_nc2),
    FUZI_Q_ITEM("reset_stream_final_1_nc4", test_reset_stream_final_1_nc4),
    FUZI_Q_ITEM("reset_stream_final_1_nc8", test_reset_stream_final_1_nc8),
    FUZI_Q_ITEM("reset_stream_final_5_nc2", test_reset_stream_final_5_nc2),
    FUZI_Q_ITEM("reset_stream_final_5_nc4", test_reset_stream_final_5_nc4),
    FUZI_Q_ITEM("reset_stream_final_5_nc8", test_reset_stream_final_5_nc8),
    /* STOP_SENDING Stream ID: Non-Canonical Varints */
    FUZI_Q_ITEM("stop_sending_sid_0_nc2", test_stop_sending_sid_0_nc2),
    FUZI_Q_ITEM("stop_sending_sid_0_nc4", test_stop_sending_sid_0_nc4),
    FUZI_Q_ITEM("stop_sending_sid_0_nc8", test_stop_sending_sid_0_nc8),
    FUZI_Q_ITEM("stop_sending_sid_1_nc2", test_stop_sending_sid_1_nc2),
    FUZI_Q_ITEM("stop_sending_sid_1_nc4", test_stop_sending_sid_1_nc4),
    FUZI_Q_ITEM("stop_sending_sid_1_nc8", test_stop_sending_sid_1_nc8),
    FUZI_Q_ITEM("stop_sending_sid_5_nc2", test_stop_sending_sid_5_nc2),
    FUZI_Q_ITEM("stop_sending_sid_5_nc4", test_stop_sending_sid_5_nc4),
    FUZI_Q_ITEM("stop_sending_sid_5_nc8", test_stop_sending_sid_5_nc8),
    /* STOP_SENDING App Error Code: Non-Canonical Varints */
    FUZI_Q_ITEM("stop_sending_err_0_nc2", test_stop_sending_err_0_nc2),
    FUZI_Q_ITEM("stop_sending_err_0_nc4", test_stop_sending_err_0_nc4),
    FUZI_Q_ITEM("stop_sending_err_0_nc8", test_stop_sending_err_0_nc8),
    FUZI_Q_ITEM("stop_sending_err_1_nc2", test_stop_sending_err_1_nc2),
    FUZI_Q_ITEM("stop_sending_err_1_nc4", test_stop_sending_err_1_nc4),
    FUZI_Q_ITEM("stop_sending_err_1_nc8", test_stop_sending_err_1_nc8),
    FUZI_Q_ITEM("stop_sending_err_5_nc2", test_stop_sending_err_5_nc2),
    FUZI_Q_ITEM("stop_sending_err_5_nc4", test_stop_sending_err_5_nc4),
    FUZI_Q_ITEM("stop_sending_err_5_nc8", test_stop_sending_err_5_nc8),
    /* MAX_DATA Maximum Data: Non-Canonical Varints */
    FUZI_Q_ITEM("max_data_0_nc2", test_max_data_0_nc2),
    FUZI_Q_ITEM("max_data_0_nc4", test_max_data_0_nc4),
    FUZI_Q_ITEM("max_data_0_nc8", test_max_data_0_nc8),
    FUZI_Q_ITEM("max_data_1_nc2", test_max_data_1_nc2),
    FUZI_Q_ITEM("max_data_1_nc4", test_max_data_1_nc4),
    FUZI_Q_ITEM("max_data_1_nc8", test_max_data_1_nc8),
    FUZI_Q_ITEM("max_data_10_nc2", test_max_data_10_nc2),
    FUZI_Q_ITEM("max_data_10_nc4", test_max_data_10_nc4),
    FUZI_Q_ITEM("max_data_10_nc8", test_max_data_10_nc8),
    /* MAX_STREAM_DATA Stream ID: Non-Canonical Varints */
    FUZI_Q_ITEM("max_sdata_sid_0_nc2", test_max_sdata_sid_0_nc2),
    FUZI_Q_ITEM("max_sdata_sid_0_nc4", test_max_sdata_sid_0_nc4),
    FUZI_Q_ITEM("max_sdata_sid_0_nc8", test_max_sdata_sid_0_nc8),
    FUZI_Q_ITEM("max_sdata_sid_1_nc2", test_max_sdata_sid_1_nc2),
    FUZI_Q_ITEM("max_sdata_sid_1_nc4", test_max_sdata_sid_1_nc4),
    FUZI_Q_ITEM("max_sdata_sid_1_nc8", test_max_sdata_sid_1_nc8),
    FUZI_Q_ITEM("max_sdata_sid_5_nc2", test_max_sdata_sid_5_nc2),
    FUZI_Q_ITEM("max_sdata_sid_5_nc4", test_max_sdata_sid_5_nc4),
    FUZI_Q_ITEM("max_sdata_sid_5_nc8", test_max_sdata_sid_5_nc8),
    /* MAX_STREAM_DATA Max Value: Non-Canonical Varints */
    FUZI_Q_ITEM("max_sdata_val_0_nc2", test_max_sdata_val_0_nc2),
    FUZI_Q_ITEM("max_sdata_val_0_nc4", test_max_sdata_val_0_nc4),
    FUZI_Q_ITEM("max_sdata_val_0_nc8", test_max_sdata_val_0_nc8),
    FUZI_Q_ITEM("max_sdata_val_1_nc2", test_max_sdata_val_1_nc2),
    FUZI_Q_ITEM("max_sdata_val_1_nc4", test_max_sdata_val_1_nc4),
    FUZI_Q_ITEM("max_sdata_val_1_nc8", test_max_sdata_val_1_nc8),
    FUZI_Q_ITEM("max_sdata_val_10_nc2", test_max_sdata_val_10_nc2),
    FUZI_Q_ITEM("max_sdata_val_10_nc4", test_max_sdata_val_10_nc4),
    FUZI_Q_ITEM("max_sdata_val_10_nc8", test_max_sdata_val_10_nc8),
    /* MAX_STREAMS (Bidi): Non-Canonical Varints */
    FUZI_Q_ITEM("max_streams_bidi_0_nc2", test_max_streams_bidi_0_nc2),
    FUZI_Q_ITEM("max_streams_bidi_0_nc4", test_max_streams_bidi_0_nc4),
    FUZI_Q_ITEM("max_streams_bidi_0_nc8", test_max_streams_bidi_0_nc8),
    FUZI_Q_ITEM("max_streams_bidi_1_nc2", test_max_streams_bidi_1_nc2),
    FUZI_Q_ITEM("max_streams_bidi_1_nc4", test_max_streams_bidi_1_nc4),
    FUZI_Q_ITEM("max_streams_bidi_1_nc8", test_max_streams_bidi_1_nc8),
    FUZI_Q_ITEM("max_streams_bidi_5_nc2", test_max_streams_bidi_5_nc2),
    FUZI_Q_ITEM("max_streams_bidi_5_nc4", test_max_streams_bidi_5_nc4),
    FUZI_Q_ITEM("max_streams_bidi_5_nc8", test_max_streams_bidi_5_nc8),
    /* MAX_STREAMS (Uni): Non-Canonical Varints */
    FUZI_Q_ITEM("max_streams_uni_0_nc2", test_max_streams_uni_0_nc2),
    FUZI_Q_ITEM("max_streams_uni_0_nc4", test_max_streams_uni_0_nc4),
    FUZI_Q_ITEM("max_streams_uni_0_nc8", test_max_streams_uni_0_nc8),
    FUZI_Q_ITEM("max_streams_uni_1_nc2", test_max_streams_uni_1_nc2),
    FUZI_Q_ITEM("max_streams_uni_1_nc4", test_max_streams_uni_1_nc4),
    FUZI_Q_ITEM("max_streams_uni_1_nc8", test_max_streams_uni_1_nc8),
    FUZI_Q_ITEM("max_streams_uni_5_nc2", test_max_streams_uni_5_nc2),
    FUZI_Q_ITEM("max_streams_uni_5_nc4", test_max_streams_uni_5_nc4),
    FUZI_Q_ITEM("max_streams_uni_5_nc8", test_max_streams_uni_5_nc8),
    /* RESET_STREAM Stream ID: Non-Canonical Varints */
    FUZI_Q_ITEM("reset_stream_sid_0_nc2", test_reset_stream_sid_0_nc2),
    FUZI_Q_ITEM("reset_stream_sid_0_nc4", test_reset_stream_sid_0_nc4),
    FUZI_Q_ITEM("reset_stream_sid_0_nc8", test_reset_stream_sid_0_nc8),
    FUZI_Q_ITEM("reset_stream_sid_1_nc2", test_reset_stream_sid_1_nc2),
    FUZI_Q_ITEM("reset_stream_sid_1_nc4", test_reset_stream_sid_1_nc4),
    FUZI_Q_ITEM("reset_stream_sid_1_nc8", test_reset_stream_sid_1_nc8),
    FUZI_Q_ITEM("reset_stream_sid_5_nc2", test_reset_stream_sid_5_nc2),
    FUZI_Q_ITEM("reset_stream_sid_5_nc4", test_reset_stream_sid_5_nc4),
    FUZI_Q_ITEM("reset_stream_sid_5_nc8", test_reset_stream_sid_5_nc8),
    /* RESET_STREAM App Error Code: Non-Canonical Varints */
    FUZI_Q_ITEM("reset_stream_err_0_nc2", test_reset_stream_err_0_nc2),
    FUZI_Q_ITEM("reset_stream_err_0_nc4", test_reset_stream_err_0_nc4),
    FUZI_Q_ITEM("reset_stream_err_0_nc8", test_reset_stream_err_0_nc8),
    FUZI_Q_ITEM("reset_stream_err_1_nc2", test_reset_stream_err_1_nc2),
    FUZI_Q_ITEM("reset_stream_err_1_nc4", test_reset_stream_err_1_nc4),
    FUZI_Q_ITEM("reset_stream_err_1_nc8", test_reset_stream_err_1_nc8),
    FUZI_Q_ITEM("reset_stream_err_5_nc2", test_reset_stream_err_5_nc2),
    FUZI_Q_ITEM("reset_stream_err_5_nc4", test_reset_stream_err_5_nc4),
    FUZI_Q_ITEM("reset_stream_err_5_nc8", test_reset_stream_err_5_nc8),
    /* RESET_STREAM Final Size: Non-Canonical Varints */
    FUZI_Q_ITEM("reset_stream_final_0_nc2", test_reset_stream_final_0_nc2),
    FUZI_Q_ITEM("reset_stream_final_0_nc4", test_reset_stream_final_0_nc4),
    FUZI_Q_ITEM("reset_stream_final_0_nc8", test_reset_stream_final_0_nc8),
    FUZI_Q_ITEM("reset_stream_final_1_nc2", test_reset_stream_final_1_nc2),
    FUZI_Q_ITEM("reset_stream_final_1_nc4", test_reset_stream_final_1_nc4),
    FUZI_Q_ITEM("reset_stream_final_1_nc8", test_reset_stream_final_1_nc8),
    FUZI_Q_ITEM("reset_stream_final_5_nc2", test_reset_stream_final_5_nc2),
    FUZI_Q_ITEM("reset_stream_final_5_nc4", test_reset_stream_final_5_nc4),
    FUZI_Q_ITEM("reset_stream_final_5_nc8", test_reset_stream_final_5_nc8),
    /* STOP_SENDING Stream ID: Non-Canonical Varints */
    FUZI_Q_ITEM("stop_sending_sid_0_nc2", test_stop_sending_sid_0_nc2),
    FUZI_Q_ITEM("stop_sending_sid_0_nc4", test_stop_sending_sid_0_nc4),
    FUZI_Q_ITEM("stop_sending_sid_0_nc8", test_stop_sending_sid_0_nc8),
    FUZI_Q_ITEM("stop_sending_sid_1_nc2", test_stop_sending_sid_1_nc2),
    FUZI_Q_ITEM("stop_sending_sid_1_nc4", test_stop_sending_sid_1_nc4),
    FUZI_Q_ITEM("stop_sending_sid_1_nc8", test_stop_sending_sid_1_nc8),
    FUZI_Q_ITEM("stop_sending_sid_5_nc2", test_stop_sending_sid_5_nc2),
    FUZI_Q_ITEM("stop_sending_sid_5_nc4", test_stop_sending_sid_5_nc4),
    FUZI_Q_ITEM("stop_sending_sid_5_nc8", test_stop_sending_sid_5_nc8),
    /* STOP_SENDING App Error Code: Non-Canonical Varints */
    FUZI_Q_ITEM("stop_sending_err_0_nc2", test_stop_sending_err_0_nc2),
    FUZI_Q_ITEM("stop_sending_err_0_nc4", test_stop_sending_err_0_nc4),
    FUZI_Q_ITEM("stop_sending_err_0_nc8", test_stop_sending_err_0_nc8),
    FUZI_Q_ITEM("stop_sending_err_1_nc2", test_stop_sending_err_1_nc2),
    FUZI_Q_ITEM("stop_sending_err_1_nc4", test_stop_sending_err_1_nc4),
    FUZI_Q_ITEM("stop_sending_err_1_nc8", test_stop_sending_err_1_nc8),
    FUZI_Q_ITEM("stop_sending_err_5_nc2", test_stop_sending_err_5_nc2),
    FUZI_Q_ITEM("stop_sending_err_5_nc4", test_stop_sending_err_5_nc4),
    FUZI_Q_ITEM("stop_sending_err_5_nc8", test_stop_sending_err_5_nc8),
    /* DATA_BLOCKED Maximum Data: Non-Canonical Varints */
    FUZI_Q_ITEM("data_blocked_0_nc2", test_data_blocked_0_nc2),
    FUZI_Q_ITEM("data_blocked_0_nc4", test_data_blocked_0_nc4),
    FUZI_Q_ITEM("data_blocked_0_nc8", test_data_blocked_0_nc8),
    FUZI_Q_ITEM("data_blocked_1_nc2", test_data_blocked_1_nc2),
    FUZI_Q_ITEM("data_blocked_1_nc4", test_data_blocked_1_nc4),
    FUZI_Q_ITEM("data_blocked_1_nc8", test_data_blocked_1_nc8),
    FUZI_Q_ITEM("data_blocked_10_nc2", test_data_blocked_10_nc2),
    FUZI_Q_ITEM("data_blocked_10_nc4", test_data_blocked_10_nc4),
    FUZI_Q_ITEM("data_blocked_10_nc8", test_data_blocked_10_nc8),
    /* STREAM_DATA_BLOCKED Stream ID: Non-Canonical Varints */
    FUZI_Q_ITEM("sdata_blocked_sid_0_nc2", test_sdata_blocked_sid_0_nc2),
    FUZI_Q_ITEM("sdata_blocked_sid_0_nc4", test_sdata_blocked_sid_0_nc4),
    FUZI_Q_ITEM("sdata_blocked_sid_0_nc8", test_sdata_blocked_sid_0_nc8),
    FUZI_Q_ITEM("sdata_blocked_sid_1_nc2", test_sdata_blocked_sid_1_nc2),
    FUZI_Q_ITEM("sdata_blocked_sid_1_nc4", test_sdata_blocked_sid_1_nc4),
    FUZI_Q_ITEM("sdata_blocked_sid_1_nc8", test_sdata_blocked_sid_1_nc8),
    FUZI_Q_ITEM("sdata_blocked_sid_5_nc2", test_sdata_blocked_sid_5_nc2),
    FUZI_Q_ITEM("sdata_blocked_sid_5_nc4", test_sdata_blocked_sid_5_nc4),
    FUZI_Q_ITEM("sdata_blocked_sid_5_nc8", test_sdata_blocked_sid_5_nc8),
    /* STREAM_DATA_BLOCKED Stream Data Limit: Non-Canonical Varints */
    FUZI_Q_ITEM("sdata_blocked_limit_0_nc2", test_sdata_blocked_limit_0_nc2),
    FUZI_Q_ITEM("sdata_blocked_limit_0_nc4", test_sdata_blocked_limit_0_nc4),
    FUZI_Q_ITEM("sdata_blocked_limit_0_nc8", test_sdata_blocked_limit_0_nc8),
    FUZI_Q_ITEM("sdata_blocked_limit_1_nc2", test_sdata_blocked_limit_1_nc2),
    FUZI_Q_ITEM("sdata_blocked_limit_1_nc4", test_sdata_blocked_limit_1_nc4),
    FUZI_Q_ITEM("sdata_blocked_limit_1_nc8", test_sdata_blocked_limit_1_nc8),
    FUZI_Q_ITEM("sdata_blocked_limit_10_nc2", test_sdata_blocked_limit_10_nc2),
    FUZI_Q_ITEM("sdata_blocked_limit_10_nc4", test_sdata_blocked_limit_10_nc4),
    FUZI_Q_ITEM("sdata_blocked_limit_10_nc8", test_sdata_blocked_limit_10_nc8),
    /* STREAMS_BLOCKED (Bidi) Maximum Streams: Non-Canonical Varints */
    FUZI_Q_ITEM("streams_blocked_bidi_0_nc2", test_streams_blocked_bidi_0_nc2),
    FUZI_Q_ITEM("streams_blocked_bidi_0_nc4", test_streams_blocked_bidi_0_nc4),
    FUZI_Q_ITEM("streams_blocked_bidi_0_nc8", test_streams_blocked_bidi_0_nc8),
    FUZI_Q_ITEM("streams_blocked_bidi_1_nc2", test_streams_blocked_bidi_1_nc2),
    FUZI_Q_ITEM("streams_blocked_bidi_1_nc4", test_streams_blocked_bidi_1_nc4),
    FUZI_Q_ITEM("streams_blocked_bidi_1_nc8", test_streams_blocked_bidi_1_nc8),
    FUZI_Q_ITEM("streams_blocked_bidi_5_nc2", test_streams_blocked_bidi_5_nc2),
    FUZI_Q_ITEM("streams_blocked_bidi_5_nc4", test_streams_blocked_bidi_5_nc4),
    FUZI_Q_ITEM("streams_blocked_bidi_5_nc8", test_streams_blocked_bidi_5_nc8),
    /* STREAMS_BLOCKED (Uni) Maximum Streams: Non-Canonical Varints */
    FUZI_Q_ITEM("streams_blocked_uni_0_nc2", test_streams_blocked_uni_0_nc2),
    FUZI_Q_ITEM("streams_blocked_uni_0_nc4", test_streams_blocked_uni_0_nc4),
    FUZI_Q_ITEM("streams_blocked_uni_0_nc8", test_streams_blocked_uni_0_nc8),
    FUZI_Q_ITEM("streams_blocked_uni_1_nc2", test_streams_blocked_uni_1_nc2),
    FUZI_Q_ITEM("streams_blocked_uni_1_nc4", test_streams_blocked_uni_1_nc4),
    FUZI_Q_ITEM("streams_blocked_uni_1_nc8", test_streams_blocked_uni_1_nc8),
    FUZI_Q_ITEM("streams_blocked_uni_5_nc2", test_streams_blocked_uni_5_nc2),
    FUZI_Q_ITEM("streams_blocked_uni_5_nc4", test_streams_blocked_uni_5_nc4),
    FUZI_Q_ITEM("streams_blocked_uni_5_nc8", test_streams_blocked_uni_5_nc8),
    /* Aggressive Padding / PMTU Probing Mimics */
    FUZI_Q_ITEM("ping_padded_to_1200", test_frame_ping_padded_to_1200),
    FUZI_Q_ITEM("ping_padded_to_1500", test_frame_ping_padded_to_1500),
    /* ACK Frame Stress Tests */
    FUZI_Q_ITEM("ack_very_many_small_ranges", test_frame_ack_very_many_small_ranges),
    FUZI_Q_ITEM("ack_alternating_large_small_gaps", test_frame_ack_alternating_large_small_gaps),
    /* Unusual but Valid Header Flags/Values (Frames) */
    FUZI_Q_ITEM("stream_id_almost_max", test_frame_stream_id_almost_max),
    FUZI_Q_ITEM("stream_offset_almost_max", test_frame_stream_offset_almost_max),
    /* Additional STREAM Frame Variants */
    FUZI_Q_ITEM("stream_off_len_fin_empty", test_frame_stream_off_len_fin_empty),
    FUZI_Q_ITEM("stream_off_no_len_fin", test_frame_stream_off_no_len_fin),
    FUZI_Q_ITEM("stream_no_off_len_fin_empty", test_frame_stream_no_off_len_fin_empty),
    FUZI_Q_ITEM("stream_just_fin_at_zero", test_frame_stream_just_fin_at_zero),
    /* Zero-Length Data Frames with Max Varint Encoding for Fields */
    FUZI_Q_ITEM("data_blocked_max_varint_offset", test_frame_data_blocked_max_varint_offset),
    FUZI_Q_ITEM("stream_data_blocked_max_varint_fields", test_frame_stream_data_blocked_max_varint_fields),
    FUZI_Q_ITEM("streams_blocked_bidi_max_varint_limit", test_frame_streams_blocked_bidi_max_varint_limit),
    FUZI_Q_ITEM("streams_blocked_uni_max_varint_limit", test_frame_streams_blocked_uni_max_varint_limit),
    /* CRYPTO Frame Edge Cases */
    FUZI_Q_ITEM("crypto_zero_len_large_offset", test_frame_crypto_zero_len_large_offset),
    /* PATH_CHALLENGE / PATH_RESPONSE Variants */
    FUZI_Q_ITEM("path_challenge_alt_pattern", test_frame_path_challenge_alt_pattern),
    FUZI_Q_ITEM("path_response_alt_pattern", test_frame_path_response_alt_pattern),
    /* NEW_TOKEN Frame Variants */
    FUZI_Q_ITEM("new_token_max_plausible_len", test_frame_new_token_max_plausible_len),
    FUZI_Q_ITEM("new_token_min_len", test_frame_new_token_min_len),
    /* CONNECTION_CLOSE Frame Variants */
    FUZI_Q_ITEM("connection_close_max_reason_len", test_frame_connection_close_max_reason_len),
    FUZI_Q_ITEM("connection_close_app_max_reason_len", test_frame_connection_close_app_max_reason_len),
    /* RETIRE_CONNECTION_ID Variants */
    FUZI_Q_ITEM("retire_cid_high_seq", test_frame_retire_cid_high_seq),
    /* MAX_STREAMS Variants (Absolute Max) */
    FUZI_Q_ITEM("max_streams_bidi_abs_max", test_frame_max_streams_bidi_abs_max),
    FUZI_Q_ITEM("max_streams_uni_abs_max", test_frame_max_streams_uni_abs_max),
    // Added new test cases
    FUZI_Q_ITEM("new_token_empty_token", test_frame_new_token_empty_token),
    FUZI_Q_ITEM("stream_type_long_encoding", test_frame_stream_type_long_encoding),
    FUZI_Q_ITEM("ack_type_long_encoding", test_frame_ack_type_long_encoding),
    FUZI_Q_ITEM("reset_stream_type_long_encoding", test_frame_reset_stream_type_long_encoding),
    FUZI_Q_ITEM("max_streams_bidi_just_over_limit", test_frame_max_streams_bidi_just_over_limit),
    FUZI_Q_ITEM("max_streams_uni_just_over_limit", test_frame_max_streams_uni_just_over_limit),
    FUZI_Q_ITEM("client_sends_stop_sending_for_server_uni_stream", test_client_sends_stop_sending_for_server_uni_stream),
    FUZI_Q_ITEM("server_sends_stop_sending_for_client_uni_stream", test_server_sends_stop_sending_for_client_uni_stream),
    FUZI_Q_ITEM("client_sends_max_stream_data_for_client_uni_stream", test_client_sends_max_stream_data_for_client_uni_stream),
    /* Newly added medium priority test cases */
    FUZI_Q_ITEM("ack_cross_pns_low_pkns", test_frame_ack_cross_pns_low_pkns),
    FUZI_Q_ITEM("new_cid_to_zero_len_peer", test_frame_new_cid_to_zero_len_peer),
    FUZI_Q_ITEM("retire_cid_to_zero_len_provider", test_frame_retire_cid_to_zero_len_provider),

    /* --- Adding More Variations (Systematic Review Part 1) --- */
    /* RESET_STREAM Non-Canonical Varints */
    FUZI_Q_ITEM("reset_stream_sid_non_canon", test_frame_reset_stream_sid_non_canon),
    FUZI_Q_ITEM("reset_stream_err_non_canon", test_frame_reset_stream_err_non_canon),
    FUZI_Q_ITEM("reset_stream_final_non_canon", test_frame_reset_stream_final_non_canon),
    /* STOP_SENDING Non-Canonical Varints */
    FUZI_Q_ITEM("stop_sending_sid_non_canon", test_frame_stop_sending_sid_non_canon),
    FUZI_Q_ITEM("stop_sending_err_non_canon", test_frame_stop_sending_err_non_canon),
    /* CRYPTO Non-Canonical Varints */
    FUZI_Q_ITEM("crypto_offset_small_non_canon", test_frame_crypto_offset_small_non_canon),
    FUZI_Q_ITEM("crypto_len_small_non_canon", test_frame_crypto_len_small_non_canon),
    /* NEW_TOKEN Non-Canonical Varint */
    FUZI_Q_ITEM("new_token_len_non_canon", test_frame_new_token_len_non_canon),
	/* Test frame for invalid ACK gap of 1 */
    FUZI_Q_ITEM("test_frame_ack_invalid_gap_1_specific_val", test_frame_ack_invalid_gap_1_specific_val),
	FUZI_Q_ITEM("ack_invalid_gap_1_specific", ack_invalid_gap_1_specific),
    /* PADDING Variation */
    FUZI_Q_ITEM("padding_single", test_frame_padding_single),
    /* ACK Variations */
    FUZI_Q_ITEM("ack_range_count_zero_first_range_set", test_frame_ack_range_count_zero_first_range_set),
    FUZI_Q_ITEM("ack_delay_potentially_large_calc", test_frame_ack_delay_potentially_large_calc),
    FUZI_Q_ITEM("ack_largest_zero_first_zero", test_frame_ack_largest_zero_first_zero),
    FUZI_Q_ITEM("ack_ecn_non_minimal_ect0", test_frame_ack_ecn_non_minimal_ect0),

    /* Frame sequence test items */
    FUZI_Q_ITEM("sequence_stream_ping_padding_val", sequence_stream_ping_padding_val),
	FUZI_Q_ITEM("sequence_stream_ping_padding", sequence_stream_ping_padding),
    FUZI_Q_ITEM("sequence_max_data_max_stream_data_val", sequence_max_data_max_stream_data_val),
	FUZI_Q_ITEM("sequence_max_data_max_stream_data", sequence_max_data_max_stream_data),

    /* Error condition test items Add commentMore actions */
    FUZI_Q_ITEM("error_stream_client_on_server_uni", error_stream_client_on_server_uni_val),
    FUZI_Q_ITEM("error_stream_len_shorter", error_stream_len_shorter_val),
    /* --- Adding More Variations (Systematic Review Part 2 - STREAM & MAX_DATA) --- */
    /* STREAM Contextual Violations (require fuzzer logic for role-specific injection) */
    FUZI_Q_ITEM("stream_client_sends_server_bidi", test_stream_client_sends_server_bidi_stream),
    FUZI_Q_ITEM("stream_client_sends_server_uni", test_stream_client_sends_server_uni_stream),
    FUZI_Q_ITEM("stream_server_sends_client_bidi", test_stream_server_sends_client_bidi_stream),
    FUZI_Q_ITEM("stream_server_sends_client_uni", test_stream_server_sends_client_uni_stream),
    /* STREAM Edge Cases */
    FUZI_Q_ITEM("stream_explicit_len_zero_with_data", test_stream_explicit_len_zero_with_data),
    FUZI_Q_ITEM("stream_fin_only_implicit_len_zero_offset", test_stream_fin_only_implicit_len_zero_offset),
    FUZI_Q_ITEM("stream_fin_len_zero_with_trailing_data", test_stream_fin_len_zero_with_trailing_data),
    /* MAX_DATA Non-Canonical Varint */
    FUZI_Q_ITEM("max_data_long_varint_8byte_small", test_frame_max_data_long_varint_8byte_small),
    /* MAX_STREAM_DATA Variations */
    FUZI_Q_ITEM("max_stream_data_id_zero_val_set", test_frame_max_stream_data_id_zero_val_set),
    FUZI_Q_ITEM("max_stream_data_sid_non_canon", test_frame_max_stream_data_sid_non_canon),
    FUZI_Q_ITEM("max_stream_data_val_non_canon", test_frame_max_stream_data_val_non_canon),
    /* MAX_STREAMS Variations */
    FUZI_Q_ITEM("max_streams_bidi_val_2_pow_50", test_frame_max_streams_bidi_val_2_pow_50),
    FUZI_Q_ITEM("max_streams_bidi_small_non_canon8", test_frame_max_streams_bidi_small_non_canon8),
    /* DATA_BLOCKED Non-Canonical Varints */
    FUZI_Q_ITEM("data_blocked_non_canon2", test_frame_data_blocked_non_canon2),
    FUZI_Q_ITEM("data_blocked_non_canon4", test_frame_data_blocked_non_canon4),

    /* --- Adding More Variations (Systematic Review Part 3 - SDB, SB, NCID) --- */
    /* STREAM_DATA_BLOCKED Variations */
    FUZI_Q_ITEM("sdb_sid_non_canon", test_frame_sdb_sid_non_canon),
    FUZI_Q_ITEM("sdb_val_non_canon", test_frame_sdb_val_non_canon),
    FUZI_Q_ITEM("sdb_sid_zero", test_frame_sdb_sid_zero),
    /* STREAMS_BLOCKED Variations */
    FUZI_Q_ITEM("streams_blocked_bidi_at_limit", test_frame_streams_blocked_bidi_at_limit),
    FUZI_Q_ITEM("streams_blocked_uni_at_limit", test_frame_streams_blocked_uni_at_limit),
    FUZI_Q_ITEM("streams_blocked_bidi_non_canon2", test_frame_streams_blocked_bidi_non_canon2),
    FUZI_Q_ITEM("streams_blocked_uni_non_canon4", test_frame_streams_blocked_uni_non_canon4),
    /* NEW_CONNECTION_ID Variations */
    FUZI_Q_ITEM("ncid_seq_non_canon", test_frame_ncid_seq_non_canon),
    FUZI_Q_ITEM("ncid_ret_non_canon", test_frame_ncid_ret_non_canon),
    FUZI_Q_ITEM("ncid_cid_len_min", test_frame_ncid_cid_len_min),

    /* --- Adding More Variations (Systematic Review Part 4 - RCID, CC, HSD) --- */
    /* RETIRE_CONNECTION_ID Variations */
    FUZI_Q_ITEM("retire_cid_seq_non_canon", test_frame_retire_cid_seq_non_canon),
    /* CONNECTION_CLOSE Variations */
    FUZI_Q_ITEM("conn_close_reserved_err", test_frame_conn_close_reserved_err),
    FUZI_Q_ITEM("conn_close_ft_non_canon", test_frame_conn_close_ft_non_canon),
    FUZI_Q_ITEM("conn_close_app_rlen_non_canon", test_frame_conn_close_app_rlen_non_canon),
    /* HANDSHAKE_DONE Variations */
    FUZI_Q_ITEM("hsd_type_non_canon", test_frame_hsd_type_non_canon),
    /* Newly added frames for non-canonical encodings (Task D20231116_154018) */
    FUZI_Q_ITEM("retire_cid_seq_non_canon_new", test_frame_retire_cid_seq_non_canon), /* Name adjusted to avoid conflict if already present elsewhere */
    FUZI_Q_ITEM("conn_close_reserved_err_new", test_frame_conn_close_reserved_err),
    FUZI_Q_ITEM("conn_close_ft_non_canon_new", test_frame_conn_close_ft_non_canon),
    FUZI_Q_ITEM("conn_close_app_rlen_non_canon_new", test_frame_conn_close_app_rlen_non_canon),
    FUZI_Q_ITEM("hsd_type_non_canon_new", test_frame_hsd_type_non_canon),
    /* Newly added test frames (Task D20231116_160216) */
    FUZI_Q_ITEM("ack_invalid_gap_1", test_frame_ack_invalid_gap_1),
    FUZI_Q_ITEM("stream_len_decl_short_actual_long", test_frame_stream_len_shorter_than_data),
    FUZI_Q_ITEM("stream_len_decl_long_actual_short", test_frame_stream_len_longer_than_data),
    FUZI_Q_ITEM("ncid_retire_current_dcid", test_frame_type_retire_connection_id),
    FUZI_Q_ITEM("connection_close_frame_encoding_error", test_frame_connection_close_frame_encoding_error),
    FUZI_Q_ITEM("stream_type_very_long_encoding", test_frame_stream_type_long_encoding),
    /* Newly added DATAGRAM test frames (Task D20231121_101010) */
    FUZI_Q_ITEM("datagram_with_len_empty", test_frame_datagram_with_len_empty),
    FUZI_Q_ITEM("datagram_len_non_canon", test_frame_datagram_len_non_canon),
    FUZI_Q_ITEM("datagram_very_large", test_frame_datagram_very_large),
    /* HTTP/3 Frame Payloads */
    FUZI_Q_ITEM("h3_data_payload", test_h3_frame_data_payload),
    FUZI_Q_ITEM("h3_headers_simple", test_h3_frame_headers_payload_simple),
    FUZI_Q_ITEM("h3_settings_empty", test_h3_frame_settings_payload_empty),
    FUZI_Q_ITEM("h3_settings_one", test_h3_frame_settings_payload_one_setting),
    FUZI_Q_ITEM("h3_goaway", test_h3_frame_goaway_payload),
    FUZI_Q_ITEM("h3_max_push_id", test_h3_frame_max_push_id_payload),
    FUZI_Q_ITEM("h3_cancel_push", test_h3_frame_cancel_push_payload),
    FUZI_Q_ITEM("h3_push_promise_simple", test_h3_frame_push_promise_payload_simple),
    FUZI_Q_ITEM("h3_origin_val_0x0c", test_frame_h3_origin_val_0x0c),
    FUZI_Q_ITEM("h3_priority_update_val_0xf0700", test_frame_h3_priority_update_val_0xf0700),
    FUZI_Q_ITEM("h3_origin_payload", test_h3_frame_origin_payload),
    FUZI_Q_ITEM("h3_priority_update_request_payload", test_h3_frame_priority_update_request_payload),
    FUZI_Q_ITEM("h3_priority_update_placeholder_payload", test_h3_frame_priority_update_placeholder_payload),
    /* Additional H3 Frame Payload Variations */
    FUZI_Q_ITEM("h3_data_empty", test_h3_frame_data_empty),
    FUZI_Q_ITEM("h3_data_len_non_canon", test_h3_frame_data_len_non_canon),
    FUZI_Q_ITEM("h3_settings_max_field_section_size_zero", test_h3_settings_max_field_section_size_zero),
    FUZI_Q_ITEM("h3_settings_max_field_section_size_large", test_h3_settings_max_field_section_size_large),
    FUZI_Q_ITEM("h3_settings_multiple", test_h3_settings_multiple),
    FUZI_Q_ITEM("h3_settings_id_non_canon", test_h3_settings_id_non_canon),
    FUZI_Q_ITEM("h3_settings_val_non_canon", test_h3_settings_val_non_canon),
	FUZI_Q_ITEM("h3_origin_payload", test_h3_frame_origin_payload),
    FUZI_Q_ITEM("h3_priority_update_placeholder_payload", test_h3_frame_priority_update_placeholder_payload),
    FUZI_Q_ITEM("h3_priority_update_request_payload", test_h3_frame_priority_update_request_payload),
    FUZI_Q_ITEM("h3_goaway_max_id", test_h3_goaway_max_id),
    FUZI_Q_ITEM("h3_goaway_id_non_canon", test_h3_goaway_id_non_canon),
    FUZI_Q_ITEM("h3_max_push_id_zero", test_h3_max_push_id_zero),
    FUZI_Q_ITEM("h3_max_push_id_non_canon", test_h3_max_push_id_non_canon),
    FUZI_Q_ITEM("h3_cancel_push_max_id", test_h3_cancel_push_max_id),
    FUZI_Q_ITEM("h3_cancel_push_id_non_canon", test_h3_cancel_push_id_non_canon),
    /* DoQ Payload */
    FUZI_Q_ITEM("doq_dns_query_payload", test_doq_dns_query_payload),

    /* RFC 9113 (HTTP/2) Frame Types */
    FUZI_Q_ITEM("h2_data_val_0x0", test_frame_h2_data_val_0x0),
    FUZI_Q_ITEM("h2_headers_val_0x1", test_frame_h2_headers_val_0x1),
    FUZI_Q_ITEM("h2_priority_val_0x2", test_frame_h2_priority_val_0x2),
    FUZI_Q_ITEM("h2_rst_stream_val_0x3", test_frame_h2_rst_stream_val_0x3),
    FUZI_Q_ITEM("h2_settings_val_0x4", test_frame_h2_settings_val_0x4),
    FUZI_Q_ITEM("h2_push_promise_val_0x5", test_frame_h2_push_promise_val_0x5),
    FUZI_Q_ITEM("h2_ping_val_0x6", test_frame_h2_ping_val_0x6),
    FUZI_Q_ITEM("h2_goaway_val_0x7", test_frame_h2_goaway_val_0x7),
    FUZI_Q_ITEM("h2_window_update_val_0x8", test_frame_h2_window_update_val_0x8),
    FUZI_Q_ITEM("h2_continuation_val_0x9", test_frame_h2_continuation_val_0x9),
    FUZI_Q_ITEM("h2_altsvc_val_0xa", test_frame_h2_altsvc_val_0xa),

    /* RFC 6455 (WebSocket) Frame Types */
    FUZI_Q_ITEM("ws_continuation_val_0x0", test_frame_ws_continuation_val_0x0),
    FUZI_Q_ITEM("ws_text_val_0x1", test_frame_ws_text_val_0x1),
    FUZI_Q_ITEM("ws_binary_val_0x2", test_frame_ws_binary_val_0x2),
    FUZI_Q_ITEM("ws_connection_close_val_0x8", test_frame_ws_connection_close_val_0x8),
    FUZI_Q_ITEM("ws_ping_val_0x9", test_frame_ws_ping_val_0x9),
    FUZI_Q_ITEM("ws_pong_val_0xa", test_frame_ws_pong_val_0xa),

    /* STREAM Frame Variations (RFC 9000, Section 19.8) */
    FUZI_Q_ITEM("stream_0x08_minimal", test_stream_0x08_minimal),
    FUZI_Q_ITEM("stream_0x08_sid_non_canon", test_stream_0x08_sid_non_canon),
    FUZI_Q_ITEM("stream_0x08_data_long", test_stream_0x08_data_long),
    FUZI_Q_ITEM("stream_0x09_minimal", test_stream_0x09_minimal),
    FUZI_Q_ITEM("stream_0x09_sid_non_canon", test_stream_0x09_sid_non_canon),
    FUZI_Q_ITEM("stream_0x0A_len_zero_no_data", test_stream_0x0A_len_zero_no_data),
    FUZI_Q_ITEM("stream_0x0A_len_zero_with_data", test_stream_0x0A_len_zero_with_data),
    FUZI_Q_ITEM("stream_0x0A_len_small", test_stream_0x0A_len_small),
    FUZI_Q_ITEM("stream_0x0A_len_large", test_stream_0x0A_len_large),
    FUZI_Q_ITEM("stream_0x0A_sid_non_canon", test_stream_0x0A_sid_non_canon),
    FUZI_Q_ITEM("stream_0x0A_len_non_canon", test_stream_0x0A_len_non_canon),
    FUZI_Q_ITEM("stream_0x0B_len_zero_no_data_fin", test_stream_0x0B_len_zero_no_data_fin),
    FUZI_Q_ITEM("stream_0x0B_len_non_canon_fin", test_stream_0x0B_len_non_canon_fin),
    FUZI_Q_ITEM("stream_0x0C_offset_zero", test_stream_0x0C_offset_zero),
    FUZI_Q_ITEM("stream_0x0C_offset_large", test_stream_0x0C_offset_large),
    FUZI_Q_ITEM("stream_0x0C_sid_non_canon", test_stream_0x0C_sid_non_canon),
    FUZI_Q_ITEM("stream_0x0C_offset_non_canon", test_stream_0x0C_offset_non_canon),
    FUZI_Q_ITEM("stream_0x0D_offset_zero_fin", test_stream_0x0D_offset_zero_fin),
    FUZI_Q_ITEM("stream_0x0D_offset_non_canon_fin", test_stream_0x0D_offset_non_canon_fin),
    FUZI_Q_ITEM("stream_0x0E_all_fields_present", test_stream_0x0E_all_fields_present),
    FUZI_Q_ITEM("stream_0x0E_all_non_canon", test_stream_0x0E_all_non_canon),
    FUZI_Q_ITEM("stream_0x0F_all_fields_fin", test_stream_0x0F_all_fields_fin),
    FUZI_Q_ITEM("stream_0x0F_all_non_canon_fin", test_stream_0x0F_all_non_canon_fin),
    /* ACK, RESET_STREAM, STOP_SENDING Frame Variations (RFC 9000) */
    FUZI_Q_ITEM("ack_ecn_ect0_large", test_frame_ack_ecn_ect0_large),
    FUZI_Q_ITEM("ack_ecn_ect1_large", test_frame_ack_ecn_ect1_large),
    FUZI_Q_ITEM("ack_ecn_ce_large", test_frame_ack_ecn_ce_large),
    FUZI_Q_ITEM("ack_ecn_all_large", test_frame_ack_ecn_all_large),
    FUZI_Q_ITEM("ack_delay_non_canon", test_frame_ack_delay_non_canon),
    FUZI_Q_ITEM("ack_range_count_non_canon", test_frame_ack_range_count_non_canon),
    FUZI_Q_ITEM("ack_first_ack_range_non_canon", test_frame_ack_first_ack_range_non_canon),
    FUZI_Q_ITEM("ack_gap_non_canon", test_frame_ack_gap_non_canon),
    FUZI_Q_ITEM("reset_stream_app_err_non_canon", test_frame_reset_stream_app_err_non_canon),
    FUZI_Q_ITEM("reset_stream_final_size_non_canon_8byte", test_frame_reset_stream_final_size_non_canon_8byte),
    FUZI_Q_ITEM("stop_sending_app_err_non_canon", test_frame_stop_sending_app_err_non_canon),
    /* Non-Canonical Field Encodings (RFC 9000) */
    FUZI_Q_ITEM("crypto_offset_non_canon_4byte", test_frame_crypto_offset_non_canon_4byte),
    FUZI_Q_ITEM("crypto_len_non_canon_4byte", test_frame_crypto_len_non_canon_4byte),
    FUZI_Q_ITEM("new_token_len_non_canon_4byte", test_frame_new_token_len_non_canon_4byte),
    FUZI_Q_ITEM("max_data_non_canon_8byte", test_frame_max_data_non_canon_8byte),
    FUZI_Q_ITEM("max_stream_data_sid_non_canon_2byte", test_frame_max_stream_data_sid_non_canon_2byte),
    FUZI_Q_ITEM("max_stream_data_val_non_canon_4byte", test_frame_max_stream_data_val_non_canon_4byte),
    FUZI_Q_ITEM("max_streams_bidi_non_canon_2byte", test_frame_max_streams_bidi_non_canon_2byte),
    FUZI_Q_ITEM("max_streams_bidi_non_canon_8byte", test_frame_max_streams_bidi_non_canon_8byte),
    FUZI_Q_ITEM("max_streams_uni_non_canon_4byte", test_frame_max_streams_uni_non_canon_4byte),
    /* DATA_BLOCKED, STREAM_DATA_BLOCKED, STREAMS_BLOCKED (non-canonical) */
    FUZI_Q_ITEM("data_blocked_val_non_canon_2byte", test_frame_data_blocked_val_non_canon_2byte),
    FUZI_Q_ITEM("sdb_sid_non_canon_4byte", test_frame_sdb_sid_non_canon_4byte),
    FUZI_Q_ITEM("sdb_val_non_canon_8byte", test_frame_sdb_val_non_canon_8byte),
    FUZI_Q_ITEM("streams_blocked_bidi_non_canon_8byte", test_frame_streams_blocked_bidi_non_canon_8byte),
    FUZI_Q_ITEM("streams_blocked_uni_non_canon_2byte", test_frame_streams_blocked_uni_non_canon_2byte),
    /* NEW_CONNECTION_ID (non-canonical) */
    FUZI_Q_ITEM("ncid_seq_non_canon_2byte", test_frame_ncid_seq_non_canon_2byte),
    FUZI_Q_ITEM("ncid_ret_non_canon_4byte", test_frame_ncid_ret_non_canon_4byte),
    /* RETIRE_CONNECTION_ID (non-canonical) */
    FUZI_Q_ITEM("retire_cid_seq_non_canon_4byte", test_frame_retire_cid_seq_non_canon_4byte),
    /* CONNECTION_CLOSE (non-canonical) */
    FUZI_Q_ITEM("conn_close_ec_non_canon", test_frame_conn_close_ec_non_canon),
    FUZI_Q_ITEM("conn_close_rlen_non_canon", test_frame_conn_close_rlen_non_canon),
    FUZI_Q_ITEM("conn_close_app_ec_non_canon", test_frame_conn_close_app_ec_non_canon),
    FUZI_Q_ITEM("conn_close_app_rlen_non_canon_2byte", test_frame_conn_close_app_rlen_non_canon_2byte),
    /* Added from Plan - Step 4 */
    /* RFC 9000, Sec 19.8, 4.5 - STREAM with explicit non-zero len, no data, FIN */
    FUZI_Q_ITEM("stream_len_set_explicit_length_no_data_fin", test_stream_len_set_explicit_length_no_data_fin),
    /* RFC 9000, Sec 19.8, 4.5 - STREAM with offset+length near max final size */
    FUZI_Q_ITEM("stream_off_len_fin_offset_plus_length_almost_max", test_stream_off_len_fin_offset_plus_length_almost_max),
    /* RFC 9000, Sec 19.3 - ACK+ECN, RangeCount=0, FirstRange set, ECN counts present */
    FUZI_Q_ITEM("ack_ecn_range_count_zero_first_range_set_with_counts", test_ack_ecn_range_count_zero_first_range_set_with_counts),
    /* RFC 9000, Sec 19.19 - CONNECTION_CLOSE (transport) minimal fields */
    FUZI_Q_ITEM("connection_close_transport_min_fields", test_connection_close_transport_min_fields),
    /* RFC 9000, Sec 19.10 - MAX_STREAM_DATA with max StreamID and max Value */
    FUZI_Q_ITEM("max_stream_data_id_max_val_max", test_max_stream_data_id_max_val_max),
    /* --- Batch 1 of New Edge Case Test Variants --- */
    /* RFC 9000, Sec 19.8, 4.5 - STREAM (Type 0x0D) with max offset, 1 byte data. Expected: FINAL_SIZE_ERROR. */
    FUZI_Q_ITEM("stream_implicit_len_max_offset_with_data", test_stream_implicit_len_max_offset_with_data),
    /* RFC 9000, Sec 19.3 - ACK Type 0x02 (no ECN) with trailing ECN-like data. Expected: Ignore or FRAME_ENCODING_ERROR. */
    FUZI_Q_ITEM("ack_type02_with_trailing_ecn_like_data", test_ack_type02_with_trailing_ecn_like_data),
    /* RFC 9000, Sec 19.15 - NEW_CONNECTION_ID with truncated CID. Expected: FRAME_ENCODING_ERROR. */
    FUZI_Q_ITEM("new_connection_id_truncated_cid", test_new_connection_id_truncated_cid),
    /* RFC 9000, Sec 19.15 - NEW_CONNECTION_ID with truncated Stateless Reset Token. Expected: FRAME_ENCODING_ERROR. */
    FUZI_Q_ITEM("new_connection_id_truncated_token", test_new_connection_id_truncated_token),
    /* RFC 9000, Sec 19.15 - NEW_CONNECTION_ID with CID data longer than Length field. Parser should use Length. */
    FUZI_Q_ITEM("new_connection_id_cid_overrun_length_field", test_new_connection_id_cid_overrun_length_field),
    /* --- Batch 2 of New Edge Case Test Variants (Flow Control) --- */
    /* RFC 9000, Sec 19.12 - DATA_BLOCKED with max value */
    FUZI_Q_ITEM("test_data_blocked_max_value", test_data_blocked_max_value),
    /* RFC 9000, Sec 19.13 - STREAM_DATA_BLOCKED with max StreamID and max Value */
    FUZI_Q_ITEM("test_stream_data_blocked_max_id_max_value", test_stream_data_blocked_max_id_max_value),
    /* --- Batch 3 of New Edge Case Test Variants (Stream Limit Frames) --- */
    /* RFC 9000, Sec 19.14 - STREAMS_BLOCKED (bidi) with Maximum Streams over 2^60 limit */
    FUZI_Q_ITEM("test_streams_blocked_bidi_over_limit", test_streams_blocked_bidi_over_limit),
    /* RFC 9000, Sec 19.14 - STREAMS_BLOCKED (uni) with Maximum Streams over 2^60 limit */
    FUZI_Q_ITEM("test_streams_blocked_uni_over_limit", test_streams_blocked_uni_over_limit),
    /* --- Batch 4 of New Edge Case Test Variants (Path Validation Frames) --- */
    /* RFC 9000, Sec 19.17 - PATH_CHALLENGE with all ones data */
    FUZI_Q_ITEM("test_path_challenge_all_ones", test_path_challenge_all_ones),
    /* RFC 9000, Sec 19.18 - PATH_RESPONSE with 0xAA pattern data */
    FUZI_Q_ITEM("test_path_response_alt_bits_AA", test_path_response_alt_bits_AA),
    /* RFC 9000, Sec 19.17 - PATH_CHALLENGE truncated (4 of 8 data bytes) */
    FUZI_Q_ITEM("test_path_challenge_truncated_4bytes", test_path_challenge_truncated_4bytes),
    /* RFC 9000, Sec 19.18 - PATH_RESPONSE truncated (0 of 8 data bytes) */
    FUZI_Q_ITEM("test_path_response_truncated_0bytes", test_path_response_truncated_0bytes),
    /* --- Batch 5 of New Edge Case Test Variants (Other Control Frames) --- */
    /* RFC 9000, Sec 19.5 - STOP_SENDING with max StreamID and max App Error Code */
    FUZI_Q_ITEM("test_stop_sending_max_id_max_error", test_stop_sending_max_id_max_error),
    /* RFC 9000, Sec 19.16 - RETIRE_CONNECTION_ID with max Sequence Number */
    FUZI_Q_ITEM("test_retire_connection_id_max_sequence", test_retire_connection_id_max_sequence),
    /* RFC 9000, Sec 19.7 - NEW_TOKEN with Token Length > 0, truncated before token data */
    FUZI_Q_ITEM("test_new_token_len_gt_zero_no_token_data_truncated", test_new_token_len_gt_zero_no_token_data_truncated),
    /* RFC 9000, Sec 19.4 - RESET_STREAM with StreamID, App Error, and Final Size all max */
    FUZI_Q_ITEM("test_reset_stream_all_fields_max_value", test_reset_stream_all_fields_max_value),
    /* --- Batch 6 of New Edge Case Test Variants (CRYPTO, DATAGRAM, PADDING) --- */
    /* RFC 9000, Sec 19.6 - CRYPTO, Len > 0, truncated before data */
    FUZI_Q_ITEM("test_crypto_len_gt_zero_no_data_truncated", test_crypto_len_gt_zero_no_data_truncated),
    /* RFC 9221, Sec 4 - DATAGRAM (Type 0x30) empty, truncated after type */
    FUZI_Q_ITEM("test_datagram_type0x30_empty_truncated", test_datagram_type0x30_empty_truncated),
    /* RFC 9221, Sec 4 - DATAGRAM (Type 0x30) one byte data */
    FUZI_Q_ITEM("test_datagram_type0x30_one_byte", test_datagram_type0x30_one_byte),
    /* RFC 9221, Sec 4 - DATAGRAM (Type 0x31) max Length field, minimal actual data */
    FUZI_Q_ITEM("test_datagram_type0x31_maxlength_field_min_data", test_datagram_type0x31_maxlength_field_min_data),
    /* RFC 9221, Sec 4 - DATAGRAM (Type 0x31) Len > 0, truncated before data */
    FUZI_Q_ITEM("test_datagram_type0x31_len_gt_zero_no_data_truncated", test_datagram_type0x31_len_gt_zero_no_data_truncated),
    /* RFC 9000, Sec 19.1, 12.4 - PADDING type non-canonically encoded (2 bytes) */
    FUZI_Q_ITEM("test_padding_type_non_canonical_2byte", test_padding_type_non_canonical_2byte),

    /* RFC 9204 QPACK Instructions */
    FUZI_Q_ITEM("qpack_enc_set_dynamic_table_capacity", test_qpack_enc_set_dynamic_table_capacity),
    FUZI_Q_ITEM("qpack_enc_insert_with_name_ref", test_qpack_enc_insert_with_name_ref),
    FUZI_Q_ITEM("qpack_enc_insert_without_name_ref", test_qpack_enc_insert_without_name_ref), /* Corresponds to "Insert with Literal Name" */
    FUZI_Q_ITEM("qpack_enc_duplicate", test_qpack_enc_duplicate),
    FUZI_Q_ITEM("qpack_dec_header_block_ack", test_qpack_dec_header_block_ack),
    FUZI_Q_ITEM("qpack_dec_stream_cancellation", test_qpack_dec_stream_cancellation),
    FUZI_Q_ITEM("qpack_dec_insert_count_increment", test_qpack_dec_insert_count_increment),
    FUZI_Q_ITEM("qpack_enc_set_dynamic_table_capacity_alt", test_qpack_dec_set_dynamic_table_capacity),
    /* WebSocket Frame Types */
    FUZI_Q_ITEM("test_ws_frame_pong", test_ws_frame_pong),
    FUZI_Q_ITEM("test_ws_frame_ping", test_ws_frame_ping),
    FUZI_Q_ITEM("test_ws_frame_connection_close", test_ws_frame_connection_close),
    FUZI_Q_ITEM("test_ws_frame_binary", test_ws_frame_binary),
    FUZI_Q_ITEM("test_ws_frame_text", test_ws_frame_text),
    FUZI_Q_ITEM("test_ws_frame_continuation", test_ws_frame_continuation),

    /* START OF JULES ADDED FUZI_Q_ITEM ENTRIES (BATCHES 1-8) */

    /* --- Batch 1: Unknown or Unassigned Frame Types --- */
    FUZI_Q_ITEM("quic_unknown_frame_0x20", test_frame_quic_unknown_0x20),
    FUZI_Q_ITEM("quic_unknown_0x3f_payload", test_frame_quic_unknown_0x3f_payload),
    FUZI_Q_ITEM("quic_unknown_greased_0x402a", test_frame_quic_unknown_greased_0x402a),
    FUZI_Q_ITEM("h3_reserved_frame_0x02", test_frame_h3_reserved_0x02),
    FUZI_Q_ITEM("h3_reserved_frame_0x06", test_frame_h3_reserved_0x06),
    FUZI_Q_ITEM("h3_unassigned_extension_0x21", test_frame_h3_unassigned_extension_0x21),

    /* --- Batch 1: Malformed Frame Lengths --- */
    FUZI_Q_ITEM("quic_stream_len0_with_data", test_frame_quic_stream_len0_with_data),
    FUZI_Q_ITEM("quic_stream_len_gt_data", test_frame_quic_stream_len_gt_data),
    FUZI_Q_ITEM("quic_stream_len_lt_data", test_frame_quic_stream_len_lt_data),
    FUZI_Q_ITEM("quic_crypto_len0_with_data", test_frame_quic_crypto_len0_with_data),
    FUZI_Q_ITEM("quic_new_token_len_gt_data", test_frame_quic_new_token_len_gt_data),
    FUZI_Q_ITEM("quic_conn_close_reason_len_gt_data", test_frame_quic_conn_close_reason_len_gt_data),

    /* --- Batch 1: Invalid Frame Field Values --- */
    FUZI_Q_ITEM("quic_max_streams_bidi_value0", test_frame_quic_max_streams_bidi_value0),
    FUZI_Q_ITEM("quic_stop_sending_large_error", test_frame_quic_stop_sending_large_error),
    FUZI_Q_ITEM("quic_max_data_value0_b1", test_frame_quic_max_data_value0_b1),
    FUZI_Q_ITEM("quic_ack_largest0_delay0_1range0_b1", test_frame_quic_ack_largest0_delay0_1range0_b1),
    FUZI_Q_ITEM("quic_ncid_retire_gt_seq_b1", test_frame_quic_ncid_retire_gt_seq_b1),

    /* --- Batch 2: More Invalid Frame Field Values --- */
    FUZI_Q_ITEM("quic_max_data_value0", test_frame_quic_max_data_value0),
    FUZI_Q_ITEM("quic_ack_largest0_delay0_1range0", test_frame_quic_ack_largest0_delay0_1range0),
    FUZI_Q_ITEM("quic_ack_range_count0_first_range_set", test_frame_quic_ack_range_count0_first_range_set),
    FUZI_Q_ITEM("h3_settings_unknown_id", test_frame_h3_settings_unknown_id),
    FUZI_Q_ITEM("h3_settings_max_field_section_size0", test_frame_h3_settings_max_field_section_size0),
    FUZI_Q_ITEM("quic_max_stream_data_value0", test_frame_quic_max_stream_data_value0),
    FUZI_Q_ITEM("quic_conn_close_reserved_error", test_frame_quic_conn_close_reserved_error),
    FUZI_Q_ITEM("quic_new_token_zero_len_invalid", test_frame_quic_new_token_zero_len_invalid),

    /* --- Batch 2: Padding Fuzzing --- */
    FUZI_Q_ITEM("quic_padding_excessive_70bytes", test_frame_quic_padding_excessive_70bytes),
    FUZI_Q_ITEM("quic_ping_then_many_padding", test_frame_quic_ping_then_many_padding),

    /* --- Batch 2: Stream ID Fuzzing (static part) --- */
    FUZI_Q_ITEM("quic_stream_id0", test_frame_quic_stream_id0),
    FUZI_Q_ITEM("quic_max_stream_data_server_uni", test_frame_quic_max_stream_data_server_uni),
    FUZI_Q_ITEM("quic_reset_stream_client_uni", test_frame_quic_reset_stream_client_uni),
    FUZI_Q_ITEM("quic_stop_sending_large_stream_id", test_frame_quic_stop_sending_large_stream_id),
    FUZI_Q_ITEM("quic_stream_client_uses_server_id", test_frame_quic_stream_client_uses_server_id),

    /* --- Batch 3: User Prioritized Frames (Part 1 - DATAGRAM) --- */
    FUZI_Q_ITEM("datagram_type30_with_len_data_error", test_frame_datagram_type30_with_len_data_error),
    FUZI_Q_ITEM("datagram_type31_missing_len_error", test_frame_datagram_type31_missing_len_error),
    FUZI_Q_ITEM("datagram_type31_len_zero_with_data_error", test_frame_datagram_type31_len_zero_with_data_error),
    FUZI_Q_ITEM("datagram_type31_len_huge_data_small", test_frame_datagram_type31_len_huge_data_small),
    FUZI_Q_ITEM("datagram_type30_empty_valid", test_frame_datagram_type30_empty_valid),
    FUZI_Q_ITEM("datagram_type31_len0_empty_valid", test_frame_datagram_type31_len0_empty_valid),

    /* --- Batch 3: User Prioritized Frames (Part 1 - H3 SETTINGS) --- */
    FUZI_Q_ITEM("h3_settings_unknown_id_b3", test_h3_settings_unknown_id_b3),
    FUZI_Q_ITEM("h3_settings_duplicate_id", test_h3_settings_duplicate_id),
    FUZI_Q_ITEM("h3_settings_invalid_value_for_id", test_h3_settings_invalid_value_for_id),

    /* --- Batch 3: User Prioritized Frames (Part 2 - H3 ORIGIN & QUIC STREAM) --- */
    FUZI_Q_ITEM("h3_origin_unnegotiated", test_h3_origin_unnegotiated),
    FUZI_Q_ITEM("h3_origin_multiple_entries", test_h3_origin_multiple_entries),
    FUZI_Q_ITEM("h3_origin_empty_entry", test_h3_origin_empty_entry),
    FUZI_Q_ITEM("stream_len_bit_no_len_field", test_stream_len_bit_no_len_field),
    FUZI_Q_ITEM("stream_off_bit_no_off_field", test_stream_off_bit_no_off_field),
    FUZI_Q_ITEM("stream_len_fin_zero_len_with_data", test_stream_len_fin_zero_len_with_data),
    FUZI_Q_ITEM("stream_type08_empty_implicit_len", test_stream_type08_empty_implicit_len),
    FUZI_Q_ITEM("stream_type0C_offset_empty_implicit_len", test_stream_type0C_offset_empty_implicit_len),

    /* --- Batch 3: User Prioritized Frames (Part 3 - QUIC STREAM type range & WebSocket) --- */
    FUZI_Q_ITEM("type_stream_range_just_below", test_frame_type_stream_range_just_below),
    FUZI_Q_ITEM("type_padding_as_stream", test_frame_type_padding_as_stream),
    FUZI_Q_ITEM("type_stream_range_lower_bound", test_frame_type_stream_range_lower_bound),
    FUZI_Q_ITEM("type_stream_range_upper_bound", test_frame_type_stream_range_upper_bound),
    FUZI_Q_ITEM("type_stream_range_just_above", test_frame_type_stream_range_just_above),
    FUZI_Q_ITEM("ws_control_frame_fin_zero_invalid", test_ws_control_frame_fin_zero_invalid),
    FUZI_Q_ITEM("ws_text_frame_rsv1_set_invalid", test_ws_text_frame_rsv1_set_invalid),
    FUZI_Q_ITEM("ws_text_fin0_then_text_continuation_part1", test_ws_text_fin0_then_text_continuation_part1),
    FUZI_Q_ITEM("ws_text_fin0_then_text_continuation_part2_invalid", test_ws_text_fin0_then_text_continuation_part2_invalid),
    FUZI_Q_ITEM("ws_len126_data_truncated", test_ws_len126_data_truncated),
    FUZI_Q_ITEM("ws_len127_data_truncated", test_ws_len127_data_truncated),

    /* --- Batch 4: More Static Frames --- */
    FUZI_Q_ITEM("quic_unknown_frame_high_value", test_frame_quic_unknown_frame_high_value),
    FUZI_Q_ITEM("h3_reserved_frame_0x08", test_frame_h3_reserved_frame_0x08),
    FUZI_Q_ITEM("h3_unassigned_type_0x4040", test_frame_h3_unassigned_type_0x4040),
    FUZI_Q_ITEM("ws_reserved_control_0x0B", test_frame_ws_reserved_control_0x0B),
    FUZI_Q_ITEM("ws_reserved_non_control_0x03", test_frame_ws_reserved_non_control_0x03),
    FUZI_Q_ITEM("h3_headers_incomplete_qpack", test_frame_h3_headers_incomplete_qpack),
    FUZI_Q_ITEM("ws_ping_payload_gt_125", test_frame_ws_ping_payload_gt_125),
    FUZI_Q_ITEM("quic_max_streams_uni_value0", test_frame_quic_max_streams_uni_value0),
    FUZI_Q_ITEM("quic_ncid_short_token", test_frame_quic_ncid_short_token),
    FUZI_Q_ITEM("quic_ncid_zero_len_cid", test_frame_quic_ncid_zero_len_cid),
    FUZI_Q_ITEM("quic_path_challenge_all_zero_data_b4", test_frame_quic_path_challenge_all_zero_data_b4),
    FUZI_Q_ITEM("quic_path_response_mismatch_data_b4", test_frame_quic_path_response_mismatch_data_b4),

    /* --- Batch 5: Further Static Frames --- */
    FUZI_Q_ITEM("quic_unknown_frame_grease_0x2A", test_frame_quic_unknown_frame_grease_0x2A),
    FUZI_Q_ITEM("h3_reserved_frame_0x09", test_frame_h3_reserved_frame_0x09),
    FUZI_Q_ITEM("ws_control_frame_0x0C_invalid", test_frame_ws_control_frame_0x0C_invalid),
    FUZI_Q_ITEM("quic_crypto_len_gt_data_b5", test_frame_quic_crypto_len_gt_data_b5),
    FUZI_Q_ITEM("h3_push_promise_incomplete_payload", test_frame_h3_push_promise_incomplete_payload),
    FUZI_Q_ITEM("quic_retire_connection_id_large_seq", test_frame_quic_retire_connection_id_large_seq),
    FUZI_Q_ITEM("h3_goaway_large_id", test_frame_h3_goaway_large_id),
    FUZI_Q_ITEM("quic_ncid_retire_gt_seq_b5", test_frame_quic_ncid_retire_gt_seq_b5),
    FUZI_Q_ITEM("quic_path_challenge_empty", test_frame_quic_path_challenge_empty),
    FUZI_Q_ITEM("quic_path_response_empty", test_frame_quic_path_response_empty),
    FUZI_Q_ITEM("quic_ack_delay_max_varint", test_frame_quic_ack_delay_max_varint),
    FUZI_Q_ITEM("quic_stream_all_fields_max_varint", test_frame_quic_stream_all_fields_max_varint),
    FUZI_Q_ITEM("h3_data_len0_with_payload", test_frame_h3_data_len0_with_payload),
    FUZI_Q_ITEM("ws_close_invalid_code", test_frame_ws_close_invalid_code),

    /* --- Batch 8: Combined Set (original Batch 6/7 + 4 new from user) --- */
    FUZI_Q_ITEM("h2_window_update_increment0_b7", test_frame_h2_window_update_increment0_b7),
    FUZI_Q_ITEM("quic_conn_close_transport_app_err_code_b7", test_frame_quic_conn_close_transport_app_err_code_b7),
    FUZI_Q_ITEM("h3_max_push_id_value0_b7", test_frame_h3_max_push_id_value0_b7),
    FUZI_Q_ITEM("quic_ncid_cid_len_gt_pico_max_b7", test_frame_quic_ncid_cid_len_gt_pico_max_b7),
    FUZI_Q_ITEM("ws_text_rsv2_set", test_frame_ws_text_rsv2_set),
    FUZI_Q_ITEM("ws_text_rsv3_set", test_frame_ws_text_rsv3_set),
    FUZI_Q_ITEM("quic_ack_non_ecn_with_ecn_counts_b7", test_frame_quic_ack_non_ecn_with_ecn_counts_b7),
    FUZI_Q_ITEM("quic_greased_type_0x5BEE_with_payload", test_frame_quic_greased_type_0x5BEE_with_payload),
    FUZI_Q_ITEM("h3_reserved_type_4byte_varint", test_frame_h3_reserved_type_4byte_varint),
    FUZI_Q_ITEM("ws_continuation_fin1_with_payload", test_frame_ws_continuation_fin1_with_payload),
    FUZI_Q_ITEM("quic_ncid_large_retire_small_seq", test_frame_quic_ncid_large_retire_small_seq),
    FUZI_Q_ITEM("quic_extension_0x21", test_frame_quic_extension_0x21),
    FUZI_Q_ITEM("h3_extension_0x2F", test_frame_h3_extension_0x2F),
    FUZI_Q_ITEM("quic_ack_double_zero_range", test_frame_quic_ack_double_zero_range),
    FUZI_Q_ITEM("ws_all_rsv_set", test_frame_ws_all_rsv_set),

    /* HTTP/2 and HPACK Frame Types */
    FUZI_Q_ITEM("test_h2_frame_type_altsvc", test_h2_frame_type_altsvc),
    FUZI_Q_ITEM("test_hpack_dynamic_table_size_update", test_hpack_dynamic_table_size_update),
    FUZI_Q_ITEM("test_hpack_literal_never_indexed", test_hpack_literal_never_indexed),
    FUZI_Q_ITEM("test_hpack_literal_no_indexing", test_hpack_literal_no_indexing),
    FUZI_Q_ITEM("test_hpack_literal_inc_indexing", test_hpack_literal_inc_indexing),
    FUZI_Q_ITEM("test_hpack_indexed_header_field", test_hpack_indexed_header_field),
    FUZI_Q_ITEM("test_h2_frame_type_continuation", test_h2_frame_type_continuation),
    FUZI_Q_ITEM("test_h2_frame_type_window_update", test_h2_frame_type_window_update),
    FUZI_Q_ITEM("test_h2_frame_type_goaway", test_h2_frame_type_goaway),
    FUZI_Q_ITEM("test_h2_frame_type_ping", test_h2_frame_type_ping),
    FUZI_Q_ITEM("test_h2_frame_type_push_promise", test_h2_frame_type_push_promise),
    FUZI_Q_ITEM("test_h2_frame_type_settings", test_h2_frame_type_settings),
    FUZI_Q_ITEM("test_h2_frame_type_rst_stream", test_h2_frame_type_rst_stream),
    FUZI_Q_ITEM("test_h2_frame_type_priority", test_h2_frame_type_priority),
    FUZI_Q_ITEM("test_h2_frame_type_headers", test_h2_frame_type_headers),
    FUZI_Q_ITEM("test_h2_frame_type_data", test_h2_frame_type_data),

    /* START OF JULES ADDED FUZI_Q_ITEM ENTRIES (BATCHES 1-8) */

    /* --- Batch 1: Unknown or Unassigned Frame Types --- */
    FUZI_Q_ITEM("quic_unknown_frame_0x20", test_frame_quic_unknown_0x20),
    FUZI_Q_ITEM("h3_reserved_frame_0x02", test_frame_h3_reserved_0x02),
    FUZI_Q_ITEM("h3_reserved_frame_0x06", test_frame_h3_reserved_0x06),

    /* --- Batch 1: Malformed Frame Lengths --- */
    FUZI_Q_ITEM("quic_stream_len0_with_data", test_frame_quic_stream_len0_with_data),
    FUZI_Q_ITEM("quic_stream_len_gt_data", test_frame_quic_stream_len_gt_data),
    FUZI_Q_ITEM("quic_stream_len_lt_data", test_frame_quic_stream_len_lt_data),

    /* --- Batch 1: Invalid Frame Field Values --- */
    FUZI_Q_ITEM("quic_max_streams_bidi_value0", test_frame_quic_max_streams_bidi_value0),
    FUZI_Q_ITEM("quic_stop_sending_large_error", test_frame_quic_stop_sending_large_error),

    /* --- Batch 2: More Invalid Frame Field Values --- */
    FUZI_Q_ITEM("quic_max_data_value0", test_frame_quic_max_data_value0),
    FUZI_Q_ITEM("quic_ack_largest0_delay0_1range0", test_frame_quic_ack_largest0_delay0_1range0),
    FUZI_Q_ITEM("quic_ack_range_count0_first_range_set", test_frame_quic_ack_range_count0_first_range_set),
    FUZI_Q_ITEM("h3_settings_unknown_id", test_frame_h3_settings_unknown_id),
    FUZI_Q_ITEM("h3_settings_max_field_section_size0", test_frame_h3_settings_max_field_section_size0),

    /* --- Batch 2: Padding Fuzzing --- */
    FUZI_Q_ITEM("quic_padding_excessive_70bytes", test_frame_quic_padding_excessive_70bytes),

    /* --- Batch 2: Stream ID Fuzzing (static part) --- */
    FUZI_Q_ITEM("quic_stream_id0", test_frame_quic_stream_id0),

    /* --- Batch 3: User Prioritized Frames (Part 1 - DATAGRAM & H3 SETTINGS) --- */
    FUZI_Q_ITEM("datagram_type30_with_len_data_error", test_frame_datagram_type30_with_len_data_error),
    FUZI_Q_ITEM("datagram_type31_missing_len_error", test_frame_datagram_type31_missing_len_error),
    FUZI_Q_ITEM("datagram_type31_len_zero_with_data_error", test_frame_datagram_type31_len_zero_with_data_error),
    FUZI_Q_ITEM("h3_settings_excessive_pairs", test_h3_settings_excessive_pairs),

    /* --- Batch 3: User Prioritized Frames (Part 2 - H3 ORIGIN & QUIC STREAM) --- */
    FUZI_Q_ITEM("h3_origin_unnegotiated", test_h3_origin_unnegotiated),
    FUZI_Q_ITEM("stream_len_bit_no_len_field", test_stream_len_bit_no_len_field),
    FUZI_Q_ITEM("stream_off_bit_no_off_field", test_stream_off_bit_no_off_field),
    FUZI_Q_ITEM("stream_len_fin_zero_len_with_data", test_stream_len_fin_zero_len_with_data),

    /* --- Batch 3: User Prioritized Frames (Part 3 - QUIC STREAM type range & WebSocket) --- */
    FUZI_Q_ITEM("type_stream_range_just_below", test_frame_type_stream_range_just_below),
    FUZI_Q_ITEM("type_stream_range_lower_bound", test_frame_type_stream_range_lower_bound),
    FUZI_Q_ITEM("type_stream_range_upper_bound", test_frame_type_stream_range_upper_bound),
    FUZI_Q_ITEM("type_stream_range_just_above", test_frame_type_stream_range_just_above),
    FUZI_Q_ITEM("ws_control_frame_fin_zero_invalid", test_ws_control_frame_fin_zero_invalid),
    FUZI_Q_ITEM("ws_text_fin0_then_text_continuation_part1", test_ws_text_fin0_then_text_continuation_part1),
    FUZI_Q_ITEM("ws_text_fin0_then_text_continuation_part2_invalid", test_ws_text_fin0_then_text_continuation_part2_invalid),

    /* --- Batch 4: More Static Frames --- */
    FUZI_Q_ITEM("quic_unknown_frame_high_value", test_frame_quic_unknown_frame_high_value),
    FUZI_Q_ITEM("h3_reserved_frame_0x08", test_frame_h3_reserved_frame_0x08),
    FUZI_Q_ITEM("h3_unassigned_type_0x4040", test_frame_h3_unassigned_type_0x4040),
    FUZI_Q_ITEM("ws_reserved_control_0x0B", test_frame_ws_reserved_control_0x0B),
    FUZI_Q_ITEM("ws_reserved_non_control_0x03", test_frame_ws_reserved_non_control_0x03),
    FUZI_Q_ITEM("h3_headers_incomplete_qpack", test_frame_h3_headers_incomplete_qpack),
    FUZI_Q_ITEM("ws_ping_payload_gt_125", test_frame_ws_ping_payload_gt_125),
    FUZI_Q_ITEM("quic_max_streams_uni_value0", test_frame_quic_max_streams_uni_value0),
    FUZI_Q_ITEM("quic_ncid_short_token", test_frame_quic_ncid_short_token),
    FUZI_Q_ITEM("quic_ncid_zero_len_cid", test_frame_quic_ncid_zero_len_cid),

    /* --- Batch 5: Further Static Frames --- */
    FUZI_Q_ITEM("quic_unknown_frame_grease_0x2A", test_frame_quic_unknown_frame_grease_0x2A),
    FUZI_Q_ITEM("h3_reserved_frame_0x09", test_frame_h3_reserved_frame_0x09),
    FUZI_Q_ITEM("ws_control_frame_0x0C_invalid", test_frame_ws_control_frame_0x0C_invalid),
    FUZI_Q_ITEM("h3_push_promise_incomplete_payload", test_frame_h3_push_promise_incomplete_payload),
    FUZI_Q_ITEM("quic_retire_connection_id_large_seq", test_frame_quic_retire_connection_id_large_seq),
    FUZI_Q_ITEM("h3_goaway_large_id", test_frame_h3_goaway_large_id),
    FUZI_Q_ITEM("quic_path_challenge_empty", test_frame_quic_path_challenge_empty),
    FUZI_Q_ITEM("quic_path_response_empty", test_frame_quic_path_response_empty),

    /* --- Batch 8: Combined Set (original Batch 6/7 + 4 new from user) --- */
    FUZI_Q_ITEM("h2_window_update_increment0_b7", test_frame_h2_window_update_increment0_b7),
    FUZI_Q_ITEM("quic_conn_close_transport_app_err_code_b7", test_frame_quic_conn_close_transport_app_err_code_b7),
    FUZI_Q_ITEM("h3_max_push_id_value0_b7", test_frame_h3_max_push_id_value0_b7),
    FUZI_Q_ITEM("quic_ncid_cid_len_gt_pico_max_b7", test_frame_quic_ncid_cid_len_gt_pico_max_b7),
    FUZI_Q_ITEM("ws_text_rsv2_set", test_frame_ws_text_rsv2_set),
    FUZI_Q_ITEM("ws_text_rsv3_set", test_frame_ws_text_rsv3_set),
    FUZI_Q_ITEM("quic_ack_non_ecn_with_ecn_counts_b7", test_frame_quic_ack_non_ecn_with_ecn_counts_b7),
    FUZI_Q_ITEM("quic_greased_type_0x5BEE_with_payload", test_frame_quic_greased_type_0x5BEE_with_payload),
    FUZI_Q_ITEM("h3_reserved_type_4byte_varint", test_frame_h3_reserved_type_4byte_varint),
    FUZI_Q_ITEM("ws_continuation_fin1_with_payload", test_frame_ws_continuation_fin1_with_payload),
    FUZI_Q_ITEM("quic_ncid_large_retire_small_seq", test_frame_quic_ncid_large_retire_small_seq),

    /* New QUIC negative test cases */
    FUZI_Q_ITEM("quic_conn_close_missing_error", test_quic_conn_close_missing_error),
    FUZI_Q_ITEM("quic_ack_bad_range", test_quic_ack_bad_range),
    FUZI_Q_ITEM("quic_reset_zero_error", test_quic_reset_zero_error),
    FUZI_Q_ITEM("quic_crypto_big_offset", test_quic_crypto_big_offset),
    FUZI_Q_ITEM("quic_new_token_empty", test_quic_new_token_empty),
    FUZI_Q_ITEM("quic_stream_id_zero", test_quic_stream_id_zero),
    FUZI_Q_ITEM("quic_max_data_zero", test_quic_max_data_zero),
    FUZI_Q_ITEM("quic_max_streams_huge", test_quic_max_streams_huge),
    FUZI_Q_ITEM("quic_ncid_bad_seq", test_quic_ncid_bad_seq),
    FUZI_Q_ITEM("quic_retire_seq_zero", test_quic_retire_seq_zero),
    FUZI_Q_ITEM("quic_path_challenge_predictable", test_quic_path_challenge_predictable),
    FUZI_Q_ITEM("quic_reserved_frame_type", test_quic_reserved_frame_type),
    FUZI_Q_ITEM("quic_stream_len_mismatch", test_quic_stream_len_mismatch),
    FUZI_Q_ITEM("quic_ack_future", test_quic_ack_future),
    FUZI_Q_ITEM("quic_datagram_bad_len", test_quic_datagram_bad_len),
    FUZI_Q_ITEM("quic_stream_noncanon_varint", test_quic_stream_noncanon_varint),
    FUZI_Q_ITEM("quic_conn_close_bad_frame_ref", test_quic_conn_close_bad_frame_ref),

    /* Additional QUIC Negative Test Cases */
    FUZI_Q_ITEM("quic_crypto_offset_max", test_frame_quic_crypto_offset_max),
    FUZI_Q_ITEM("quic_stream_invalid_final_size", test_frame_quic_stream_invalid_final_size),
    FUZI_Q_ITEM("quic_ack_pkt_overflow", test_frame_quic_ack_pkt_overflow),
    FUZI_Q_ITEM("quic_handshake_done_invalid", test_frame_quic_handshake_done_invalid),
    FUZI_Q_ITEM("quic_multiple_handshake_done", test_frame_quic_multiple_handshake_done),
    FUZI_Q_ITEM("quic_stream_id_maximum", test_frame_quic_stream_id_maximum),
    FUZI_Q_ITEM("quic_ping_invalid_context", test_frame_quic_ping_invalid_context),
    FUZI_Q_ITEM("quic_conn_close_max_err", test_frame_quic_conn_close_max_err),
    FUZI_Q_ITEM("quic_reset_after_fin_violation", test_frame_quic_reset_after_fin_violation),
    FUZI_Q_ITEM("quic_unknown_type_0x40", test_frame_quic_unknown_type_0x40),
    FUZI_Q_ITEM("quic_unknown_type_0x41", test_frame_quic_unknown_type_0x41),
    FUZI_Q_ITEM("quic_unknown_type_0x42", test_frame_quic_unknown_type_0x42),
    FUZI_Q_ITEM("quic_path_response_incorrect", test_frame_quic_path_response_incorrect),
    FUZI_Q_ITEM("quic_stream_flow_violation", test_frame_quic_stream_flow_violation),
    FUZI_Q_ITEM("quic_crypto_0rtt_invalid", test_frame_quic_crypto_0rtt_invalid),
    FUZI_Q_ITEM("quic_ack_0rtt_invalid", test_frame_quic_ack_0rtt_invalid),
    FUZI_Q_ITEM("quic_ncid_flood_seq1", test_frame_quic_ncid_flood_seq1),
    FUZI_Q_ITEM("quic_ncid_flood_seq2", test_frame_quic_ncid_flood_seq2),
    FUZI_Q_ITEM("quic_max_streams_rapid1", test_frame_quic_max_streams_rapid1),
    FUZI_Q_ITEM("quic_max_streams_rapid2", test_frame_quic_max_streams_rapid2),
    FUZI_Q_ITEM("quic_stream_zero_len_explicit", test_frame_quic_stream_zero_len_explicit),
    FUZI_Q_ITEM("quic_path_challenge_replay", test_frame_quic_path_challenge_replay),
    FUZI_Q_ITEM("quic_max_stream_data_decrease", test_frame_quic_max_stream_data_decrease),
    FUZI_Q_ITEM("quic_streams_blocked_invalid_limit", test_frame_quic_streams_blocked_invalid_limit),
    FUZI_Q_ITEM("quic_invalid_varint", test_frame_quic_invalid_varint),
    FUZI_Q_ITEM("quic_ack_malformed", test_frame_quic_ack_malformed),
    FUZI_Q_ITEM("quic_multiple_path_challenge", test_frame_quic_multiple_path_challenge),
    FUZI_Q_ITEM("quic_stream_fragment1", test_frame_quic_stream_fragment1),
    FUZI_Q_ITEM("quic_stream_fragment2", test_frame_quic_stream_fragment2),
    FUZI_Q_ITEM("quic_ack_many_ranges", test_frame_quic_ack_many_ranges),
    FUZI_Q_ITEM("quic_ncid_duplicate", test_frame_quic_ncid_duplicate),
    FUZI_Q_ITEM("quic_ncid_max_len", test_frame_quic_ncid_max_len),
    FUZI_Q_ITEM("quic_reset_flood1", test_frame_quic_reset_flood1),
    FUZI_Q_ITEM("quic_reset_flood2", test_frame_quic_reset_flood2),
    FUZI_Q_ITEM("quic_crypto_overflow", test_frame_quic_crypto_overflow),
    FUZI_Q_ITEM("quic_new_token_overflow", test_frame_quic_new_token_overflow),
    FUZI_Q_ITEM("quic_ack_timing_suspicious", test_frame_quic_ack_timing_suspicious),

    /* END OF JULES ADDED FUZI_Q_ITEM ENTRIES */
};

size_t nb_fuzi_q_frame_list = sizeof(fuzi_q_frame_list) / sizeof(fuzi_q_frames_t);
