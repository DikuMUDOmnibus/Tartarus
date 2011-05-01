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

#ifndef __GAME_OBJECT_H__
#define __GAME_OBJECT_H__

#include <string.h>

#include "shared.h"
#include "color.h"

#define COMMON_COLOR    ANSI_DGREY
#define LIMITED_COLOR   ANSI_DGREEN
#define RARE_COLOR      ANSI_DRED
#define ELITE_COLOR     ANSI_DBLUE
#define LEGENDARY_COLOR ANSI_PURPLE

#define WEAR_HEAD       (1 << 0)
#define WEAR_CHEST      (1 << 1)
#define WEAR_SHOULDERS  (1 << 2)
#define WEAR_ARMS       (1 << 3)
#define WEAR_LWRIST     (1 << 4)
#define WEAR_RWRIST     (1 << 5)
#define WEAR_LFINGER    (1 << 6)
#define WEAR_RFINGER    (1 << 7)
#define WEAR_WAIST      (1 << 8)
#define WEAR_LEGS       (1 << 9)
#define WEAR_FEET       (1 << 10)

enum object_rarity {
    COMMON = 0,
    LIMITED,
    RARE,
    ELITE,
    LEGENDARY
};

enum object_type {
    WEAPON_TYPE = 0,
    ARMOR_TYPE,
    KEY_TYPE
};

typedef struct game_object_s {
    char name[MAX_NAME_LEN];
    /* TODO: I don't think game objects need IDs simply because they are mapped with
     * pointers and don't need to be indexed/referenced by anything else */
    int id;

    int num_keywords;
    char keywords[MAX_KEYWORD_LEN][MAX_KEYWORDS];

    enum object_rarity rarity;
    enum object_type type;
    unsigned int wear_location;

    /* rooms can have static (non-removable) objects
     * and dynamic (removable) objects */
    bool is_static;

    /* how much armor this object provides */
    int armor;

    /* how much damage this can deal */
    int damage;
    struct game_object_s *next;
} game_object_t;

#define MAX_GAME_OBJECTS 4096

/* Every object in the game is here, but there are copies of each object spread out
 * among players, rooms, mobiles, etc */
extern game_object_t *object_table[MAX_GAME_OBJECTS];

/* load all game objects */
int load_objects_file(const char *filename);

void free_game_objects(void);

/* game_object.c */
bool object_matches_key(const game_object_t *obj, const char *key);
int remove_game_object_from_list(game_object_t **list, game_object_t *obj);
void colorize_object_name(game_object_t *obj, char *writebuf);

int keywords_from_json(char out[MAX_KEYWORD_LEN][MAX_KEYWORDS], json_t *json);
game_object_t *game_object_from_json(json_t *json);
json_t *game_object_to_json(game_object_t *obj);

#endif
