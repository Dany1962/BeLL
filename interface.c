#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <math.h>

#include "board.h"
#include "movegen.h"
#include "interface.h"
#include "engine.h"
#include "eval.h"
#include "hash.h"
#include "version.h"
#include "init.h"
#include "edit.h"

void affiche_echiquier()
{
    char pieceName[] = ".PCFTDRpcftdr";
    int i,j;

    for(i=112; i>=0; i-=16)
    {
        printf("%d + ",ROW(i) + 1);
        for(j=i; j<=i+7; ++j)
        {
            if(piece[j] == VIDE)
                printf("%c ",'.');
            else
                printf("%c ", pieceName[piece[j] + (couleur[j] == BLANC ? 0 : 6)]);
        }
        printf("\n");
    }
    printf("\n");
    printf("    + + + + + + + + \n");
    printf("    a b c d e f g h\n\n");
    affiche_infos();
}

void affiche_infos()
{
    U64 hash_hits = 0;

    printf("-----------------------------------\n");
    printf("Couleur en cours    : %s\n",((side == BLANC) ? "Blanc" :
                                         (side == NOIR) ? "Noir" : "Vide"));
    printf("Couleur BeLL        : %s\n",((computer_side == BLANC) ? "Blanc" :
                                         (computer_side == NOIR) ? "Noir" : "Vide"));
    printf("Numero coup         : %d\n",hdp);
    printf("Roque               : %d\n",castle);
    printf("Ep                  : %d\n",ep);
    printf("Cinquante           : %d\n",cinquante);
    printf("Phase               : %d\n",phase());
    printf("-----------------------------------\n");
    printf("Eval :\n");
    printf("Materiel (op et eg) : %d  %d \n",mat_op,mat_eg);
    printf("PSQT     (op et eg) : %d  %d\n",pst_op,pst_eg);
    printf("-----------------------------------\n");
    printf("Hash :\n");
    printf("Hash code : %" PRIx64 "\n", hash_position);
    printf("Hash test : %" PRId64 "\n", hash_test);
    printf("Hash hits : %" PRId64 "\n", hash_found);
    if(hash_test != 0)
    {
        hash_hits = (hash_found/hash_test) * 100;
        printf("Hash hits : %" PRId64 "\n",hash_hits);
    }
    printf("-----------------------------------\n");
    printf("Nodes : \n");
    printf("Total nodes  : %" PRId64 "\n",total_nodes);
    printf("Total qnodes : %" PRId64 "\n",total_qnodes);
    printf("PV nodes     : %" PRId64 "\n",pv_nodes);
    printf("Cut nodes    : %" PRId64 "\n",cut_nodes);
    printf("All nodes    : %" PRId64 "\n",all_nodes);
    printf("Killer moves : %" PRId64 "\n",killer_moves);
    printf("-----------------------------------\n");

}

void generation_des_coups()
{
    //int couleur;
    //int cat;
    MOVE moveBuf[200];
    int ctr = 0;
    int i;
    //char a[3];
    char s[6];
    int f,t;
    int index = -1;
    int x;

    while(hdp < 10)
    {
        index = -1;

        scanf("%s", s);

        f = s[0] - 'a';
        f += (s[1] - '1') * 16;
        t = s[2] - 'a';
        t += (s[3] - '1') * 16;

        printf("%s  =  %d %d\n",s,f,t);


        //vérification que le coup existe
        ctr = gen_coups(side, moveBuf);

        //affichage liste
        //affiche_liste(moveBuf, ctr);

        for (i = 0; i < ctr; ++i)
        {
            if((f == moveBuf[i].from) && (t == moveBuf[i].to))
            {
                index = i;
                break;
            }
        }
        if(index == -1)
            printf("coup errone\n");
        else
        {
            printf("index = %d\n",index);
            if(jouer_coup(moveBuf[index]))
                printf("coup illegal \n");
            else
            {
                affiche_echiquier();
            }
        }
    }

    while(hdp != 0)
    {
        scanf("%d",&x);
        dejouer_coup();
        affiche_echiquier();
    }
}

