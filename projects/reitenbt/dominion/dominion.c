#include "dominion.h"
#include "dominion_helpers.h"
#include "rngs.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

int compare(const void* a, const void* b) {
    if (*(int*)a > *(int*)b)
        return 1;
    if (*(int*)a < *(int*)b)
        return -1;
    return 0;
}

struct gameState* newGame() {
    struct gameState* g = malloc(sizeof(struct gameState));
    return g;
}

int* kingdomCards(int k1, int k2, int k3, int k4, int k5, int k6, int k7,
                  int k8, int k9, int k10) {
    int* k = malloc(10 * sizeof(int));
    k[0] = k1;
    k[1] = k2;
    k[2] = k3;
    k[3] = k4;
    k[4] = k5;
    k[5] = k6;
    k[6] = k7;
    k[7] = k8;
    k[8] = k9;
    k[9] = k10;
    return k;
}

int initializeGame(int numPlayers, int kingdomCards[10], int randomSeed,
                   struct gameState *state) {
    int i;
    int j;
    int it;

    //set up random number generator
    SelectStream(1);
    PutSeed((long)randomSeed);

    //check number of players
    if (numPlayers > MAX_PLAYERS || numPlayers < 2)
    {
        return -1;
    }

    //set number of players
    state->numPlayers = numPlayers;

    //check selected kingdom cards are different
    for (i = 0; i < 10; i++)
    {
        for (j = 0; j < 10; j++)
        {
            if (j != i && kingdomCards[j] == kingdomCards[i])
            {
                return -1;
            }
        }
    }


    //initialize supply
    ///////////////////////////////

    //set number of Curse cards
    if (numPlayers == 2)
    {
        state->supplyCount[curse] = 10;
    }
    else if (numPlayers == 3)
    {
        state->supplyCount[curse] = 20;
    }
    else
    {
        state->supplyCount[curse] = 30;
    }

    //set number of Victory cards
    if (numPlayers == 2)
    {
        state->supplyCount[estate] = 8;
        state->supplyCount[duchy] = 8;
        state->supplyCount[province] = 8;
    }
    else
    {
        state->supplyCount[estate] = 12;
        state->supplyCount[duchy] = 12;
        state->supplyCount[province] = 12;
    }

    //set number of Treasure cards
    state->supplyCount[copper] = 60 - (7 * numPlayers);
    state->supplyCount[silver] = 40;
    state->supplyCount[gold] = 30;

    //set number of Kingdom cards
    for (i = adventurer; i <= treasure_map; i++)       	//loop all cards
    {
        for (j = 0; j < 10; j++)           		//loop chosen cards
        {
            if (kingdomCards[j] == i)
            {
                //check if card is a 'Victory' Kingdom card
                if (kingdomCards[j] == great_hall || kingdomCards[j] == gardens)
                {
                    if (numPlayers == 2) {
                        state->supplyCount[i] = 8;
                    }
                    else {
                        state->supplyCount[i] = 12;
                    }
                }
                else
                {
                    state->supplyCount[i] = 10;
                }
                break;
            }
            else    //card is not in the set choosen for the game
            {
                state->supplyCount[i] = -1;
            }
        }

    }

    ////////////////////////
    //supply intilization complete

    //set player decks
    for (i = 0; i < numPlayers; i++)
    {
        state->deckCount[i] = 0;
        for (j = 0; j < 3; j++)
        {
            state->deck[i][j] = estate;
            state->deckCount[i]++;
        }
        for (j = 3; j < 10; j++)
        {
            state->deck[i][j] = copper;
            state->deckCount[i]++;
        }
    }

    //shuffle player decks
    for (i = 0; i < numPlayers; i++)
    {
        if ( shuffle(i, state) < 0 )
        {
            return -1;
        }
    }

    //draw player hands
    for (i = 0; i < numPlayers; i++)
    {
        //initialize hand size to zero
        state->handCount[i] = 0;
        state->discardCount[i] = 0;
        //draw 5 cards
        // for (j = 0; j < 5; j++)
        //	{
        //	  drawCard(i, state);
        //	}
    }

    //set embargo tokens to 0 for all supply piles
    for (i = 0; i <= treasure_map; i++)
    {
        state->embargoTokens[i] = 0;
    }

    //initialize first player's turn
    state->outpostPlayed = 0;
    state->phase = 0;
    state->numActions = 1;
    state->numBuys = 1;
    state->playedCardCount = 0;
    state->whoseTurn = 0;
    state->handCount[state->whoseTurn] = 0;
    //int it; move to top

    //Moved draw cards to here, only drawing at the start of a turn
    for (it = 0; it < 5; it++) {
        drawCard(state->whoseTurn, state);
    }

    updateCoins(state->whoseTurn, state, 0);

    return 0;
}

int shuffle(int player, struct gameState *state)
{
    int newDeck[MAX_DECK];
    int newDeckPos = 0;
    int card;
    int i;

    if (state->deckCount[player] < 1)
        return -1;
    qsort ((void*)(state->deck[player]), state->deckCount[player], sizeof(int), compare);
    /* SORT CARDS IN DECK TO ENSURE DETERMINISM! */

    while (state->deckCount[player] > 0) {
        card = floor(Random() * state->deckCount[player]);
        newDeck[newDeckPos] = state->deck[player][card];
        newDeckPos++;
        for (i = card; i < state->deckCount[player]-1; i++) {
            state->deck[player][i] = state->deck[player][i+1];
        }
        state->deckCount[player]--;
    }
    for (i = 0; i < newDeckPos; i++) {
        state->deck[player][i] = newDeck[i];
        state->deckCount[player]++;
    }

    return 0;
}

