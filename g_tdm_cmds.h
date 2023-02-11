/*===============
Commands Stuff
===============*/

char *TDM_SettingsString (void);
void TDM_Timeout_f (edict_t *ent);
void TDM_PrintPlayers (edict_t *ent);
void TDM_Motd_f (edict_t *ent);
void TDM_OldScores_f (edict_t *ent);
void TDM_CheckSafety(void);
void TDM_ServerDemoRecord_f(edict_t *ent);
void TDM_ServerDemoStop_f(edict_t *ent);

#define COMMAND_SPECLOCK		0x1

