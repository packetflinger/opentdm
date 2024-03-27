/**
 * Macro stuff
 */

typedef struct {
    const char *symbol;
    const size_t symbol_length; // to avoid mass strlen calls
    const char* (*replacement)(edict_t *ent, size_t *length);
} tdm_macro_t;

const char *TDM_Macro_CurrentAmmo(edict_t *ent, size_t *length);
const char *TDM_Macro_Health (edict_t *ent, size_t *length);
const char *TDM_Macro_Location (edict_t *ent, size_t *length);
const char *TDM_Macro_LongArmor (edict_t *ent, size_t *length);
const char *TDM_Macro_LongWeapon (edict_t *ent, size_t *length);
const char *TDM_Macro_NearByAll (edict_t *ent, size_t *length);
const char *TDM_Macro_NearByTeam (edict_t *ent, size_t *length);
const char *TDM_Macro_RawArmor (edict_t *ent, size_t *length);
const char *TDM_Macro_RawHealth (edict_t *ent, size_t *length);
const char *TDM_Macro_ShortArmor (edict_t *ent, size_t *length);
const char *TDM_Macro_ShortWeapon (edict_t *ent, size_t *length);
const char *TDM_Macro_TeammateAmmo(edict_t *ent, size_t *length);
