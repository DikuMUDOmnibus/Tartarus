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
#include <ctype.h>
#include <string.h>

#include "commands.h"

/* forward declarations */
static int do_say(player_t *ch, char *arg);
static int do_north(player_t *ch, char *arg);

/* command hash table */
static int (*command_table[CMD_HASH_SIZE]) (player_t *, char *);

static struct command_s commands[] = {
    {"n", do_north},
    {"north", do_north},
    {"say", do_say}
};

static int do_say(player_t *ch, char *arg) {
    printf("say %s\n", arg);
    return 0;
}

static int do_north(player_t *ch, char *arg) {
    printf("go north\n");
    return 0;
}

static long generateHashValue(const char *name) {
    int i;
    char c;
    long hash;

    hash = 0;
    i = 0;
    for (i = 0; i < strlen(name); ++i) {
        c = tolower(name[i]);
        hash = (hash << 4) ^ (hash >> 28) ^ c;
    }

    /* clamp the hash to CMD_HASH_SIZE-1 */
    hash &= CMD_HASH_SIZE-1;
    return hash;
}

void cmd_init(void) {
    int i, len;
    len = sizeof(commands) / sizeof(struct command_s);

    for (i = 0; i < len; ++i)
        command_table[generateHashValue(commands[i].name)] = commands[i].cmd;
}

int (*cmd_lookup(const char *cmd))(player_t *, char *) {
    return command_table[generateHashValue(cmd)];
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
    return func(c, arg);
}
