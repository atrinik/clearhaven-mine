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
 * Handles misc. input request - things like hash table, malloc, maps,
 * who, etc. */

#include <global.h>
#include <sproto.h>

/**
 * Show maps information.
 * @param op The object to show the information to. */
void map_info(object *op)
{
	mapstruct *m;
	char buf[MAX_BUF], map_path[MAX_BUF];
	long sec = seconds();

#ifdef MAP_RESET
	LOG(llevSystem, "Current time is: %02ld:%02ld:%02ld.\n", (sec % 86400) / 3600, (sec % 3600) / 60, sec % 60);

	new_draw_info_format(NDI_UNIQUE, 0, op, "Current time is: %02ld:%02ld:%02ld.", (sec % 86400) / 3600, (sec % 3600) / 60, sec % 60);
	new_draw_info(NDI_UNIQUE, 0, op, "Path               Pl PlM IM   TO Dif Reset");
#else
	new_draw_info(NDI_UNIQUE, 0, op, "Pl Pl-M IM   TO Dif");
#endif

	for (m = first_map; m != NULL; m = m->next)
	{
#ifndef MAP_RESET
		if (m->in_memory == MAP_SWAPPED)
		{
			continue;
		}
#endif

		/* Print out the last 18 characters of the map name... */
		if (strlen(m->path) <= 18)
		{
			strcpy(map_path, m->path);
		}
		else
		{
			strcpy(map_path, m->path + strlen(m->path) - 18);
		}

#ifndef MAP_RESET
		sprintf(buf, "%-18.18s %c %2d   %c %4ld %2ld", map_path, m->in_memory ? (m->in_memory == MAP_IN_MEMORY ? 'm' : 's') : 'X', players_on_map(m), m->in_memory, m->timeout, m->difficulty);
#else
		LOG(llevSystem,"%s pom:%d status:%c timeout:%d diff:%d  reset:%02d:%02d:%02d\n", m->path, players_on_map(m), m->in_memory ? (m->in_memory == MAP_IN_MEMORY ? 'm' : 's') : 'X', m->timeout, m->difficulty, (MAP_WHEN_RESET(m) % 86400) / 3600, (MAP_WHEN_RESET(m) % 3600) / 60, MAP_WHEN_RESET(m) % 60);

		sprintf(buf, "%-18.18s %2d   %c %4d %2d  %02d:%02d:%02d", map_path, players_on_map(m), m->in_memory ? (m->in_memory == MAP_IN_MEMORY ? 'm' : 's') : 'X', m->timeout, m->difficulty, (MAP_WHEN_RESET(m) % 86400) / 3600, (MAP_WHEN_RESET(m) % 3600) / 60, MAP_WHEN_RESET(m) % 60);
#endif

		new_draw_info(NDI_UNIQUE, 0, op, buf);
	}
}

/**
 * Show message of the day.
 * @param op The object calling this.
 * @param params Parameters.
 * @return Always returns 1. */
int command_motd(object *op, char *params)
{
	(void) params;

	display_motd(op);
	return 1;
}

/**
 * Command to roll a magical die.
 *
 * Parameters should be XdY where X is how many times to roll the die and
 * Y how many sides should the die have.
 * @param op Object rolling the die.
 * @param params Parameters.
 * @return Always returns 1. */
int command_roll(object *op, char *params)
{
	int times, sides, i;
	char buf[MAX_BUF];

	/* No params, or params not in format of <times>d<sides>. */
	if (params == NULL || !sscanf(params, "%dd%d", &times, &sides))
	{
		new_draw_info(NDI_UNIQUE, 0, op, "Usage: /roll <times>d<sides>");
		return 1;
	}

	/* Make sure times is a valid value. */
	if (times > 10)
	{
		times = 10;
	}
	else if (times <= 0)
	{
		times = 1;
	}

	/* Make sure sides is a valid value. */
	if (sides > 100)
	{
		sides = 100;
	}
	else if (sides <= 0)
	{
		sides = 1;
	}

	snprintf(buf, sizeof(buf), "%s rolls a magical die (%dd%d) and gets: ", op->name, times, sides);

	for (i = 1; i <= times; i++)
	{
		char tmp[MAX_BUF];

		snprintf(tmp, sizeof(tmp), "%d%s", rndm(1, sides), i < times ? ", " : ".");
		strncat(buf, tmp, sizeof(buf) - strlen(buf) - 1);
	}

	new_draw_info(NDI_ORANGE | NDI_UNIQUE, 0, op, buf);
	new_info_map_except(NDI_ORANGE, op->map, op->x, op->y, MAP_INFO_NORMAL, op, op, buf);

	return 1;
}

