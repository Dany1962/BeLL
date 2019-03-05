#include <stdio.h>

#include "board.h"
#include "movegen.h"
#include "engine.h"
#include "edit.h"
#include "init.h"

//copie des coups dans la liste
void copie_coup(int f, int t, int type, int en_passant, MOVE * pBuf, int *pMCount)
{
    MOVE move;

    move.from         = f;
    move.to           = t;
    move.piece_from   = piece[f];
    move.piece_to     = piece[t];
    move.piece_promue = VIDE;
    move.type         = type;
    move.ep_flag      = en_passant;
    if(piece[t] != VIDE)
        move.evaluation = (30000 + (10 * val_pieces_op[piece[t]] - piece[f]));
    //move.evaluation = (30000 + (val_pieces_op[piece[t]] - val_pieces_op[piece[f]]));
    else if(killer1[prof].from == f && killer1[prof].to == t &&
            killer1[prof].piece_from == piece[f] && killer1[prof].piece_to == piece[t])
        move.evaluation = 25000;
    else
        move.evaluation = history[piece[f]][f][t];
    pBuf[*pMCount]    = move;
    *pMCount          = *pMCount + 1;
}

//copie des promotions dans la liste
void copie_promotion(int f, int t, int type, int en_passant, MOVE * pBuf, int *pMCount)
{
    MOVE move;
    int pce;

    for(pce=CAVALIER; pce<ROI; ++pce)
    {
        move.from         = f;
        move.to           = t;
        move.piece_from   = piece[f];
        move.piece_to     = piece[t];
        move.piece_promue = pce;
        move.type         = type;
        move.ep_flag      = en_passant;
        if(piece[t] != VIDE)
            move.evaluation = (30000 + (10 * val_pieces_op[piece[t]] - piece[f] + val_pieces_op[pce]));
        else if(killer1[prof].from == f && killer1[prof].to == t &&
                killer1[prof].piece_from == piece[f] && killer1[prof].piece_to == piece[t])
            move.evaluation = 25000;
        else
            move.evaluation = (30000 + val_pieces_op[pce] - piece[f]);
        pBuf[*pMCount]    = move;
        *pMCount          = *pMCount + 1;
    }
}

