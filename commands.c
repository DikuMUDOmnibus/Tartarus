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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "commands.h"
#include "area.h"
#include "comm.h"

/* forward declarations */
static int has_arg(char **arg);
static int do_what(player_t *ch, char *arg);

static int do_east(player_t *ch, char *arg);
static int do_north(player_t *ch, char *arg);
static int do_south(player_t *ch, char *arg);
static int do_west(player_t *ch, char *arg);

static int do_drop(player_t *ch, char *arg);
static int do_equipment(player_t *ch, char *arg);
static int do_inventory(player_t *ch, char *arg);
static int do_kill(player_t *ch, char *arg);
static int do_look(player_t *ch, char *arg);
static int do_say(player_t *ch, char *arg);
static int do_take(player_t *ch, char *arg);
static int do_use(player_t *ch, char *arg);
static int do_wear(player_t *ch, char *arg);
static int do_remove(player_t *ch, char *arg);

static int do_quit(player_t *ch, char *arg);
static int do_save(player_t *ch, char *arg);

static int (*cmd_lookup(const char *cmd))(player_t *, char *);

/* command hash table */
static int (*command_table[CMD_HASH_SIZE]) (player_t *, char *);

static struct command_s commands[] = {
    {"d", do_drop},
    {"drop", do_drop},

    {"eq", do_equipment},
    {"equipment", do_equipment},

    {"e", do_east},
    {"east", do_east},

    {"i", do_inventory},
    {"inventory", do_inventory},

    {"k", do_kill},
    {"kill", do_kill},

    {"l", do_look},
    {"look", do_look},

    {"n", do_north},
    {"north", do_north},

    {"quit", do_quit},

    {"remove", do_remove},

    {"s", do_south},
    {"south", do_south},
    {"save", do_save},
    {"say", do_say},

    {"t", do_take},
    {"take", do_take},

    {"use", do_use},

    {"wear", do_wear},

    {"w", do_west},
    {"west", do_west}
};

static int has_arg(char **arg) {
    /*
     * check if arg has anymore chars in it for processing
     * since some commands require an arg after the command.
     * returns 0 if no arg, 1 if arg. **arg is required to modify
     * its pointer and return the updated string to caller.
     */
    char *p = *arg;
    while (*p == ' ')
        ++p;
    *arg = p;
    return (p && *p != '\0');
}

static int do_quit(player_t *c, char *arg) {
    char buf[MAXBUF];

    save_player_file(c);

    snprintf(buf, MAXBUF, "\n%s quits.\n", c->username);
    client_set_state(c, conn_closing);
    send_to_char(c, "You quit.\n");
    send_to_all_except(c, buf);
    return 0;
}

static int do_save(player_t *c, char *arg) {
    int res;
    if ((res = save_player_file(c)) == 0)
        send_to_char(c, "Saved.\n");

    return res;
}

static int do_what(player_t *c, char *arg) {
    /* default for input that doesn't resolve to anything */
    send_to_char(c, "What do you want to do?\n");
    return 0;
}

static int do_say(player_t *c, char *arg) {
    char buf[MAXBUF];
    char pbuf[MAXBUF];

    if (!has_arg(&arg)) {
        send_to_char(c, "Say what?\n");
        return -1;
    }

    CHAR_SAYS(buf, c->username, arg);
    send_to_room_from_char(c, buf);
    YOU_SAY(pbuf, arg);
    send_to_char(c, pbuf);
    return 0;
}

static int do_take(player_t *c, char *arg) {
    room_t *room;
    game_object_t *roomobj;

    if (!has_arg(&arg)) {
        send_to_char(c, "What do you want to take?\n");
        return -1;
    }

    room = area_table[c->area_id]->rooms[c->room_id];
    roomobj = lookup_room_object(room, arg);

    if (roomobj && !roomobj->is_static) {
        if (remove_game_object_from_list(&room->objects, roomobj) == -1) {
            printf("wtf?\n");
            return -1;
        }

        roomobj->next = c->inventory;
        c->inventory = roomobj;

        send_object_interaction(c, roomobj, "\n%s takes '%s'\n", "You take '%s'\n");
    } else if (roomobj && roomobj->is_static) {
        send_to_char(c, "You can't take that.\n");
    } else {
        send_to_char(c, "You don't see anything like that.\n");
    }

    return 0;
}

static int do_drop(player_t *c, char *arg) {
    room_t *room;
    game_object_t *userobj;

    if (!has_arg(&arg)) {
        send_to_char(c, "What do you want to drop?\n");
        return -1;
    }

    room = area_table[c->area_id]->rooms[c->room_id];
    userobj = lookup_inventory_object(c, arg);

    if (userobj) {
        if (remove_game_object_from_list(&c->inventory, userobj) == -1) {
            printf("wtf?\n");
            return -1;
        }

        userobj->next = room->objects;
        room->objects = userobj;

        send_object_interaction(c, userobj, "\n%s dropped '%s'\n", "You dropped '%s'\n");
    } else {
        send_to_char(c, "You aren't carrying that.\n");
    }

    return 0;
}

