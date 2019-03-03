# OpenTDM

OpenTDM is an open source team deathmatch and duel mod for Quake II.

## Client Commands
`commands` Show available commands

`id` Enable player ID of whoever is closest to your crosshair

`accept` Accept a team invite

`admin/referee <password>` Become an admin

`captain` Set/Show team captain

`ready` Once all players are ready the match will start

`notready/unready/noready` Unset your ready status

`talk` Send a message to a specific player

`details/players/playerlist` Show current players and their IDs

`vote` Propose changes for players to vote on

`yes/no` Cast your vote

`menu` Show the OpenTDM menu

`join/team <team>` Join specified team

`settings/matchinfo` Show the current match settings

`observer/obs` Leave your team, joining spectators

`chase [quad/invuln/killer/leader/none]` Enter selected chasecam mode

`overtime` Show overtime settings

`timelimit/tl` Show timelimit

`bfg` Show BFG settings

`powerups` Show powerup settings

`obsmode` Show obsmode settings

`stats/accuracy [playername]` Show your statistics or player of your choice 

`oldstats/oldaccuracy` Show stats from the previous match

`teamstats/teamaccuracy` Show stats for the team

`oldteamstats/oldteamaccuracy` Show stats for the team for the previous match

`rejoin/ghost/restore/recover` Rejoin your team during a match

`tskin [model/skin]` Sets the team skin

`eskin [model/skin]` Sets the enemy team skin

`topshots/bottomshots` Show best/worst accuracy for all weapons from previous match for all players


## Team Captain Commands
`kickplayer/removeplayer/remove <id>` Remove a player from your team

`lock/lockteam` Locks a team, prevents anyone from joining it

`pick/pickplayer <id>` Picks a player to join your team

`invite <id>` Invite a player to join your team

`teamskin [model/skin]` Change your team skin

`teamname <name>` Change your team name

`teamready/readyteam` Force all players on your team to ready

`teamnotready/notreadyteam` For all players on your team to not ready

`time/calltime/pause/ctime` Call a timeout during a match, freezing all players and stopping the clock

`speclock` Lock team against spectators

`specinvite <id>` Allow particular spectators to observe a speclocked team

## Admin/Referee Commands
`acommands` Show admin command list

`overtime/ot <0/1/2> [minutes]` Set the overtime mode

Value | Description
------ | ------
0 | *Tie* - the match ends even if scores are tied
1 | *Overtime* - Adds [minutes] to the clock
2 | *Sudden death* - game ends on the next frag

`timelimit/tl <minutes>` Sets the timelimit

`bfg [0/1]` Turn off/on BFG

`powerups [0/1]` Turn off/on powerups (quad, powershield, etc)

`obsmode [speak/whisper/shutup]`

Value | Description
------ | ------
0 | *Speak* - everyone is able to talk during a match
1 | *Whisper* - players can talk, spectators can talk to each other
2 | *Shutup* - only player can talk, specs are muted

`break` End the current match

`time/hold` Pause the current match

`changemap <mapname>` Change the current map

`mute [id] <minutes>` Mute a player, default time is 5 minutes, 60 minute max

`unmute [id]` Restore a player's ability to speak after being muted

`bans` Show current bans

`readyall/startcountdown` Force all players ready

`notreadyall` For all players not ready


## Server CVARs
`g_admin_password <string>` Admin/referee password

`g_admin_vote_decide <0/1>` Whether admin instantly decides votes

`g_team_a_name <string>` The name for the first team

`g_team_b_name <string>` The name for the second team

`g_locked_names <0/1>` Players can/cannot change team names

`g_team_a_skin <string>` The model/skin used for the first team

`g_team_b_skin <string>` The model/skin used for the second team

`g_locked_skins <0/1>` Players can/cannot change team skins

`g_match_time <integer>` Match time limit in seconds, *default: 600*

`g_match_countdown <integer>` Match countdown time in seconds, *default: 15*

`g_vote_time <integer>` How long votes last in seconds, *default: 30*

`g_intermission_time <float>` How long intermissions lasts (scoreboard after match) in seconds, *default: 5*

`g_force_screenshot <0/1>` Force all players to take a screenshot of the intermission screen, *default: 0*

`g_force_record <0/1>` Force all players to record a demo of the match, *default: 0*

`g_tdm_allow_pick <0/1>`

Value | Description 
------ | -----
0 | *default*, pick acts like invite command
1 | allow direct picking of players to a team, they don't have to accept

`g_tdmflags <integer>` dmflags for tdm mode. Add up the values below

Value | Description
------ | ------
1 | no health
2 | no powerups
4 | weapons stay
8 | no falling damage
16 | instant powerups, they activate on pickup
32 | same map
64 | teams by skin
128 | teams by model
256 | no friendly fire
512 | spawn farthest
1024 | force respawn
2048 | no armor
4096 | allow exit
8192 | infinite ammo
16384 | quad drop
32768 | fixed FOV
 | *default: 1040*

