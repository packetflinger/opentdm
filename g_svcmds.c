/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "g_local.h"
#include "g_svcmds.h"


/*
==============================================================================

PACKET FILTERING
 

You can add or remove addresses from the filter list with:

addip <ip>
removeip <ip>

The ip address is specified in dot format, and any unspecified digits will match any value, so you can specify an entire class C network with "addip 192.246.40".

Removeip will only remove an address specified exactly the same way.  You cannot addip a subnet, then removeip a single host.

listip
Prints the current list of filters.

writeip
Dumps "addip <ip>" commands to listip.cfg so it can be execed at a later date.  The filter lists are not saved and restored by default, because I beleive it would cause too much confusion.

filterban <0 or 1>

If 1 (the default), then ip addresses matching the current list will be prohibited from entering the game.  This is the default setting.

If 0, then only addresses matching the list will be allowed.  This lets you easily set up a private game, or a game that only allows players from your local network.


==============================================================================
*/

ipfilter_t  ipfilters[MAX_IPFILTERS];
unsigned    numipfilters;

/**
 * Convert a string representation of an IP address into an ipfilter_t
 *
 * Allowed input format examples:
 *   192.0.2.5
 *   192.0.2.0/27
 *   2002:db8::b00b:face
 *   2002:db8::b00b:face/64
 */
qboolean StringToFilter(const char *s, ipfilter_t *f, int seconds)
{
    memset(&f->addr, 0, sizeof(netadr_t));
    f->addr = net_parseIPAddressMask(s);

    if (seconds) {
        f->expire = time(NULL) + seconds;
    } else {
        f->expire = -1;
    }

    return true;
}

/*
=================
RemoveIP
=================
Removes IP from the filter list defined by index i in the array.
*/
void RemoveIP (int i)
{
	int			j;

	for (j = i+1; j < numipfilters; j++)
		ipfilters[j-1] = ipfilters[j];

	numipfilters--;
}

/*
==============
TDM_CheckBans
==============
Decrease timeout for timed bans and remove expired ones.
*/
void TDM_CheckBans (void)
{
	unsigned		i;
	unsigned		now;

	now = (unsigned)time(NULL);
	
	for (i = 0; i < numipfilters; i++)
	{
		if (ipfilters[i].expire && ipfilters[i].expire < now)
		{
			RemoveIP (i);
			i--;
		}
	}
}

/*
=================
SV_FilterPacket
=================
*/
qboolean SV_FilterPacket (netadr_t *addr)
{
	int			i;
	TDM_CheckBans ();
	for (i = 0; i < numipfilters; i++) {
	    if (net_contains(&ipfilters[i].addr, addr)) {
			return (int)filterban->value;
	    }
	}
	return (int)!filterban->value;
}


/*
=================
SV_AddIP_f
=================
*/
void SVCmd_AddIP_f(edict_t *ent, char *ip, int expiry)
{
    ipfilter_t new_filter;

    if (!ip[0]) {
        gi.cprintf(ent, PRINT_HIGH, "Usage: %s <ip-mask>%s\n",
                ent == NULL ? "addip" : "ban",
                ent == NULL ? "" : " [duration]");
        return;
    }

    TDM_CheckBans();

    // check if list is full
    if (numipfilters == MAX_IPFILTERS) {
        gi.cprintf(ent, PRINT_HIGH, "IP filter list is full\n");
        return;
    }

    // minutes to seconds
    expiry *= 60;

    if (!StringToFilter(ip, &new_filter, expiry)) {
        return;
    }

    ipfilters[numipfilters] = new_filter;
    numipfilters++;
}