int playCard(int handPos, int choice1, int choice2, int choice3, struct gameState *state)
{
    int card;
    int coin_bonus = 0; 		//tracks coins gain from actions

    //check if it is the right phase
    if (state->phase != 0)
    {
        return -1;
    }

    //check if player has enough actions
    if ( state->numActions < 1 )
    {
        return -1;
    }

    //get card played
    card = handCard(handPos, state);

    //check if selected card is an action
    if ( card < adventurer || card > treasure_map )
    {
        return -1;
    }

    //play card
    if ( cardEffect(card, choice1, choice2, choice3, state, handPos, &coin_bonus) < 0 )
    {
        return -1;
    }

    //reduce number of actions
    state->numActions--;

    //update coins (Treasure cards may be added with card draws)
    updateCoins(state->whoseTurn, state, coin_bonus);

    return 0;
}

int buyCard(int supplyPos, struct gameState *state) {
    int who;
    if (DEBUG) {
        printf("Entering buyCard...\n");
    }

    // I don't know what to do about the phase thing.

    who = state->whoseTurn;

    if (state->numBuys < 1) {
        if (DEBUG)
            printf("You do not have any buys left\n");
        return -1;
    } else if (supplyCount(supplyPos, state) <1) {
        if (DEBUG)
            printf("There are not any of that type of card left\n");
        return -1;
    } else if (state->coins < getCost(supplyPos)) {
        if (DEBUG)
            printf("You do not have enough money to buy that. You have %d coins.\n", state->coins);
        return -1;
    } else {
        state->phase=1;
        //state->supplyCount[supplyPos]--;
        gainCard(supplyPos, state, 0, who); //card goes in discard, this might be wrong.. (2 means goes into hand, 0 goes into discard)

        state->coins = (state->coins) - (getCost(supplyPos));
        state->numBuys--;
        if (DEBUG)
            printf("You bought card number %d for %d coins. You now have %d buys and %d coins.\n", supplyPos, getCost(supplyPos), state->numBuys, state->coins);
    }

    //state->discard[who][state->discardCount[who]] = supplyPos;
    //state->discardCount[who]++;

    return 0;
}

int numHandCards(struct gameState *state) {
    return state->handCount[ whoseTurn(state) ];
}

int handCard(int handPos, struct gameState *state) {
    int currentPlayer = whoseTurn(state);
    return state->hand[currentPlayer][handPos];
}

int supplyCount(int card, struct gameState *state) {
    return state->supplyCount[card];
}

int fullDeckCount(int player, int card, struct gameState *state) {
    int i;
    int count = 0;

    for (i = 0; i < state->deckCount[player]; i++)
    {
        if (state->deck[player][i] == card) count++;
    }

    for (i = 0; i < state->handCount[player]; i++)
    {
        if (state->hand[player][i] == card) count++;
    }

    for (i = 0; i < state->discardCount[player]; i++)
    {
        if (state->discard[player][i] == card) count++;
    }

    return count;
}

int whoseTurn(struct gameState *state) {
    return state->whoseTurn;
}

int endTurn(struct gameState *state)
{
	// Local variable for the current player.
    int currentPlayer = whoseTurn(state);

    // Discard the player's hand.
	removeAllToDiscard(currentPlayer, state);

	// Discard the played pile.
	for (int i = 0; i < state->playedCardCount; i++)
	{
		state->discard[currentPlayer][state->discardCount[currentPlayer]] = state->playedCards[i];
		state->playedCards[i] = -1;
		state->discardCount[currentPlayer]++;
	}

	// Reset the played pile count.
	state->playedCardCount = 0;

	// Current player draws 5 cards after all other end-of-turn sequencing.
	for (int i = 0; i < 5; i++)
		drawCard(currentPlayer, state);

    // Determine the next player.
	state->whoseTurn = (whoseTurn(state) + 1) % state->numPlayers;

	// Reset turn-by-turn state attributes.
    state->outpostPlayed = 0;
    state->phase = 0;
    state->numActions = 1;
    state->coins = 0;
    state->numBuys = 1;

    //Update money
    updateCoins(state->whoseTurn, state, 0);

    return 0;
}

int isGameOver(struct gameState *state) {
    int i;
    int j;

    //if stack of Province cards is empty, the game ends
    if (state->supplyCount[province] == 0)
    {
        return 1;
    }

    //if three supply pile are at 0, the game ends
    j = 0;
    for (i = 0; i < 25; i++)
    {
        if (state->supplyCount[i] == 0)
        {
            j++;
        }
    }
    if ( j >= 3)
    {
        return 1;
    }

    return 0;
}