char *move_str(MOVE m)
{
    static char str[5];

    sprintf(str,"%c%d%c%d",COL(m.from) + 'a', ROW(m.from)+1,COL(m.to) + 'a', ROW(m.to)+1);

    if(m.type == PROMO || m.type == PROMO_CAPTURE)
    {
        switch(m.piece_promue)
        {
        case CAVALIER:
            strcat(str,"N");
            break;
        case FOU:
            strcat(str,"B");
            break;
        case TOUR:
            strcat(str,"R");
            break;
        case DAME:
            strcat(str,"Q");
            break;
        default:
            break;
        }
    }
    return str;
}


void convert_0x88_a(int sq, char * a)
{
    a[0] = COL(sq) + 'a';
    a[1] = ROW(sq) + '1';
    a[2] = 0;
}

int a_to_0x88(char * a)
{
    int sq;
    sq = a[0] - 'a';
    sq += (a[1] - '1') * 16;

    return sq;
}

void affiche_liste()
{
    int i;
    //char a[3];
    MOVE moveBuf[200];
    int ctr = 0;

    ctr = gen_coups(side, moveBuf);

    printf("\n");
    printf("------------------------------------------------------\n");
    printf("        index   pfrom  pto  ppromue  type  ep  eval\n");
    printf("------------------------------------------------------\n");
    for (i = 0; i < ctr; ++i)
    {
        /*convert_0x88_a(moveBuf[i].from, a);
        printf(a);
        printf("-");
        convert_0x88_a(moveBuf[i].to, a);
        printf(a);
        printf("   :");*/
        printf("%s  %3d     %3d  %3d  %3d      %3d   %3d  %3d\n",
               move_str(moveBuf[i]),
               i,
               moveBuf[i].piece_from,
               moveBuf[i].piece_to,
               moveBuf[i].piece_promue,
               moveBuf[i].type,
               moveBuf[i].ep_flag,
               moveBuf[i].evaluation);
    }
    printf("------------------------------------------------------\n");
}


void affiche_plist()
{
    int i,j;

    printf("pieces blanches : \n");

    for(i=0; i<16; ++i)
        printf("%d ",plist[BLANC][i]);
    printf("\n");
    printf("pieces noires : \n");

    for(i=0; i<16; ++i)
        printf("%d ",plist[NOIR][i]);
    printf("\n\n\n");

    for(i=112; i>=0; i-=16)
    {
        for(j=i; j<=i+7; ++j)
        {
            printf("%3d ",indices[j]);
        }
        printf("\n");
    }
}

int verif_coup(char *s)
{
    MOVE moveBuf[200];
    int movecnt;
    int i;
    int f,t;

    f = s[0] - 'a';
    f += (s[1] - '1') * 16;
    t = s[2] - 'a';
    t += (s[3] - '1') * 16;

    prof = 0;
    movecnt = gen_coups(side, moveBuf);

    for (i = 0; i < movecnt; i++)
        if (moveBuf[i].from == f && moveBuf[i].to == t)
        {
            if (piece[f] == PION && (t < 8 || t > 103))
            {
                if(piece[t] != VIDE)
                    moveBuf[i].type = PROMO_CAPTURE;
                else
                    moveBuf[i].type = PROMO;
                switch (s[4])
                {
                case 'q':
                    moveBuf[i].piece_promue = DAME;
                    break;
                case 'r':
                    moveBuf[i].piece_promue = TOUR;
                    break;
                case 'b':
                    moveBuf[i].piece_promue = FOU;
                    break;
                case 'n':
                    moveBuf[i].piece_promue = CAVALIER;
                    break;
                default:
                    moveBuf[i].piece_promue = DAME;
                }
            }
            if (jouer_coup(moveBuf[i]))
                return FAUX;
            else
                return VRAI;
            break;
        }

    return 0;
}

