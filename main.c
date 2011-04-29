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

#include <netinet/tcp.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "shared.h"
#include "player.h"
#include "npc.h"
#include "comm.h"
#include "events.h"
#include "area.h"
#include "game_object.h"

/* pointer to head of list */
player_t *players;

area_t *area_table[MAX_AREAS];
game_object_t *object_table[MAX_GAME_OBJECTS];

npc_t *npc_table[MAX_NPCS];

char welcome_screen[MAXBUF];

void room_free(room_t *room) {
    int i;
    if (room) {
        for (i = 0; i < room->num_objects; ++i) {
            free(room->objects[i]);
        }
        free(room);
    }
}

void area_free(area_t *area) {
    if (area) {
        if (area->rooms) {
            int i;
            for (i = 0; i < area->num_rooms; ++i) {
                room_free(area->rooms[i]);
            }
            free(area->rooms);
        }
        free(area);
    }
}

void free_all_areas(void) {
    int i;
    for (i = 0; i < MAX_AREAS; ++i) {
        area_free(area_table[i]);
    }
}

void inventory_free(player_t *c) {
    int i;
    game_object_t *obj;
    for (i = 0; i < c->inventory_size; ++i) {
        obj = c->inventory[i];
        if (obj) {
            free(obj);
        }
    }
}

void client_free(player_t *c) {
    player_t *p, *prev = NULL;
    room_t *room;

    if (c) {
        player_room(c, &room);

        for (p = players; p; prev = p, p = p->next) {
            if (p == c) {
                /* remove pointer from list */
                if (!prev) {
                    /* move head node to next node */
                    players = p->next;
                } else {
                    prev->next = p->next;
                }
                break;
            }
        }

        event_del(&c->event);

        if (c->rbuf)
            free(c->rbuf);
        if (c->wbuf)
            free(c->wbuf);

        remove_player_from_room(room, c);
        inventory_free(c);
        free(c);
    }
}

void client_set_state(player_t *c, enum conn_states state) {
    assert(c != NULL);
    if (state != c->conn_state)
        c->conn_state = state;
}

static int server_socket(int s_family, int s_type) {
    int sfd;
    int flags = 1;
    struct linger ling = {0, 0};

    if ((sfd = socket(s_family, s_type, 0)) == -1) {
        perror("socket()");
        return -1;
    }

    setnonblock(sfd);

    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (void *)&flags, sizeof(flags)) == -1)
        perror("setsockopt()");
    if (setsockopt(sfd, SOL_SOCKET, SO_KEEPALIVE, (void *)&flags, sizeof(flags)) == -1)
        perror("setsockopt()");
    if (setsockopt(sfd, SOL_SOCKET, SO_LINGER, (void *)&ling, sizeof(ling)) == -1)
        perror("setsockopt()");
    if (setsockopt(sfd, IPPROTO_TCP, TCP_NODELAY, (void *)&flags, sizeof(flags)) == -1)
        perror("setsockopt()");

    return sfd;
}

int main(int argc, char **argv) {
    /* TODO: catch signals like ^C and free memory */

    int sfd;

    memset(area_table, 0, sizeof(area_table));
    memset(object_table, 0, sizeof(object_table));

    area_t *area;
    area = (area_t *)malloc(sizeof(area_t));
    load_area_file(area, "default_area.js");
    area_table[area->id] = area;

    /* proof-of-concept */
    npc_t *simple, *mobile;

    simple = (npc_t *)malloc(sizeof(npc_t));
    load_npc_file(simple, "simple.js");

    mobile = (npc_t *)malloc(sizeof(npc_t));
    load_npc_file(mobile, "mobile.js");

    room_t *npc_room = area_table[simple->area_id]->rooms[simple->room_id];
    add_npc_to_room(npc_room, simple);
    npc_table[0] = simple;

    add_npc_to_room(npc_room, mobile);
    npc_table[1] = mobile;

    /* run the block below when running valgrind --leak-check=full --show-reachable=yes
     * to determine memory leaks */

    /*
    free_all_areas();
    free_all_npcs();
    return 0;
    */

    /* load welcome screen */
    int nbytes;
    FILE *fp;

    if (NULL == (fp = fopen("data/welcome.html", "r"))) {
        fprintf(stderr, "Can't open file: %s\n", "data/welcome.html");
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    nbytes = ftell(fp);
    rewind(fp);

    fread(welcome_screen, nbytes, 1, fp);
    fclose(fp);

    welcome_screen[nbytes] = '\0';

    sfd = server_socket(AF_INET, SOCK_STREAM);
    ev_main_loop(sfd);

    //scripting_close();
    free_all_areas();
    free_all_npcs();
    return 0;
}
