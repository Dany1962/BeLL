#include "init.h"
#include "board.h"
#include "eval.h"
#include "hash.h"
#include "engine.h"

void init()
{
    initialise_board();
    initialise_eval();
    initialise_hash();
}

void initialise_board()
{
    int i;

    //échiquier
    for(i=0; i<128; ++i)
    {
        piece[i] = init_pieces[i];
        couleur[i] = init_couleurs[i];
    }

    //cases des rois
    pos_roi[BLANC] = 4;
    pos_roi[NOIR]  = 116;

    //liste des pièces
    for(i=0; i<128; ++i)
        indices[i] = init_indices[i];

    for(i=0; i<16; ++i)
    {
        plist[BLANC][i] = init_plists[BLANC][i];
        plist[NOIR][i]  = init_plists[NOIR][i];
    }

    //init nb pieces
    for(i=0; i<7; ++i)
    {
        nb_pieces[BLANC][i] = init_nb_pieces[i];
        nb_pieces[NOIR][i]  = init_nb_pieces[i];
    }

    side = BLANC;               //couleur en cours
    computer_side = NOIR;       //couleur des pieces du programme
    max_depth = 4;              //profondeur de recherche (par defaut 4 coups)
    hdp = 0;                    //nbre de coups joués
    castle = 15;
    ep = -1;
    cinquante = 0;
    algo_ab = VRAI;
    algo_pvs = FAUX;
}

void initialise_eval()
{
    int i;

    mat_op = 0;
    mat_eg = 0;
    pst_op = 0;
    pst_eg = 0;

    //materiel
    for(i=1; i<7; ++i)
    {
        materiel_op[BLANC][i] = val_pieces_op[i];
        materiel_eg[BLANC][i] = val_pieces_eg[i];
        materiel_op[NOIR][i] = -val_pieces_op[i];
        materiel_eg[NOIR][i] = -val_pieces_eg[i];
    }
    for(i=0; i<128; ++i)
    {
        if(EN_JEU(i) && couleur[i] != VIDE)
        {
            mat_op += materiel_op[couleur[i]][piece[i]];
            mat_eg += materiel_eg[couleur[i]][piece[i]];
        }
    }

    //psqt
    for(i=0; i<128; ++i)
    {
        psqt_op[BLANC][PION][i]     = init_psqt_pion_op[i];
        psqt_op[BLANC][CAVALIER][i] = init_psqt_cavalier_op[i];
        psqt_op[BLANC][FOU][i]      = init_psqt_fou_op[i];
        psqt_op[BLANC][TOUR][i]     = init_psqt_tour_op[i];
        psqt_op[BLANC][DAME][i]     = init_psqt_dame_op[i];
        psqt_op[BLANC][ROI][i]      = init_psqt_roi_op[i];
        psqt_eg[BLANC][PION][i]     = init_psqt_pion_eg[i];
        psqt_eg[BLANC][CAVALIER][i] = init_psqt_cavalier_eg[i];
        psqt_eg[BLANC][FOU][i]      = init_psqt_fou_eg[i];
        psqt_eg[BLANC][TOUR][i]     = init_psqt_tour_eg[i];
        psqt_eg[BLANC][DAME][i]     = init_psqt_dame_eg[i];
        psqt_eg[BLANC][ROI][i]      = init_psqt_roi_eg[i];
        psqt_op[NOIR][PION][i]      = -init_psqt_pion_op[flip[i]];
        psqt_op[NOIR][CAVALIER][i]  = -init_psqt_cavalier_op[flip[i]];
        psqt_op[NOIR][FOU][i]       = -init_psqt_fou_op[flip[i]];
        psqt_op[NOIR][TOUR][i]      = -init_psqt_tour_op[flip[i]];
        psqt_op[NOIR][DAME][i]      = -init_psqt_dame_op[flip[i]];
        psqt_op[NOIR][ROI][i]       = -init_psqt_roi_op[flip[i]];
        psqt_eg[NOIR][PION][i]      = -init_psqt_pion_eg[flip[i]];
        psqt_eg[NOIR][CAVALIER][i]  = -init_psqt_cavalier_eg[flip[i]];
        psqt_eg[NOIR][FOU][i]       = -init_psqt_fou_eg[flip[i]];
        psqt_eg[NOIR][TOUR][i]      = -init_psqt_tour_eg[flip[i]];
        psqt_eg[NOIR][DAME][i]      = -init_psqt_dame_eg[flip[i]];
        psqt_eg[NOIR][ROI][i]       = -init_psqt_roi_eg[flip[i]];
    }

    for(i=0; i<128; ++i)
    {
        if(EN_JEU(i) && couleur[i] != VIDE)
        {
            pst_op += psqt_op[couleur[i]][piece[i]][i];
            pst_eg += psqt_eg[couleur[i]][piece[i]][i];
        }
    }

    //précalcul des coefficient pour "tappered eval"
    for(i=0; i<25; ++i)
    {
        coef_phase[i] = (i * 256 + (24 / 2)) / 24;
        //printf("phase et coef : %2d  %3d\n",i,coef_phase[i]);
    }
}

void initialise_hash()
{
    init_hash_table();
    hash_position = init_code_position();
}
