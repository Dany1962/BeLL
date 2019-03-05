#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/timeb.h>
#include <inttypes.h>

#include "board.h"
#include "init.h"
#include "interface.h"
#include "engine.h"
#include "movegen.h"
#include "eval.h"
#include "hash.h"

MOVE programme_joue(int max_depth, int output)
{
    MOVE m;
    MOVE tmp;
    int score = 0;
    int i;
    int temps_intermediaire = 0;
    int ALPHA,BETA;
    int pce,f,t;

    //initialise meilleur coup
    m.ep_flag = -1;
    m.evaluation = -1;
    m.from = -1;
    m.index_cap = -1;
    m.piece_from = -1;
    m.piece_promue = -1;
    m.piece_to = -1;
    m.to = -1;
    m.type = -1;

    //initialise hash tablett[i]->
    for(i=0; i<=HASH_TABLE_SIZE; ++i)
    {
        tt[i].key   = 0;   //codes positions
        tt[i].depth = 0;   //profondeur
        tt[i].flag  = 0;   //exact , alpha ou beta
        tt[i].value = 0;   //evaluation
    }

    //initialise history table
    for(pce=PION; pce<=ROI; ++pce)
    {
        for(f=0; f<128; ++f)
        {
            for(t=0; t<128; ++t)
            {
                history[pce][f][t] = VIDE;
            }
        }
    }

    //temps pour jouer que si partie chrono
    if(max_depth == 100)
    {
        debut = get_ms();
        fin = debut + time_left;
    }

    //initialise bounds
    ALPHA = -MATE;
    BETA  = MATE;

    //initialise nodes et qnodes
    nodes = 0;
    qnodes = 0;
    total_nodes = 0;
    total_qnodes = 0;
    pv_nodes = 0;
    cut_nodes = 0;
    all_nodes = 0;
    killer_moves = 0;

    //initialise hash positions
    hash_found = 0;
    hash_test  = 0;

    if(output == 1)
    {
        printf("  P  Coup   Eval  Temps       Nodes      Qnodes\n");
        printf("-----------------------------------------------\n");
    }

    for(i=2; i<=max_depth; ++i)
    {
        nodes = 0;
        qnodes = 0;
        /*if(algo_ab)
            score = alpha_beta(ALPHA, BETA, i, &m, 0);
        else if(algo_pvs)*/
            score = pvs(ALPHA, BETA, i, &m, 0);
        /*else
            printf("quel algo je prends ?\n");*/
        if(max_depth == 100)
        {
            temps_intermediaire = get_ms();
            if(fin_recherche)
                break;
        }

        total_nodes += nodes;
        total_qnodes += qnodes;

        tmp = m;

        if(output == 1) // mode console
        {

            float val1 = (float)score/100;
            float val2 = ((float)(temps_intermediaire - debut)/1000);
            printf("%3d  %s  %-+2.2f  %2.2f   %10d  %10d\n",i,move_str(tmp),val1,val2,nodes,qnodes);

        }
        else if(output == 2)        //mode GUI  (exemple : 9 ply, score=1.56, time = 10.84 seconds, nodes=48000, PV = "Nf3 Nc6 Nc3 Nf6" )
        {
            printf("%3d %3d %3d %10d %s\n",i,score, (temps_intermediaire-debut)/1000, nodes,move_str(tmp));
            fflush(stdout);
        }

        if((score > 90000) || (score < -90000))
            break;
    }
    fin = get_ms();
    if(output == 1)
    {
        printf("-----------------------------------------------\n\n");
    }
    return tmp;
}

