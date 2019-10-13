#ifndef _DOMINION_HELPERS_H
#define _DOMINION_HELPERS_H

#include "dominion.h"

int drawCard(int player, struct gameState *state);
int updateCoins(int player, struct gameState *state, int bonus);
int removeToPlayed(int handPos, struct gameState *state);
int removeToDiscard(int handPos, struct gameState *state);
int removeAllToDiscard(int player, struct gameState *state);
int removeToTrash(int handPos, struct gameState *state);
int shiftHand(int removedPos, struct gameState *state);
int gainCard(int supplyPos, struct gameState *state, int toFlag, int player);
int getCost(int cardNumber);
int cardEffect(int card, int choice1, int choice2, int choice3,
               struct gameState *state, int handPos, int *bonus);
int cardEffectBaron(int handPos, int choice1, struct gameState* state);
int cardEffectMinion(int handPos, int choice1, struct gameState* state);
int cardEffectAmbassador(int handPos, int choice1, int choice2, struct gameState* state);
int cardEffectTribute(int handPos, struct gameState* state);
int cardEffectMine(int handPos, int choice1, int choice2, struct gameState* state);


#endif
