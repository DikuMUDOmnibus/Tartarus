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

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/time.h>

#include "jansson/jansson.h"

#include "shared.h"
#include "color.h"

int randint(int max) {
    struct timeval t;
    float ms;

    gettimeofday(&t, NULL);
    ms = t.tv_sec*1000 + t.tv_usec*0.001;

    srand(ms);
    return rand() % max;
}

int setnonblock(const int fd) {
    int flags;

    flags = fcntl(fd, F_GETFL);
    if (flags < 0 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0)
        return -1;
    return 0;
}

/* colorized string helpers */
const char *lookup_color_code(char c, char op) {
    if (op == '&') {
        /* foreground color */
        switch (c) {
            case 'D': return ANSI_RESET;
            case 'R': return ANSI_RED;
            case 'r': return ANSI_DRED;
            case 'G': return ANSI_GREEN;
            case 'g': return ANSI_DGREEN;
            case 'Y': return ANSI_YELLOW;
            case 'O': return ANSI_ORANGE;
            case 'B': return ANSI_BLUE;
            case 'b': return ANSI_DBLUE;
            case 'P': return ANSI_PINK;
            case 'p': return ANSI_PURPLE;
            case 'C': return ANSI_LBLUE;
            case 'c': return ANSI_CYAN;
            case 'W': return ANSI_WHITE;
            case 'w': return ANSI_GREY;
            case 'z': return ANSI_DGREY;
            case 'x': return ANSI_BLACK;
        }
    } else if (op == '}') {
        /* background color */
        switch (c) {
            case 'x': return BACK_BLACK;
            case 'r': return BACK_DRED;
            case 'g': return BACK_DGREEN;
            case 'O': return BACK_ORANGE;
            case 'b': return BACK_DBLUE;
            case 'p': return BACK_PURPLE;
            case 'c': return BACK_CYAN;
            case 'z': return BACK_GREY;
        }
    }

    return NULL;
}

void colorize_string(const char *str, char *writebuf) {
    /* assumes writebuf is already allocated */

    const char *p, *code;
    const char *next;

    int i;

    p = str;
    next = p;

    for (i = 0; *p != '\0'; ++p) {
        if (*p == '&' || *p == '}') {
            next = ++p;
            if (next) {
                code = lookup_color_code(*next, *(p-1));
                if (code) {
                    /* write code to buffer */
                    i += sprintf(writebuf+i, "%s", code);
                } else {
                    /* write '&' char and whatever comes after it */
                    writebuf[i++] = *(p-1);
                    writebuf[i++] = *next;
                }
            } else {
                writebuf[i++] = *p;
            }
        } else {
            writebuf[i++] = *p;
        }
    }

    /* this is here because every command needs to be reset after colorizing.
     * this is a double ANSI_RESET for 'say' */
    i += sprintf(writebuf+i, "%s", ANSI_RESET);

    writebuf[i] = '\0';
}

/* string helpers */
void strlower(char *str) {
    /* TODO: may want to think about making this function return a new string
     * without modifying the old string */
    int i;
    for (i = 0; i < strlen(str); ++i) {
        str[i] = tolower(str[i]);
    }
}

/* JSON helpers */
int json_int_from_obj_key(json_t *obj, const char *key) {
    json_t *value;
    value = json_object_get(obj, key);
    return json_integer_value(value);
}

char *json_str_from_obj_key(json_t *obj, const char *key) {
    json_t *value;
    value = json_object_get(obj, key);
    return (char *)json_string_value(value);
}