int jouer_coup(MOVE m)
{
    int xside = OPP(side);
    int f = m.from;
    int t = m.to;
    int pf = m.piece_from;
    int pt = m.piece_to;
    int pp = m.piece_promue;
    int ty = m.type;
    int tmp;
    int echec;
    int sq_roi = -1;

    //sauvegardes
    hist[hdp].m           = m;
    hist[hdp].cap         = m.piece_to;
    hist[hdp].m.index_cap = (pt != VIDE ? indices[t] : VIDE);
    hist[hdp].castle      = castle;
    hist[hdp].cinquante   = cinquante;
    hist[hdp].ep          = ep;
    hist[hdp].hash_pos = hash_position;

    //exécution du coup par type
    switch(ty)
    {
    case NORMAL :
        vider_la_case(f, pf, side);
        occuper_la_case(t, pf, side);
        break;
    case CAPTURE :
        vider_la_case(f, pf, side);
        vider_la_case(t, pt, xside);
        occuper_la_case(t, pf, side);
        break;
    case EN_PASSANT :
        vider_la_case(f, PION, side);
        occuper_la_case(t, PION, side);
        if(side == BLANC)
        {
            vider_la_case((t-16), PION, xside);
            hist[hdp].m.index_cap = indices[(t-16)];
            plist[xside][indices[(t-16)]] = VIDE;
            indices[(t-16)] = VIDE;
        }
        else
        {
            vider_la_case((t+16), PION, xside);
            hist[hdp].m.index_cap = indices[(t+16)];
            plist[xside][indices[(t+16)]] = VIDE;
            indices[(t+16)] = VIDE;
        }
        break;
    case PROMO_CAPTURE :
        vider_la_case(f, PION, side);
        vider_la_case(t, pt, xside);
        occuper_la_case(t, pp, side);
        break;
    case PROMO :
        vider_la_case(f, PION, side);
        occuper_la_case(t, pp, side);
        break;
    case PETIT_ROQUE :
        vider_la_case(f, ROI, side);
        occuper_la_case(t, ROI, side);
        //déplacer la tour
        vider_la_case(from_pr[side], TOUR, side);
        occuper_la_case(to_pr[side], TOUR, side);
        //update plist tour roque
        tmp = indices[from_pr[side]];
        indices[to_pr[side]] = tmp;
        indices[from_pr[side]] = VIDE;
        plist[side][tmp] = to_pr[side];
        break;
    case GRAND_ROQUE :
        vider_la_case(f, ROI, side);
        occuper_la_case(t, pf, side);
        //déplacer la tour
        vider_la_case(from_gr[side], TOUR, side);
        occuper_la_case(to_gr[side], TOUR, side);
        //update plist tour roque
        tmp = indices[from_gr[side]];
        indices[to_gr[side]] = tmp;
        indices[from_gr[side]] = VIDE;
        plist[side][tmp] = to_gr[side];
        break;
    default:
        break;
    }

    //update plist
    if(pt != VIDE)
        plist[xside][indices[t]] = VIDE;
    tmp = indices[f];
    indices[t] = tmp;
    indices[f] = VIDE;
    plist[side][tmp] = t;


    //update position du roi (si celui ci a jouée)
    if(pf == ROI)
        pos_roi[side] = t;

    //update profondeur et nbre de coups
    prof++;
    hdp++;

    //update ep flag et hash code
    if(ep != -1)
        hash_position ^= val_en_passant[ep];
    ep = m.ep_flag;
    if(ep != -1)
        hash_position ^= val_en_passant[ep];

    //update 50 moves rule
    if(pf == PION || ty == CAPTURE || ty == EN_PASSANT)
        cinquante = 0;
    else
        ++cinquante;

    //update droit aux roques et hash code
    hash_position ^= droit_aux_roques[castle];
    castle &= castle_mask[f] & castle_mask[t];
    hash_position ^= droit_aux_roques[castle];

    //ici , test si le roi est attaqué
    sq_roi = pos_roi[side];
    echec = case_attaquee(sq_roi, xside);

    //update couleur et hash code
    side = (BLANC + NOIR) - side;
    hash_position ^= val_couleur[side];

    if(echec)
    {
        dejouer_coup();
        return VRAI;
    }

    return FAUX;
}

