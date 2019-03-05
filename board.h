#ifndef BOARD_H_INCLUDED
#define BOARD_H_INCLUDED

//entier long 64 bits
typedef unsigned long long U64;

//booléens
#define VRAI     1
#define FAUX     0
#define LEGAL    1
#define ILLEGAL  0

//spécial pour 0x88 générateur
#define COL(x)              (x & 7)
#define ROW(x)              (x >> 4)
#define EN_JEU(x)           (((x) & 0x88) ? (FAUX) : (VRAI))
#define INDEX(x)            ROW(x) * 16 + COL(x)

//couleur
#define BLANC       0
#define NOIR        1
#define VIDE        200
#define HORS_JEU    300

//pieces
#define	PION        1
#define	CAVALIER    2
#define	FOU         3
#define	TOUR        4
#define	DAME        5
#define	ROI         6

//couleur opposee
#define OPP(x)  (x == 0 ? 1 : 0)

//types des coups
#define NORMAL          0
#define CAPTURE         1
#define EN_PASSANT      2
#define PROMO_CAPTURE   3
#define PROMO           4
#define PETIT_ROQUE     5
#define GRAND_ROQUE     6

//cases utiles
#define A1      0
#define B1      1
#define C1      2
#define D1      3
#define E1      4
#define F1      5
#define G1      6
#define H1      7
#define A8      112
#define B8      113
#define C8      114
#define D8      115
#define E8      116
#define F8      117
#define G8      118
#define H8      119

//structure d'un coup
typedef struct coup
{
    int from;           //case origine pièce jouée
    int to;             //case arrivée pièce jouée
    int piece_from;     //pièce jouée
    int piece_to;       //pièce capturée (ou VIDE)
    int index_cap;      //index de la pièce capturée pour  plist
    int piece_promue;   //pièce promue (si promotion)
    int type;           //type de coup joué (normal , capture , roque etc...)
    int ep_flag;        //valeur en passant si coup de pion de deux cases , -1 sinon
    int evaluation;     //score du coup
}   MOVE;

//liste partie
typedef struct tag_HIST
{
    MOVE m;             //coup joué
    int cap;            //pièce capturée (ou VIDE)
    int castle;         //valeur roque sauvegardée
    int ep;             //valeur en passant sauvegardée
    int cinquante;      //valeur règle 50 coups sauvegardée
    U64 hash_pos;       //stocke les hashcodes pour 3 fold
}   HIST;

//coups de la partie (max 900)
HIST hist[900];

//position
int piece[128];         //pièces sur l'échiquier
int couleur[128];       //couleur des pièces
int pos_roi[3];         //position des rois
int plist[2][16];       //liste des pieces noires et blanches (max 16 pieces , 0-15 , couleur 0 blanc , 1 noir
int indices[128];       //indices des pieces sur les cases val VIDE , HORS JEU , 0 a 15
int nb_pieces[2][7];    //nombre de pièces de chaque couleur

//pour recherche
int side;           //couleur en cours
int computer_side;  //couleur du programme
int hdp;            //numéro du coup en cours ds la recherche ou la partie
int nodes;          //compteur de noeuds visités
int q_nodes;        // noeuds visités qs
int prof;           //profondeur de recherche
int max_depth;      //profondeur de recherche maximum
int castle;         //roque
int ep;             //en passant
int cinquante;      //règle des cinquante coups

//vecteurs pièces (pour générateur de coups 0x88)
static const int v_pion_blanc[4] = {16, 32, 15, 17 };
static const int v_pion_noir[4]  = {-16, -32, -15, -17 };
static const int v_cavalier[8]   = {14, 18, 31, 33, -14, -18, -31, -33};
static const int v_fou[4]        = {15, 17, -15, -17};
static const int v_tour[8]       = {1, 16, -1, -16};
static const int v_dame[8]       = {1, 15, 16, 17, -1, -15, -16, -17};
static const int v_roi[8]        = {1, 15, 16, 17, -1, -15, -16, -17};

