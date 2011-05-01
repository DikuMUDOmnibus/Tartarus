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
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#include "player.h"
#include "shared.h"

#define PLAYER_DATA_DIR "data/players"

int valid_username(const char *username) {
    if (!username)
        return -1;

    int i = 0;
    while (username[i] != '\0') {
        if (!isalpha(username[i]))
            return -1;
        ++i;
    }

    return 0;
}

int readfile(const char *filename, char **buffer) {
    /* TODO: Move this */
    FILE *fp;
    char *buf;
    unsigned int nbytes;

    if (NULL == (fp = fopen(filename, "r"))) {
        fprintf(stderr, "Can't open file: %s\n", filename);
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    nbytes = ftell(fp);
    rewind(fp);

    buf = (char *)malloc(nbytes+1);
    if (!buf) {
        fprintf(stderr, "Can't allocate memory for file: %s\n", filename);
        return -1;
    }

    fread(buf, nbytes, 1, fp);
    buf[nbytes] = '\0';
    *buffer = buf;
    return nbytes;
}

int load_player_file(player_t *ch, const char *filename) {
    /* takes a player_t pointer of a connected user and loads their user data */
    char path[PATH_MAX];
    int inv_size, i;
    game_object_t *inv_obj;
    json_t *jsp, *inv, *js_inventory;
    json_error_t jserror;

    sprintf(path, "%s/%s.js", PLAYER_DATA_DIR, filename);
    jsp = json_load_file(path, 0, &jserror);
    if (!jsp) {
        return -1;
    }

    sprintf(ch->username, "%s", json_str_from_obj_key(jsp, "username"));
    ch->area_id = json_int_from_obj_key(jsp, "area_id");
    ch->room_id = json_int_from_obj_key(jsp, "room_id");
    ch->ch_state = json_int_from_obj_key(jsp, "ch_state");
    ch->armor = json_int_from_obj_key(jsp, "armor");

    ch->inventory = NULL;
    ch->equipment = NULL;

    inv = json_object_get(jsp, "inventory");
    inv_size = json_array_size(inv);
    for (i = 0; i < inv_size; ++i) {
        js_inventory = json_array_get(inv, i);
        inv_obj = game_object_from_json(js_inventory);
        inv_obj->next = ch->inventory;
        ch->inventory = inv_obj;
    }

    inv = json_object_get(jsp, "equipment");
    inv_size = json_array_size(inv);
    for (i = 0; i < inv_size; ++i) {
        js_inventory = json_array_get(inv, i);
        inv_obj = game_object_from_json(js_inventory);
        inv_obj->next = ch->equipment;
        ch->equipment = inv_obj;
        ch->wearing |= inv_obj->wear_location;
    }

    ch->weapon = NULL;
    js_inventory = json_object_get(jsp, "weapon");
    if (js_inventory) {
        inv_obj = game_object_from_json(js_inventory);
        ch->weapon = inv_obj;
    }

    json_decref(jsp);
    return 0;
}

static char *player_json(player_t *ch) {
    /* this function returns the JSON representation of the player
     * so it can be saved at a later time.
     * - http://www.digip.org/jansson/doc/1.0/apiref.html */

    char *res;
    json_t *jsp, *val, *arr, *jsobj;
    game_object_t *obj;

    jsp = json_object();

    /* TODO: Find a better way to save player data without having to have blocks
     * like below for _every single_ property */
    val = json_string(ch->username);
    json_object_set(jsp, "username", val);
    json_decref(val);

    val = json_integer(ch->area_id);
    json_object_set(jsp, "area_id", val);
    json_decref(val);

    val = json_integer(ch->room_id);
    json_object_set(jsp, "room_id", val);
    json_decref(val);

    val = json_integer(ch->ch_state);
    json_object_set(jsp, "ch_state", val);
    json_decref(val);

    val = json_integer(ch->armor);
    json_object_set(jsp, "armor", val);
    json_decref(val);

    arr = json_array();
    for (obj = ch->inventory; obj; obj = obj->next) {
        jsobj = game_object_to_json(obj);
        json_array_append(arr, jsobj);
        json_decref(jsobj);
    }

    json_object_set(jsp, "inventory", arr);
    json_decref(arr);

    arr = json_array();
    for (obj = ch->equipment; obj; obj = obj->next) {
        jsobj = game_object_to_json(obj);
        json_array_append(arr, jsobj);
        json_decref(jsobj);
    }

    json_object_set(jsp, "equipment", arr);
    json_decref(arr);

    if (ch->weapon) {
        jsobj = game_object_to_json(ch->weapon);
        json_object_set(jsp, "weapon", jsobj);
        json_decref(jsobj);
    }

    res = json_dumps(jsp, JSON_INDENT(4));
    json_decref(jsp);

    return res;
}

int save_player_file(player_t *c) {
    /* perform an atomic write by writing to a tmpfile and mv'ing it
     * to the proper save location if everything goes well. */
    char *tmp, *buf;
    char path[PATH_MAX];
    int fd, len, nbytes;

    buf = player_json(c);

    tmp = (char *)malloc(strlen(c->username)+7);
    sprintf(tmp, "%sXXXXXX", c->username);

    /* actual save path */
    snprintf(path, PATH_MAX, "%s/%s.js", PLAYER_DATA_DIR, c->username);
    strlower(path);

    fd = mkstemp(tmp);
    if (fd == -1) {
        perror("mkstemp");
        return -1;
    }

    len = strlen(buf);
    nbytes = write(fd, buf, len);
    close(fd);

    if (nbytes != len)
        return -1;

    rename(tmp, path);
    unlink(tmp);
    free(tmp);

    return 0;
}

static game_object_t *lookup_object(game_object_t *list, const char *key) {
    game_object_t *obj, *tmp;

    obj = NULL;

    for (tmp = list; tmp; tmp = tmp->next) {
        if (object_matches_key(tmp, key)) {
            obj = tmp;
            break;
        }
    }

    return obj;
}

game_object_t *lookup_inventory_object(player_t *c, const char *key) {
    return lookup_object(c->inventory, key);
}

game_object_t *lookup_equipped_object(player_t *c, const char *key) {
    /* try weapon first */
    if (c->weapon) {
        if (object_matches_key(c->weapon, key))
            return c->weapon;
    }

    return lookup_object(c->equipment, key);
}