static int do_use(player_t *c, char *arg) {
    game_object_t *invobj;

    if (!has_arg(&arg)) {
        send_to_char(c, "What do you want to use?\n");
        return -1;
    }

    invobj = lookup_inventory_object(c, arg);
    if (invobj && invobj->type == WEAPON_TYPE) {
        if (remove_game_object_from_list(&c->inventory, invobj) == -1) {
            printf("wtf?\n");
            return -1;
        }

        if (c->weapon) {
            /* put old weapon back in inventory */
            c->weapon->next = c->inventory;
            c->inventory = c->weapon;
        }

        invobj->next = NULL;
        c->weapon = invobj;
        c->damage += c->weapon->damage;

        send_object_interaction(c, invobj, "\n%s equipped '%s'\n", "You equipped '%s'\n");
    } else if (invobj && invobj->type != WEAPON_TYPE) {
        send_to_char(c, "You can't use that.\n");
    } else {
        send_to_char(c, "You aren't carrying that.\n");
    }

    return 0;
}

static int do_wear(player_t *c, char *arg) {
    game_object_t *invobj;

    if (!has_arg(&arg)) {
        send_to_char(c, "What do you want to wear?\n");
        return -1;
    }

    invobj = lookup_inventory_object(c, arg);

    if (invobj && invobj->type == ARMOR_TYPE) {
        if ((c->wearing & invobj->wear_location) != 0) {
            /* TODO: better message about location */
            send_to_char(c, "You're already wearing something like that.\n");
            return 0;
        }

        if (remove_game_object_from_list(&c->inventory, invobj) == -1) {
            printf("wtf?\n");
            return -1;
        }

        invobj->next = c->equipment;
        c->equipment = invobj;
        c->wearing |= invobj->wear_location;
        c->armor += invobj->armor;

        send_object_interaction(c, invobj, "\n%s equipped '%s'\n", "You equipped '%s'\n");
    } else if (invobj && invobj->type != ARMOR_TYPE) {
        send_to_char(c, "You can't wear that!\n");
    } else {
        send_to_char(c, "You aren't carrying that.\n");
    }

    return 0;
}

static int do_remove(player_t *c, char *arg) {
    game_object_t *obj, *cur, *prev;

    if (!has_arg(&arg)) {
        send_to_char(c, "What do you want to remove?\n");
        return -1;
    }

    obj = lookup_equipped_object(c, arg);
    cur = prev = NULL;

    if (obj != c->weapon) {
        if (remove_game_object_from_list(&c->equipment, obj) == -1) {
            printf("wtf?\n");
            return -1;
        }

        obj->next = c->inventory;
        c->inventory = obj;
        c->wearing &= ~obj->wear_location;
        c->armor -= obj->armor;

        send_object_interaction(c, obj, "\n%s removed '%s'\n", "You removed '%s'\n");
    } else if (obj && obj == c->weapon) {
        obj->next = c->inventory;
        c->inventory = obj;
        c->weapon = NULL;
        c->damage -= obj->damage;
        send_object_interaction(c, obj, "\n%s removed '%s'\n", "You removed '%s'\n");
    } else {
        send_to_char(c, "You aren't wearing that.\n");
    }

    return 0;
}

static int do_kill(player_t *c, char *arg) {
    char buf[MAXBUF];
    npc_t *npc;
    room_t *room;
    int dam;

    if (!has_arg(&arg)) {
        send_to_char(c, "Who are you trying to kill?\n");
        return -1;
    }

    player_room(c, &room);
    for (npc = room->npcs; npc; npc = npc->next_in_room) {
        if (npc_matches_key(npc, arg))
            break;
    }

    if (npc && npc->ch_state > CHAR_DEAD) {
        dam = npc->armor + npc->curhp;

        /* deal extra damage if the enemy can't fight back */
        if (npc->ch_state == CHAR_DYING || npc->ch_state == CHAR_SLEEPING)
            dam -= c->str * c->damage * 2;
        else
            dam -= c->str * c->damage;

        if (dam <= 0) {
            if (npc->ch_state == CHAR_SLEEPING)
                sprintf(buf, "You murdered %s%s&D in their sleep!\n", npc->color, npc->name);
            else
                sprintf(buf, "You killed %s%s&D!\n", npc->color, npc->name);

            npc->curhp = 0;
            npc->ch_state = CHAR_DEAD;
            send_to_char(c, buf);
        } else {
            sprintf(buf, "%s%s&D says, \"Don't even think about it!\"\n", npc->color, npc->name);
            send_to_char(c, buf);
        }
    } else if (npc && npc->ch_state == CHAR_DEAD) {
        send_to_char(c, "They are already dead.\n");
    } else {
        send_to_char(c, "They aren't here.\n");
    }

    return 0;
}

static int do_look(player_t *c, char *arg) {
    char buf[MAXBUF];
    room_t *room;

    if (!arg || (arg && *arg == '\0')) {
        room = area_table[c->area_id]->rooms[c->room_id];
        room_description(room, c, buf);
        send_to_char(c, buf);
    } else {
        /* TODO: parse arg for something to look at */
        send_to_char(c, "What are you looking at?\n");
    }
    return 0;
}

