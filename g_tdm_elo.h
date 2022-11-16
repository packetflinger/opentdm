/*
 * Elo rating system for balancing teams
 *
 * Expected result forumla:
 * Ra == current elo ranking for player a
 * Rb == current elo ranking for player b
 *
 * Ea = 1/(1+10^(Rb-Ra)/D)
 *
 *
 */

double  TDM_Elo_CalcScore(double current, double expected, qboolean winner);
double  TDM_Elo_ExpectedScore(double currentScore, double challengerScore);
void    TDM_Elo_UpdateScore(edict_t *ent, edict_t *challenger, qboolean winner)
