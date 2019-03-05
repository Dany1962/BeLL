#ifndef ENGINE_H_INCLUDED
#define ENGINE_H_INCLUDED

#define MAXPLY 100
#define MATE   100000
#define R(x)   ((x>=6) ? 3 : 1)

static const int from_pr[2] = {H1, H8};
static const int to_pr[2] = {F1, F8};
static const int from_gr[2] = {A1, A8};
static const int to_gr[2] = {D1, D8};

//recherche
int max_depth;
int time_left;
int debut,fin;
int fin_recherche;
int long_pv[MAXPLY+1];
MOVE pv[MAXPLY+1][MAXPLY+1];
int history[7][128][128];    //piece , from , to scheme
int nodes,qnodes;
U64 total_nodes;
U64 total_qnodes;
U64 pv_nodes;
U64 cut_nodes;
U64 all_nodes;
U64 hash_test;
U64 hash_found;
U64 killer_moves;
//killers moves
MOVE killer1[MAXPLY+1];
MOVE killer2[MAXPLY+1];
//futility margin
static const int margin[4] = {0, 125, 325, 525};
//algo alpha beta ou pvs
int algo_pvs;
int algo_ab;

//fonctions
//BeLL joue
MOVE programme_joue(int max_depth, int output);
//make unmake moves
int jouer_coup(MOVE m);
void dejouer_coup();
void vider_la_case(int sq, int pce, int coul);
void occuper_la_case(int sq, int pce, int coul);
int case_attaquee(int sq_r, int opp_coul);
int traverse_delta(int sq_p, int sq_a);
//temps
int get_ms();
int controle_si_temps_depasse();
//perft
long int perft (int p);
//recherche
int alpha_beta(int alpha, int beta, int depth, MOVE * pBestMove, int nm);
int ok_pour_nul_move();
void jouer_coup_nul();
void dejouer_coup_nul();
int quiesce(int alpha, int beta, MOVE * pBestMove);
void meilleur_coup_suivant(MOVE *ptable, int nb_coups, int debut);
int pvs(int alpha, int beta, int depth, MOVE * pBestMove, int nm);
int triple_repetition();
#endif // ENGINE_H_INCLUDED