int scoreFor (int player, struct gameState *state) {

    int i;
    int score = 0;
    //score from hand
    for (i = 0; i < state->handCount[player]; i++)
    {
        if (state->hand[player][i] == curse) {
            score = score - 1;
        };
        if (state->hand[player][i] == estate) {
            score = score + 1;
        };
        if (state->hand[player][i] == duchy) {
            score = score + 3;
        };
        if (state->hand[player][i] == province) {
            score = score + 6;
        };
        if (state->hand[player][i] == great_hall) {
            score = score + 1;
        };
        if (state->hand[player][i] == gardens) {
            score = score + ( fullDeckCount(player, 0, state) / 10 );
        };
    }

    //score from discard
    for (i = 0; i < state->discardCount[player]; i++)
    {
        if (state->discard[player][i] == curse) {
            score = score - 1;
        };
        if (state->discard[player][i] == estate) {
            score = score + 1;
        };
        if (state->discard[player][i] == duchy) {
            score = score + 3;
        };
        if (state->discard[player][i] == province) {
            score = score + 6;
        };
        if (state->discard[player][i] == great_hall) {
            score = score + 1;
        };
        if (state->discard[player][i] == gardens) {
            score = score + ( fullDeckCount(player, 0, state) / 10 );
        };
    }

    //score from deck
    for (i = 0; i < state->discardCount[player]; i++)
    {
        if (state->deck[player][i] == curse) {
            score = score - 1;
        };
        if (state->deck[player][i] == estate) {
            score = score + 1;
        };
        if (state->deck[player][i] == duchy) {
            score = score + 3;
        };
        if (state->deck[player][i] == province) {
            score = score + 6;
        };
        if (state->deck[player][i] == great_hall) {
            score = score + 1;
        };
        if (state->deck[player][i] == gardens) {
            score = score + ( fullDeckCount(player, 0, state) / 10 );
        };
    }

    return score;
}

int getWinners(int players[MAX_PLAYERS], struct gameState *state) {
    int i;
    int j;
    int highScore;
    int currentPlayer;

    //get score for each player
    for (i = 0; i < MAX_PLAYERS; i++)
    {
        //set unused player scores to -9999
        if (i >= state->numPlayers)
        {
            players[i] = -9999;
        }
        else
        {
            players[i] = scoreFor (i, state);
        }
    }

    //find highest score
    j = 0;
    for (i = 0; i < MAX_PLAYERS; i++)
    {
        if (players[i] > players[j])
        {
            j = i;
        }
    }
    highScore = players[j];

    //add 1 to players who had less turns
    currentPlayer = whoseTurn(state);
    for (i = 0; i < MAX_PLAYERS; i++)
    {
        if ( players[i] == highScore && i > currentPlayer )
        {
            players[i]++;
        }
    }

    //find new highest score
    j = 0;
    for (i = 0; i < MAX_PLAYERS; i++)
    {
        if ( players[i] > players[j] )
        {
            j = i;
        }
    }
    highScore = players[j];

    //set winners in array to 1 and rest to 0
    for (i = 0; i < MAX_PLAYERS; i++)
    {
        if ( players[i] == highScore )
        {
            players[i] = 1;
        }
        else
        {
            players[i] = 0;
        }
    }

    return 0;
}

int drawCard(int player, struct gameState *state)
{
	// If the player's deck is empty...
	if (state->deckCount[player] < 1)
	{
		// Copy their discard pile into it.
		for (int i = 0; i < state->discardCount[player]; i++)
		{
			state->deck[player][i] = state->discard[player][i];
			state->discard[player][i] = -1;
		}

		// Adjust the deck and discard counts.
		state->deckCount[player] = state->discardCount[player];
		state->discardCount[player] = 0;

		// Shuffle the deck.
		shuffle(player, state);
	}

	// If the player's deck is still empty, they cannot draw a card.
	if (state->deckCount[player] == 0)
		return -1;

	// Draw a card.
	state->hand[player][state->handCount[player]] = state->deck[player][state->deckCount[player]];
	state->deck[player][state->deckCount[player]] = -1;
	state->handCount[player]++;
	state->deckCount[player]--;

	return 0;
}

int getCost(int cardNumber)
{
    switch( cardNumber )
    {
    case curse:
        return 0;
    case estate:
        return 2;
    case duchy:
        return 5;
    case province:
        return 8;
    case copper:
        return 0;
    case silver:
        return 3;
    case gold:
        return 6;
    case adventurer:
        return 6;
    case council_room:
        return 5;
    case feast:
        return 4;
    case gardens:
        return 4;
    case mine:
        return 5;
    case remodel:
        return 4;
    case smithy:
        return 4;
    case village:
        return 3;
    case baron:
        return 4;
    case great_hall:
        return 3;
    case minion:
        return 5;
    case steward:
        return 3;
    case tribute:
        return 5;
    case ambassador:
        return 3;
    case cutpurse:
        return 4;
    case embargo:
        return 2;
    case outpost:
        return 5;
    case salvager:
        return 4;
    case sea_hag:
        return 4;
    case treasure_map:
        return 4;
    }

    return -1;
}

