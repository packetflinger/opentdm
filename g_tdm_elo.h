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

// used for players without a score
#define ELO_DEFAULT_SCORE   500

// constants for the formula
#define ELO_CONST_D   400
#define ELO_CONST_K   16

double TDM_Elo_CalcScore(double current, double expected, qboolean winner);
double TDM_Elo_ExpectedScore(double currentScore, double challengerScore);