/*
=================
SV_RemoveIP_f
=================
*/
void SVCmd_RemoveIP_f (edict_t *ent, char *ip)
{
	ipfilter_t	f;
	int			i;

	if (!ip[0])
	{
		// wision: different message for server and ingame admin
		gi.cprintf(ent, PRINT_HIGH, "Usage: %s <ip-mask>\n", ent == NULL ? "sv removeip" : "unban");
		return;
	}

	if (!StringToFilter (ip, &f, 0)) {
		return;
	}

	TDM_CheckBans ();

	for (i = 0; i < numipfilters ;i++) {
		if (f.addr.mask_bits == ipfilters[i].addr.mask_bits && memcmp(&f.addr.ip, &ipfilters[i].addr.ip, sizeof(netadrip_t))) {
			RemoveIP (i);
			gi.cprintf (ent, PRINT_HIGH, "Removed %s.\n", ip);
			return;
		}
	}
	gi.cprintf (ent, PRINT_HIGH, "Didn't find %s.\n", ip);
}

/*
=================
SV_ListIP_f
=================
*/
void SVCmd_ListIP_f (edict_t *ent)
{
	int			i;
	char		value[32];
	unsigned	now;

	TDM_CheckBans ();

	now = (unsigned)time(NULL);

	gi.cprintf (ent, PRINT_HIGH, "Filter list:\n Duration    IP\n");
	for (i = 0; i < numipfilters; i++)
	{
		unsigned	remaining, minutes;

		remaining = ipfilters[i].expire - now;

		minutes = remaining / 60;

		if (ipfilters[i].expire == -1) {
			strcpy (value, "permanent");
		} else {
			sprintf (value, "%d min%s", minutes, minutes == 1 ? "" : "s");
		}
		gi.cprintf (ent, PRINT_HIGH, " %-12s%s\n", value, IPMASK(&ipfilters[i].addr));
	}
}

/*
=================
SV_WriteIP_f
=================
*/
void SVCmd_WriteIP_f (void)
{
	FILE	*f;
	char	name[MAX_OSPATH];
	int i;
	cvar_t	*game;

	game = gi.cvar ("game", "", 0);

	if (!*game->string) {
		sprintf (name, "%s/listip.cfg", GAMEVERSION);
	} else {
		Com_sprintf (name, sizeof(name), "%s/listip.cfg", game->string);
	}

	gi.cprintf (NULL, PRINT_HIGH, "Writing %s.\n", name);

	f = fopen (name, "wb");
	if (!f) {
		gi.cprintf (NULL, PRINT_HIGH, "Couldn't open %s\n", name);
		return;
	}
	
	fprintf(f, "set filterban %d\n", (int)filterban->value);

	TDM_CheckBans ();

	for (i = 0; i < numipfilters; i++) {
		// only write permanent bans to disk
		if (ipfilters[i].expire) {
			continue;
		}
		fprintf (f, "sv addip %s\n", IPMASK(&ipfilters[i].addr));
	}
	
	fclose (f);
}

void Svcmd_Itemlist_f (void)
{
	const gitem_t	*i;
	int				j;

	for (j = 1; j < game.num_items; j++)
	{
		i = &itemlist[j];
		gi.cprintf (NULL, PRINT_HIGH, "ITEM_%s,\n", i->classname);
	}
}

/*
=================
ServerCommand

ServerCommand will be called when an "sv" command is issued.
The game can issue gi.argc() / gi.argv() commands to get the rest
of the parameters
=================
*/
void	ServerCommand (void)
{
	char	*cmd;

	cmd = gi.argv(1);

	if (Q_stricmp (cmd, "itemlist") == 0)
		Svcmd_Itemlist_f ();
	else if (Q_stricmp (cmd, "addip") == 0)
		SVCmd_AddIP_f (NULL, gi.argv(2), atoi(gi.argv(3)));
	else if (Q_stricmp (cmd, "removeip") == 0)
		SVCmd_RemoveIP_f (NULL, gi.argv(2));
	else if (Q_stricmp (cmd, "listip") == 0)
		SVCmd_ListIP_f (NULL);
	else if (Q_stricmp (cmd, "writeip") == 0)
		SVCmd_WriteIP_f ();
	else if (TDM_ServerCommand (cmd))
	{
		//nothing, processed bg g_tdmstuff
		;
	}
	else
		gi.cprintf (NULL, PRINT_HIGH, "Unknown server command \"%s\"\n", cmd);
}
