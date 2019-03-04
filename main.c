#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "board.h"
#include "init.h"
#include "interface.h"
#include "engine.h"
#include "edit.h"
#include "version.h"
#include "movegen.h"
#include "eval.h"
#include "hash.h"

int main()
{
    char s[256];
    long int count = 0;
    int debut,fin;
    int max_depth;
    int coup_legal;
    MOVE bestmove;
    int temps_max;
    int mps,base,inc;
    int score;

    printf("--- BeLL %ld.%ld.%ld ---\n",MAJOR,MINOR,BUILD);
    printf("Date :  %s/%s/%s\n\n",DATE,MONTH,YEAR);

    //initialisations
    initialise_board();
    initialise_eval();
    initialise_hash();

    affiche_echiquier();

    //boucle infinie , entrées utilisateur
    for(;;)
    {
        if(side == computer_side)   //programme joue
        {
            bestmove = programme_joue(max_depth, 1);
            jouer_coup(bestmove);
            affiche_echiquier();
            affiche_resultat();
            continue;
        }

        if (scanf("%s", s) == EOF)  //ferme le programme
            return 0;
        if (!strcmp(s, "quit"))     //quitte l'application
        {
            printf("bye ! ;-) \n");
            return 0;
        }
        if (!strcmp(s, "d"))        //affichage de l'échiquier et des infos
        {
            affiche_echiquier();
            continue;
        }
        if (!strcmp(s, "go"))       //le programme joue avec la couleur en cours
        {
            computer_side = side;
            //affiche_resultat();
            continue;
        }
        if (!strcmp(s, "sd"))       //profondeur fixe
        {
            scanf("%d", &max_depth);
            time_left = 2000000;
            continue;
        }
        if (!strcmp(s, "st"))       //jeu chronométré
        {
            scanf("%d", &time_left);
            max_depth = 100;
            continue;
        }
        if (!strcmp(s, "level"))       //pour jeu blitz ou tournament (dans la GUI) entrée de MPS, BASE , INC
        {
            scanf("%d %d %d",&mps,&base,&inc);
            if(mps)
            {
                temps_max = (((base * 60) / (mps - hdp)) * 1000);
                printf("Temps Par Coup : %d\n",temps_max);
            }
            max_depth = 100;
            continue;
        }
        if (!strcmp(s, "undo"))     //annule un coup
        {
            if(hdp)
            {
                dejouer_coup();
                computer_side = VIDE;
                affiche_echiquier();
            }
            continue;
        }
        if (!strcmp(s, "new"))      //commence une nouvelle partie
        {
            init();
            affiche_echiquier();
            continue;
        }
        if (!strcmp(s, "quit"))     //quitte l'application
        {
            printf("bye ! ;-) \n");
            return 0;
        }
        if (!strcmp(s, "xboard"))   //passe en mode GUI
        {
            xboard();
            continue;
        }
        if (!strcmp(s, "stop"))     //arrete le programme avec la couleur en cours
        {
            computer_side = VIDE;
            continue;
        }
        //utilisées pour débuggage
        if (!strcmp(s, "gen"))
        {
            generation_des_coups();
            continue;
        }
        if (!strcmp(s, "lst"))
        {
            affiche_liste();
            continue;
        }
        if (!strcmp(s, "perft"))
        {
            scanf ("%d", &max_depth);
            debut = get_ms();
            count = perft(max_depth);
            fin = get_ms();
            float val2 = ((float)(fin - debut)/1000);
            printf ("Noeuds = %ld\n", count);
            printf ("Temps  = %2.2f secondes\n", val2);
            continue;
        }
        if (!strcmp(s, "edit"))
        {
            edit();
            continue;
        }
        if (!strcmp(s, "e"))     //evaluation statique détaillée
        {
            e_valide = VRAI;
            score = eval();
            printf("SCORE : %d\n",score);
            e_valide = FAUX;
            continue;
        }
        if (!strcmp(s, "ab"))
        {
            algo_pvs = FAUX;
            algo_ab  = VRAI;
            continue;
        }
        if (!strcmp(s, "pvs"))
        {
            algo_pvs = VRAI;
            algo_ab  = FAUX;
            continue;
        }
        //ici entrée inconnue , on vérifie si l'utilisateur a entré un coup
        coup_legal = verif_coup(s);
        if(coup_legal)
        {
            affiche_echiquier();
            affiche_resultat();
        }
        else
            printf("Illegal move.\n");
    }

    return 0;
}
