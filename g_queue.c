#include "g_local.h"
#include "g_tdm.h"

cl_queue_t clq;

static void Queue_Shift(cl_queue_t *q) {
    for (int i=1; i<q->occupancy; i++) {
        //memmove(q, q+i,sizeof(gclient_t));
    }
}

void Queue_Init() {
    clq.queue = gi.TagMalloc((int)g_queuesize->value * sizeof(clq.queue), TAG_GAME);
    memset(&clq, 0, sizeof(clq));
    clq.capacity = 8;
}

qboolean Queue_Add(uint8_t entnum) {
    //uint8_t *head;
    if (clq.occupancy < clq.capacity) {
        //clq.queue[clq.occupancy] = entnum;
        *clq.queue = entnum;
        //head = clq.queue + clq.occupancy;
        // = entnum;
        clq.occupancy++;
        return true;
    }

    return false;
}

/**
 * Get the client at the front and shift everyone down
 */
edict_t *Queue_GetHead() {
    edict_t *head;

    head = getEnt(clq.queue[0]);
    Queue_Shift(&clq);
    clq.occupancy--;

    return head;
}
