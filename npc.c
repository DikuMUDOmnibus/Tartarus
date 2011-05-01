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

#include <stdlib.h>
#include <limits.h>

#include "npc.h"
#include "game_object.h"

#define NPC_DATA_DIR "data/npcs"

void npc_free(npc_t *npc) {
    int i;
    game_object_t *obj;

    if (npc) {
        for (i = 0; i < npc->inventory_size; ++i) {
            obj = npc->inventory[i];
            if (obj)
                free(obj);
        }

        free(npc);
    }
}

void free_all_npcs(void) {
    int i;
    for (i = 0; i < MAX_NPCS; ++i) {
        npc_free(npc_table[i]);
    }
}

bool npc_matches_key(const npc_t *npc, const char *key) {
    int i;
    bool res = false;
    if (npc) {
        for (i = 0; i < MAX_KEYWORDS; ++i) {
            if (npc->keywords[i]) {
                if (strncasecmp(npc->keywords[i], key, strlen(key)) == 0) {
                    res = true;
                    break;
                }
            }
        }
    }

    return res;
}

int load_npc_file(npc_t *npc, const char *filename) {
    char path[PATH_MAX];
    int inv_size, i;
    game_object_t *inv_obj;
    json_t *jsp, *inv;
    json_error_t jserror;

    sprintf(path, "%s/%s", NPC_DATA_DIR, filename);
    jsp = json_load_file(path, 0, &jserror);
    if (!jsp)
        return -1;

    sprintf(npc->name, "%s", json_str_from_obj_key(jsp, "name"));
    sprintf(npc->color, "%s", json_str_from_obj_key(jsp, "color"));

    npc->area_id = json_int_from_obj_key(jsp, "area_id");
    npc->room_id = json_int_from_obj_key(jsp, "room_id");
    npc->ch_state = json_int_from_obj_key(jsp, "ch_state");
    npc->is_mobile = json_int_from_obj_key(jsp, "is_mobile");

    npc->maxhp = npc->curhp = json_int_from_obj_key(jsp, "maxhp");
    npc->armor = json_int_from_obj_key(jsp, "armor");

    keywords_from_json(npc->keywords, jsp);

    memset(npc->inventory, 0, sizeof(npc->inventory));

    inv = json_object_get(jsp, "inventory");
    inv_size = json_array_size(inv);
    npc->inventory_size = inv_size;

    for (i = 0; i < inv_size; ++i) {
        json_t *js_inventory = json_array_get(inv, i);
        inv_obj = game_object_from_json(js_inventory);
        npc->inventory[i] = inv_obj;
    }

    json_decref(jsp);
    return 0;
}
