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

#ifndef __AREA_H__
#define __AREA_H__

#include "game_object.h"
#include "player.h"
#include "npc.h"

#define MAX_ROOM_EXITS 4

typedef struct room_s {
    char name[MAX_NAME_LEN];
    char description[512];
    int id;
    int area_id;        /* area this room belongs to */

    /* room exit indexes: -1 indicates no exit */ 
    /* n = 0, e = 1, s = 2, w = 3 */
    int exits[MAX_ROOM_EXITS];

    /* area indexes for these exits */
    int exit_areas[MAX_ROOM_EXITS];

    /* generally a room won't have thousands of objects */
    game_object_t *objects;

    /* players in this room */
    player_t *players;
    npc_t *npcs;
} room_t;

typedef struct area_s {
    int id;
    int num_rooms;
    char name[64];
    room_t **rooms;
} area_t;

/* TODO: dynamically allocate based on area metadata */
#define MAX_AREAS 16

extern area_t *area_table[MAX_AREAS];
extern const char *exit_names[];
extern const char *reverse_exit_names[];

int load_area_file(area_t *area, const char *filename);
int room_description(room_t *room, player_t *ch, char *buf);

game_object_t *lookup_room_object(room_t *room, const char *key);
void add_player_to_room(room_t *room, player_t *ch);
void remove_player_from_room(room_t *room, player_t *ch);
void add_npc_to_room(room_t *room, npc_t *npc);
void remove_npc_from_room(room_t *room, npc_t *npc);

void player_room(player_t *ch, room_t **r);
void npc_room(npc_t *npc, room_t **r);

void area_free(area_t *area);
void free_all_areas(void);

#endif
