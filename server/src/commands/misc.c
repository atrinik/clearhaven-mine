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
 * Handles misc. input request - things like hash table, malloc, maps,
 * who, etc. */

#include <global.h>

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

	new_draw_info_format(NDI_UNIQUE, op, "Current time is: %02ld:%02ld:%02ld.", (sec % 86400) / 3600, (sec % 3600) / 60, sec % 60);
	new_draw_info(NDI_UNIQUE, op, "Path               Pl PlM IM   TO Dif Reset");
#else
	new_draw_info(NDI_UNIQUE, op, "Pl Pl-M IM   TO Dif");
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

		new_draw_info(NDI_UNIQUE, op, buf);
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
		new_draw_info(NDI_UNIQUE, op, "Usage: /roll <times>d<sides>");
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

	new_draw_info(NDI_ORANGE | NDI_UNIQUE, op, buf);
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

	new_draw_info_format(NDI_UNIQUE, op, "Sizeof: object=%ld  player=%ld  map=%ld", (long) sizeof(object), (long) sizeof(player), (long) sizeof(mapstruct));

	dump_mempool_statistics(op, &sum_used, &sum_alloc);

	new_draw_info_format(NDI_UNIQUE, op, "%4d active objects", count_active());

	new_draw_info_format(NDI_UNIQUE, op, "%4d maps allocated:  %8d", nrofmaps, i = (nrofmaps * sizeof(mapstruct)));
	sum_alloc += i;
	sum_used += nrm * sizeof(mapstruct);

	new_draw_info_format(NDI_UNIQUE, op, "%4d maps in memory:  %8d", nrm, mapmem);
	sum_alloc += mapmem;
	sum_used += mapmem;

	new_draw_info_format(NDI_UNIQUE, op, "%4d archetypes:      %8d", anr, i = (anr * sizeof(archetype)));
	sum_alloc += i;
	sum_used += i;

	new_draw_info_format(NDI_UNIQUE, op, "%4d animations:      %8d", anims, i = (anims * sizeof(Fontindex)));
	sum_alloc += i;
	sum_used += i;

	new_draw_info_format(NDI_UNIQUE, op, "%4d spells:          %8d", NROFREALSPELLS, i = (NROFREALSPELLS * sizeof(spell)));
	sum_alloc += i;
	sum_used += i;

	new_draw_info_format(NDI_UNIQUE, op, "%4d treasurelists    %8d", tlnr, i = (tlnr * sizeof(treasurelist)));
	sum_alloc += i;
	sum_used += i;

	new_draw_info_format(NDI_UNIQUE, op, "%4ld treasures        %8d", nroftreasures, i = (nroftreasures * sizeof(treasure)));
	sum_alloc += i;
	sum_used += i;

	new_draw_info_format(NDI_UNIQUE, op, "%4ld artifacts        %8d", nrofartifacts, i = (nrofartifacts * sizeof(artifact)));
	sum_alloc += i;
	sum_used += i;

	new_draw_info_format(NDI_UNIQUE, op, "%4ld artifacts strngs %8d", nrofallowedstr, i = (nrofallowedstr * sizeof(linked_char)));
	sum_alloc += i;
	sum_used += i;

	new_draw_info_format(NDI_UNIQUE, op, "%4d artifactlists    %8d", alnr, i = (alnr * sizeof(artifactlist)));
	sum_alloc += i;
	sum_used += i;

	new_draw_info_format(NDI_UNIQUE, op, "Total space allocated:%8d", sum_alloc);
	new_draw_info_format(NDI_UNIQUE, op, "Total space used:     %8d", sum_used);
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

	new_draw_info_format(NDI_UNIQUE, op, "%s (%s)", m->name, m->path);

	if (QUERY_FLAG(op, FLAG_WIZ))
	{
		new_draw_info_format(NDI_UNIQUE, op, "players: %d difficulty: %d size: %dx%d start: %dx%d", players_on_map(m), MAP_DIFFICULTY(m), MAP_WIDTH(m), MAP_HEIGHT(m), MAP_ENTER_X(m), MAP_ENTER_Y(m));
	}

	if (m->msg)
	{
		new_draw_info(NDI_UNIQUE, op, m->msg);
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

	new_draw_info(NDI_UNIQUE, op, " ");

	wiz = QUERY_FLAG(op, FLAG_WIZ);

	(void) params;

	for (pl = first_player; pl; pl = pl->next)
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
				snprintf(buf, sizeof(buf), "%s (%s) [%s] (#%d)", pl->ob->name, pl->socket.host, pl->ob->map->path, pl->ob->count);
			}
			else
			{
				snprintf(buf, sizeof(buf), "%s the %s %s (lvl %d)", pl->ob->name, sex, pl->ob->race, pl->ob->level);

				if (QUERY_FLAG(pl->ob, FLAG_WIZ))
				{
					strncat(buf, " [WIZ]", sizeof(buf) - strlen(buf) - 1);
				}

				if (pl->afk)
				{
					strncat(buf, " [AFK]", sizeof(buf) - strlen(buf) - 1);
				}

				if (pl->socket.is_bot)
				{
					strncat(buf, " [BOT]", sizeof(buf) - strlen(buf) - 1);
				}
			}

			new_draw_info(NDI_UNIQUE, op, buf);
		}
	}

	new_draw_info_format(NDI_UNIQUE, op, "There %s %d player%s online (%d in login).", ip + il > 1 ? "are" : "is", ip + il, ip + il > 1 ? "s" : "", il);

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
	hiscore_display(op, 25, params);
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
		new_draw_info(NDI_UNIQUE, op, "You can not save on unholy ground.");
	}
	else if (!op->stats.exp)
	{
		new_draw_info(NDI_UNIQUE, op, "To avoid too many unused player accounts, you must get some experience before you can save.");
	}
	else
	{
		if (save_player(op, 1))
		{
			new_draw_info(NDI_UNIQUE, op, "You have been saved.");
		}
		else
		{
			new_draw_info(NDI_UNIQUE, op, "SAVE FAILED!");
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
		new_draw_info(NDI_UNIQUE, op, "You are no longer AFK.");
	}
	else
	{
		CONTR(op)->afk = 1;
		new_draw_info(NDI_UNIQUE, op, "You are now AFK.");
	}

	CONTR(op)->socket.ext_title_flag = 1;

	return 1;
}