//pour roque
static const int castle_mask[128] =
{
    13, 15, 15, 15, 12, 15, 15, 14, 7, 7, 7, 7, 7, 7, 7, 7,
    15, 15, 15, 15, 15, 15, 15, 15, 7, 7, 7, 7, 7, 7, 7, 7,
    15, 15, 15, 15, 15, 15, 15, 15, 7, 7, 7, 7, 7, 7, 7, 7,
    15, 15, 15, 15, 15, 15, 15, 15, 7, 7, 7, 7, 7, 7, 7, 7,
    15, 15, 15, 15, 15, 15, 15, 15, 7, 7, 7, 7, 7, 7, 7, 7,
    15, 15, 15, 15, 15, 15, 15, 15, 7, 7, 7, 7, 7, 7, 7, 7,
    15, 15, 15, 15, 15, 15, 15, 15, 7, 7, 7, 7, 7, 7, 7, 7,
     7, 15, 15, 15,  3, 15, 15, 11, 7, 7, 7, 7, 7, 7, 7, 7
};

//table pour les attaques (checker si un roi est en échec entre autre)
//Shared by Mediocre programmer , thanks to Jonathan Paterson  :-)
//http://mediocrechess.sourceforge.net/guides/attackedsquares.html

static const int attack_none  = 0;      // Deltas that no piece can move
static const int attack_kqr   = 1;      // One square up down left and right
static const int attack_qr    = 2;      // More than one square up down left and right
static const int attack_kqbwp = 3;      // One square diagonally up
static const int attack_kqbbp = 4;      // One square diagonally down
static const int attack_qb    = 5;      // More than one square diagonally
static const int attack_n     = 6;      // Knight moves

static const int table_attaques[257] =
{
    0,0,0,0,0,0,0,0,0,5,0,0,0,0,0,0,2,0,0,0,     //0-19
    0,0,0,5,0,0,5,0,0,0,0,0,2,0,0,0,0,0,5,0,     //20-39
    0,0,0,5,0,0,0,0,2,0,0,0,0,5,0,0,0,0,0,0,     //40-59
    5,0,0,0,2,0,0,0,5,0,0,0,0,0,0,0,0,5,0,0,     //60-79
    2,0,0,5,0,0,0,0,0,0,0,0,0,0,5,6,2,6,5,0,     //80-99
    0,0,0,0,0,0,0,0,0,0,6,4,1,4,6,0,0,0,0,0,     //100-119
    0,2,2,2,2,2,2,1,0,1,2,2,2,2,2,2,0,0,0,0,     //120-139
    0,0,6,3,1,3,6,0,0,0,0,0,0,0,0,0,0,0,5,6,     //140-159
    2,6,5,0,0,0,0,0,0,0,0,0,0,5,0,0,2,0,0,5,     //160-179
    0,0,0,0,0,0,0,0,5,0,0,0,2,0,0,0,5,0,0,0,     //180-199
    0,0,0,5,0,0,0,0,2,0,0,0,0,5,0,0,0,0,5,0,     //200-219
    0,0,0,0,2,0,0,0,0,0,5,0,0,5,0,0,0,0,0,0,     //220-239
    2,0,0,0,0,0,0,5,0,0,0,0,0,0,0,0,0            //240-256
};

static const int table_delta[257] =
{
    0,   0,   0,   0,   0,   0,   0,   0,   0, -17,   0,   0,   0,   0,   0,   0, -16,   0,   0,   0,
    0,   0,   0, -15,   0,   0, -17,   0,   0,   0,   0,   0, -16,   0,   0,   0,   0,   0, -15,   0,
    0,   0,   0, -17,   0,   0,   0,   0, -16,   0,   0,   0,   0, -15,   0,   0,   0,   0,   0,   0,
    -17,   0,   0,   0, -16,   0,   0,   0, -15,   0,   0,   0,   0,   0,   0,   0,   0, -17,   0,   0,
    -16,   0,   0, -15,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, -17, -33, -16, -31, -15,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0, -18, -17, -16, -15, -14,   0,   0,   0,   0,   0,
    0,  -1,  -1,  -1,  -1,  -1,  -1,  -1,   0,   1,   1,   1,   1,   1,   1,   1,   0,   0,   0,   0,
    0,   0,  14,  15,  16,  17,  18,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  15,  31,
    16,  33,  17,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  15,   0,   0,  16,   0,   0,  17,
    0,   0,   0,   0,   0,   0,   0,   0,  15,   0,   0,   0,  16,   0,   0,   0,  17,   0,   0,   0,
    0,   0,   0,  15,   0,   0,   0,   0,  16,   0,   0,   0,   0,  17,   0,   0,   0,   0,  15,   0,
    0,   0,   0,   0,  16,   0,   0,   0,   0,   0,  17,   0,   0,  15,   0,   0,   0,   0,   0,   0,
    16,   0,   0,   0,   0,   0,   0,  17,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

//pour montrer l'eval
int e_valide;

#endif // BOARD_H_INCLUDED