/**
 * Counts the number of objects on the list of active objects.
 * @return The number of active objects. */
static int count_active()
{
	int i = 0;
	object *tmp = active_objects;

	while (tmp != NULL)
	{
		tmp = tmp->active_next, i++;
	}

	return i;
}

/**
 * Give out of info about memory usage.
 * @param op The object requesting this. */
void malloc_info(object *op)
{
	int players, nrofmaps;
	int nrm = 0, mapmem = 0, anr, anims, sum_alloc = 0, sum_used = 0, i, tlnr, alnr;
	treasurelist *tl;
	player *pl;
	mapstruct *m;
	archetype *at;
	artifactlist *al;

	for (tl = first_treasurelist, tlnr = 0; tl != NULL; tl = tl->next, tlnr++)
	{
	}

	for (al = first_artifactlist, alnr = 0; al != NULL; al = al->next, alnr++)
	{
	}

	for (at = first_archetype, anr = 0, anims = 0; at != NULL; at = at->more == NULL ? at->next : at->more, anr++)
	{
	}

	for (i = 1; i < num_animations; i++)
	{
		anims += animations[i].num_animations;
	}

	for (pl = first_player, players = 0; pl != NULL; pl = pl->next, players++)
	{
	}

	for (m = first_map, nrofmaps = 0; m != NULL; m = m->next, nrofmaps++)
	{
		if (m->in_memory == MAP_IN_MEMORY)
		{
			mapmem += MAP_WIDTH(m) * MAP_HEIGHT(m) * (sizeof(object *) + sizeof(MapSpace));
			nrm++;
		}
	}

	new_draw_info_format(NDI_UNIQUE, 0, op, "Sizeof: object=%ld  player=%ld  map=%ld", (long) sizeof(object), (long) sizeof(player), (long) sizeof(mapstruct));

	dump_mempool_statistics(op, &sum_used, &sum_alloc);

	new_draw_info_format(NDI_UNIQUE, 0, op, "%4d active objects", count_active());

	new_draw_info_format(NDI_UNIQUE, 0, op, "%4d maps allocated:  %8d", nrofmaps, i = (nrofmaps * sizeof(mapstruct)));
	sum_alloc += i;
	sum_used += nrm * sizeof(mapstruct);

	new_draw_info_format(NDI_UNIQUE, 0, op, "%4d maps in memory:  %8d", nrm, mapmem);
	sum_alloc += mapmem;
	sum_used += mapmem;

	new_draw_info_format(NDI_UNIQUE, 0, op, "%4d archetypes:      %8d", anr, i = (anr * sizeof(archetype)));
	sum_alloc += i;
	sum_used += i;

	new_draw_info_format(NDI_UNIQUE, 0, op, "%4d animations:      %8d", anims, i = (anims * sizeof(Fontindex)));
	sum_alloc += i;
	sum_used += i;

	new_draw_info_format(NDI_UNIQUE, 0, op, "%4d spells:          %8d", NROFREALSPELLS, i = (NROFREALSPELLS * sizeof(spell)));
	sum_alloc += i;
	sum_used += i;

	new_draw_info_format(NDI_UNIQUE, 0, op, "%4d treasurelists    %8d", tlnr, i = (tlnr * sizeof(treasurelist)));
	sum_alloc += i;
	sum_used += i;

	new_draw_info_format(NDI_UNIQUE, 0, op, "%4ld treasures        %8d", nroftreasures, i = (nroftreasures * sizeof(treasure)));
	sum_alloc += i;
	sum_used += i;

	new_draw_info_format(NDI_UNIQUE, 0, op, "%4ld artifacts        %8d", nrofartifacts, i = (nrofartifacts * sizeof(artifact)));
	sum_alloc += i;
	sum_used += i;

	new_draw_info_format(NDI_UNIQUE, 0, op, "%4ld artifacts strngs %8d", nrofallowedstr, i = (nrofallowedstr * sizeof(linked_char)));
	sum_alloc += i;
	sum_used += i;

	new_draw_info_format(NDI_UNIQUE, 0, op, "%4d artifactlists    %8d", alnr, i = (alnr * sizeof(artifactlist)));
	sum_alloc += i;
	sum_used += i;

	new_draw_info_format(NDI_UNIQUE, 0, op, "Total space allocated:%8d", sum_alloc);
	new_draw_info_format(NDI_UNIQUE, 0, op, "Total space used:     %8d", sum_used);
}

