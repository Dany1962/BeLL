#include <stdio.h>

#include "eval.h"
#include "board.h"
#include "init.h"

int eval()
{
    int score = 0;
    int op = 0;
    int eg = 0;
    int pse = 0;
    int cp;
    int sq;
    int m;
    int mob = 0;

    //si nulle par matériel insuffisant (KK, KBK, KNK) retourne 0
    if(draw_recognizer())
        return 0;

    //calcul phase en cours
    pse = phase();

    //examen des pieces , evaluation mobilité
    for (cp=0; cp<16; ++cp)
    {
        sq = plist[BLANC][cp];
        if (sq != VIDE)
        {
            switch (piece[sq])
            {
            case PION:
                break;
            case CAVALIER:
                m = mobilite_cavalier(sq);
                mob += mob_cavalier[m];
                if(e_valide)
                    printf("mobilite cavalier : %2d (%2d)\n",mob_cavalier[m],m);
                break;
            case FOU:
                m = mobilite_fou(sq);
                mob += mob_fou[m];
                if(e_valide)
                    printf("mobilite fou      : %2d (%2d)\n",mob_fou[m],m);
                break;
            case TOUR:
                m = mobilite_tour(sq);
                mob += mob_tour[m];
                if(e_valide)
                    printf("mobilite tour     : %2d (%2d)\n",mob_tour[m],m);
                break;
            case DAME:
                break;
            case ROI:
                break;
            default:
                break;
            }
        }
        sq = plist[NOIR][cp];
        if (sq != VIDE)
        {
            switch (piece[sq])
            {
            case PION:
                break;
            case CAVALIER:
                m = mobilite_cavalier(sq);
                mob -= mob_cavalier[m];
                if(e_valide)
                    printf("mobilite cavalier : %2d (%2d)\n",mob_cavalier[m],m);
                break;
            case FOU:
                m = mobilite_fou(sq);
                mob -= mob_fou[m];
                if(e_valide)
                    printf("mobilite fou      : %2d (%2d)\n",mob_fou[m],m);
                break;
            case TOUR:
                m = mobilite_tour(sq);
                mob -= mob_tour[m];
                if(e_valide)
                    printf("mobilite tour     : %2d (%2d)\n",mob_tour[m],m);
                break;
            case DAME:
                break;
            case ROI:
                break;
            default:
                break;
            }
        }
    }

    //score op et eg (matériel + psqt)
    op = (mat_op + pst_op + mob);
    eg = (mat_eg + pst_eg + mob);

    score = ((op * (256 - coef_phase[pse])) + (eg * coef_phase[pse])) / 256;
    if(e_valide)
    {
        printf("Materiel : (%d , %d)\n",mat_op,mat_eg);
        printf("Psqt     : (%d , %d)\n",pst_op,pst_eg);
        printf("Mobilite : (%d)\n",mob);
        printf("Score OP : (%d)\n",op);
        printf("Score EG : (%d)\n",eg);
        printf("Score    : (%d)\n",score);
    }

    //retour du score blanc pov
    if (side == BLANC)
        return (score);
    return -(score);
}

//eval mobilité fou tour cavalier
int mobilite_cavalier(int c)
{
    int i;
    int v;
    int score = 0;

    for(i=0; i<8; ++i)
    {
        v = v_cavalier[i];
        if(EN_JEU(c+v))
        {
            if(couleur[c+v] == VIDE)
                score++;
        }
    }
    return score;
}

int mobilite_fou(int c)
{
    int i;
    int w;
    int score = 0;

    for(i=0; i<4; ++i)
    {
        for(w=c;;)
        {
            w += v_fou[i];
            if(!EN_JEU(w))
                break;
            if(piece[w] == VIDE)
                score++;
            else
                break;
        }
    }
    return score;
}

int mobilite_tour(int c)
{
    int i;
    int w;
    int score = 0;

    for(i=0; i<4; ++i)
    {
        for(w=c;;)
        {
            w += v_tour[i];
            if(!EN_JEU(w))
                break;
            if(piece[w] == VIDE)
                score++;
            else
                break;
        }
    }
    return score;
}

int phase()
{
    return (24 - (4 * (nb_pieces[BLANC][DAME] + nb_pieces[NOIR][DAME])) -
            (2 * (nb_pieces[BLANC][TOUR] + nb_pieces[NOIR][TOUR])) -
            (nb_pieces[BLANC][CAVALIER] + nb_pieces[NOIR][CAVALIER] +
             nb_pieces[BLANC][FOU] + nb_pieces[NOIR][FOU]));
}

int draw_recognizer()
{
    int pl = pieces_lourdes(); //nombre de pièces lourdes (tours et dames)
    int pm = pieces_mineures(); //nombre de pièces mineures (cavaliers et fous)
    int pns = pions(); //nombre de pions
    int pmb = pieces_mineures_blanc();
    int pmn = pieces_mineures_noir();

    if(!pl && !pns)
    {
        //KK?
        if(!mat_op && !pm)
            return VRAI;
        //KBK ou KNK ?
        if(pm == 1)
            return VRAI;
        //KBKB ou KNKN ou KNKB ou KBKN ?
        if(pmb == 1 && pmn == 1)
            return VRAI;
    }
    return FAUX;
}

int pieces_lourdes()
{
    return (nb_pieces[BLANC][DAME] + nb_pieces[NOIR][DAME] +
            nb_pieces[BLANC][TOUR] + nb_pieces[NOIR][TOUR]);
}

int pieces_mineures()
{
    return (nb_pieces[BLANC][CAVALIER] + nb_pieces[NOIR][CAVALIER] +
            nb_pieces[BLANC][FOU] + nb_pieces[NOIR][FOU]);
}

int pions()
{
    return (nb_pieces[BLANC][PION] + nb_pieces[NOIR][PION]);
}

int pieces_mineures_blanc()
{
    return (nb_pieces[BLANC][CAVALIER] + nb_pieces[BLANC][FOU]);
}

int pieces_mineures_noir()
{
    return (nb_pieces[NOIR][CAVALIER] + nb_pieces[NOIR][FOU]);
}
