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

#ifndef __SHARED_H__
#define __SHARED_H__

#if HAVE_STDBOOL_H
#include <stdbool.h>
#else
#define bool int
#define false 0
#define true 1
#endif

#include "jansson/jansson.h"

#define MAXBUF 4096

#define MAX_USERNAME_LEN 32
#define MAX_NAME_LEN 64

#define MAX_KEYWORDS 16
#define MAX_KEYWORD_LEN 32

enum character_states {
    CHAR_DEAD = 0,
    CHAR_DYING,
    CHAR_SLEEPING,
    CHAR_SITTING,
    CHAR_STANDING,
    CHAR_FIGHTING
};

#define CHAR_DEAD_STR       "laying here dead."
#define CHAR_DYING_STR      "bleeding all over the place."
#define CHAR_SLEEPING_STR   "sleeping here quietly."
#define CHAR_SITTING_STR    "sitting here."
#define CHAR_STANDING_STR   "standing here."
#define CHAR_FIGHTING_STR   "fighting for their life."

void randseed(void);
int randint(int max);

int setnonblock(const int fd);

/* colorized string helpers */
const char *lookup_color_code(char c, char op);
void colorize_string(const char *str, char *writebuf);

/* string helpers */
void strlower(char *str);
char *char_status_string(enum character_states ch_state);

/* JSON helpers */
int json_int_from_obj_key(json_t *, const char *);
char *json_str_from_obj_key(json_t *, const char *);

#endif
