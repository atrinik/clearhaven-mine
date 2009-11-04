/************************************************************************
*            Atrinik, a Multiplayer Online Role Playing Game            *
*                                                                       *
*                     Copyright (C) 2009 Alex Tokar                     *
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
 * This file contains animation related code. */

#include <global.h>
#include <stdio.h>

/**
 * Free all animations loaded */
void free_all_anim()
{
	int i;

	for (i = 0; i <= num_animations; i++)
	{
		FREE_AND_CLEAR_HASH(animations[i].name);
		free(animations[i].faces);
	}

	free(animations);
}

/**
 * Initialize animations structure, read the animations
 * data from a file. */
void init_anim()
{
	char buf[MAX_BUF];
	FILE *fp;
	static int anim_init = 0;
	int num_frames = 0, faces[MAX_ANIMATIONS], i;

	if (anim_init)
		return;

	animations_allocated = 9;
	num_animations = 0;

	/* Make a default.  New animations start at one, so if something
	 * thinks it is animated but hasn't set the animation_id properly,
	 * it will have a default value that should be pretty obvious. */
	animations = malloc(10 * sizeof(Animations));
	/* set the name so we don't try to dereferance null.
	 * Put # at start so it will be first in alphabetical
	 * order. */
	animations[0].name = NULL;
	FREE_AND_COPY_HASH(animations[0].name, "###none" );
	animations[0].num_animations = 1;
	animations[0].faces = malloc(sizeof(Fontindex));
	animations[0].faces[0] = 0;
	animations[0].facings = 0;

	sprintf(buf, "%s/animations", settings.datadir);
	LOG(llevDebug, "Reading animations from %s...\n", buf);

	if ((fp = fopen(buf, "r")) == NULL)
		LOG(llevError, "ERROR: Can not open animations file Filename=%s\n", buf);

	while (fgets(buf, MAX_BUF - 1, fp) != NULL)
	{
		if (*buf == '#')
			continue;

		/* Kill the newline */
		buf[strlen(buf) - 1] = '\0';

		if (!strncmp(buf, "anim ", 5))
		{
			if (num_frames)
			{
				LOG(llevError, "ERROR: Didn't get a mina before %s\n", buf);
				num_frames = 0;
			}
			num_animations++;

			if (num_animations == animations_allocated)
			{
				animations = realloc(animations, sizeof(Animations) * (animations_allocated + 10));
				animations_allocated += 10;
			}

			animations[num_animations].name = NULL;
			FREE_AND_COPY_HASH(animations[num_animations].name, buf + 5);
			/* for bsearch */
			animations[num_animations].num = num_animations;
			animations[num_animations].facings = 1;
		}
		else if (!strncmp(buf, "mina", 4))
		{
			animations[num_animations].faces = malloc(sizeof(Fontindex) * num_frames);
			for (i = 0; i < num_frames; i++)
				animations[num_animations].faces[i] = faces[i];

			animations[num_animations].num_animations = num_frames;

			if (num_frames % animations[num_animations].facings)
			{
				LOG(llevDebug, "Animation %s frame numbers (%d) is not a multiple of facings (%d)\n", STRING_SAFE(animations[num_animations].name), num_frames, animations[num_animations].facings);
			}

			num_frames = 0;
		}
		else if (!strncmp(buf, "facings", 7))
		{
			if (!(animations[num_animations].facings = atoi(buf + 7)))
			{
				LOG(llevDebug, "Animation %s has 0 facings, line=%s\n", STRING_SAFE(animations[num_animations].name), buf);
				animations[num_animations].facings = 1;
			}

			if (animations[num_animations].facings != 9 && animations[num_animations].facings != 25)
			{
				LOG(llevDebug, "Animation %s has invalid facings parameter (%d - allowed are 9 or 25 only).", STRING_SAFE(animations[num_animations].name), animations[num_animations].facings);
				animations[num_animations].facings = 1;
			}
		}
		else
		{
			if (!(faces[num_frames++] = find_face(buf, 0)))
				LOG(llevBug, "BUG: Could not find face %s for animation %s\n", buf, STRING_SAFE(animations[num_animations].name));
		}
	}

	fclose(fp);

	LOG(llevDebug, "done. (got %d)\n", num_animations);
}

/**
 * Compare two animations.
 *
 * Used for bsearch in find_animation().
 * @param a First animation to compare
 * @param b Second animation to compare
 * @return Return value of strcmp for the animation names */
static int anim_compare(Animations *a, Animations *b)
{
	return strcmp(a->name, b->name);
}

/**
 * Tries to find the animation ID that matches name.
 * @param name Animation name to find
 * @return ID of the animation if found, 0 otherwise (animation 0 is
 * initialized as the 'bug' face). */