void dejouer_coup()
{
    hash_position ^= val_couleur[side];
    side = (BLANC + NOIR) - side;

    int xside = OPP(side);
    int f;
    int t;
    int pf;
    int pt;
    int pp;
    int ty;
    int index;

    //retour en arriere d'un coup
    hdp--;
    prof--;

    f     = hist[hdp].m.from;
    t     = hist[hdp].m.to;
    pf    = hist[hdp].m.piece_from;
    pt    = hist[hdp].m.piece_to;
    pp    = hist[hdp].m.piece_promue;
    ty    = hist[hdp].m.type;
    index = hist[hdp].m.index_cap;

    //annulation du coup par type
    switch(ty)
    {
    case NORMAL :
        vider_la_case(t, pf, side);
        occuper_la_case(f, pf, side);
        break;
    case CAPTURE :
        vider_la_case(t, pf, side);
        occuper_la_case(f, pf, side);
        occuper_la_case(t, pt, xside);
        break;
    case EN_PASSANT :
        vider_la_case(t, PION, side);
        occuper_la_case(f, PION, side);
        if(side == BLANC)
        {
            occuper_la_case((t-16), PION, xside);
            indices[(t-16)] = index;
            plist[xside][index] = (t-16);
        }
        else
        {
            occuper_la_case((t+16), PION, xside);
            indices[(t+16)] = index;
            plist[xside][index] = (t+16);
        }
        break;
    case PROMO_CAPTURE :
        vider_la_case(t, pp, side);
        occuper_la_case(f, PION, side);
        occuper_la_case(t, pt, xside);
        break;
    case PROMO :
        vider_la_case(t, pp, side);
        occuper_la_case(f, PION, side);
        break;
    case PETIT_ROQUE :
        vider_la_case(t, ROI, side);
        occuper_la_case(f, ROI, side);
        vider_la_case(to_pr[side], TOUR, side);
        occuper_la_case(from_pr[side], TOUR, side);
        //update plist tour
        indices[from_pr[side]] = indices[to_pr[side]];
        indices[to_pr[side]] = VIDE;
        plist[side][indices[from_pr[side]]] = from_pr[side];
        break;
    case GRAND_ROQUE :
        vider_la_case(t, ROI, side);
        occuper_la_case(f, pf, side);
        vider_la_case(to_gr[side], TOUR, side);
        occuper_la_case(from_gr[side], TOUR, side);
        //update plist tour
        indices[from_gr[side]] = indices[to_gr[side]];
        indices[to_gr[side]] = VIDE;
        plist[side][indices[from_gr[side]]] = from_gr[side];
        break;
    default:
        break;
    }

    //update plist
    indices[f] = indices[t];
    indices[t] = VIDE;
    plist[side][indices[f]] = f;
    if(pt != VIDE)
    {
        indices[t] = index;
        plist[xside][index] = t;
    }

    //backup et hash code
    hash_position ^= droit_aux_roques[castle];
    castle    = hist[hdp].castle;
    hash_position ^= droit_aux_roques[castle];
    if(ep != -1)
        hash_position ^= val_en_passant[ep];
    ep        = hist[hdp].ep;
    if(ep != -1)
        hash_position ^= val_en_passant[ep];

    cinquante = hist[hdp].cinquante;

    if(pf == ROI)
        pos_roi[side] = f;
}

void vider_la_case(int sq, int pce, int coul)
{
    //update echiquier et couleur
    piece[sq]   = VIDE;
    couleur[sq] = VIDE;

    //update materiel
    mat_op -= materiel_op[coul][pce];
    mat_eg -= materiel_eg[coul][pce];

    //update psqt
    pst_op -= psqt_op[coul][pce][sq];
    pst_eg -= psqt_eg[coul][pce][sq];

    //update nb pièces
    nb_pieces[coul][pce]--;

    //update hash code
    hash_position ^= hash_table[coul][pce][sq];
}

void occuper_la_case(int sq, int pce, int coul)
{
    //update echiquier et couleur
    piece[sq]   = pce;
    couleur[sq] = coul;

    //update materiel
    mat_op += materiel_op[coul][pce];
    mat_eg += materiel_eg[coul][pce];

    //update psqt
    pst_op += psqt_op[coul][pce][sq];
    pst_eg += psqt_eg[coul][pce][sq];

    //update nb pièces
    nb_pieces[coul][pce]++;

    //update hash code
    hash_position ^= hash_table[coul][pce][sq];
}

int case_attaquee(int sq_r, int opp_coul)
{
    int cp;
    int sq;
    int pa;

    for (cp=0; cp<16; ++cp)
    {
        sq = plist[opp_coul][cp];
        if (sq != VIDE)
        {
            switch(piece[sq])
            {
            case PION:
                if(opp_coul == BLANC)
                {
                    if(EN_JEU(sq_r-17) && piece[sq_r-17] == PION && couleur[sq_r-17] == BLANC)
                        return VRAI;
                    if(EN_JEU(sq_r-15) && piece[sq_r-15] == PION && couleur[sq_r-15] == BLANC)
                        return VRAI;
                }
                else
                {
                    if(EN_JEU(sq_r+17) && piece[sq_r+17] == PION && couleur[sq_r+17] == NOIR)
                        return VRAI;
                    if(EN_JEU(sq_r+15) && piece[sq_r+15] == PION && couleur[sq_r+15] == NOIR)
                        return VRAI;
                }
                break;
            case CAVALIER:
                if(table_attaques[sq_r - sq + 128] == attack_n)
                    return VRAI;
                break;
            case FOU:
                pa = table_attaques[sq_r - sq + 128];
                if(pa == attack_kqbbp || pa == attack_kqbwp || pa == attack_qb)
                {
                    if(traverse_delta(sq, sq_r))
                        return VRAI;
                }
                break;
            case TOUR:
                pa = table_attaques[sq_r - sq + 128];
                if(pa == attack_kqr || pa == attack_qr)
                {
                    if(traverse_delta(sq, sq_r))
                        return VRAI;
                }
                break;
            case DAME:
                pa = table_attaques[sq_r - sq + 128];
                if(pa != attack_none && pa != attack_n)
                {
                    if(traverse_delta(sq, sq_r))
                        return VRAI;
                }
                break;
            case ROI:
                pa = table_attaques[sq_r - sq + 128];
                if(pa == attack_kqbbp || pa == attack_kqbwp || pa == attack_kqr)
                {
                    if(traverse_delta(sq, sq_r))
                        return VRAI;
                }
                break;
            default:
                break;
            }
        }
    }
    return FAUX;
}

