#ifndef EVAL_H_INCLUDED
#define EVAL_H_INCLUDED

#define MAT_NUL   20500

//matériel
int mat_op;
int mat_eg;
int materiel_op[2][7];
int materiel_eg[2][7];

//psqt
int pst_op;
int pst_eg;
int psqt_op[2][7][128];
int psqt_eg[2][7][128];

//mobilité
static const int mob_cavalier[9] = {-30, -25, -20, -15, -8, 5, 15, 30, 15};
static const int mob_fou[14] = {-30, -28, -25, -21, -16, -10, -3, 5, 9, 14, 20, 27, 35, 20};
static const int mob_tour[15] = {-20, -15, -15, -10, -10, -5, -5, 5, 5, 10, 10, 15, 15, 20, 10};
//pour tappered eval
int coef_phase[25];

//fonctions
int eval();
int mobilite_cavalier(int c);
int mobilite_fou(int c);
int mobilite_tour(int c);
int phase();
int draw_recognizer();
int pieces_lourdes();
int pieces_mineures();
int pions();
int pieces_mineures_blanc();
int pieces_mineures_noir();

#endif // EVAL_H_INCLUDED