int cardEffect(int card, int choice1, int choice2, int choice3, struct gameState *state, int handPos, int *bonus)
{
    int i;
    int j;
    int k;
    int x;
    int index;
    int currentPlayer = whoseTurn(state);
    int nextPlayer = (currentPlayer + 1) % state->numPlayers;

    int temphand[MAX_HAND];// moved above the if statement
    int drawntreasure=0;
    int cardDrawn;
    int z = 0;// this is the counter for the temp hand
    if (nextPlayer > (state->numPlayers - 1)) {
        nextPlayer = 0;
    }


    //uses switch to select card and perform actions
    switch( card )
    {
    case adventurer:
        while(drawntreasure<2) {
            if (state->deckCount[currentPlayer] <1) { //if the deck is empty we need to shuffle discard and add to deck
                shuffle(currentPlayer, state);
            }
            drawCard(currentPlayer, state);
            cardDrawn = state->hand[currentPlayer][state->handCount[currentPlayer]-1];//top card of hand is most recently drawn card.
            if (cardDrawn == copper || cardDrawn == silver || cardDrawn == gold)
                drawntreasure++;
            else {
                temphand[z]=cardDrawn;
                state->handCount[currentPlayer]--; //this should just remove the top card (the most recently drawn one).
                z++;
            }
        }
        while(z-1>=0) {
            state->discard[currentPlayer][state->discardCount[currentPlayer]++]=temphand[z-1]; // discard all cards in play that have been drawn
            z=z-1;
        }
        return 0;

    case council_room:
        //+4 Cards
        for (i = 0; i < 4; i++)
        {
            drawCard(currentPlayer, state);
        }

        //+1 Buy
        state->numBuys++;

        //Each other player draws a card
        for (i = 0; i < state->numPlayers; i++)
        {
            if ( i != currentPlayer )
            {
                drawCard(i, state);
            }
        }

        //put played card in played card pile
        removeToPlayed(handPos, state);

        return 0;

    case feast:
        //gain card with cost up to 5
        //Backup hand
        for (i = 0; i <= state->handCount[currentPlayer]; i++) {
            temphand[i] = state->hand[currentPlayer][i];//Backup card
            state->hand[currentPlayer][i] = -1;//Set to nothing
        }
        //Backup hand

        //Update Coins for Buy
        updateCoins(currentPlayer, state, 5);
        x = 1;//Condition to loop on
        while( x == 1) {//Buy one card
            if (supplyCount(choice1, state) <= 0) {
                if (DEBUG)
                    printf("None of that card left, sorry!\n");

                if (DEBUG) {
                    printf("Cards Left: %d\n", supplyCount(choice1, state));
                }
            }
            else if (state->coins < getCost(choice1)) {
                printf("That card is too expensive!\n");

                if (DEBUG) {
                    printf("Coins: %d < %d\n", state->coins, getCost(choice1));
                }
            }
            else {

                if (DEBUG) {
                    printf("Deck Count: %d\n", state->handCount[currentPlayer] + state->deckCount[currentPlayer] + state->discardCount[currentPlayer]);
                }

                gainCard(choice1, state, 0, currentPlayer);//Gain the card
                x = 0;//No more buying cards

                if (DEBUG) {
                    printf("Deck Count: %d\n", state->handCount[currentPlayer] + state->deckCount[currentPlayer] + state->discardCount[currentPlayer]);
                }

            }
        }

        //Reset Hand
        for (i = 0; i <= state->handCount[currentPlayer]; i++) {
            state->hand[currentPlayer][i] = temphand[i];
            temphand[i] = -1;
        }
        //Reset Hand

        return 0;

    case gardens:
        return -1;

    case mine:
		if (cardEffectMine(handPos, choice1, choice2, state) == 0)
			return 0;
		else
			return -1;

    case remodel:
        j = state->hand[currentPlayer][choice1];  //store card we will trash

        if ( (getCost(state->hand[currentPlayer][choice1]) + 2) > getCost(choice2) )
        {
            return -1;
        }

        gainCard(choice2, state, 0, currentPlayer);

        //discard card from hand
        removeToPlayed(handPos, state);

        //discard trashed card
        for (i = 0; i < state->handCount[currentPlayer]; i++)
        {
            if (state->hand[currentPlayer][i] == j)
            {
                removeToTrash(i, state);
                break;
            }
        }


        return 0;

    case smithy:
        //+3 Cards
        for (i = 0; i < 3; i++)
        {
            drawCard(currentPlayer, state);
        }

        //discard card from hand
        removeToPlayed(handPos, state);
        return 0;

    case village:
        //+1 Card
        drawCard(currentPlayer, state);

        //+2 Actions
        state->numActions = state->numActions + 2;

        //discard played card from hand
        removeToPlayed(handPos, state);
        return 0;

    case baron:
		if (cardEffectBaron(handPos, choice1, state) == 0)
			return 0;
		else
			return -1;

    case great_hall:
        //+1 Card
        drawCard(currentPlayer, state);

        //+1 Actions
        state->numActions++;

        //discard card from hand
        removeToPlayed(handPos, state);
        return 0;

    case minion:
		if (cardEffectMinion(handPos, choice1, state) == 0)
			return 0;
		else
			return -1;

    case steward:
        if (choice1 == 1)
        {
            //+2 cards
            drawCard(currentPlayer, state);
            drawCard(currentPlayer, state);
        }
        else if (choice1 == 2)
        {
            //+2 coins
            state->coins = state->coins + 2;
        }
        else
        {
            //trash 2 cards in hand
            removeToTrash(choice2, state);
            removeToTrash(choice3, state);
        }

        //discard card from hand
        removeToPlayed(handPos, state);
        return 0;

    case tribute:
		if (cardEffectTribute(handPos, state) == 0)
			return 0;
		else
			return -1;

    case ambassador:
		if (cardEffectAmbassador(handPos, choice1, choice2, state) == 0)
			return 0;
		else
			return -1;

    case cutpurse:

        updateCoins(currentPlayer, state, 2);
        for (i = 0; i < state->numPlayers; i++)
        {
            if (i != currentPlayer)
            {
                for (j = 0; j < state->handCount[i]; j++)
                {
                    if (state->hand[i][j] == copper)
                    {
                        /* discardCard(j, i, state, 0); */
                        break;
                    }
                    if (j == state->handCount[i])
                    {
                        for (k = 0; k < state->handCount[i]; k++)
                        {
                            if (DEBUG)
                                printf("Player %d reveals card number %d\n", i, state->hand[i][k]);
                        }
                        break;
                    }
                }

            }

        }

        //discard played card from hand
        removeToPlayed(handPos, state);

        return 0;


    case embargo:
        //+2 Coins
        state->coins = state->coins + 2;

        //see if selected pile is in play
        if ( state->supplyCount[choice1] == -1 )
        {
            return -1;
        }

        //add embargo token to selected supply pile
        state->embargoTokens[choice1]++;

        //trash card
        removeToTrash(handPos, state);
        return 0;

    case outpost:
        //set outpost flag
        state->outpostPlayed++;

        //discard card
        removeToPlayed(handPos, state);
        return 0;

    case salvager:
        //+1 buy
        state->numBuys++;

        if (choice1)
        {
            //gain coins equal to trashed card
            state->coins = state->coins + getCost( handCard(choice1, state) );
            //trash card
            removeToTrash(choice1, state);
        }

        //discard card
        removeToPlayed(handPos, state);
        return 0;

    case sea_hag:
        for (i = 0; i < state->numPlayers; i++) {
            if (i != currentPlayer) {
                state->discard[i][state->discardCount[i]] = state->deck[i][state->deckCount[i]--];
                state->deckCount[i]--;
                state->discardCount[i]++;
                state->deck[i][state->deckCount[i]--] = curse;//Top card now a curse
            }
        }
        return 0;

    case treasure_map:
        //search hand for another treasure_map
        index = -1;
        for (i = 0; i < state->handCount[currentPlayer]; i++)
        {
            if (state->hand[currentPlayer][i] == treasure_map && i != handPos)
            {
                index = i;
                break;
            }
        }
        if (index > -1)
        {
            //trash both treasure cards
            removeToTrash(handPos, state);
            removeToTrash(index, state);

            //gain 4 Gold cards
            for (i = 0; i < 4; i++)
            {
                gainCard(gold, state, 1, currentPlayer);
            }

            //return success
            return 1;
        }

        //no second treasure_map found in hand
        return -1;
    }

    return -1;
}