int traverse_delta(int sq_p, int sq_a)
{
    int index = sq_p;
    int delta = table_delta[sq_a - sq_p + 128];

    while(1)
    {
        index += delta;
        if(index == sq_a)
            return VRAI;
        if(piece[index] != VIDE)
            return FAUX;
    }
}

//fonction gestion de temps
int ftime_ok = FAUX;
int get_ms()
{
    struct timeb timebuffer;
    ftime(&timebuffer);
    if (timebuffer.millitm != 0)
        ftime_ok = VRAI;
    return (timebuffer.time * 1000) + timebuffer.millitm;
}

//controle de temps
int controle_si_temps_depasse()
{
    if(get_ms() >= fin)
        return VRAI;

    return FAUX;
}

//fonction calcule perft
long int perft (int p)
{
    int i;
    int movecnt;
    long int nodes = 0;

    if (!p)
        return 1;
    MOVE moveBuf[200];
    movecnt = gen_coups(side, moveBuf);
    for (i = 0; i < movecnt; ++i)
    {
        if (jouer_coup(moveBuf[i]))
            continue;
        nodes += perft (p - 1);
        dejouer_coup();
    }
    return nodes;
}

//--------------------------------------------------------------------
//                      RECHERCHE
int alpha_beta(int alpha, int beta, int depth, MOVE * pBestMove, int nm)
{
    MOVE moveBuf[200];
    int ctr = 0;
    int i,k;
    int coup_legal = 0;
    int score = 0;
    MOVE tmpMove;
    int echec = FAUX;
    int xside = OPP(side);
    int rep = FAUX;
    int hashf = hashfALPHA;

    //controle temps
    if(time_left != 2000000)
    {
        fin_recherche = controle_si_temps_depasse();
        if(fin_recherche)
            return 0;
    }

    //3 fold rep
    rep = triple_repetition();
    if(prof && (rep >= 2))
        return 0;

    //position dans la table de hashage ?
    if(prof)
    {
        score = probe_hash(depth, alpha, beta);
        if(score != valINCONNUE)
            return score;
    }

    //nulle 50 coups
    if(cinquante >= 100)
        return 0;

    //si echec ? extension 1 profondeur
    echec = case_attaquee(pos_roi[side], xside);
    if(echec)
        ++depth;

    //bas de l'arbre , qs
    if(depth <= 0)
        return quiesce(alpha, beta, &tmpMove);

    //nulmove pruning
    if(!echec && ok_pour_nul_move() && depth >= 2 && nm)
    {
        jouer_coup_nul();
        score = -alpha_beta(-beta, -(beta-1), depth - R(depth) - 1, &tmpMove, 0);
        dejouer_coup_nul();
        if (score >= beta)
            return beta;
    }

    //génération des coups
    ctr = gen_coups(side, moveBuf);

    //boucle des coups
    for (i = 0; i < ctr; ++i)
    {
        meilleur_coup_suivant(moveBuf, ctr, i);

        if(jouer_coup(moveBuf[i]))
            continue;
        coup_legal++;
        ++nodes;

        //récursion alpha beta
        score = -alpha_beta(-beta, -alpha, depth - 1, &tmpMove, 1);

        dejouer_coup();

        if(fin_recherche)
            return 0;

        if(score >= beta)
        {
            save_hash(depth, beta, hashfBETA, pBestMove);
            if(moveBuf[i].type == NORMAL)
            {
                //update history
                int pce = moveBuf[i].piece_from;
                int f   = moveBuf[i].from;
                int t   = moveBuf[i].to;
                history[pce][f][t] += (depth * depth);
                if(history[pce][f][t] >= 20000)
                    history[pce][f][t] /= 2;
                for(k=0; k<i; ++k)
                {
                    if(moveBuf[k].type == NORMAL)
                    {
                        history[moveBuf[k].piece_from][moveBuf[k].from][moveBuf[k].to] -= depth;
                        if(history[moveBuf[k].piece_from][moveBuf[k].from][moveBuf[k].to] < -20000)
                            history[moveBuf[k].piece_from][moveBuf[k].from][moveBuf[k].to] /= 2;
                    }
                }
                //update killer
                killer_moves++;
                killer1[prof].from = moveBuf[i].from;
                killer1[prof].to   = moveBuf[i].to;
                killer1[prof].piece_from = moveBuf[i].piece_from;
                killer1[prof].piece_to   = moveBuf[i].piece_to;
            }
            cut_nodes++;
            return beta;   //  fail hard beta-cutoff
        }

        if(score > alpha)
        {
            alpha = score; // alpha acts like max in MiniMax
            *pBestMove = moveBuf[i];
            hashf = hashfEXACT;
            pv_nodes++;
        }
        else
            all_nodes++;
    }
    //aucun coup légal ? situation de Mat ou de Pat retourné
    if(!coup_legal)
    {
        if(case_attaquee(pos_roi[side], OPP(side)))
            return -MATE + prof;
        else
            return 0;
    }

    save_hash(depth, alpha, hashf, pBestMove);
    return alpha;
}

