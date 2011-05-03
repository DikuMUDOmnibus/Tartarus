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

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <netinet/tcp.h>

#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <time.h>

#include "events.h"
#include "comm.h"
#include "area.h"
#include "commands.h"

static struct event_base *main_base;
static struct event game_tick, mobile_tick;

enum try_read_result {
    READ_DATA_RECEIVED,
    READ_NO_DATA_RECEIVED,
    READ_ERROR,
    READ_MEMORY_ERROR
};

/* forward declarations */
static int handle_input(player_t *c);
static enum try_read_result try_read_network(player_t *c);

static void ev_game_tick(const int fd, const short which, void *arg) {
    /* update game state every X seconds */
    struct timeval t = {.tv_sec = 120, .tv_usec = 0};
    static bool initialized = false;

    if (initialized) {
        evtimer_del(&game_tick);
    } else {
        initialized = true;
    }

    evtimer_set(&game_tick, ev_game_tick, 0);
    event_base_set(main_base, &game_tick);
    evtimer_add(&game_tick, &t);
    send_to_all("\n&RSuper-heated air from the river of fire envelops you.\n");
}

static void ev_mobile_tick(const int fd, const short which, void *arg) {
    /* update mobiles every N seconds */
    struct timeval t = {.tv_sec = 5, .tv_usec = 0};
    static bool initialized = false;

    if (initialized) {
        evtimer_del(&game_tick);
    } else {
        initialized = true;
    }

    evtimer_set(&mobile_tick, ev_mobile_tick, 0);
    event_base_set(main_base, &mobile_tick);
    evtimer_add(&mobile_tick, &t);

    int i, j, dir = -1;
    int bfrom = 0, bto = 0;
    char frombuf[MAXBUF], tobuf[MAXBUF];

    npc_t *npc;
    room_t *from_room, *to_room;

    for (i = 0; i < MAX_NPCS; ++i) {
        npc = npc_table[i];
        if (npc && npc->is_mobile) {
            npc_room(npc, &from_room);

            for (j = 0; j < 4; ++j) {
                /* only try to move at most 4 times */
                dir = randint(4);
                if (from_room->exits[dir] != -1)
                    break;
                else
                    dir = -1;
            }

            if (dir != -1) {
                /* npc can move */
                const char *todir = exit_names[dir];
                const char *fromdir = reverse_exit_names[dir];
                memset(tobuf, 0, MAXBUF);
                memset(frombuf, 0, MAXBUF);

                /* buffer writes until loops are complete */
                remove_npc_from_room(from_room, npc);
                bto = snprintf(tobuf, MAXBUF,
                               "\n%s leaves to the %s.\n", npc->name, todir);

                npc->area_id = from_room->exit_areas[dir];
                npc->room_id = from_room->exits[dir];

                to_room = area_table[npc->area_id]->rooms[npc->room_id];

                /* buffer writes until loops are complete */
                add_npc_to_room(to_room, npc);
                bfrom = snprintf(frombuf, MAXBUF,
                                 "\n%s enters from the %s.\n", npc->name, fromdir);
                send_to_room(from_room, tobuf);
                send_to_room(to_room, frombuf);
            }
        }
    }
}

void ev_main_loop(const int sfd, const int port) {
    struct sockaddr_in saddr;
    struct event ev_accept;

    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    saddr.sin_port = htons(port);

    if (bind(sfd, (struct sockaddr *)&saddr, sizeof(saddr)) == -1) {
        perror("bind()");
        close(sfd);
        exit(EXIT_FAILURE);
    }

    if (listen(sfd, 1024) == -1) {
        perror("listen()");
        close(sfd);
        exit(EXIT_FAILURE);
    }

    main_base = event_init();
    players = NULL;

    event_set(&ev_accept, sfd, EV_READ | EV_PERSIST, ev_socket_accept, NULL);
    event_base_set(main_base, &ev_accept);
    event_add(&ev_accept, NULL);

    ev_game_tick(0, 0, 0);
    ev_mobile_tick(0, 0, 0);

    event_base_loop(main_base, 0);
}