int removeToPlayed(int handPos, struct gameState *state)
{
    // Add card to the played pile.
    state->playedCards[state->playedCardCount] = state->hand[whoseTurn(state)][handPos];
    state->playedCardCount++;

	// Shift the player's hand to compensate for the removed card.
	shiftHand(handPos, state);

    return 0;
}

int removeToDiscard(int handPos, struct gameState *state)
{
	// Local variable for current player.
	int currentPlayer = whoseTurn(state);

	// Add card to the discard pile.
	state->discard[currentPlayer][state->discardCount[currentPlayer]] = state->hand[currentPlayer][handPos];
	state->discardCount[currentPlayer]++;

	// Shift the player's hand to compensate for the removed card.
	shiftHand(handPos, state);

	return 0;
}

int removeAllToDiscard(int player, struct gameState *state)
{
	// Until the player's hand is empty...
	while (state->handCount[player] > 0)
	{
		// Place the right-most card in hand onto the player's discard pile.
		state->discard[player][state->discardCount[player]] = state->hand[player][state->handCount[player]];
		state->discardCount[player]++;

		// Adjust the player's hand.
		state->hand[player][state->handCount[player]] = -1;
		state->handCount[player]--;
	}

	return 0;
}

int removeToTrash(int handPos, struct gameState *state)
{
	// Add card to the trash pile.
	state->trash[state->trashCount] = state->hand[whoseTurn(state)][handPos];
	state->trashCount++;

	// Shift the player's hand to compensate for the removed card.
	shiftHand(handPos, state);

	return 0;
}

int removeToSupply(int handPos, struct gameState *state)
{
	// Add card to the Supply pile.
	state->supplyCount[handCard(handPos, state)]++;

	// Shift the player's hand to compensate for the removed card.
	shiftHand(handPos, state);

	return 0;
}

int shiftHand(int removedPos, struct gameState *state)
{
	// Local variable for current player.
	int currentPlayer = whoseTurn(state);

	// Determine if the card removed was not the right-most in hand.
	if (removedPos != state->handCount[currentPlayer] - 1)
	{
		// Shift all cards in hand over to fill the gap created by discarding the card.
		for (int i = removedPos; i < state->handCount[currentPlayer] - 1; i++)
		{
			state->hand[currentPlayer][i] = state->hand[currentPlayer][i + 1];
		}
	}

	// Set the previously right-most slot to -1.
	// This applies whether or not the removed card was the final card in hand.
	state->hand[currentPlayer][state->handCount[currentPlayer] - 1] = -1;

	// Decrement the number of cards in hand.
	state->handCount[currentPlayer]--;

	return 0;
}

