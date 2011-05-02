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
 *   3. Neither the name of Tartarus nor the names of its contributors
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

#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <event.h>

#include "game_object.h"

enum conn_states {
    conn_read,
//    conn_write,
    conn_parse_input,
    conn_waiting,
    conn_closing,
    conn_max_state
};

enum game_states {
    game_state_login,
    game_state_playing
};

/* arbitrary limit. inventory size will ultimately be determined
 * by weight I think */
#define MAX_INVENTORY 32

typedef struct player_s {
    int sfd;
    int ev_flags;

    enum conn_states conn_state;
    enum game_states game_state;
    enum character_states ch_state;

    struct event event;

    char *rbuf;
    char *rcurr;    // pointer into rbuf; start reading from this point
    int rsize;      // total allocated memory for rbuf
    int rbytes;     // how much data read into rbuf

    char *wbuf;
    int wsize;
    int wbytes;

    struct player_s *next;
    struct player_s *next_in_room;

    /* player data */
    char username[MAX_USERNAME_LEN];
    int area_id;
    int room_id;

    game_object_t *inventory;   /* pointer to head of inventory */
    game_object_t *equipment;   /* worn objects, not in inventory */
    game_object_t *weapon;
    unsigned int wearing;       /* bitfield of wear locations being used */

    int armor;
    int str;
    /* how much damage the player can deal */
    int damage;
} player_t;

extern player_t *players;
extern char welcome_screen[MAXBUF];

/* main.c */
void client_set_state(player_t *c, enum conn_states state);
void client_free(player_t *c);

/* player.c */
int valid_username(const char *username);
int load_player_file(player_t *ch, const char *filename);
int save_player_file(player_t *ch);
game_object_t *lookup_inventory_object(player_t *c, const char *key);
game_object_t *lookup_equipped_object(player_t *c, const char *key);

#endif