int ok_pour_nul_move()
{
    int p;
   /* int pl = pieces_lourdes(); //nombre de pièces lourdes (tours et dames)
    int pmb = pieces_mineures_blanc();
    int pmn = pieces_mineures_noir();*/

    p = phase();

    //pas de nulmove si finale de pions
    if(p >= 18)
        return FAUX;

    //pas de nulmove si plus de pieces lourdes et pieces mineures <=  2 (des deux camps)
    /*if(!pl && pmb <= 2 && pmn <= 2)
        return FAUX;*/

    return VRAI;
}

void jouer_coup_nul()
{
    //sauvegardes
    hist[hdp].castle      = castle;
    hist[hdp].cinquante   = cinquante;
    hist[hdp].ep          = ep;

    prof++;
    hdp++;

    //update couleur et hash code
    side = (BLANC + NOIR) - side;
    hash_position ^= val_couleur[side];

    ep = -1;
}

void dejouer_coup_nul()
{
    hash_position ^= val_couleur[side];
    side = (BLANC + NOIR) - side;

    prof--;
    hdp--;

    //backup
    castle = hist[hdp].castle;
    ep = hist[hdp].ep;
    cinquante = hist[hdp].cinquante;
}

int quiesce(int alpha, int beta, MOVE * pBestMove)
{
    MOVE moveBuf[200];
    int ctr = 0;
    int stand_pat = eval();
    int i;
    int score = 0;
    MOVE tmpMove;

    //controle temps
    if(time_left != 2000000)
    {
        fin_recherche = controle_si_temps_depasse();
        if(fin_recherche)
            return 0;
    }

    //stand pat
    if(stand_pat >= beta)
        return beta;
    if(alpha < stand_pat)
        alpha = stand_pat;

    //génération des coups
    ctr = gen_caps(side, moveBuf);

    //boucle coups , on regagarde que captures , promos , en passant
    for (i = 0; i < ctr; ++i)
    {
        meilleur_coup_suivant(moveBuf, ctr, i);

        if(jouer_coup(moveBuf[i]))
            continue;

        ++qnodes;

        score = -quiesce(-beta, -alpha, &tmpMove);

        dejouer_coup();
        if(fin_recherche)
            return 0;

        if(score >= beta)
        {
            cut_nodes++;
            return beta;
        }

        if(score > alpha)
        {
            alpha = score;
            *pBestMove = moveBuf[i];
            pv_nodes++;
        }
        else
            all_nodes++;
    }
    return alpha;
}

void meilleur_coup_suivant(MOVE *ptable, int nb_coups, int debut)
{
    int ms;
    int mi = 0;
    int i;
    MOVE temp;

    ms = -MATE;
    for(i = debut; i < nb_coups; ++i)
    {
        if(ptable[i].evaluation > ms)
        {
            mi = i;
            ms = ptable[i].evaluation;
        }
    }

    temp = ptable[debut];
    ptable[debut] = ptable[mi];
    ptable[mi] = temp;
}

