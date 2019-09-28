/*===============
Voting Stuff
===============*/
void TDM_CheckVote (void);
void TDM_ResetLevel (void);
void TDM_RemoveVote (void);
void TDM_Vote_f (edict_t *ent);
qboolean TDM_ParseVoteConfigLine (char *line, int line_number, void *param);
qboolean TDM_RateLimited (edict_t *ent, int penalty);
void TDM_VoteWebConfigResult (edict_t *ent, int code, tdm_config_t *param);
void TDM_SetupVote (edict_t *ent);
void TDM_ConfigDownloaded (tdm_download_t *download, int code, byte *buff, int len);
void TDM_CreateConfiglist (void);
void TDM_VoteMenuApply (edict_t *ent);
void TDM_UpdateVoteConfigString (void);
void TDM_ApplyVote (void);

int LookupPlayer (const char *match, edict_t **out, edict_t *ent);
int TDM_WeaponStringToBitmask(const char *str);
int TDM_ArmorStringToBitmask(const char *str);

//votemenu.c
void OpenVoteMenu (edict_t *ent);
void VoteMenuGameMode (edict_t *ent);
void VoteMenuMap (edict_t *ent);
void VoteMenuConfig (edict_t *ent);
void VoteMenuMatchRestart (edict_t *ent);
void VoteMenuTimelimit (edict_t *ent);
void VoteMenuOvertime (edict_t *ent);
void VoteMenuPowerups (edict_t *ent);
void VoteMenuBFG (edict_t *ent);
void VoteMenuKick (edict_t *ent);
void VoteMenuChat (edict_t *ent);
void VoteMenuBugs (edict_t *ent);

#define VOTE_MENU_GAMEMODE	0x1
#define VOTE_MENU_MAP		0x2
#define VOTE_MENU_CONFIG	0x4
#define VOTE_MENU_TIMELIMIT	0x8
#define VOTE_MENU_OVERTIME	0x10
#define VOTE_MENU_POWERUPS	0x20
#define VOTE_MENU_BFG		0x40
#define VOTE_MENU_KICK		0x80
#define VOTE_MENU_CHAT		0x100
#define VOTE_MENU_BUGS		0x200

#define VOTE_MENU_ALL		0xFFFFFFFFU 

typedef struct weaponvote_s
{
	const char		*names[2];
	unsigned		value;
	int				itemindex;
} weaponinfo_t;

typedef struct powerupvote_s
{
	const char		*names[1];
	unsigned		value;
	int				itemindex;
} powerupinfo_t;

typedef struct armorvote_s
{
	const char		*names[2];
	unsigned		value;
	int				itemindex;
} armorinfo_t;

#define WEAPON_MAX		10
#define POWERUP_MAX		7
#define ARMOR_MAX       4

extern const weaponinfo_t   weaponvotes[WEAPON_MAX];
extern const powerupinfo_t  powerupvotes[POWERUP_MAX];
extern const armorinfo_t    armorvotes[ARMOR_MAX];

extern vote_t	vote;

extern char			**tdm_configlist;

#define WEAPON_SHOTGUN			(1<<1)
#define	WEAPON_SSHOTGUN			(1<<2)
#define	WEAPON_MACHINEGUN		(1<<3)
#define	WEAPON_CHAINGUN			(1<<4)
#define WEAPON_GRENADES			(1<<5)
#define WEAPON_GRENADELAUNCHER	(1<<6)
#define	WEAPON_ROCKETLAUNCHER	(1<<7)
#define WEAPON_RAILGUN			(1<<8)
#define WEAPON_BFG10K			(1<<9)
#define	WEAPON_HYPERBLASTER		(1<<10)

#define POWERUP_QUAD			(1<<1)
#define POWERUP_INVULN			(1<<2)
#define POWERUP_POWERSHIELD		(1<<3)
#define POWERUP_POWERSCREEN		(1<<4)
#define POWERUP_SILENCER		(1<<5)
#define POWERUP_REBREATHER		(1<<6)
#define POWERUP_ENVIROSUIT		(1<<7)

#define ITEM_ARMOR_JACKET       (1<<1)
#define ITEM_ARMOR_COMBAT       (1<<2)
#define ITEM_ARMOR_BODY         (1<<3)
#define ITEM_ARMOR_SHARD        (1<<4)

#define VOTE_TIMELIMIT          0x1
#define VOTE_MAP                0x2
#define VOTE_KICK               0x4
#define VOTE_WEAPONS            0x8
#define VOTE_POWERUPS           0x10
#define VOTE_GAMEMODE           0x20
#define VOTE_TELEMODE           0x80
#define VOTE_TIEMODE            0x100
#define VOTE_SWITCHMODE         0x200
#define VOTE_OVERTIME           0x400
//distinct so a local and remote '1v1' config are distinguishable
#define VOTE_CONFIG             0x800
#define VOTE_WEBCONFIG          0x1000
#define VOTE_CHAT               0x2000
#define VOTE_RESTART            0x4000
#define VOTE_BUGS               0x8000
#define VOTE_TDM_SPAWNMODE      0x10000
#define VOTE_1V1_SPAWNMODE      0x20000
#define VOTE_ABORT              0x40000
#define VOTE_SHUFFLE            0x80000
#define VOTE_ARMOR_TIMER        0x100000
#define VOTE_WEAPON_TIMER       0x200000
