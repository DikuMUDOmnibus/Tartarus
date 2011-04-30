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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#include "area.h"
#include "npc.h"
#include "shared.h"

#define AREA_DATA_DIR "data/areas"

/* have to be indexed by exit index in order */
const char *exit_names[] = {"north", "east", "south", "west"};
const char *reverse_exit_names[] = {"south", "west", "north", "east"};

int load_area_file(area_t *area, const char *filename) {
    /* assumes area is malloc'd already */

    char path[PATH_MAX];
    unsigned int num_rooms;
    int i, j;
    room_t *room;

    json_t *jsp, *obj, *rooms_obj;
    json_error_t jserror;

    sprintf(path, "%s/%s", AREA_DATA_DIR, filename);
    jsp = json_load_file(path, 0, &jserror);

    if (!jsp)
        return -1;

    area->id = json_int_from_obj_key(jsp, "id");
    sprintf(area->name, "%s", json_str_from_obj_key(jsp, "name"));

    rooms_obj = json_object_get(jsp, "rooms");
    num_rooms = json_array_size(rooms_obj);

    area->num_rooms = num_rooms;
    area->rooms = (room_t **)malloc(sizeof(room_t) * num_rooms);

    for (i = 0; i < num_rooms; ++i) {
        room = (room_t *)malloc(sizeof(room_t));

        /* room obj at index i */
        json_t *room_array_obj = json_array_get(rooms_obj, i);

        room->id = json_int_from_obj_key(room_array_obj, "id");
        room->area_id = json_int_from_obj_key(room_array_obj, "area_id");
        sprintf(room->name, "%s", json_str_from_obj_key(room_array_obj, "name"));

        obj = json_object_get(room_array_obj, "exits");
        for (j = 0; j < sizeof(room->exits)/sizeof(int); ++j) {
            json_t *exitobj = json_array_get(obj, j);
            room->exits[j] = json_integer_value(exitobj);
        }

        obj = json_object_get(room_array_obj, "exit_areas");
        for (j = 0; j < sizeof(room->exit_areas)/sizeof(int); ++j) {
            json_t *exit_area_obj = json_array_get(obj, j);
            room->exit_areas[j] = json_integer_value(exit_area_obj);
        }

        obj = json_object_get(room_array_obj, "objects");
        int numobjs = json_array_size(obj);

        room->objects = NULL;

        json_t *js_gameobj;
        game_object_t *gameobj;

        for (j = 0; j < numobjs; ++j) {
            js_gameobj = json_array_get(obj, j);
            gameobj = game_object_from_json(js_gameobj);
            gameobj->next = room->objects;
            room->objects = gameobj;
        }

        area->rooms[room->id] = room;
    }

    json_decref(jsp);

    return 0;
}

int room_description(room_t *room, player_t *ch, char *buf) {
    /* returns a description in buf and number of bytes written to buf. */

    int i, j, n;
    int empty = 1;

    if (!room) {
        return -1;
    }

    n = sprintf(buf, "\n%s\nExits: ", room->name);
    for (i = 0, j = 0; i < MAX_ROOM_EXITS; ++i) {
        if (room->exits[i] > -1) {
            if (j > 0 && j < MAX_ROOM_EXITS - 1) {
                n += sprintf(buf+n, ", ");
            }
            n += sprintf(buf+n, "%s", exit_names[i]);
            ++j;
        }
    }

    game_object_t *roomobj;
    /* this is only here to provide enough space for color escape codes */
    char obj_name[MAX_NAME_LEN * 2];

    for (roomobj = room->objects; roomobj; roomobj = roomobj->next) {
        if (empty) {
            /* only show room items string if there are any items */
            empty = 0;
            n += sprintf(buf+n, "\nItems:\n");
        }
        colorize_object_name(roomobj, obj_name);
        n += sprintf(buf+n, "  %s\n", obj_name);
    }

    n += sprintf(buf+n, "\n");

    npc_t *npc;
    for (npc = room->npcs; npc; npc = npc->next_in_room) {
        n += sprintf(buf+n, "%s\n", npc->name);
    }

    player_t *p;
    int no_chars = 1;

    for (p = room->players; p; p = p->next_in_room) {
        if (p != ch) {
            if (no_chars) {
                no_chars = 0;
                n += sprintf(buf+n, "\nPeople in this room:");
            }
            n += sprintf(buf+n, "\n  %s", p->username);
        }
    }

    n += sprintf(buf+n, "\n");

    return n;
}

game_object_t *lookup_room_object(room_t *room, const char *key) {
    game_object_t *obj, *tmp;

    obj = NULL;

    for (tmp = room->objects; tmp; tmp = tmp->next) {
        if (object_matches_key(tmp, key)) {
            obj = tmp;
            break;
        }
    }

    return obj;
}

void add_player_to_room(room_t *room, player_t *ch) {
    ch->next_in_room = room->players;
    room->players = ch;
}

void remove_player_from_room(room_t *room, player_t *ch) {
    player_t *p, *prev = NULL;

    for (p = room->players; p; prev = p, p = p->next_in_room) {
        if (p == ch) {
            /* remove pointer from list */
            if (!prev) {
                /* move head node to next node */
                room->players = p->next_in_room;
            } else {
                prev->next_in_room = p->next_in_room;
            }
            break;
        }
    }
}

void add_npc_to_room(room_t *room, npc_t *npc) {
    npc->next_in_room = room->npcs;
    room->npcs = npc;
}

void remove_npc_from_room(room_t *room, npc_t *npc) {
    npc_t *p, *prev = NULL;

    for (p = room->npcs; p; prev = p, p = p->next_in_room) {
        if (p == npc) {
            if (!prev) {
                room->npcs = p->next_in_room;
            } else {
                prev->next_in_room = p->next_in_room;
            }
            break;
        }
    }
}

void player_room(player_t *ch, room_t **r) {
    *r = area_table[ch->area_id]->rooms[ch->room_id];
}

void npc_room(npc_t *npc, room_t **r) {
    *r = area_table[npc->area_id]->rooms[npc->room_id];
}
