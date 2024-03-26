typedef struct {
    netadr_t addr;
    int expire;
} ipfilter_t;

#define MAX_IPFILTERS   1024

extern ipfilter_t ipfilters[MAX_IPFILTERS];
extern unsigned numipfilters;

void SVCmd_AddIP_f(edict_t *ent, char *ip, int exp);
void SVCmd_RemoveIP_f(edict_t *ent, char *ip);
void SVCmd_ListIP_f(edict_t *ent);
qboolean StringToFilter(const char *s, ipfilter_t *f, int exp);
qboolean CheckIpFilterList(edict_t *ent, unsigned *i);