void ev_socket_accept(const int fd, const short which, void *arg) {
    int sfd;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    player_t *c;

    if ((sfd = accept(fd, (struct sockaddr *)&addr, &addrlen)) == -1) {
        perror("accept()");
        return;
    }

    if (setnonblock(sfd) == -1) {
        perror("setnonblock()");
        return;
    }

    c = (player_t *)calloc(1, sizeof(player_t));
    if (c == NULL) {
        perror("calloc()");
        return;
    }

    c->sfd = sfd;
    c->conn_state = conn_read;
    c->game_state = game_state_login;
    c->rsize = c->wsize = MAXBUF;
    c->rbytes = 0;
    c->wbytes = 0;
    c->rbuf = (char *)malloc((size_t)c->rsize);
    c->wbuf = (char *)malloc((size_t)c->wsize);

    c->rcurr = c->rbuf;

    if (c->rbuf == 0 || c->wbuf == 0) {
        perror("malloc()");
        client_free(c);
        return;
    }

    c->next = players;
    players = c;

    send_to_char(c, welcome_screen);
    send_to_char(c, "Enter your name: ");

    event_set(&c->event, sfd, EV_WRITE | EV_PERSIST, ev_socket_write, c);
    event_base_set(main_base, &c->event);

    if (event_add(&c->event, 0) == -1) {
        perror("event_add()");
        client_free(c);
        return;
    }
}

void ev_socket_read(const int fd, const short which, void *arg) {
    player_t *c = (player_t *)arg;
    enum try_read_result res;
    char *end, *cont;

    switch (c->conn_state) {
    case conn_waiting:
        if (!update_event(c, EV_READ | EV_PERSIST, ev_socket_read)) {
            client_set_state(c, conn_closing);
            break;
        }

        client_set_state(c, conn_read);
        break;

    case conn_closing:
        close(c->sfd);
        client_free(c);
        break;

    case conn_max_state:
        break;

    case conn_read:
        res = try_read_network(c);

        switch (res) {
        case READ_DATA_RECEIVED:
            client_set_state(c, conn_parse_input);
            break;

        case READ_NO_DATA_RECEIVED:
            client_set_state(c, conn_waiting);
            break;

        case READ_ERROR:
            client_set_state(c, conn_closing);
            break;

        case READ_MEMORY_ERROR:
            client_set_state(c, conn_closing);
            break;
        }

    case conn_parse_input:
        if (c->rbytes == 0) {
            client_set_state(c, conn_waiting);
            break;
        }

        end = memchr(c->rcurr, '\n', c->rbytes);
        if (!end) {
            client_set_state(c, conn_waiting);
            break;
        }

        cont = end+1;
        if ((end - c->rcurr) > 1 && *(end-1) == '\r') {
            --end;
        }

        *end = '\0';

        handle_input(c);
        break;
    }
}

void ev_socket_write(const int fd, const short which, void *arg) {
    /* This function should only manage writing to a single client
     * since it's known to be ready for writing to. */

    char prompt[MAXBUF];
    char writebuf[MAXBUF];

    int len, res, to_write;
    struct sockaddr peer;

    player_t *c = (player_t *)arg;
    assert(c != NULL);

    colorize_string(c->wbuf, writebuf);
    to_write = strlen(writebuf);

    len = 0;
    while (len < to_write) {
        len += write(c->sfd, writebuf, to_write);
    }

    /* Make sure client is still connected */
    memset(&peer, 0, sizeof(struct sockaddr));
    socklen_t peerlen;

    res = getpeername(c->sfd, &peer, &peerlen);
    if (res == -1) {
        c->conn_state = conn_closing;
    }

    if (c->conn_state == conn_closing) {
        /* for quit, you want to disconnect client after writing to the socket one
         * last time */
        close(c->sfd);
        client_free(c);
        return;
    }

    if (c->game_state == game_state_playing) {
        /* Write prompt to socket */
        snprintf(prompt, MAXBUF, "\n> ");

        to_write = strlen(prompt)+1;
        len = 0;
        while (len < to_write) {
            res = write(c->sfd, prompt, to_write);
            if (res < 0) {
                perror("write");
            }
            len += res;
        }
    }

    memset(c->rbuf, 0, c->rsize);
    memset(c->wbuf, 0, c->wsize);
    c->conn_state = conn_read;
    c->rbytes = 0;
    c->wbytes = 0;

    update_event(c, EV_READ | EV_PERSIST, ev_socket_read);
}

