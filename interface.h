#ifndef INTERFACE_H_INCLUDED
#define INTERFACE_H_INCLUDED

#define ON          2
#define OFF         0
#define ANALYZE     3
#define INVALID     666
#define MAXMOVES    500  /* maximum game length  */
#define MAXPLY      100   /* maximum search depth */
#define DEFAULT_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

int ponder;
int randomize;
int postThinking;
int resign;         // engine-defined option
int contemptFactor; // likewise
int temps_par_coup;

//fonctions
void affiche_echiquier();
void affiche_infos();
void generation_des_coups();
char *move_str(MOVE m);
int a_to_0x88(char * a);
void convert_0x88_a(int sq, char * a);
void affiche_liste();
void affiche_plist();
int verif_coup(char *s);
void affiche_resultat();
int xboard();

#endif // INTERFACE_H_INCLUDED