/**
 * Give out some info about the map op is located at.
 * @param op The object requesting this information. */
static void current_map_info(object *op)
{
	mapstruct *m = op->map;

	if (!m)
	{
		return;
	}

	new_draw_info_format(NDI_UNIQUE, 0, op, "%s (%s)", m->name, m->path);

	if (QUERY_FLAG(op, FLAG_WIZ))
	{
		new_draw_info_format(NDI_UNIQUE, 0, op, "players: %d difficulty: %d size: %dx%d start: %dx%d", players_on_map(m), MAP_DIFFICULTY(m), MAP_WIDTH(m), MAP_HEIGHT(m), MAP_ENTER_X(m), MAP_ENTER_Y(m));
	}

	if (m->msg)
	{
		new_draw_info(NDI_UNIQUE, NDI_NAVY, op, m->msg);
	}
}

/**
 * Print out a list of all logged in players in the game.
 * @param op The object requesting this
 * @param params Command parameters
 * @return Always returns 1.
 * @todo Perhaps make a GUI from this like the party GUI
 * and you would be able to press enter on player name
 * and that would bring up the /tell console? */
int command_who(object *op, char *params)
{
	player *pl;
	int ip = 0, il = 0, wiz;
	char buf[MAX_BUF];

	if (!op)
	{
		return 1;
	}

	wiz = QUERY_FLAG(op, FLAG_WIZ);

	(void) params;

	for (pl = first_player; pl != NULL; pl = pl->next)
	{
		if (pl->dm_stealth && !wiz)
		{
			continue;
		}

		if (!pl->ob->map)
		{
			il++;
			continue;
		}

		ip++;

		if (pl->state == ST_PLAYING)
		{
			char *sex = "neuter";

			if (QUERY_FLAG(pl->ob, FLAG_IS_MALE))
			{
				sex = QUERY_FLAG(pl->ob, FLAG_IS_FEMALE) ? "hermaphrodite" : "male";
			}
			else if (QUERY_FLAG(pl->ob, FLAG_IS_FEMALE))
			{
				sex = "female";
			}

			if (wiz)
			{
				snprintf(buf, sizeof(buf), "%s the %s %s (@%s) [%s]%s%s (%d)", pl->ob->name, sex, pl->ob->race, pl->socket.host, pl->ob->map->path, QUERY_FLAG(pl->ob, FLAG_WIZ) ? " [WIZ]" : "", pl->afk ? " [AFK]" : "", pl->ob->count);
			}
			else
			{
				snprintf(buf, sizeof(buf), "%s the %s %s (lvl %d)%s%s", pl->ob->name, sex, pl->ob->race, pl->ob->level, QUERY_FLAG(pl->ob, FLAG_WIZ) ? " [WIZ]" : "", pl->afk ? " [AFK]" : "");
			}

			new_draw_info(NDI_UNIQUE, 0, op, buf);
		}
	}

	new_draw_info_format(NDI_UNIQUE, 0, op, "There %s %d player%s online (%d in login).", ip + il > 1 ? "are" : "is", ip + il, ip + il > 1 ? "s" : "", il);

	return 1;
}

/**
 * Map info command.
 * @param op Object requesting this.
 * @param params Command parameters.
 * @return Always returns 1. */
int command_mapinfo(object *op, char *params)
{
	(void) params;

	current_map_info(op);
	return 1;
}

/**
 * Time command. Print out game time information.
 * @param op Object requesting this.
 * @param params Command parameters.
 * @return Always returns 1. */
int command_time(object *op, char *params)
{
	(void) params;

	time_info(op);
	return 1;
}

/**
 * Highscore command, shows the highscore.
 * @param op Object requesting this.
 * @param params Parameters.
 * @return 1. */
int command_hiscore(object *op, char *params)
{
	display_high_score(op, op == NULL ? 9999 : 50, params);
	return 1;
}

/**
 * Output version information.
 * @param op Object requesting this
 * @param params Command parameters
 * @return Always returns 1 */
int command_version(object *op, char *params)
{
	(void) params;

	version(op);

	return 1;
}