void affiche_resultat()
{
    MOVE moveBuf[200];
    int movecnt;
    int i;
    int legal = FAUX;
    int rep;
    int xside = OPP(side);
    int echec;

    //on vérifie si la partie est gagnée par un joueur ou nulle
    movecnt = gen_coups(side, moveBuf);

    for (i = 0; i < movecnt; ++i)
    {
        if (jouer_coup(moveBuf[i]))
            continue;
        else
        {
            legal = VRAI;
            dejouer_coup();
            break;
        }
    }
    if(!legal)
    {
        echec = case_attaquee(pos_roi[side], xside);
        if(echec)
        {
            if (side == BLANC)
            {
                printf("0-1 {Les blancs sont mat}\n");
                computer_side = VIDE;
            }
            else
            {
                printf("1-0 {Les noirs sont mat}\n");
                computer_side = VIDE;
            }
        }
        else
        {
            printf("1/2-1/2 {Pat}\n");
            computer_side = VIDE;
        }
    }

    //on vérifie la règle de triple réptition
    rep = triple_repetition();
    if(rep == 3)
    {
        printf("La partie est nulle par triple repetition\n");
        computer_side = VIDE;
    }

    //on vérifie la règle des 50 coups
    if(cinquante >= 100)
    {
        printf("La partie est nulle par la règle des cinquante coups\n");
        computer_side = VIDE;
    }

    //on vérifie la nulle par matériel insufisant
    if(draw_recognizer())
    {
        printf("La partie est nulle par materiel insufisant\n");
        computer_side = VIDE;
    }

}

