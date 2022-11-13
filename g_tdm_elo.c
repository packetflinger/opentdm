#include "g_local.h"
#include "g_tdm.h"

/**
 * This is used to calculate a player's new elo score after
 * having just played a match.
 */
int TDM_Elo_CalcScore(int current, int expected, qboolean winner)
{
    int s = 0;

    if (winner) {
        s = 1;
    }

    return (K * (s - expected)) + current;
}


/**
 * An expected score is the probability a player will win vs another
 * player based on their current elo scores. This is used in calculating
 * their new scores after the match.
 */
float TDM_Elo_ExpectedScore(int currentScore, int challengerScore)
{
    return 1/(10^((challengerScore - currentScore)/D));
}