int gainCard(int supplyPos, struct gameState *state, int toFlag, int player)
{
    // Note: supplyPos is enum of chosen card

    // Check if the Supply pile of chosen card is empty (0) or not used in game (-1)
    if ( supplyCount(supplyPos, state) < 1 )
    {
        return -1;
    }

	// Deliver the card to the proper destination.
    // toFlag = 0 : add to discard
    // toFlag = 1 : add to deck
    // toFlag = 2 : add to hand

	if (toFlag == 0)
	{
		state->discard[player][state->discardCount[player]] = supplyPos;
		state->discardCount[player]++;
	}
	else if (toFlag == 1)
    {
        state->deck[ player ][ state->deckCount[player] ] = supplyPos;
        state->deckCount[player]++;
    }
    else if (toFlag == 2)
    {
        state->hand[ player ][ state->handCount[player] ] = supplyPos;
        state->handCount[player]++;
    }
	else
	{
		// Internal error, passed an invalid toFlag value.
		return -1;
	}


    // Decrement the supply pile.
    state->supplyCount[supplyPos]--;

	// If gaining this card has depleted the pile, check to see if the game has ended.
	if (state->supplyCount[supplyPos] == 0)
		isGameOver(state);

    return 0;
}

int updateCoins(int player, struct gameState *state, int bonus)
{
    int i;

    //reset coin count
    state->coins = 0;

    //add coins for each Treasure card in player's hand
    for (i = 0; i < state->handCount[player]; i++)
    {
        if (state->hand[player][i] == copper)
        {
            state->coins += 1;
        }
        else if (state->hand[player][i] == silver)
        {
            state->coins += 2;
        }
        else if (state->hand[player][i] == gold)
        {
            state->coins += 3;
        }
    }

    //add bonus
    state->coins += bonus;

    return 0;
}

// Card: Baron
// Type: Action
// Cost: 4
//
// +1 Buy
// You may discard an Estate
// for +4 Coins. If you don't, gain
// an Estate.
int cardEffectBaron(int handPos, int choice1, struct gameState *state)
{
	// Note: if the player has chosen to discard an Estate
	// but cannot, the player will be forced to gain an Estate.
	// Future implementation may allow reconsideration on the player's part.
	// Future implementation will also allow choice of which Estate to discard.
	
	// handPos = position of Baron
	// choice1 = 1 for discard Estate, 0 for gain Estate

	int i;

	// Confirm that the player's selection is valid.
	if (choice1 != 1 && choice1 != 0)
		return -1;

	// Local variable for current player.
	int currentPlayer = whoseTurn(state);

	// Local variable to track if the player has discarded an Estate.
	int estateDiscarded = 0;

	// Confirm the player can discard an Estate if chosen.
	if (choice1 == 1)
	{
		int canDiscard = 0;
		for (i = 0; i < state->handCount[currentPlayer]; i++)
		{
			if (state->hand[currentPlayer][i] == estate)
				canDiscard = 1;
		}
		if (canDiscard == 0)
			return -1;
	}

	// Discard Baron to the played pile.
	removeToPlayed(handPos, state);

	// Increment the player's Buys.
	state->numBuys++;

	// Determine if the player chose to discard an Estate.
	if (choice1 > 0)
	{
		// Find the first (left-most) Estate in hand.
		for (i = 0; i < state->handCount[currentPlayer]; i++)
		{
			// If an Estate is found...
			if (state->hand[currentPlayer][i] == estate)
			{
				/* BARON NON-BUG 1 OF 2
				// Increase the player's number of Coins by 4.
				state->coins += 4;
				// END BARON NON-BUG 1 OF 2 */

				// BARON BUG 1 OF 2
				// Increase the player's number of Coins by 4.
				while (state->coins < 4)
					state->coins++;
				// END BARON BUG 1 OF 2

				// Discard the found Estate to the discard pile.
				removeToDiscard(i, state);

				// Exit the for loop after declaring that an Estate was discarded.
				estateDiscarded = 1;
				break;
			}
		}
		/*
		// Report a message if an Estate could not be discarded.
		if (DEBUG && estateDiscarded == 0)
		{
			printf("No estate cards in your hand, invalid choice\n");
			printf("Must gain an estate if there are any\n");
			return -1;
		}
		*/
	}

	// If the player chose to gain an Estate or failed to discard an Estate.
	if (choice1 == 0) // || estateDiscarded == 0)
	{
		/* BEGIN BARON NON-BUG 2 OF 2
		// Gain an Estate to the discard pile.
		gainCard(estate, state, 0, currentPlayer);
		// END BARON NON-BUG 2 OF 2 */

		// BEGIN BARON BUG 2 OF 2
		// Gain an Estate to the discard pile.
		gainCard(estate, state, 1, currentPlayer);
		// END BARON BUG 2 OF 2
	}

	return 0;
}

