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

/**
 * Calculate an expected score based on the entire opposite team
 */
void TDM_Elo_ExpectedTeamScore(edict_t *ent)
{
    int i;
    int num_players;
    unsigned opponent_team;
    double score;
    edict_t *e;

    if (!(int)g_elo_enable->value) {
        return;
    }

    score = 0.0;
    opponent_team = (ent->client->pers.team == TEAM_A) ? TEAM_B : TEAM_A;
    num_players = teaminfo[opponent_team].players;

    // special case for duel
    if (num_players == 1) {
        for (i=0; i<current_matchinfo.num_teamplayers; i++) {
            e = current_matchinfo.teamplayers[i].client;

            if (TEAM(e) == opponent_team) {
                break;
            }
        }

        score = TDM_Elo_ExpectedScore(ent->client->pers.elo_score, e->client->pers.elo_score);
        ent->client->pers.elo_expected = score;
        return;
    }

    for (i=0; i < game.maxclients; i++) {
        e = g_edicts + i + 1;

        if (!e->inuse) {
            continue;
        }

        if (!e->client) {
            continue;
        }

        if (e->client->pers.team != opponent_team) {
            continue;
        }

        score += TDM_Elo_ExpectedScore(ent->client->pers.elo_score, e->client->pers.elo_score);
    }

    score = score / ((num_players * (num_players - 1)) / 2);
    ent->client->pers.elo_expected = score;
}

/**
 * Show a user their elo score. Refs can see everyone's score
 */
void TDM_EloScore_f(edict_t *ent)
{
    int i;
    edict_t *e;

    if (!(int)g_elo_enable->value) {
        gi.cprintf(ent, PRINT_HIGH, "Elo ranking is disabled on this server\n");
        return;
    }

    if (ent->client->pers.admin) {
        gi.cprintf(ent, PRINT_HIGH, "Team Player     Elo Score\n");
        gi.cprintf(ent, PRINT_HIGH, "-------------------------\n");

        for (i=0; i < game.maxclients; i++) {
            e = g_edicts + i + 1;

            if (!e->inuse) {
                continue;
            }

            if (!e->client) {
                continue;
            }

            if (TEAM(e) > TEAM_SPEC) {
                gi.cprintf(ent, PRINT_HIGH, "%-15s %-.3f\n", NAME(ent), ent->client->pers.elo_score);
            }
        }
        return;
    }

    gi.cprintf(ent, PRINT_HIGH, "Your elo score: %.3f\n", ent->client->pers.elo_score);
}


/**
 * Update ent's elo score based on the outcome of their match with challenger
 */
void TDM_Elo_UpdateScore(edict_t *ent, edict_t *challenger, qboolean winner)
{

}