//génération de tout les coups
int gen_coups(int current_side, MOVE * pBuf)
{
    int sq;
    int cp;
    int i;
    int v,w;
    int ctr = 0;  //compteur du nombre de coups
    int xc = OPP(current_side);
    int sq_roi;

    for (cp=0; cp<16; ++cp)
    {
        sq = plist[current_side][cp];
        if (sq != VIDE)
        {
            switch (piece[sq])
            {
            case PION:
                if(current_side == BLANC)
                {
                    //coups normaux + promos
                    if(piece[sq+16] == VIDE)
                    {
                        if(ROW(sq) == 6)  //promotions
                            copie_promotion(sq, (sq+16), PROMO, -1, pBuf, &ctr);
                        else
                        {
                            copie_coup(sq, (sq+16), NORMAL, -1, pBuf, &ctr);
                            if((ROW(sq) == 1) && (piece[(sq+32)] == VIDE))  //avance de deux cases
                                copie_coup(sq, (sq+32), NORMAL, (sq+16), pBuf, &ctr);
                        }
                    }
                    //captures
                    if(EN_JEU(sq+15) && couleur[(sq+15)] == NOIR)
                    {
                        if(ROW(sq) == 6)
                            copie_promotion(sq, (sq+15), PROMO_CAPTURE, -1, pBuf, &ctr);
                        else
                            copie_coup(sq, (sq+15), CAPTURE, -1, pBuf, &ctr);
                    }
                    if(EN_JEU(sq+17) && couleur[(sq+17)] == NOIR)
                    {
                        if(ROW(sq) == 6)
                            copie_promotion(sq, (sq+17), PROMO_CAPTURE, -1, pBuf, &ctr);
                        else
                            copie_coup(sq, (sq+17), CAPTURE, -1, pBuf, &ctr);
                    }
                }
                else
                {
                    if(piece[sq-16] == VIDE)
                    {
                        if(ROW(sq) == 1)    //promotions
                            copie_promotion(sq, (sq-16), PROMO, -1, pBuf, &ctr);
                        else
                        {
                            //coups normaux
                            copie_coup(sq, (sq-16), NORMAL, -1, pBuf, &ctr);
                            if((ROW(sq) == 6) && (piece[sq-32] == VIDE))  //avance de deux cases
                                copie_coup(sq, (sq-32), NORMAL, (sq-16), pBuf, &ctr);
                        }
                    }
                    //captures
                    if(EN_JEU(sq-15) && couleur[sq-15] == BLANC)
                    {
                        if(ROW(sq) == 1)
                            copie_promotion(sq, (sq-15), PROMO_CAPTURE, -1, pBuf, &ctr);
                        else
                            copie_coup(sq, (sq-15), CAPTURE, -1, pBuf, &ctr);
                    }
                    if(EN_JEU(sq-17) && couleur[sq-17] == BLANC)
                    {
                        if(ROW(sq) == 1)
                            copie_promotion(sq, (sq-17), PROMO_CAPTURE, -1, pBuf, &ctr);
                        else
                            copie_coup(sq, (sq-17), CAPTURE, -1, pBuf, &ctr);
                    }
                }
                break;
            case CAVALIER:
                for(i=0; i<8; ++i)
                {
                    v = v_cavalier[i];
                    if(EN_JEU(sq+v))
                    {
                        if(couleur[sq+v] == VIDE)
                            copie_coup(sq, (sq+v), NORMAL, -1, pBuf, &ctr);
                        if(couleur[sq+v] == xc)
                            copie_coup(sq, (sq+v), CAPTURE, -1, pBuf, &ctr);
                    }
                }
                break;
            case FOU:
                for(i=0; i<4; ++i)
                {
                    for(w=sq;;)
                    {
                        w += v_fou[i];
                        if(!EN_JEU(w))
                            break;
                        if(piece[w] == VIDE)
                            copie_coup(sq, w, NORMAL, -1, pBuf, &ctr);
                        else if(couleur[w] == xc)
                        {
                            copie_coup(sq, w, CAPTURE, -1, pBuf, &ctr);
                            break;
                        }
                        else
                            break;
                    }
                }
                break;
            case TOUR:
                for(i=0; i<4; ++i)
                {
                    for(w=sq;;)
                    {
                        w += v_tour[i];
                        if(!EN_JEU(w))
                            break;
                        if(piece[w] == VIDE)
                            copie_coup(sq, w, NORMAL, -1, pBuf, &ctr);
                        else if(couleur[w] == xc)
                        {
                            copie_coup(sq, w, CAPTURE, -1, pBuf, &ctr);
                            break;
                        }
                        else
                            break;
                    }
                }
                break;
            case DAME:
                for(i=0; i<8; ++i)
                {
                    for(w=sq;;)
                    {
                        w += v_dame[i];
                        if(!EN_JEU(w))
                            break;
                        if(piece[w] == VIDE)
                            copie_coup(sq, w, NORMAL, -1, pBuf, &ctr);
                        else if(couleur[w] == xc)
                        {
                            copie_coup(sq, w, CAPTURE, -1, pBuf, &ctr);
                            break;
                        }
                        else
                            break;
                    }
                }
                break;
            case ROI:
                for(i=0; i<8; ++i)
                {
                    v = v_roi[i];
                    if(EN_JEU(sq+v))
                    {
                        if(couleur[sq+v] == VIDE)
                            copie_coup(sq, (sq+v), NORMAL, -1, pBuf, &ctr);
                        if(couleur[sq+v] == xc)
                            copie_coup(sq, (sq+v), CAPTURE, -1, pBuf, &ctr);
                    }
                }
                break;
            default:
                break;
            }
        }
    }

    //roques
    if(current_side == BLANC)
    {
        sq_roi = pos_roi[BLANC];
        if (castle & 1)     //0-0 blanc
        {
            if(piece[F1] == VIDE && piece[G1] == VIDE && (!case_attaquee(sq_roi, NOIR)) &&
                    (!case_attaquee(F1, NOIR)) && (!case_attaquee(G1, NOIR)))
                copie_coup(E1, G1, PETIT_ROQUE, -1, pBuf, &ctr);
        }

        if (castle & 2)     //0-0-0 blanc
        {
            if(piece[B1] == VIDE && piece[C1] == VIDE && piece[D1] == VIDE && (!case_attaquee(sq_roi, NOIR)) &&
                    (!case_attaquee(C1, NOIR)) && (!case_attaquee(D1, NOIR)))
                copie_coup(E1, C1, GRAND_ROQUE, -1, pBuf, &ctr);
        }

    }
    else
    {
        sq_roi = pos_roi[NOIR];
        if (castle & 4)     //0-0 noir
        {
            if(piece[F8] == VIDE && piece[G8] == VIDE && (!case_attaquee(sq_roi, BLANC)) &&
                    (!case_attaquee(F8, BLANC)) && (!case_attaquee(G8, BLANC)))
                copie_coup(E8, G8, PETIT_ROQUE, -1, pBuf, &ctr);
        }
        if (castle & 8)     //0-0-0 noir
        {
            if(piece[B8] == VIDE && piece[C8] == VIDE && piece[D8] == VIDE && (!case_attaquee(sq_roi, BLANC)) &&
                    (!case_attaquee(C8, BLANC)) && (!case_attaquee(D8, BLANC)))
                copie_coup(E8, C8, GRAND_ROQUE, -1, pBuf, &ctr);
        }
    }

    //prises en passant
    if(ep != -1)
    {
        if(current_side == BLANC)
        {
            if((EN_JEU(ep-15)) && si_piece(PION, BLANC, ep-15))
                copie_coup((ep-15), ep, EN_PASSANT, -1, pBuf, &ctr);
            if((EN_JEU(ep-17)) && si_piece(PION, BLANC, ep-17))
                copie_coup((ep-17), ep, EN_PASSANT, -1, pBuf, &ctr);
        }
        else
        {
            if((EN_JEU(ep+15)) && si_piece(PION, NOIR, ep+15))
                copie_coup((ep+15), ep, EN_PASSANT, -1, pBuf, &ctr);
            if((EN_JEU(ep+17)) && si_piece(PION, NOIR, ep+17))
                copie_coup((ep+17), ep, EN_PASSANT, -1, pBuf, &ctr);
        }
    }

    return ctr;
}

