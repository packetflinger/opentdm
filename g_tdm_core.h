/**
 * TDM core stuff
 */
typedef struct tdm_config_s {
    struct tdm_config_s *next;
    char name[32];
    char description[128];
    vote_t settings;
    unsigned int last_downloaded;
} tdm_config_t;

extern char **tdm_maplist;
extern tdm_download_t tdm_vote_download;

void CountPlayers(void);
void droptofloor(edict_t *ent);
int LookupPlayer(const char *match, edict_t **out, edict_t *ent);
void TDM_AsciiToConsole(char *out, char *in);
void TDM_BeginCountdown(void);
void TDM_BeginIntermission(void);
void TDM_BeginMatch(void);
qboolean TDM_Checkmap(edict_t *ent, const char *mapname);
qboolean TDM_CheckMapExists(const char *mapname);
void TDM_CheckMatchStart(void);
void TDM_CheckTimes(void);
edict_t* TDM_ClosestTeammate(edict_t *ent);
void TDM_CreateMaplist(void);
void TDM_DeleteMVD(char *name);
void TDM_EndIntermission(void);
void TDM_EndMatch(void);
void TDM_Error(const char *fmt, ...);
void TDM_FixDeltaAngles(void);
char* TDM_GetSmartMap(int playercount);
int TDM_GetTeamFromArg(edict_t *ent, const char *value);
void TDM_HandleDownload(tdm_download_t *download, char *buff, int len, int code);
void TDM_Init(void);
qboolean TDM_Is1V1(void);
void TDM_LoadSmartMapLists(void);
const char* TDM_MakeDemoName(edict_t *ent);
char* TDM_MakeServerDemoName(void);
void TDM_MapChanged(void);
void TDM_NagUnreadyPlayers(void);
void TDM_Overtime(void);
int TDM_PingHandicap(int ping);
qboolean TDM_ProcessText(char *buff, int len,
        qboolean (*func)(char*, int, void*), void *param);
void TDM_RandomizeTeams(void);
void TDM_RecordMVD(void);
void TDM_ResetGameState(void);
void TDM_ResetLevel(void);
void TDM_ResetVotableVariables(void);
void TDM_ResumeGame(void);
void TDM_SaveDefaultCvars(void);
char* TDM_ScoreBoardString(edict_t *ent);
const char* TDM_SecsToString(int seconds);
void TDM_ServerDemoStatus(edict_t *ent);
void TDM_SetAllTeamSkins(edict_t *target_to_set_skins_for);
void TDM_SetCaptain(int team, edict_t *ent);
char* TDM_SetColorText(char *buffer);
void TDM_SetFrameTime(void);
void TDM_SetSkins(void);
void TDM_SetTeamSkins(edict_t *cl, edict_t *target_to_set_skins_for);
void TDM_SetupSounds(void);
void TDM_SetupSpawns(void);
void TDM_StopMVD(void);
void TDM_SuddenDeath(void);
void TDM_TeamsChanged(void);
void TDM_UpdateConfigStrings(qboolean forceUpdate);
void TDM_UpdateServerInfo(void);
void TDM_UpdateSpectator(edict_t *ent);
void TDM_UpdateSpectatorsOnEvent(int spec_mode, edict_t *target,
        edict_t *killer);
void TDM_UpdateTeamNames(void);
void TDM_WriteMaplist(edict_t *ent);
void UpdateMatchStatus(void);
void UpdatePlayerTeamMenu(edict_t *ent);
void UpdateTeamMenu(void);