int find_animation(char *name)
{
	Animations search, *match;

	search.name = name;

	match = (Animations*)bsearch(&search, animations, (num_animations + 1), sizeof(Animations), (int (*)())anim_compare);

	if (match)
		return match->num;

	LOG(llevBug, "BUG: Unable to find animation %s\n", STRING_SAFE(name));
	return 0;
}

/**
 * Updates the face variable of an object.
 * If the object is the head of a multi object, all objects are animated.
 * @param op Object to animate
 * @param count State count of the animation */
void animate_object(object *op, int count)
{
	int numfacing, numanim;
	/* Max animation state object should be drawn in */
	int max_state;
	/* starting index # to draw from */
	int base_state;
	int	dir;

	numanim = NUM_ANIMATIONS(op);
	numfacing = NUM_FACINGS(op);

	if (!op->animation_id || !numanim || op->head)
	{
#if 0
		/* ONLY activate for active debugging */
		if (op->animation_id)
			LOG(llevBug,"BUG: Object %s (arch %s) lacks animation. (is tail: %s)\n", STRING_OBJ_NAME(op), STRING_OBJ_ARCH_NAME(op), op->head ? "yes" : "no");
#endif
		return;
	}

	/* a animation is not only changed by anim_speed.
	 * If we turn the object by a teleporter for example, the direction & facing can
	 * change - outside the normal animation loop.
	 * We have then to change the frame and not increase the state */
	if ((!QUERY_FLAG(op, FLAG_SLEEP) && !QUERY_FLAG(op, FLAG_PARALYZED)))
		/* ||  (!QUERY_FLAG(op,FLAG_MONSTER) && op->type != PLAYER))  */
		/* only monster & players should be have sleep & paralyze? if not, attach upper line */
		/* increase draw state (of the animation frame) */
		op->state += count;

	if (!count)
	{
		if (op->type == PLAYER)
		{
			/* this should be changed if complexer flags are added */
			if (!CONTR(op)->anim_flags && op->anim_moving_dir == op->anim_moving_dir_last && op->anim_last_facing == op->anim_last_facing_last)
				return;
		}
		else
		{
			/* object needs no update for moving */
			if (op->anim_enemy_dir == op->anim_enemy_dir_last && op->anim_moving_dir == op->anim_moving_dir_last && op->anim_last_facing == op->anim_last_facing_last)
				return;
		}
	}

	dir = op->direction;

	/* If object is turning, then max animation state is half through the
	 * animations.  Otherwise, we can use all the animations. */
	max_state= numanim / numfacing;
	base_state = 0;

	/* 0: "stay" "non direction" face
	 * 1-8: point of the compass the object is facing. */
	if (numfacing == 9)
	{
		base_state = dir * (numanim / 9);
		/* If beyond drawable states, reset */
		if (op->state >= max_state)
			op->state = 0;
	}

	/* thats the new extended animation: base_state is */
	/* 0:     thats the dying anim - "non direction" facing */
	/* 1-8:   guard/stand_still anim frames */
	/* 9-16:  move anim frames */
	/* 17-24: close fight anim frames */
	/* TODO: allow different number of faces in each frame */
	else if (numfacing >= 25)
	{
		if (op->type == PLAYER)
		{
			/*LOG(-1, "ppA: %s fdir:%d mdir:%d ldir:%d (%d %d %d) state:%d\n", op->name, op->anim_enemy_dir, op->anim_moving_dir, op->anim_last_facing, CONTR(op)->anim_flags & PLAYER_AFLAG_ENEMY, CONTR(op)->anim_flags & PLAYER_AFLAG_ADDFRAME, CONTR(op)->anim_flags & PLAYER_AFLAG_FIGHT, op->state); */

			/* lets check flags - perhaps we have hit something in close fight */
			if ((CONTR(op)->anim_flags & PLAYER_AFLAG_ADDFRAME || CONTR(op)->anim_flags & PLAYER_AFLAG_ENEMY) && !(CONTR(op)->anim_flags & PLAYER_AFLAG_FIGHT))
			{
				/* lets do a swing animation, starting at frame 0 */
				op->state = 0;

				if (CONTR(op)->anim_flags & PLAYER_AFLAG_ENEMY)
				{
					/* so we do one more swing */
					CONTR(op)->anim_flags |= PLAYER_AFLAG_ADDFRAME;
					/* so we do one swing */
					CONTR(op)->anim_flags |= PLAYER_AFLAG_FIGHT;
					/* this can perhaps be skipped when we have a clean enemy handling in attack.c */
					/* save this for be sure to skip unneeded animation */
					CONTR(op)->anim_enemy = op->enemy;
					CONTR(op)->anim_enemy_count = op->enemy_count;
				}
				else
				{
					/* only do ADDFRAME if we still fight something */
					if (OBJECT_VALID(op->enemy, CONTR(op)->anim_enemy_count))
						CONTR(op)->anim_flags |= PLAYER_AFLAG_FIGHT;

					/* we do our additional frame*/
					CONTR(op)->anim_flags &=~PLAYER_AFLAG_ADDFRAME;
				}

				/* clear enemy, set fight */
				CONTR(op)->anim_flags &=~PLAYER_AFLAG_ENEMY;
			}

			/* now setup the best animation for our action */
			if (CONTR(op)->anim_flags & PLAYER_AFLAG_FIGHT)
			{
				op->anim_enemy_dir_last = op->anim_enemy_dir;

				/* test of moving when swing */
				if (op->anim_moving_dir != -1)
				{
					/* lets face in moving direction */
					dir = op->anim_moving_dir;
					op->anim_moving_dir_last = op->anim_moving_dir;
				}
				else
				{
					/* lets face to last direction we had done something */
					if (op->anim_enemy_dir != -1)
						dir = op->anim_enemy_dir;
					else
						dir = op->anim_last_facing;
				}

				/* special case, if we have no idea where we fac, we face to enemy */
				if (!dir || dir == -1)
					dir = 4;

				op->anim_last_facing = dir;
				op->anim_last_facing_last = -1;
				dir += 16;
			}
			/* test of moving */
			else if (op->anim_moving_dir != -1)
			{
				/* lets face in moving direction */
				dir = op->anim_moving_dir;
				op->anim_moving_dir_last = op->anim_moving_dir;
				op->anim_enemy_dir_last = -1;

				/* special case, same spot will be mapped to south dir */
				if (!dir)
					dir = 4;

				op->anim_last_facing = dir;
				op->anim_last_facing_last = -1;
				dir += 8;
			}
			/* if nothing to do: object do nothing. use original facing */
			else
			{
				/* lets face to last direction we had done something */
				if (op->anim_enemy_dir != -1)
					dir = op->anim_enemy_dir;
				else
					dir = op->anim_last_facing;

				op->anim_last_facing_last = dir;

				/* special case, same spot will be mapped to south dir */
				if (!dir || dir == -1)
					op->anim_last_facing = dir = 4;
			}

			base_state = dir * (numanim / numfacing);
			/* If beyond drawable states, reset */
			if (op->state >= max_state)
			{
				op->state = 0;
				/* clear always fight flag */
				CONTR(op)->anim_flags &=~PLAYER_AFLAG_FIGHT;
			}
		}
		/* mobs & non player anims */
		else
		{
			/* mob has targeted an enemy and face him. when me move, we strave sidewards */
			/* here is a side effect: scared can be set for a player... */
			if (op->anim_enemy_dir != -1 && (!QUERY_FLAG(op, FLAG_RUN_AWAY) && !QUERY_FLAG(op, FLAG_SCARED)))
			{
				/* lets face to the enemy position */
				dir = op->anim_enemy_dir;
				op->anim_enemy_dir_last = op->anim_enemy_dir;
				op->anim_moving_dir_last = -1;

				/* special case, same spot will be mapped to south dir */
				if (!dir)
					dir = 4;

				op->anim_last_facing = dir;
				op->anim_last_facing_last = -1;
				dir += 16;
			}
			/* test of moving */
			else if (op->anim_moving_dir != -1)
			{
				/* lets face in moving direction */
				dir = op->anim_moving_dir;
				op->anim_moving_dir_last = op->anim_moving_dir;
				op->anim_enemy_dir_last = -1;

				/* special case, same spot will be mapped to south dir */
				if (!dir)
					dir = 4;

				op->anim_last_facing = dir;
				op->anim_last_facing_last = -1;
				dir += 8;
			}
			/* if nothing to do: object do nothing. use original facing */
			else
			{
				/* lets face to last direction we had done something */
				dir = op->anim_last_facing;
				op->anim_last_facing_last = dir;

				/* special case, same spot will be mapped to south dir */
				if (!dir || dir == -1)
					op->anim_last_facing = dir = 4;
			}

			base_state = dir * (numanim / numfacing);
			/* If beyond drawable states, reset */
			if (op->state >= max_state)
				op->state = 0;
		}
	}
	else
	{
		/* If beyond drawable states, reset */
		if (op->state >= max_state)
			op->state = 0;
	}

	/*LOG(-1, "B: %s(%d)::dir:%d face:%d (%d) ->%d (%d)\n", op->name, count, op->direction, op->facing, op->anim_last_facing, base_state, op->state);*/

	SET_ANIMATION(op, op->state + base_state);

	/* this will force a "below windows update" - NOT a map face update.
	 * map faces are updated in the map send command checking the object itself.
	 * disabling this, will remove animations in the below windows, but also not
	 * forcing an update of it every turn, But in one of the next steps, we will
	 * add animation playing to the client and remove it from server. */

	/* I re-enabled this for the time being.
	 * But yeah, it should be coded client-side. Maybe sometime in the future.
	 * TODO: Code this client-side.
	 * -- A.T. 2009 */
	update_object(op, UP_OBJ_FACE);
}
