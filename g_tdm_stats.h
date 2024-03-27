/*===============
Stats Stuff
===============*/
//extern teamplayer_t	*current_teamplayers ;
//extern int			num_current_teamplayers;

//extern teamplayer_t	*old_teamplayers ;
//extern int			num_old_teamplayers;

typedef struct {
    float percentage;
    int index;
} statsort_t;

typedef struct {
    int amount;
    int index;
} intsort_t;

void TDM_BeginDamage(void);
char* TDM_BuildItemsStatsString(edict_t *ent, matchinfo_t *m_info,
        teamplayer_t *p_info);
char* TDM_BuildTeamItemsStatsString(edict_t *ent, matchinfo_t *m_info,
        unsigned team);
char* TDM_BuildTeamWeaponsStatsString(edict_t *ent, matchinfo_t *m_info,
        unsigned team);
char* TDM_BuildTopBottomShotsString(edict_t *ent, int team, qboolean top_shots);
char* TDM_BuildWeaponsStatsString(edict_t *ent, matchinfo_t *m_info,
        teamplayer_t *p_info);
void TDM_Damage(edict_t *ent, edict_t *victim, edict_t *inflictor, int damage);
int TDM_DamageSort(void const *a, void const *b);
void TDM_EndDamage(void);
teamplayer_t* TDM_FindTeamplayerForJoinCode(unsigned code);
void TDM_GeneralStats_f(edict_t *ent, matchinfo_t *m_info, teamplayer_t *p_info,
        unsigned team);
teamplayer_t* TDM_GetInfoForPlayer(edict_t *ent, matchinfo_t *matchinfo);
int TDM_GetTeamFromMatchInfo(edict_t *ent, matchinfo_t *matchinfo);
qboolean TDM_IsTrackableItem(edict_t *ent);
void TDM_ItemGrabbed(edict_t *ent, edict_t *player);
void TDM_ItemSpawned(edict_t *ent);
void TDM_ItemsStats_f(edict_t *ent, matchinfo_t *m_info, teamplayer_t *p_info);
void TDM_Killed(edict_t *attacker, edict_t *victim, int mod);
int TDM_PercentageSort(void const *a, void const *b);
void TDM_RemoveStatsLink(edict_t *ent);
void TDM_SetupMatchInfoAndTeamPlayers(void);
void TDM_SetupTeamInfoForPlayer(edict_t *ent, teamplayer_t *info);
qboolean TDM_StatCheatCheck(edict_t *ent, matchinfo_t *info, unsigned team);
void TDM_Stats_f(edict_t *ent, matchinfo_t *m_info);
void TDM_TeamItemsStats_f(edict_t *ent, matchinfo_t *m_info, int team);
void TDM_TeamStats_f(edict_t *ent, matchinfo_t *m_info);
void TDM_TopBottomShots_f(edict_t *ent, qboolean team_stats, qboolean top_shots);
void TDM_WeaponFired(edict_t *ent);






/*
void TDM_Stats_f (edict_t *ent, matchinfo_t *m_info);
void TDM_WeaponsStats_f (edict_t *ent, matchinfo_t *m_info, teamplayer_t *p_info);
void TDM_ItemsStats_f (edict_t *ent, matchinfo_t *m_info, teamplayer_t *p_info);

void TDM_TeamStats_f (edict_t *ent, matchinfo_t *m_info);
void TDM_TeamWeaponsStats_f (edict_t *ent, matchinfo_t *m_info, int team);
void TDM_TeamItemsStats_f (edict_t *ent, matchinfo_t *m_info, int team);

void TDM_TopBottomShots_f (edict_t *ent, qboolean team_stats, qboolean top_shots);

void TDM_RemoveStatsLink (edict_t *ent);
void TDM_SetupMatchInfoAndTeamPlayers (void);
void TDM_WriteStatsString (edict_t *ent, matchinfo_t *m_info);
teamplayer_t *TDM_FindTeamplayerForJoinCode (unsigned code);
teamplayer_t *TDM_GetInfoForPlayer (edict_t *ent, matchinfo_t *matchinfo);
void TDM_SetupTeamInfoForPlayer (edict_t *ent, teamplayer_t *info);
void TDM_Killed (edict_t *attacker, edict_t *victim, int mod);
*/