bool update_event(player_t *c, const int new_flags,
                  void (*event_handler)(const int, const short, void *)) {
    assert(c != NULL);

    struct event_base *base = c->event.ev_base;

    if (c->ev_flags == new_flags)
        return true;

    if (event_del(&c->event) == -1)
        return false;

    event_set(&c->event, c->sfd, new_flags, event_handler, (void *)c);
    event_base_set(base, &c->event);

    if (event_add(&c->event, 0) == -1) {
        return false;
    }
    return true;
}

/* taken from memcached */
static enum try_read_result try_read_network(player_t *c) {
    enum try_read_result gotdata = READ_NO_DATA_RECEIVED;
    int res;
    int num_allocs = 0;

    assert(c != NULL);

    if (c->rcurr != c->rbuf) {
        if (c->rbytes != 0)
            memmove(c->rbuf, c->rcurr, c->rbytes);
        c->rcurr = c->rbuf;
    }

    while (1) {
        if (c->rbytes >= c->rsize) {
            if (num_allocs == 4)
                return gotdata;
            ++num_allocs;

            char *new_rbuf = realloc(c->rbuf, c->rsize * 2);
            if (!new_rbuf) {
                /* unable to realloc memory.
                ** TODO: write error to wbuf and write to client when ready */
                c->rbytes = 0;
                close(c->sfd);
                client_free(c);
                return READ_MEMORY_ERROR;
            }

            c->rcurr = c->rbuf = new_rbuf;
            c->rsize *= 2;
        }

        int avail = c->rsize - c->rbytes;
        res = read(c->sfd, c->rbuf + c->rbytes, avail);

        if (res > 0) {
            gotdata = READ_DATA_RECEIVED;
            c->rbytes += res;
            if (res == avail)
                continue;       // realloc and read
            else
                break;
        }

        if (res == 0)
            return READ_ERROR;

        if (res == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            return READ_ERROR;
        }
    }

    return gotdata;
}

static int handle_input(player_t *c) {
    switch (c->game_state) {
    case game_state_login:
        if (strlen(c->rbuf) < 32) {
            if (*c->rbuf == '\r') {
                send_to_char(c, "\nEnter your name: ");
                break;
            }

            if (valid_username(c->rbuf) != 0) {
                send_to_char(c, "\nLetters only please.\nEnter your name: ");
                break;
            }

            strlower(c->rbuf);

            int res = load_player_file(c, c->rbuf);
            if (res == -1) {
                /* no pfile */
                memcpy(c->username, c->rbuf, strlen(c->rbuf));
                c->username[strlen(c->rbuf)] = '\0';
                c->area_id = c->room_id = 0;
                c->ch_state = CHAR_STANDING;
                c->inventory = NULL;
                c->equipment = NULL;
                c->keychain = NULL;
                c->weapon = NULL;
                c->armor = 0;
                c->str = 1;
                c->damage = 1;

                if (save_player_file(c) == -1) {
                    close(c->sfd);
                    client_free(c);
                    break;
                }
            }

            memset(c->rbuf, 0, c->rsize);
            c->rbytes = 0;
            c->game_state = game_state_playing;
            c->username[0] = toupper(c->username[0]);

            /* add player to room->players list */
            room_t *room = area_table[c->area_id]->rooms[c->room_id];
            add_player_to_room(room, c);

            char buf[MAXBUF];
            snprintf(buf, MAXBUF, "\n%s has entered Tartarus.\n", c->username);
            send_to_all_except(c, buf);

            send_to_char(c, "&RYou've arrived in Tartarus.\n");
        } else {
            /* TODO: fix this... it's a hack for now */
            close(c->sfd);
            client_free(c);
        }
        break;

    case game_state_playing:
        /* TODO: real size check plz */
        if (strlen(c->rbuf)+2 < c->wsize) {
            return dispatch_command(c, c->rbuf);
        } else {
            strcpy(c->wbuf, "buffer overflow detected\n");
        }
        break;
    }
    return 0;
}
