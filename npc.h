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

#ifndef __NPC_H__
#define __NPC_H__

#include "shared.h"
#include "game_object.h"

#define MAX_NPC_INVENTORY 32

typedef struct npc_s {
    char name[MAX_NAME_LEN];
    char color[8];

    int maxhp, curhp;
    int armor;

    int area_id;
    int room_id;

    enum character_states ch_state;

    int num_keywords;
    char keywords[MAX_KEYWORD_LEN][MAX_KEYWORDS];

    int inventory_size;
    game_object_t *inventory[MAX_NPC_INVENTORY];

    /* can this NPC move around to different rooms? */
    bool is_mobile;

    struct npc_s *next_in_room;
} npc_t;

#define MAX_NPCS 1024

extern npc_t *npc_table[MAX_NPCS];

/* loads a single npc from file */
int load_npc_file(npc_t *npc, const char *filename);

/* loads all npcs from file and places them in the proper locations (rooms).
 * also fills the npc_table */
int load_all_npcs(const char *filename);

void npc_free(npc_t *npc);
void free_all_npcs(void);

#endif
