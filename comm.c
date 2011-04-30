/*
 * Copyright (c) 2011, David Reynolds <david@alwaysmovefast.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *   3. Neither the name of the Owner nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <string.h>

#include "comm.h"
#include "events.h"
#include "area.h"

void send_to_char(player_t *c, const char *msg) {
    char *cur = c->wbuf + c->wbytes;
    int avail = c->wsize - c->wbytes;
    int to_write = strlen(msg);

    /* TODO: resize buffer? */
    if (to_write < avail) {
        strncpy(cur, msg, to_write);
        c->wbytes += to_write;
    }

    update_event(c, EV_WRITE | EV_PERSIST, ev_socket_write);
}

void send_to_all(const char *msg) {
    player_t *p;
    for (p = players; p; p = p->next) {
        if (p->game_state == game_state_playing) {
            send_to_char(p, msg);
        }
    }
}

void send_to_all_except(player_t *c, const char *msg) {
    player_t *p;
    for (p = players; p; p = p->next) {
        if (p != c) {
            if (p->game_state == game_state_playing) {
                send_to_char(p, msg);
            }
        }
    }
}

void send_to_room_from_char(player_t *c, const char *msg) {
    player_t *p;
    room_t *room;

    room = area_table[c->area_id]->rooms[c->room_id];
    for (p = room->players; p; p = p->next_in_room) {
        if (p != c) {
            if (p->game_state == game_state_playing)
                send_to_char(p, msg);
        }
    }
}

void send_to_room(room_t *room, const char *msg) {
    player_t *p;

    for (p = room->players; p; p = p->next_in_room) {
        if (p->game_state == game_state_playing) {
            send_to_char(p, msg);
        }
    }
}

void send_object_interaction(player_t *c, game_object_t *obj,
                             const char *roomstr, const char *mystr) {
    /* used for take, drop, wear, remove, etc */
    char buf[MAXBUF];
    char pbuf[MAXBUF];
    char objname[MAX_NAME_LEN * 2];

    colorize_object_name(obj, objname);
    snprintf(buf, MAXBUF, roomstr, c->username, objname);
    send_to_room_from_char(c, buf);

    snprintf(pbuf, MAXBUF, mystr, objname);
    send_to_char(c, pbuf);
}
