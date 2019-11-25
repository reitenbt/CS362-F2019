#include "dominion.h"
#include "dominion_helpers.h"
#include <stdio.h>
#include <stdlib.h>

const int NUMBER_OF_TESTS = 1000;

int soft_assert(int test_number, int condition, const char* action)
{
	if (condition == 0)
		fprintf(stderr, "Test #%d failed to assert: %s\n", test_number, action);
	return 0;
}

// Random tests for minionCardEffect()
int main()
{
	int i, j;
	int test_counter;
	struct gameState* G = newGame();
	int num_players;
	int kingdom_cards[10] = { ambassador, baron, mine, minion, tribute,
								adventurer, council_room, feast, gardens, remodel };
	int random_seed;
	int current_player;

	// Minion specific
	int num_actions;
	int num_coins;
	int num_cards_in_discard;
	int num_cards_in_hand;
	int num_other_cards_in_hand[3]; // hand count for other players; one value is unused
	int choice1, choice2; // 0 = +2 Coins, 1 = redraw

	fprintf(stderr, "*** BEGIN MINION TESTING ***\n");

	for (test_counter = 0; test_counter < NUMBER_OF_TESTS; test_counter++)
	{
		// Randomize game values
		num_players = rand() % 3 + 2;
		random_seed = rand();

		// Initialize game
		initializeGame(num_players, kingdom_cards, random_seed, G);
		current_player = G->whoseTurn;

		// Randomize other values
		num_actions = rand() % 10 + 1;
		num_coins = rand() % 11;
		num_cards_in_discard = rand() % 11;
		num_cards_in_hand = rand() % 5 + 1;
		for (i = 0; i < 3; i++)
			num_other_cards_in_hand[i] = rand() % 11;
		choice1 = rand() % 2;
		choice2 = !choice1;

		// Apply them
		G->numActions = num_actions;
		G->coins = num_coins;
		G->discardCount[G->whoseTurn] = num_cards_in_discard;
		for (i = 0; i < num_cards_in_discard; i++)
			G->discard[G->whoseTurn][i] = kingdom_cards[rand() % 10];

		// Player hand
		G->handCount[G->whoseTurn] = num_cards_in_hand;
		G->hand[G->whoseTurn][0] = minion;
		for (i = 1; i < num_cards_in_hand; i++)
			G->hand[G->whoseTurn][i] = kingdom_cards[rand() % 10];

		// Other player hands
		for (i = 0; i < G->numPlayers; i++)
		{
			if (i != G->whoseTurn)
			{
				G->handCount[i] = num_other_cards_in_hand[i];
				for (j = 0; j < G->handCount[i]; j++)
					G->hand[i][j] = kingdom_cards[rand() % 10];
			}
		}

		// Play Minion. No choice validation as both are always valid
		minionCardEffect(G, 0, current_player, choice1, choice2);

		// Choice 0: +2 Coins
		if (choice1 == 0)
		{
			soft_assert(test_counter, G->coins == num_coins + 2, "+2 Coins");
			soft_assert(test_counter, G->handCount[G->whoseTurn] == num_cards_in_hand - 1, "-1 Card in Hand");
		}
		// Choice 1: Discard, +4 Cards, all others with 5+ cards do the same
		else
		{
			soft_assert(test_counter, G->coins == num_coins, "+0 Coins");
			soft_assert(test_counter, G->handCount[G->whoseTurn] < 5, "<5 Cards in Hand");

			// Other players
			for (i = 0; i < G->numPlayers; i++)
			{
				if (i == G->whoseTurn)
					continue;

				// If hand size was greater than 4, hand size should now be exactly 4
				if (num_other_cards_in_hand[i] > 4)
					soft_assert(test_counter, G->handCount[i] == 4, "Other Player Discarded and Drawn");

				// If hand size was less 5, hand size should be the same
				else
					soft_assert(test_counter, G->handCount[i] == num_other_cards_in_hand[i], "Other Player Hand Remains Same");
			}
		}
	}

	fprintf(stderr, "*** END MINION TESTING ***\n");

	return 0;
}
