/************************************************************************
*            Atrinik, a Multiplayer Online Role Playing Game            *
*                                                                       *
*    Copyright (C) 2009-2010 Alex Tokar and Atrinik Development Team    *
*                                                                       *
* Fork from Daimonin (Massive Multiplayer Online Role Playing Game)     *
* and Crossfire (Multiplayer game for X-windows).                       *
*                                                                       *
* This program is free software; you can redistribute it and/or modify  *
* it under the terms of the GNU General Public License as published by  *
* the Free Software Foundation; either version 2 of the License, or     *
* (at your option) any later version.                                   *
*                                                                       *
* This program is distributed in the hope that it will be useful,       *
* but WITHOUT ANY WARRANTY; without even the implied warranty of        *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
* GNU General Public License for more details.                          *
*                                                                       *
* You should have received a copy of the GNU General Public License     *
* along with this program; if not, write to the Free Software           *
* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.             *
*                                                                       *
* The author can be reached at admin@atrinik.org                        *
************************************************************************/

/**
 * @file
 * Monster placement for random maps. */

#include <global.h>
#include <random_map.h>
#include <rproto.h>

/**
 * Inserts a monster in the map.
 * Some monsters are multisquare, and these guys require special
 * handling.
 * @param new_obj Monster object to insert. Its x and y fields must be
 * the desired location.
 * @param map Map where to insert to. */
void insert_multisquare_ob_in_map(object *new_obj, mapstruct *map)
{
	int x, y;
	archetype *at;
	object *old_seg;
	object *head;

	/* first insert the head */
	insert_ob_in_map(new_obj, map, new_obj, INS_NO_MERGE | INS_NO_WALK_ON);

	x = new_obj->x;
	y = new_obj->y;
	old_seg = new_obj;
	head = new_obj;

	for (at = new_obj->arch->more; at != NULL; at = at->more)
	{
		object *new_seg = arch_to_object(at);

		new_seg->x = x + at->clone.x;
		new_seg->y = y + at->clone.y;
		new_seg->map = old_seg->map;

		insert_ob_in_map(new_seg, new_seg->map, new_seg, INS_NO_MERGE | INS_NO_WALK_ON);

		new_seg->head = head;
		old_seg->more = new_seg;
		old_seg = new_seg;
	}

	old_seg->more = NULL;
}

/**
 * Place some monsters into the map.
 * @param map Where to put monsters on.
 * @param monsterstyle Monster style. Can be NULL, in which case a random
 * one is used.
 * @param difficulty How difficult the monsters should be, and how many
 * there should be.
 * @param RP Random map parameters. */
void place_monsters(mapstruct *map, char *monsterstyle, int difficulty, RMParms *RP)
{
	mapstruct *style_map = NULL;
	int failed_placements = 0, number_monsters = 0;

	style_map = find_style("/styles/monsterstyles", monsterstyle, difficulty);

	if (style_map == NULL)
	{
		return;
	}

	while (number_monsters < RP->num_monsters && failed_placements < 100)
	{
		object *this_monster = pick_random_object(style_map);
		int x, y, freeindex;

		if (this_monster == NULL)
		{
			return;
		}

		x = RANDOM() % RP->Xsize;
		y = RANDOM() % RP->Ysize;
		freeindex = find_first_free_spot(this_monster->arch, NULL, map, x, y);

		if (freeindex != -1)
		{
			object *new_monster = object_create_clone(this_monster);

			x += freearr_x[freeindex];
			y += freearr_y[freeindex];
			new_monster->x = x;
			new_monster->y = y;
			insert_multisquare_ob_in_map(new_monster, map);
			number_monsters++;
		}
		else
		{
			failed_placements++;
		}
	}
}
