#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "board.h"
#include "edit.h"
#include "interface.h"
#include "eval.h"
#include "init.h"
#include "hash.h"

//----------------------------------------------------------------------------------------------------------------
//                              entrée d'une position (manuellement)
//----------------------------------------------------------------------------------------------------------------
void edit()
{
    int coul = BLANC;           //par défaut couleur BLANC
    int d;                      //case selectionnée par l'utilisateur
    char s[256];                //chaine entrée par l'utilisateur
    int fin_saisie = FAUX;     //booléen de fin de saisie
    int i;
    int prb = 0,grb = 0,prn = 0,grn = 0;   //roques

    for(;;)                     //boucle infinie de saisie
    {
        //affiche_echiquier();
        //affiche_infos();
        scanf("%s", s);
        d = s[1] - 'a';
        d += (s[2] - '1') * 16;
        switch(s[0])
        {
        case 'P':
            vider_sq(couleur[d], piece[d], d);
            occuper_sq(coul, PION, d);
            break;
        case 'N':
            vider_sq(couleur[d], piece[d], d);
            occuper_sq(coul, CAVALIER, d);
            break;
        case 'B':
            vider_sq(couleur[d], piece[d], d);
            occuper_sq(coul, FOU, d);
            break;
        case 'R':
            vider_sq(couleur[d], piece[d], d);
            occuper_sq(coul, TOUR, d);
            break;
        case 'Q':
            vider_sq(couleur[d], piece[d], d);
            occuper_sq(coul, DAME, d);
            break;
        case 'K':
            vider_sq(couleur[d], piece[d], d);
            occuper_sq(coul, ROI, d);
            pos_roi[coul] = d;
            break;
        case '.':
            ep = -1;
            fin_saisie = VRAI;
            //on determine "castle"
            if(pos_roi[BLANC] == E1)
            {
                if(si_piece(TOUR, BLANC, H1))
                    prb = 1;
                if(si_piece(TOUR, BLANC, A1))
                    grb = 2;
            }
            if(pos_roi[NOIR] == E8)
            {
                if(si_piece(TOUR, NOIR, H8))
                    prn = 4;
                if(si_piece(TOUR, NOIR, A8))
                    grn = 8;
            }
            castle = (prb ^ grb ^ prn ^ grn);
            break;
        case 'x':
            vider_sq(couleur[d], piece[d], d);
            break;
        case '#':  //vide l'échiquier
            for(i=0; i<120; ++i)
            {
                if(EN_JEU(i))
                    vider_sq(couleur[i], piece[i], i);
            }
            ep = -1;
            castle = 0;
            cinquante = 0;
            break;
        case 'c':
            if(coul == BLANC)
                coul = NOIR;
            else
                coul = BLANC;
            break;
        default :
            break;
        }
        if(fin_saisie)
            break;
        affiche_echiquier();
    }
    hdp = 1;
    hash_position = init_code_position();
}


void vider_sq(int c, int p, int sq)
{
    if(piece[sq] != VIDE)
    {
        //update plist
        plist[c][indices[sq]] = VIDE;
        indices[sq] = VIDE;

        //update position roi
        if(piece[sq] == ROI)
            pos_roi[c] = -1;

        //update matériel
        mat_op -= materiel_op[c][p];
        mat_eg -= materiel_eg[c][p];

        //update psqt
        pst_op -= psqt_op[c][p][sq];
        pst_eg -= psqt_eg[c][p][sq];

        //update nb pieces
        nb_pieces[c][p]--;

        //update echiquier
        piece[sq]  = VIDE;
        couleur[sq]  = VIDE;
    }
}

void occuper_sq(int c, int p, int sq)
{
    int sq2;
    int index;

    //enlève l'ancienne pièce du jeu (si besoin)
    if(piece[sq] != VIDE)
    {
        sq2 = sq;
        vider_sq(couleur[sq2], piece[sq2], sq2);
    }

    //place la nouvelle pièce
    //update nb pieces
    nb_pieces[c][p]++;

    //update plist
    index = (ind_start[p] + (nb_pieces[c][p] - 1));
    plist[c][index] = sq;
    indices[sq] = index;

    //update position roi
    if(p == ROI)
        pos_roi[c] = sq;

    //update materiel
    mat_op += materiel_op[c][p];
    mat_eg += materiel_eg[c][p];

    //update psqt
    pst_op += psqt_op[c][p][sq];
    pst_eg += psqt_eg[c][p][sq];

    //update echiquier
    piece[sq] = p;
    couleur[sq] = c;
}

int si_piece(int p, int c, int k)
{
    return(((piece[k] == p) && (couleur[k] == c)) ? VRAI : FAUX);  //optimisation voir gprof1
}