`g_itdmflags <integer>` dmflags for itdm mode, *default: 142427*

`g_1v1flags <integer>` dmflags for 1v1 mode, *default: 1040*

`g_itemflags <integer>` weapons to remove, add em up

Value | Item
------ | ------
2 | shotgun
4 | super shotgun
8 | machinegun
16 | chaingun
32 | grenades
64 | grenade launcher
128 | rocket launcher
256 | railgun
512 | bfg
1024 | hyperblaster
 | *default: 0*

`g_powerupflags <integer>` powerups to remove, add em up

Value | Item
------ | ------
2 | quad damage
4 | invulnerability
8 | powershield
16 | powerscreen
32 | silencer
64 | rebreather
128 | environment suit
 | *default: 0*

`g_gamemode <0/1/2>` The game mode

Value | Description
------ | ------
0 | team deathmatch mode
1 | instagib team deathmatch mode
2 | duel mode
 | *default: 0*

`g_tie_mode <0/1/2>` What to do if a match is tied

Value | Description
------ | ------
0 | tie, end the match anyway
1 | overtime, add time to the clock (g_overtime)
2 | sudden death, next frag wins 
 | *default: 1* 

`g_overtime <integer>` Overtime in seconds if g_tie_mode set to 1, *default: 60*

`g_chat_mode <0/1/2>` How players are allowed to chat

Value | Description
------ | ------
0 | Everyone can chat
1 | Players chat freely, spectators only with other spectators
2 | Only players can talk
 | *default: 0*

`g_teleporter_nofreeze <0/1>`

Value | Description
------ | ------
0 | Default q2 behavior, forward motion is stopped when going through teleporters
1 | Q3 style, continue motion through teleporters
 | *default: 0*

`g_fast_weap_switch <0/1/2/3/4>` how fast weapons switch

Value | Description
------ | ------ 
0 | Normal speed
1 | Fast
2 | Instant
3 | Insane
4 | Extreme
 | *default: 0

`g_max_timeout <integer>` Max length of a timeout in seconds, *default: 300*

`g_respawn_time <integer>` Number of seconds players respawn if force respawn flag is enabled, *default: 5*

`g_idle_time <integer>` Number of seconds an inactive player is removed from a team, *default: 300*

`flood_waves <integer>` Number of taunts to be considered a flood, *default: 6*

`flood_waves_perminute <integer>` Number of taunts per minute to consider a flood, *default: 6*

`flood_waves_waitdelay <integer>` Number of minutes players can't taunt after a flood, *default: 10*

`g_maplistfile <string>` Filename for the maplist, disabled if blank, *default: blank*

`g_bugs <0/1/2>` Controls bug fixes to original q2

Value | Description
------ | ------ 
0 | Fix all bugs 
1 | Enable gameplay bugs like quad-grenades
2 | Enable serious bugs like shooting through walls/corpses 
 | *default: 0 

`g_allow_vote_config <0/1>` Enable voting for config files in the /configs folder, *default: 1*

`g_auto_rejoin_match <0/1>` Players from previous match will/won't auto rejoin the same teams, *default: 1*

`g_auto_rejoin_map <0/1>` Players from previous map will/won't rejoin teams automatically, *default: 1*

`g_motd_message <string>` Sets message of the day

`g_allow_name_change_during_match <0/1>` Players can't/can change names during match, *default: 1*

`g_http_enabled <0/1>` Disables/Enables HTTP support for webconfigs, webstats, *default: 1*

`g_http_bind <string>` IP address to use for HTTP, *default: blank*

`g_http_proxy <string>` IP:port or hostname:port for proxying HTTP requests, *default: blank*

`g_http_baseurl <string>` Url for api calls, *default: http://www.opentdm.net/api*

`g_1v1_spawn_mode <0/1/2>` How spawns are picked in duel mode

Value | Description
------ | ------
0 | 3.20, avoid closest, bugged
1 | 3.20, fixed
2 | true random
 | *default: 1*

`g_tdm_spawn_mode <0/1/2>` How spawns are picked in TDM mode, *default: 1*

`g_vote_mask <integer>` Enable particular vote options, add them up

Value | Description
------ | ------ 
1 | timelimit 
2 | map 
4 | kick 
8 | weapons 
16 | powerups 
32 | gamemode 
64 | *unused*
128 | teleport mode
256 | tiemode
512 | weapon switch mode
1024 | overtime
2048 | local config
4096 | web config
8192 | chat
16384 | match restart
32768 | bugs
65536 | TDM spawn mode
131072 | 1v1 spawn mode
262144 | match abort
 | *default: -1 (all)*

`g_command_mask <integer>` Enable particular commands

Value | Description
------ | ------
1 | speclock
 | *default -1 (all)*