//génération des captures + promos
int gen_caps(int current_side, MOVE * pBuf)
{
    int sq;
    int cp;
    int i;
    int v,w;
    int ctr = 0;  //compteur du nombre de coups
    int xc = OPP(current_side);

    for (cp=0; cp<16; ++cp)
    {
        sq = plist[current_side][cp];
        if (sq != VIDE)
        {
            switch (piece[sq])
            {
            case PION:
                if(current_side == BLANC)
                {
                    //promos
                    if(piece[sq+16] == VIDE)
                    {
                        if(ROW(sq) == 6)  //promotions
                            copie_promotion(sq, (sq+16), PROMO, -1, pBuf, &ctr);
                    }
                    //captures
                    if(EN_JEU(sq+15) && couleur[(sq+15)] == NOIR)
                    {
                        if(ROW(sq) == 6)
                            copie_promotion(sq, (sq+15), PROMO_CAPTURE, -1, pBuf, &ctr);
                        else
                            copie_coup(sq, (sq+15), CAPTURE, -1, pBuf, &ctr);
                    }
                    if(EN_JEU(sq+17) && couleur[(sq+17)] == NOIR)
                    {
                        if(ROW(sq) == 6)
                            copie_promotion(sq, (sq+17), PROMO_CAPTURE, -1, pBuf, &ctr);
                        else
                            copie_coup(sq, (sq+17), CAPTURE, -1, pBuf, &ctr);
                    }
                }
                else
                {
                    if(piece[sq-16] == VIDE)
                    {
                        if(ROW(sq) == 1)    //promotions
                            copie_promotion(sq, (sq-16), PROMO, -1, pBuf, &ctr);
                    }
                    //captures
                    if(EN_JEU(sq-15) && couleur[sq-15] == BLANC)
                    {
                        if(ROW(sq) == 1)
                            copie_promotion(sq, (sq-15), PROMO_CAPTURE, -1, pBuf, &ctr);
                        else
                            copie_coup(sq, (sq-15), CAPTURE, -1, pBuf, &ctr);
                    }
                    if(EN_JEU(sq-17) && couleur[sq-17] == BLANC)
                    {
                        if(ROW(sq) == 1)
                            copie_promotion(sq, (sq-17), PROMO_CAPTURE, -1, pBuf, &ctr);
                        else
                            copie_coup(sq, (sq-17), CAPTURE, -1, pBuf, &ctr);
                    }
                }
                break;
            case CAVALIER:
                for(i=0; i<8; ++i)
                {
                    v = v_cavalier[i];
                    if(EN_JEU(sq+v))
                    {
                        if(couleur[sq+v] == xc)
                            copie_coup(sq, (sq+v), CAPTURE, -1, pBuf, &ctr);
                    }
                }
                break;
            case FOU:
                for(i=0; i<4; ++i)
                {
                    for(w=sq;;)
                    {
                        w += v_fou[i];
                        if(!EN_JEU(w))
                            break;
                        if(piece[w] == VIDE)
                            continue;
                        else if(couleur[w] == xc)
                        {
                            copie_coup(sq, w, CAPTURE, -1, pBuf, &ctr);
                            break;
                        }
                        else
                            break;
                    }
                }
                break;
            case TOUR:
                for(i=0; i<4; ++i)
                {
                    for(w=sq;;)
                    {
                        w += v_tour[i];
                        if(!EN_JEU(w))
                            break;
                        if(piece[w] == VIDE)
                            continue;
                        else if(couleur[w] == xc)
                        {
                            copie_coup(sq, w, CAPTURE, -1, pBuf, &ctr);
                            break;
                        }
                        else
                            break;
                    }
                }
                break;
            case DAME:
                for(i=0; i<8; ++i)
                {
                    for(w=sq;;)
                    {
                        w += v_dame[i];
                        if(!EN_JEU(w))
                            break;
                        if(piece[w] == VIDE)
                            continue;
                        else if(couleur[w] == xc)
                        {
                            copie_coup(sq, w, CAPTURE, -1, pBuf, &ctr);
                            break;
                        }
                        else
                            break;
                    }
                }
                break;
            case ROI:
                for(i=0; i<8; ++i)
                {
                    v = v_roi[i];
                    if(EN_JEU(sq+v))
                    {
                        if(couleur[sq+v] == xc)
                            copie_coup(sq, (sq+v), CAPTURE, -1, pBuf, &ctr);
                    }
                }
                break;
            default:
                break;
            }
        }
    }

    //prises en passant
    if(ep != -1)
    {
        if(current_side == BLANC)
        {
            if((EN_JEU(ep-15)) && si_piece(PION, BLANC, ep-15))
                copie_coup((ep-15), ep, EN_PASSANT, -1, pBuf, &ctr);
            if((EN_JEU(ep-17)) && si_piece(PION, BLANC, ep-17))
                copie_coup((ep-17), ep, EN_PASSANT, -1, pBuf, &ctr);
        }
        else
        {
            if((EN_JEU(ep+15)) && si_piece(PION, NOIR, ep+15))
                copie_coup((ep+15), ep, EN_PASSANT, -1, pBuf, &ctr);
            if((EN_JEU(ep+17)) && si_piece(PION, NOIR, ep+17))
                copie_coup((ep+17), ep, EN_PASSANT, -1, pBuf, &ctr);
        }
    }

    return ctr;
}
















