#ifndef _G_QUEUE_H
#define _G_QUEUE_H

// gets the offset in entity array (client number)
#define getEntOffset(ent)   (((char *)ent - (char *)globals.edicts) / globals.edict_size)

// takes a client number and gives you a pointer to that edict
#define getEnt(entnum)      (edict_t *)((char *)globals.edicts + (globals.edict_size * entnum))

typedef struct {
    uint8_t capacity;   // max maxclients->value
    uint8_t occupancy;  // current census
    uint8_t *queue;     // client number - zero is valid
} cl_queue_t;

extern cl_queue_t clq;

void Queue_Init();
qboolean Queue_Add(uint8_t entnum);
edict_t *Queue_GetHead();

#endif
