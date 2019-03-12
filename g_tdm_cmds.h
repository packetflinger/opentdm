/*===============
Commands Stuff
===============*/
#include <sys/time.h>

char *TDM_SettingsString (void);
void TDM_Timeout_f (edict_t *ent);
void TDM_PrintPlayers (edict_t *ent);
void TDM_Motd_f (edict_t *ent);
void TDM_OldScores_f (edict_t *ent);
void TDM_CheckSafety(void);

#define COMMAND_SPECLOCK		0x1

