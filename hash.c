#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <inttypes.h>

#include "board.h"
#include "hash.h"
#include "engine.h"

void init_hash_table()
{
    int c, p, sq;
    int ctr = 0;

    memset(hash_table, 0, sizeof(hash_table));

    //code couleur/piece/case
    for(c=BLANC;c<=NOIR;++c)
    {
        for(p=PION;p<=ROI;++p)
        {
            for(sq=0;sq<=119;++sq)
            {
                if(EN_JEU(sq))
                {
                    hash_table[c][p][sq] = jeu_de_valeurs_64_bits[ctr];
                    ++ctr;
                }
            }
        }
    }

    //droit aux roques
    for(c=0;c<16;++c)
    {
        droit_aux_roques[c] = jeu_de_valeurs_64_bits[ctr];
        ++ctr;
    }

    //codes cases en passant
    for(c=0;c<120;++c)
    {
        if(EN_JEU(c))
        {
            val_en_passant[c] = jeu_de_valeurs_64_bits[ctr];
            ++ctr;
        }
    }
    //codes couleurs
    val_couleur[BLANC] = jeu_de_valeurs_64_bits[ctr];
    ++ctr;
    val_couleur[NOIR]  = jeu_de_valeurs_64_bits[ctr];
    ++ctr;
}

U64 init_code_position()
{
    int c;
    U64 code = 0;

    for(c=0;c<120;++c)
    {
        if(EN_JEU(c) && piece[c] != VIDE)
            code ^= hash_table[couleur[c]][piece[c]][c];
    }
    code ^= droit_aux_roques[castle];
    if(ep != -1)
        code ^= val_en_passant[ep];
    code ^= val_couleur[side];
    return code;
}

int probe_hash(int depth, int alpha, int beta)
{
    HASHE *phashe = &tt[hash_position % HASH_TABLE_SIZE];

    hash_test++;

    if(phashe->key == hash_position)
    {
        hash_found++;
        if(phashe->depth >= depth)
        {
            if(phashe->flag == hashfEXACT)
                return phashe->value;
            if((phashe->flag == hashfALPHA) && (phashe->value <= alpha))
                return alpha;
            if((phashe->flag == hashfBETA) && (phashe->value >= beta))
                return beta;
        }
    }
    return valINCONNUE;
}


void save_hash(int depth, int val, int hashf, MOVE *pbest)
{
    HASHE *phashe = &tt[hash_position % HASH_TABLE_SIZE];

    phashe->key     = hash_position;
    phashe->depth   = depth;
    phashe->flag    = hashf;
    phashe->value   = val;
    phashe->best    = pbest;
}

