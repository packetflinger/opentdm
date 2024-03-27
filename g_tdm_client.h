/**
 * TDM client stuff
 */

#define SPAWN_RANDOM_ON_SMALL_MAPS  4

qboolean CanJoin(edict_t *ent, unsigned team);
void JoinedTeam(edict_t *ent, qboolean reconnected, qboolean notify);
void JoinTeam1(edict_t *ent);
void JoinTeam2(edict_t *ent);
void SelectNextHelpPage(edict_t *ent);
void TDM_AddPlayerToMatchinfo(edict_t *ent);
const char* TDM_CreatePlayerDmStatusBar(edict_t *player);
const char* TDM_CreateSpectatorStatusBar(edict_t *player);
void TDM_Disconnected(edict_t *ent);
void TDM_DownloadPlayerConfig(edict_t *ent);
edict_t* TDM_FindPlayerForTeam(unsigned team);
int TDM_GetArmorValue(edict_t *ent);
int TDM_GetPlayerIdView(edict_t *ent);
void TDM_LeftTeam(edict_t *ent, qboolean notify);
void TDM_MergePlayerConfig(edict_t *ent);
qboolean TDM_ParsePlayerConfigLine(char *line, int line_number, void *param);
void TDM_PlayerConfigDownloaded(tdm_download_t *download, int code, byte *buff,
        int len);
void TDM_PlayerNameChanged(edict_t *ent);
qboolean TDM_ProcessJoinCode(edict_t *ent, unsigned value);
void TDM_SendPlayerStatusBar(edict_t *ent);
void TDM_SendSpectatorStatusBar(edict_t *ent);
void TDM_SetInitialItems(edict_t *ent);
qboolean TDM_SetupClient(edict_t *ent);
void TDM_ShowTeamMenu(edict_t *ent);
void TDM_UpdateHud(edict_t *ent, qboolean force);
void ToggleChaseCam(edict_t *ent);
edict_t *TDM_FindPlayerForTeam(unsigned team);
int TDM_GetPlayerIdView(edict_t *ent);
void TDM_SendPlayerStatusBar(edict_t *ent);
void TDM_SendSpectatorStatusBar(edict_t *ent);
void TDM_DownloadPlayerConfig(edict_t *ent);
void TDM_UpdateHud(edict_t *ent, qboolean force);
