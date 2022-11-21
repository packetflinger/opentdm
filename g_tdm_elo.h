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

/**
 * current - the elo score read from userinfo or updated from playing
 * expected - the percent chance the player has of winning
 * actual - the actual result of the match
 * updated - the new current
 */
typedef struct {
    double current;
    double expected;
    double actual;
    double updated;
    unsigned opposing_team; // the other team
    int damage;             // dealt - received
    int rank;
    int qty_beat;           // the number of players we out-performed
} elo_t;

void    TDM_Elo_Calculate(teamplayer_t *p);
double  TDM_Elo_ExpectedScore(double currentScore, double challengerScore);
void    TDM_Elo_ExpectedTeamScore(edict_t *ent);
void    TDM_EloScore_f(edict_t *ent);
void    TDM_Elo_UpdateScore(edict_t *ent, edict_t *challenger, qboolean winner);