/**
 * Command shortcut to /party say.
 * @param op Object requesting this.
 * @param params Command paramaters (the message).
 * @return 1 on success, 0 on failure. */
int command_gsay(object *op, char *params)
{
	char party_params[MAX_BUF];

	params = cleanup_chat_string(params);

	if (!params || *params == '\0')
	{
		return 0;
	}

	strcpy(party_params, "say ");
	strcat(party_params, params);
	command_party(op, party_params);
	return 0;
}

/**
 * Party command, handling things like /party help, /party say, /party
 * leave, etc.
 * @param op Object requesting this.
 * @param params Command parameters.
 * @return Always returns 1. */
int command_party(object *op, char *params)
{
	char buf[MAX_BUF];

	if (params == NULL)
	{
		if (!CONTR(op)->party)
		{
			new_draw_info(NDI_UNIQUE, op, "You are not a member of any party.");
			new_draw_info(NDI_UNIQUE, op, "For help try: /party help");
		}
		else
		{
			new_draw_info_format(NDI_UNIQUE, op, "You are a member of party %s.", CONTR(op)->party->name);
		}

		return 1;
	}

	if (strcmp(params, "help") == 0)
	{
		new_draw_info(NDI_UNIQUE, op, "To form a party type: /party form <partyname>");
		new_draw_info(NDI_UNIQUE, op, "To join a party type: /party join <partyname>");
		new_draw_info(NDI_UNIQUE, op, "If the party has a password, it will prompt you for it.");
		new_draw_info(NDI_UNIQUE, op, "For a list of current parties type: /party list");
		new_draw_info(NDI_UNIQUE, op, "To leave a party type: /party leave");
		new_draw_info(NDI_UNIQUE, op, "To change a password for a party type: /party password <password>");
		new_draw_info(NDI_UNIQUE, op, "There is a 8 character max for password.");
		new_draw_info(NDI_UNIQUE, op, "To talk to party members type: /party say <msg> or /gsay <msg>");
		new_draw_info(NDI_UNIQUE, op, "To see who is in your party: /party who");
		return 1;
	}
	else if (strncmp(params, "say ", 4) == 0)
	{
		if (!CONTR(op)->party)
		{
			new_draw_info(NDI_UNIQUE, op, "You are not a member of any party.");
			return 1;
		}

		params += 4;
		params = cleanup_chat_string(params);

		if (!params || *params == '\0')
		{
			return 1;
		}

		snprintf(buf, sizeof(buf), "[%s] %s says: %s", CONTR(op)->party->name, op->name, params);
		send_party_message(CONTR(op)->party, buf, PARTY_MESSAGE_CHAT, NULL);
		LOG(llevInfo, "CLOG PARTY: %s [%s] >%s<\n", query_name(op, NULL), CONTR(op)->party->name, params);
		return 1;
	}
	else if (strcmp(params, "leave") == 0)
	{
		if (!CONTR(op)->party)
		{
			new_draw_info(NDI_UNIQUE, op, "You are not a member of any party.");
			return 1;
		}

		new_draw_info_format(NDI_UNIQUE, op, "You leave party %s.", CONTR(op)->party->name);
		snprintf(buf, sizeof(buf), "%s leaves party %s.", op->name, CONTR(op)->party->name);
		send_party_message(CONTR(op)->party, buf, PARTY_MESSAGE_STATUS, op);

		remove_party_member(CONTR(op)->party, op);
		return 1;
	}

	return 1;
}

/**
 * The '/whereami' command will display some information about the region
 * the player is located in.
 * @param op Player.
 * @param params Parameters.
 * @return 1. */
int command_whereami(object *op, char *params)
{
	if (!op->map->region)
	{
		new_draw_info(NDI_UNIQUE, op, "You appear to be lost somewhere...");
		return 1;
	}

	(void) params;

	new_draw_info_format(NDI_UNIQUE, op, "You are in %s.\n%s", get_region_longname(op->map->region), get_region_msg(op->map->region));
	return 1;
}

/**
 * Toggle metaserver privacy on/off.
 * @param op Player.
 * @param params Parameters.
 * @return 1. */
int command_ms_privacy(object *op, char *params)
{
	if (CONTR(op)->ms_privacy)
	{
		CONTR(op)->ms_privacy = 0;
		new_draw_info(NDI_UNIQUE, op, "Metaserver privacy turned off.");
	}
	else
	{
		CONTR(op)->ms_privacy = 1;
		new_draw_info(NDI_UNIQUE, op, "Metaserver privacy turned on.");
	}

	(void) params;
	return 1;
}