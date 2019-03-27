/*===============
Client Stuff
===============*/
edict_t *TDM_FindPlayerForTeam(unsigned team);
int TDM_GetPlayerIdView(edict_t *ent);
void TDM_SendPlayerStatusBar(edict_t *ent);
void TDM_SendSpectatorStatusBar(edict_t *ent);
void TDM_DownloadPlayerConfig(edict_t *ent);
void TDM_UpdateHud(edict_t *ent, qboolean force);

#define SPAWN_RANDOM_ON_SMALL_MAPS	4