/**
 * Pray command, used to start praying to your god.
 * @param op Object requesting this
 * @param params Command parameters
 * @return Always returns 1 */
int command_praying(object *op, char *params)
{
	(void) params;

	CONTR(op)->praying = 1;
	return 1;
}

/**
 * Scans input and returns if this is ON value (1) or OFF (0).
 * @param line The input string.
 * @return 1 for ON, 0 for OFF. */
int onoff_value(char *line)
{
	int i;

	if (sscanf(line, "%d", &i))
	{
		return (i != 0);
	}

	switch (line[0])
	{
		case 'o':
			switch (line[1])
			{
				/* on */
				case 'n':
					return 1;

				/* o[ff] */
				default:
					return 0;
			}

		/* y[es] */
		case 'y':
		/* k[ylla] */
		case 'k':
		case 's':
		case 'd':
			return 1;

		/* n[o] */
		case 'n':
		/* e[i] */
		case 'e':
		case 'u':
		default:
			return 0;
	}
}

/**
 * Receive a player name, and force the first letter to be uppercase.
 * @param op Object. */
void receive_player_name(object *op)
{
	adjust_player_name(CONTR(op)->write_buf + 1);

	if (!check_name(CONTR(op), CONTR(op)->write_buf + 1))
	{
		get_name(op);
		return;
	}

	FREE_AND_COPY_HASH(op->name, CONTR(op)->write_buf + 1);
#if 0
	new_draw_info(NDI_UNIQUE, 0, op,CONTR(op)->write_buf);
	/* Flag: redraw all stats */
	CONTR(op)->last_value= -1;
#endif

	CONTR(op)->name_changed = 1;
	get_password(op);
}

/**
 * Receive player password.
 * @param op Object. */
void receive_player_password(object *op)
{
	unsigned int pwd_len = strlen(CONTR(op)->write_buf);

	if (pwd_len <= 1 || pwd_len > 17)
	{
		get_name(op);
		return;
	}

	if (CONTR(op)->state == ST_CONFIRM_PASSWORD)
	{
		char cmd_buf[] = "X";

		if (!check_password(CONTR(op)->write_buf + 1, CONTR(op)->password))
		{
			send_socket_message(NDI_RED, &CONTR(op)->socket, "The passwords did not match.");
			get_name(op);
			return;
		}

		esrv_new_player(CONTR(op), 0);
		Write_String_To_Socket(&CONTR(op)->socket, BINARY_CMD_NEW_CHAR, cmd_buf, 1);
		LOG(llevInfo, "NewChar send for %s\n", op->name);
		CONTR(op)->state = ST_ROLL_STAT;

		return;
	}

	strcpy(CONTR(op)->password, crypt_string(CONTR(op)->write_buf + 1, NULL));
	CONTR(op)->state = ST_ROLL_STAT;
	check_login(op);
	return;
}

/**
 * Save command.
 *
 * Cannot be used on unholy ground or if you don't have any experience.
 * @param op Object requesting this.
 * @param params Command parameters.
 * @return Always returns 1. */
int command_save(object *op, char *params)
{
	(void) params;

	if (blocks_cleric(op->map, op->x, op->y))
	{
		new_draw_info(NDI_UNIQUE, 0, op, "You can not save on unholy ground.");
	}
	else if (!op->stats.exp)
	{
		new_draw_info(NDI_UNIQUE, 0, op, "To avoid too many unused player accounts, you must get some experience before you can save.");
	}
	else
	{
		if (save_player(op, 1))
		{
			new_draw_info(NDI_UNIQUE, 0, op, "You have been saved.");
		}
		else
		{
			new_draw_info(NDI_UNIQUE, 0, op, "SAVE FAILED!");
		}
	}

	return 1;
}

/**
 * Away from keyboard command.
 * @param op Object requesting this.
 * @param params Command parameters.
 * @return Always returns 1. */
int command_afk(object *op, char *params)
{
	(void) params;

	if (CONTR(op)->afk)
	{
		CONTR(op)->afk = 0;
		new_draw_info(NDI_UNIQUE, 0, op, "You are no longer AFK.");
	}
	else
	{
		CONTR(op)->afk = 1;
		new_draw_info(NDI_UNIQUE, 0, op, "You are now AFK.");
	}

	CONTR(op)->socket.ext_title_flag = 1;

	return 1;
}