static int do_inventory(player_t *c, char *arg) {
    /*
     * TODO: verify that bytes < MAXBUF. if bytes >= MAXBUF,
     *       the "best" practice is to malloc more space and use
     *       n to determine how many more bytes are needed.
     */
    int n, bytes, empty;
    char buf[MAXBUF], obj_name[MAX_NAME_LEN * 2];
    game_object_t *obj;

    n = snprintf(buf, MAXBUF, "Inventory:\n");
    bytes = n;
    empty = 1;

    for (obj = c->inventory; obj; obj = obj->next) {
        empty = 0;
        colorize_object_name(obj, obj_name);
        n = snprintf(buf+bytes, MAXBUF-bytes, "  %s\n", obj_name);
        bytes += n;
    }

    if (empty) {
        n = snprintf(buf+bytes, MAXBUF-bytes, "  You aren't carrying anything.\n");
        bytes += n;
    }

    n = snprintf(buf+bytes, MAXBUF-bytes, "\n");
    send_to_char(c, buf);

    return 0;
}

static int do_equipment(player_t *c, char *arg) {
    int n, bytes, empty;
    char buf[MAXBUF], objname[MAX_NAME_LEN * 2];
    game_object_t *obj;

    n = snprintf(buf, MAXBUF, "Equipment:\n");
    bytes = n;
    empty = 1;

    for (obj = c->equipment; obj; obj = obj->next) {
        empty = 0;
        colorize_object_name(obj, objname);
        n = snprintf(buf+bytes, MAXBUF-bytes, "  %s\n", objname);
        bytes += n;
    }

    if (empty) {
        n = snprintf(buf+bytes, MAXBUF-bytes, "  You aren't wearing anything.\n");
        bytes += n;
    }

    n = snprintf(buf+bytes, MAXBUF-bytes, "\n");
    send_to_char(c, buf);

    return 0;
}

static int do_move(player_t *c, int dir) {
    /* called by do_{nesw} */
    char buf[MAXBUF];
    room_t *cur;

    player_room(c, &cur);
    if (cur->exits[dir] == -1) {
        send_to_char(c, "You can't go that way!\n");
        return -1;
    }

    if (cur->locked_exits[dir] == 1) {
        send_to_char(c, "That door is locked.\n");
        return -1;
    }

    const char *todir = exit_names[dir];
    snprintf(buf, MAXBUF, "\n%s leaves to the %s.\n", c->username, todir);
    send_to_room_from_char(c, buf);
    memset(buf, 0, MAXBUF);

    remove_player_from_room(cur, c);

    c->area_id = cur->exit_areas[dir];
    c->room_id = cur->exits[dir];

    player_room(c, &cur);
    add_player_to_room(cur, c);

    const char *fromdir = reverse_exit_names[dir];
    snprintf(buf, MAXBUF, "\n%s enters from the %s.\n", c->username, fromdir);
    send_to_room_from_char(c, buf);

    do_look(c, NULL);
    return 0;
}

static int do_north(player_t *c, char *arg) {
    return do_move(c, 0);
}

static int do_east(player_t *c, char *arg) {
    return do_move(c, 1);
}

static int do_south(player_t *c, char *arg) {
    return do_move(c, 2);
}

static int do_west(player_t *c, char *arg) {
    return do_move(c, 3);
}

static long generateHashValue(const char *name) {
    int i;
    char c;
    long hash;

    hash = 0;
    for (i = 0; i < strlen(name); ++i) {
        c = tolower(name[i]);
        hash = (hash << 4) ^ (hash >> 28) ^ c;
    }

    /* clamp the hash to CMD_HASH_SIZE-1 */
    hash &= CMD_HASH_SIZE-1;
    return hash;
}

static int (*cmd_lookup(const char *cmd))(player_t *, char *) {
    return command_table[generateHashValue(cmd)];
}

void cmd_init(void) {
    int i, len;
    len = sizeof(commands) / sizeof(struct command_s);

    for (i = 0; i < len; ++i)
        command_table[generateHashValue(commands[i].name)] = commands[i].cmd;
}

/* Public API into commands module */
int dispatch_command(player_t *c, char *arg) {
    /* accept `arg` param because sometimes you may not want to use c->rbuf and pass another buf
     * instead */

    /* TODO: Bounds checking! */

    char *p;
    char cmd[32];
    int i;
    int (*func)(player_t *, char *);

    if (!arg)
        return 0;

    while (*arg == ' ')
        ++arg;

    /* start of command */
    p = arg;

    while (*arg != '\0' && *arg != ' ') {
        /* find next space or end of string to determine
         * the command */
        ++arg;
    }

    i = (arg-p);

    strncpy(cmd, p, i);
    cmd[i] = '\0';

    func = cmd_lookup(cmd);

    if (!func)
        return do_what(c, arg);

    return func(c, arg);
}
