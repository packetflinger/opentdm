#include "g_local.h"
#include "g_tdm.h"

int TDM_Elo_CalcScore(int current, int expected, qboolean winner)
{
    int s = 0;

    if (winner) {
        s = 1;
    }

    return (K * (s - expected)) + current;
}

int TDM_Elo_ExpectedScore(int currentScore, int challengerScore)
{
    return 1/(10^((challengerScore - currentScore)/D));
}