// Card: Minion
// Type: Action
// Cost: 5
//
// +1 Action
// Choose one: +2 Coins; or discard
// your hand, +4 Cards, and
// each other player with at least
// 5 cards in hand discards their
// hand and draws 4 cards.
int cardEffectMinion(int handPos, int choice1, struct gameState *state)
{
	// handPos = position of Minion
	// choice1 = 1 for +2 Coins, 2 for redraw

	// Local variable to store current player.
	int currentPlayer = whoseTurn(state);

	// Confirm that the player's selection is valid.
	if (choice1 != 1 && choice1 != 2)
		return -1;

	// Discard Minion to the played pile.
	removeToPlayed(handPos, state);

	// Increment the player's actions.
	state->numActions++;

	// If the player chose the first option, increase their Coins by 2.
	if (choice1 == 1)
		state->coins += 2;
	else
	{
		// Discard the player's hand.
		removeAllToDiscard(currentPlayer, state);

		/* BEGIN MINION NON-BUG 1 OF 2
		// Current player draws 4 cards.
		for (int i = 0; i < 4; i++)
			drawCard(currentPlayer, state);
		// END MINION NON-BUG 1 OF 2 */

		// BEGIN MINION BUG 1 OF 2
		// Current player draws 4 cards.
		while (numHandCards(state) < 4)
			drawCard(currentPlayer, state);
		// END MINION BUG 1 OF 2
		// Iterate through all other players.
		for (int i = 0; i < state->numPlayers; i++)
		{
			/* BEGIN MINION NON-BUG 2 OF 2
			// If the player is the current player, skip this iteration.
			if (i == currentPlayer)
				continue;
			// END MINION NON-BUG 2 OF 2 */

			// If hand size is greater than 4...
			if (state->handCount[i] > 4)
			{
				// Discard all cards.
				removeAllToDiscard(i, state);

				// Draw 4 cards.
				for (int j = 0; j < 4; j++)
					drawCard(i, state);
			}
		}
	}

	return 0;
}

// Card: Ambassador
// Type: Action-Attack
// Cost: 3
//
// Reveal a card from your
// hand. Return up to 2 copies
// of it from your hand to the
// Supply. Then each other
// player gains a copy of it.
int cardEffectAmbassador(int handPos, int choice1, int choice2, struct gameState *state)
{
	// handPos = position of Ambassador played
	// choice1 = position of revealed card
	// choice2 = number of revealed card to return to Supply

	// Local variable for current player.
	int currentPlayer = whoseTurn(state);

	// Create a small array for the revealed card.
	// j[0] = Counter for how many of the chosen card the player has in hand.
	// j[1] = Position of the first left-most matching card.
	// j[2] = Position of the second left-most matching card.
	int j[3] = {0, -1, -1};

	int revealedCard = handCard(choice1, state);

	// Confirm that Ambassador itself was not chosen as the card to reveal.
	// Confirm that the player has not chosen to return less than 0 or greater than 2 copies.
	if (choice1 == handPos || choice2 < 0 || choice2 > 2)
		return -1;

	// Find and count copies of the chosen card in hand.
	for (int i = 0; i < numHandCards(state); i++)
	{
		if (i != handPos && handCard(i, state) == revealedCard)
		{
			/* BEGIN MINION NON-BUG 1 OF 2
			// Increment the number of found matching cards.
			j[0]++;
			// END MINION NON-BUG 1 OF 2 */

			// Store the position of the matching card.
			if (j[0] == 1)
				j[1] = i;
			else
			{
				// Store the final relevant card and break the loop.
				j[2] = i;
				break;
			}
		}
	}

	// Confirm the player has sufficient copies of the chosen card to return to the Supply.
	if (j[0] < choice2)
		return -1;

	// Return the chosen number of cards to the supply.
	switch (choice2)
	{
		case 2:
			// Return the second left-most card to the Supply.
			removeToSupply(j[2], state);

			// Adjust the position of Ambassador to compensate for hand shifting.
			// This line changes the passed in value, handPos.
			// The first left-most card is always to the left of the second left-most card,
			// and therefore is never impacted by hand shifting.
			if (j[2] < handPos)
				handPos--;
		case 1:
			// Return the first left-most card to the Supply.
			removeToSupply(j[1], state);

			// Adjust the position of Ambassador to compensate for hand shifting.
			if (j[1] < handPos)
				handPos--;
		default:
			// Discard Ambassador to the played pile.
			removeToPlayed(handPos, state);
	}

	// Distribute a copy of the returned card to each other player, in turn order.
	// Some players may not receive a copy due to an empty Supply. This is intended behavior.
	for (int i = 1; i < state->numPlayers + 1; i++)
	{
		/* BEGIN MINION NON-BUG 2 OF 2
		gainCard(revealedCard, state, 0, (currentPlayer + i) % state->numPlayers);
		// END MINION NON-BUG 2 OF 2 */

		// BEGIN MINION BUG 2 OF 2
		gainCard(revealedCard, state, 0, currentPlayer + i);
		// END MINION BUG 2 OF 2
	}

	return 0;
}

