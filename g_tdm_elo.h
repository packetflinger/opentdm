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
#define DEFAULT_ELO_SCORE   500

// constants for the formula
#define D   400
#define K   32

double TDM_Elo_CalcScore(double current, double expected, qboolean winner);
double TDM_Elo_ExpectedScore(double currentScore, double challengerScore);
