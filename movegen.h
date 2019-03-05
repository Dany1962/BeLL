#ifndef MOVEGEN_H_INCLUDED
#define MOVEGEN_H_INCLUDED

//fonctions
//copie des coups
void copie_coup(int f, int t, int type, int en_passant, MOVE * pBuf, int *pMCount);
void copie_promotion(int f, int t, int type, int en_passant, MOVE * pBuf, int *pMCount);
//génération de tout les coups
int gen_coups(int current_side, MOVE * pBuf);
int gen_caps(int current_side, MOVE * pBuf);

#endif // MOVEGEN_H_INCLUDED
