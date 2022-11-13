#include "g_local.h"
#include "g_tdm.h"

/**
 * This is used to calculate a player's new elo score after
 * having just played a match.
 */
double TDM_Elo_CalcScore(double current, double expected, qboolean winner)
{
    if (!(int)g_elo_enable->value) {
        return 0.0;
    }

    return (double)(g_elo_const_k->value * (((winner) ? 1 : 0) - expected)) + current;
}


/**
 * An expected score is the probability a player will win vs another
 * player based on their current elo scores. This is used in calculating
 * their new scores after the match.
 *
 * This is a percentage
 */
double TDM_Elo_ExpectedScore(double currentScore, double challengerScore)
{
    if (!(int)g_elo_enable->value) {
        return 0.0;
    }

    return 1/(1+(double)(pow(10, (((float) (challengerScore-currentScore)) / g_elo_const_d->value))));
}