// Card: Tribute
// Type: Action
// Cost: 5
//
// The player to your left reveals then
// discards the top 2 cards of his deck.
// For each differently named card
// revealed, if it is an...
// Action Card, +2 Actions
// Treasure Card, +2 Coins
// Victory Card, +2 Cards
int cardEffectTribute(int handPos, struct gameState *state)
{
	// Local variables for players and revealed cards.
	int currentPlayer = whoseTurn(state);
	int nextPlayer = (currentPlayer + 1) % state->numPlayers;
	int revealedCards[2] = {-1, -1};

	// Discard Tribute into the played pile.
	removeToPlayed(handPos, state);

	// If the next player's deck and hand is empty, return; Tribute does nothing.
	if (state->deckCount[nextPlayer] + state->discardCount[nextPlayer] == 0)
		return 0;

	// Attempt to reveal the top two cards from the next player's deck.
	for (int i = 0; i < 2; i++)
	{
		// If the next player's deck is empty...
		if (state->deckCount[nextPlayer] == 0)
		{
			// Attempt to move next player's discard pile into their deck.
			while(state->discardCount[nextPlayer] > 0)
			{
				state->deck[nextPlayer][state->deckCount[nextPlayer]] = state->discard[nextPlayer][state->discardCount[nextPlayer]];
				state->discard[nextPlayer][state->discardCount[nextPlayer]] = -1;
				state->deckCount[nextPlayer]++;
				state->discardCount[nextPlayer]--;

				// Shuffle the next player's deck.
				shuffle(nextPlayer, state);
			}
		}

		/* BEGIN TRIBUTE NON-BUG 1 OF 2
		// If there is still no deck to reveal from, break this loop.
		if (state->deckCount[nextPlayer] == 0)
			break;
		// END TRIBUTE NON-BUG 2 OF 2 */

		// Reveal the top card of the next player's deck.
		revealedCards[i] = state->deck[nextPlayer][state->deckCount[nextPlayer]];

		// Remove it from the next player's deck. Do not discard it yet.
		state->deck[nextPlayer][state->deckCount[nextPlayer]] = -1;
		state->deckCount[nextPlayer]--;
	}
	
	// TODO: Re-work the type system, rather than naming cards indivudally in the following loop.
	// Allows for further implementation in a much cleaner fashion.

	// Grant the respective rewards to the current player for the revealed cards.
	for (int i = 0; i < 2; i++)
	{
		/* BEGIN TRIBUTE NON-BUG 2 OF 2
		// If only one card was revealed, break this loop.
		if (revealedCards[i] == -1)
			break;
		// END TRIBUTE NON-BUG 2 OF 2 */

		switch (revealedCards[i])
		{
			// If the card is a Treasure, grant 2 Coins.
			case copper:
			case silver:
			case gold:
				state->coins += 2;
				break;

			// If the card is a Victory, draw 2 Cards.
			case estate:
			case duchy:
			case province:
			case gardens:
			case great_hall:
				drawCard(currentPlayer, state);
				drawCard(currentPlayer, state);
				break;

			// Otherwise, if the card is an Action, gain 2 Actions.
			default:
				state->numActions += 2;
		}

		// If the two revealed cards are identical, break this loop.
		if (revealedCards[0] == revealedCards[1])
			break;
	}

	// Discard each revealed card into the next player's discard pile.
	for (int i = 0; i < 2; i++)
	{
		// If there are no (more) revealed cards, break this loop.
		if (revealedCards[i] == -1)
			break;

		// Discard the card.
		state->discard[nextPlayer][state->discardCount[nextPlayer]] = revealedCards[i];
		state->discardCount[nextPlayer]++;
	}

	return 0;
}

// Card: Mine
// Type: Action
// Cost: 5
//
// You may trash a Treasure
// from your hand. Gain a
// Treasure to your hand
// costing up to 3 Coins more
// than it.
int cardEffectMine(int handPos, int choice1, int choice2, struct gameState *state)
{
	// Note: Mine does not require that the player trash a card.
	// If the player has chosen not to do so, Mine will be discarded
	// to the played pile and Dominion resumes.

	// handPos = position of Mine
	// choice1 = position of Treasure to trash
	// choice2 = enum of Treasure to gain to hand

	// If the player has chosen not to trash a card...
	if (choice1 == -1)
	{
		// Discard Mine to the played pile and return.
		removeToPlayed(handPos, state);

		return 0;
	}

	// Confirm that the player's selections are valid.
	if (choice1 < -1 || choice1 > numHandCards(state))
		return -1;

	if (choice2 < BEGIN_ENUM_CARD + 1 || choice2 > END_ENUM_CARD - 1)
		return -1;

	// Local variable to store the value of the chosen card to trash.
	int cardToTrash = handCard(choice1, state);

	// Confirm that the chosen card to trash is a Treasure.
	switch (cardToTrash)
	{
		case copper:
		case silver:
		case gold:
			break;
		default:
			return -1;
	}

	// Confirm that the chosen card to gain is a Treasure costing up to 3 more.
	switch (choice2)
	{
		case copper:
		case silver:
		case gold:
			if (getCost(cardToTrash) + 3 < getCost(choice2))
				return -1;
			break;
		default:
			return -1;
	}

	// Discard Mine to the played pile.
	removeToPlayed(handPos, state);

	/* BEGIN MINE NON-BUG 1 OF 2
	// Adjust the trashed card's position to compensate for hand shifting.
	if (handPos < choice1)
		choice1--;
	// END MINE NON-BUG 1 OF 2 */

	// BEGIN MINE BUG 1 OF 2
	// Adjust the trashed card's position to compensate for hand shifting.
	if (handPos < choice1)
		choice1++;
	// END MINE BUG 1 OF 2

	/* BEGIN MINE NON-BUG 2 OF 2
	// Discard the chosen card to the trash pile.
	removeToTrash(choice1, state);
	// END MINE NON-BUG 2 OF 2 */

	// Gain the desired Treasure to hand.
	gainCard(choice2, state, 2, whoseTurn(state));

	return 0;
}

//end of dominion.c