int xboard()
{
    char inBuf[80], command[80];
    MOVE bestMove;
    //int min,sec;
    int moves_left = 0;
    int nbcoups = 0;
    int i;
    int coups_joues = 0;
    int inc_wb = 0;
    int temps_max;
    int mps,base,inc;

    while(1)
    {

        fflush(stdout);

        if(side == computer_side)   // programme joue
        {
            bestMove = programme_joue(max_depth, postThinking);
            printf("move   %s\n", move_str(bestMove));
            jouer_coup(bestMove);
            //affiche_echiquier();
            //affiche_infos();
            affiche_resultat();
            continue;
        }

        fflush(stdout);
               //ponder mode à developper plus tard
        if(computer_side == ANALYZE)
        {

        }
        else
        {

        }

noPonder:

        if(!strcmp(command, "quit"))
            break;    // breaks out of infinite loop

        if (!fgets(inBuf, 256, stdin))
            return 0;

        // extract the first word
        sscanf(inBuf, "%s", command);

        // recognize the command,and execute it
        if (inBuf[0] == '\n')
            continue;

        if(!strcmp(command, "force"))
        {
            computer_side = VIDE;
            continue;
        }

        /*if(!strcmp(command, "analyze"))
        {
            computer_side = ANALYZE;
            continue;
        }*/

        if(!strcmp(command, "exit"))
        {
            computer_side = VIDE;
            continue;
        }

        if(!strcmp(command, "post"))
        {
            postThinking = ON;
            continue;
        }
        if(!strcmp(command, "nopost"))
        {
            postThinking = OFF;
            continue;
        }

        if(!strcmp(command, "level"))   //pour jeu blitz ou tournament (dans la GUI) entrée de MPS, BASE , INC
        {
            sscanf(inBuf,"level %d %d %d",&mps,&base,&inc);
            max_depth = 100;
            continue;
        }

        if(!strcmp(command, "otim"))
        {
            goto noPonder;    // do not start pondering after receiving time commands, as move will follow immediately
        }

        if(!strcmp(command, "time"))
        {
            sscanf(inBuf, "time %d", &temps_max);
            if(mps == 0)
            {
                temps_max *=10;
                if(inc == 0)
                {
                    if(hdp > 50)
                        temps_max /= 50;
                    else
                        temps_max /= 40;
                }
                else
                {
                    (temps_max /= 30);
                    temps_max += inc;
                }
                time_left = temps_max;
            }
            else
            {
                inc_wb = (inc * 1000);
                nbcoups = (hdp / 2 + 1);
                for(i=mps;; i+=mps)
                {
                    if(nbcoups == i) //dernier coups de la session time_left - 10 secondes
                    {
                        time_left = (temps_max / 2);
                        time_left += inc_wb;
                        break;
                    }
                    else if(nbcoups < i)
                    {
                        moves_left = (i - nbcoups );
                        moves_left++;
                        time_left  = (temps_max / moves_left);
                        time_left *= 10;
                        time_left += inc;
                        break;
                    }
                }
                temps_max -= time_left;
                printf("HDP ET MPS : %d %d\n",hdp,mps);
                printf("NBCOUPS    : %d\n",nbcoups);
                printf("COUPS JOUES: %d\n",coups_joues);
                printf("i          : %d\n",i);
                printf("MOVE LEFT  : %d\n",moves_left);
                printf("TIME LEFT  : %d\n",time_left);
                temps_max -= time_left;
                temps_max += inc_wb;
            }
            max_depth = 100;
            continue;
        }
        if (!strcmp(command, "protover"))
        {
            printf("feature done=0\n");
            printf("feature myname=\"BeLL %ld.%ld.%ld\"\n",MAJOR,MINOR,BUILD);
            printf("feature done=1\n");
            continue;
        }
        if(!strcmp(command, "option"))   // setting of engine-define option; find out which
        {
            //if(scanf(inBuf+7, "Resign=%d",   &resign)         == 1) continue;
            //if(scanf(inBuf+7, "Contempt=%d", &contemptFactor) == 1) continue;
            continue;
        }
        if(!strcmp(command, "sd"))
        {
            sscanf(inBuf, "sd %d", &max_depth);
            temps_max = 2000000;
            time_left = 2000000;
            continue;
        }
        if(!strcmp(command, "st"))
        {
            sscanf(inBuf, "st %d", &temps_max);
            temps_max *= 1000;
            time_left = temps_max;
            max_depth = 100;
            continue;
        }
        if(!strcmp(command, "memory"))
        {
            //SetMemorySize(atoi(inBuf+7));
            continue;
        }
        if(!strcmp(command, "ping"))
        {
            //printf("pong%s", inBuf+4);
            continue;
        }
        if(!strcmp(command, "new"))
        {
            computer_side = VIDE;
            init();
            continue;
        }
        if(!strcmp(command, "setboard"))
        {
            //engineSide = NONE;
            //stm = Setup(inBuf+9);
            continue;
        }
        if(!strcmp(command, "easy"))
        {
            //ponder = OFF;
            continue;
        }
        if(!strcmp(command, "hard"))
        {
            //ponder = ON;
            continue;
        }
        if(!strcmp(command, "undo"))
        {
            if(hdp)
            {
                dejouer_coup();
                computer_side = VIDE;
            }
            continue;
        }
        if(!strcmp(command, "remove"))
        {
            //stm = TakeBack(2);
            continue;
        }
        if(!strcmp(command, "go"))
        {
            computer_side = side;
            continue;
        }

        if(!strcmp(command, "edit"))
        {
            edit();
            continue;
        }
        if(!strcmp(command, "random"))
        {
            //randomize = ON;
            continue;
        }
        if(!strcmp(command, "hint"))
        {
            //if(ponderMove != INVALID) printf("Hint: %s\n", MoveToText(ponderMove));
            continue;
        }
        if(!strcmp(command, "book"))
        {
            continue;
        }
        if(!strcmp(command, "xboard"))
        {
            continue;
        }
        if(!strcmp(command, "computer"))
        {
            continue;
        }
        if(!strcmp(command, "name"))
        {
            continue;
        }
        if(!strcmp(command, "ics"))
        {
            continue;
        }
        if(!strcmp(command, "accepted"))
        {
            continue;
        }
        if(!strcmp(command, "rejected"))
        {
            continue;
        }
        if(!strcmp(command, "variant"))
        {
            continue;
        }
        if(!strcmp(command, ""))
        {
            continue;
        }
        if(command[1] >= '0' && command[1] <= '9')
        {
            printf("%c  \n",command[1]);
            int coup_legal;
            coup_legal = verif_coup(command);
            if(coup_legal)
            {
                //affiche_echiquier();
                affiche_resultat();
            }
            else
            {
                printf("Illegal move.\n");
            }
            continue;
        }
    }
    return 0;
}