int triple_repetition()
{
    int occurences = 0;
    int i;

    for(i = hdp - cinquante; i < hdp; ++i)
    {
        if(hist[i].hash_pos == hash_position)
            ++occurences;
    }
    return occurences;
}

int pvs(int alpha, int beta, int depth, MOVE * pBestMove, int nm)
{
    MOVE moveBuf[200];
    int ctr = 0;
    int i,k;
    int coup_legal = 0;
    int score = 0;
    MOVE tmpMove;
    int echec = FAUX;
    int xside = OPP(side);
    int rep = FAUX;
    int hashf = hashfALPHA;
    int pv_node =((beta-alpha>1) ? VRAI : FAUX);

    //controle temps
    if(time_left != 2000000)
    {
        fin_recherche = controle_si_temps_depasse();
        if(fin_recherche)
            return 0;
    }

    //position dans la table de hashage ?
    if(prof)
    {
        score = probe_hash(depth, alpha, beta);
        if(score != valINCONNUE)
            return score;
    }

    //nulle 50 coups
    if(cinquante >= 100)
        return 0;

    //3 fold rep
    rep = triple_repetition();
    if(prof && (rep >= 2))
        return 0;

    //si echec ? extension 1 profondeur
    echec = case_attaquee(pos_roi[side], xside);
    if(echec)
        ++depth;

    //bas de l'arbre , qs
    if(depth <= 0)
        return quiesce(alpha, beta, &tmpMove);

    //nulmove pruning
    if(!echec && ok_pour_nul_move() && depth >= 2 && nm && !pv_node)
    {
        jouer_coup_nul();
        score = -alpha_beta(-beta, -(beta-1), depth - R(depth) - 1, &tmpMove, 0);
        dejouer_coup_nul();
        if (score >= beta)
            return beta;
    }

    //génération des coups
    ctr = gen_coups(side, moveBuf);

    //boucle des coups
    for (i = 0; i < ctr; ++i)
    {
        meilleur_coup_suivant(moveBuf, ctr, i);

        if(jouer_coup(moveBuf[i]))
            continue;
        coup_legal++;
        ++nodes;

        if(coup_legal == 1)
            score = -pvs(-beta, -alpha, depth - 1, &tmpMove, 1);
        else
        {
            score = -pvs(-alpha-1, -alpha, depth - 1, &tmpMove, 1);
            if(score > alpha && score < beta)
                score = -pvs(-beta, -alpha, depth - 1, &tmpMove, 1);
        }

        dejouer_coup();

        if(fin_recherche)
            return 0;

        if(score >= beta)
        {
            save_hash(depth, beta, hashfBETA, pBestMove);
            if(moveBuf[i].type == NORMAL)
            {
                //update history
                int pce = moveBuf[i].piece_from;
                int f   = moveBuf[i].from;
                int t   = moveBuf[i].to;
                history[pce][f][t] += (depth * depth);
                if(history[pce][f][t] >= 20000)
                    history[pce][f][t] /= 2;
                for(k=0; k<i; ++k)
                {
                    if(moveBuf[k].type == NORMAL)
                    {
                        history[moveBuf[k].piece_from][moveBuf[k].from][moveBuf[k].to] -= depth;
                        if(history[moveBuf[k].piece_from][moveBuf[k].from][moveBuf[k].to] < -20000)
                            history[moveBuf[k].piece_from][moveBuf[k].from][moveBuf[k].to] /= 2;
                    }
                }
                //update killer
                killer_moves++;
                killer1[prof].from = moveBuf[i].from;
                killer1[prof].to   = moveBuf[i].to;
                killer1[prof].piece_from = moveBuf[i].piece_from;
                killer1[prof].piece_to   = moveBuf[i].piece_to;
            }
            cut_nodes++;
            return beta;   //  fail hard beta-cutoff
        }

        if(score > alpha)
        {
            alpha = score; // alpha acts like max in MiniMax
            *pBestMove = moveBuf[i];
            hashf = hashfEXACT;
            pv_nodes++;
        }
        else
            all_nodes++;
    }
    //aucun coup légal ? situation de Mat ou de Pat retourné
    if(!coup_legal)
    {
        if(case_attaquee(pos_roi[side], OPP(side)))
            return -MATE + prof;
        else
            return 0;
    }

    save_hash(depth, alpha, hashf, pBestMove);
    return alpha;
}

