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
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <assert.h>
#include <signal.h>

#include <getopt.h>

#include "shared.h"
#include "player.h"
#include "npc.h"
#include "comm.h"
#include "events.h"
#include "area.h"
#include "game_object.h"
#include "commands.h"

/* pointer to head of list */
player_t *players;

area_t *area_table[MAX_AREAS];
game_object_t *object_table[MAX_GAME_OBJECTS];

npc_t *npc_table[MAX_NPCS];

char welcome_screen[MAXBUF];

static void daemonize(void);
static int server_socket(int s_family, int s_type);
static int load_welcome_screen(const char *filename);
static void load_areas(void);
static void load_npcs(void);

void room_free(room_t *room) {
    game_object_t *obj, *next;

    if (room) {
        obj = room->objects;
        while (obj) {
            next = obj->next;
            free(obj);
            obj = next;
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
    game_object_t *obj, *next;
    obj = c->inventory;
    while (obj) {
        next = obj->next;
        free(obj);
        obj = next;
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
        free(c);
    }
}

void terminate_process() {
    signal(SIGTERM, terminate_process);

    free_all_areas();
    free_all_npcs();
    exit(EXIT_SUCCESS);
}

void interrupt_process() {
    signal(SIGINT, interrupt_process);

    free_all_areas();
    free_all_npcs();
    exit(EXIT_SUCCESS);
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

static int load_welcome_screen(const char *filename) {
    int nbytes;
    FILE *fp;

    if (NULL == (fp = fopen(filename, "r"))) {
        fprintf(stderr, "Can't open file: %s\n", filename);
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    nbytes = ftell(fp);
    rewind(fp);

    fread(welcome_screen, nbytes, 1, fp);
    fclose(fp);

    welcome_screen[nbytes] = '\0';
    return 0;
}

static void load_areas(void) {
    memset(area_table, 0, sizeof(area_table));
    memset(object_table, 0, sizeof(object_table));

    area_t *area;
    area = (area_t *)malloc(sizeof(area_t));
    load_area_file(area, "default_area.js");
    area_table[area->id] = area;
}

static void load_npcs(void) {
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
}

static void daemonize(void) {
    /* TODO: use pidfile? */
    int si, so, se;

    if (fork() > 0)
        exit(0);

    setsid();
    chdir(".");
    umask(022);

    if (fork() > 0)
        exit(EXIT_SUCCESS);

    si = open("/dev/null", O_RDONLY);
    so = open("/dev/null", O_APPEND);
    se = open("/dev/null", O_APPEND);

    dup2(si, 0);
    dup2(so, 1);
    dup2(se, 2);
}

int main(int argc, char **argv) {
    /* TODO: catch signals like ^C and free memory */

    int sfd, port;
    int c, iopt;
    int daemonize_process;

    static struct option long_opts[] = {
        {"daemon", no_argument,       0, 'd'},
        {"port",   required_argument, 0, 'p'}
    };

    signal(SIGTERM, terminate_process);
    signal(SIGINT, interrupt_process);

    daemonize_process = 0;
    port = 6666;

    while (1) {
        iopt = 0;
        c = getopt_long(argc, argv, "dp:", long_opts, &iopt);

        if (c == -1)
            break;

        switch (c) {
        case 'd':
            daemonize_process = 1;
            break;

        case 'p':
            port = atoi(optarg);
            break;
        }
    }

    /* load welcome screen */
    if (load_welcome_screen("data/welcome.txt") == -1)
        return -1;

    load_areas();
    load_npcs();

    /* initialize command hash table */
    cmd_init();

    sfd = server_socket(AF_INET, SOCK_STREAM);

    if (daemonize_process)
        daemonize();

    randseed();
    ev_main_loop(sfd, port);

    free_all_areas();
    free_all_npcs();
    return 0;
}
