/*
 * Copyright (c) 2011, David Reynolds <david@alwaysmovefast.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *   3. Neither the name of the Owner nor the
 *      names of its contributors may be used to endorse or promote products
 *      derived from this software without specific prior written permission.
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
#include <strings.h> /* strncasecmp */

struct player_s {
    char name[32];
};

int do_say(struct player_s *ch, char *arg) {
    printf("say %s\n", arg);
    return 0;
}

int do_north(struct player_s *ch, char *arg) {
    printf("go north\n");
    return 0;
}

#define CMD_HASH_SIZE 1024
static int (*command_table[CMD_HASH_SIZE]) (struct player_s *, char *s);

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

int (*cmd_lookup(const char *cmd)) (struct player_s *, char *) {
    return command_table[generateHashValue(cmd)];
}

int main(int argc, char **argv) {
    int (*cmd)(struct player_s *, char *);
    /* init command hash table */
    command_table[generateHashValue("north")] = do_north;
    command_table[generateHashValue("n")] = do_north;
    command_table[generateHashValue("say")] = do_say;

    cmd = cmd_lookup("north");
    cmd(NULL, "david");
    cmd = cmd_lookup("say");
    cmd(NULL, "david");
    return 0;
}
