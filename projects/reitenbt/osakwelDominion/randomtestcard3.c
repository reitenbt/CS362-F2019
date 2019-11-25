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

// Random tests for tributeCardEffect()
int main()
{
	int i;
	int test_counter;
	struct gameState* G = newGame();
	int num_players;
	int kingdom_cards[10] = { ambassador, baron, mine, minion, tribute,
								adventurer, council_room, feast, gardens, remodel };
	int random_seed;
	int current_player;
	int next_player;

	// Tribute specific
	int num_actions;
	int num_coins;
	int num_cards_in_hand;
	int next_player_cards[2];	// Randomly consists of Minion, Copper, Estate, or nothing
	int num_next_player_cards;
	int num_minions, num_coppers, num_estates; // of next player
	int revealed_cards[2];

	fprintf(stderr, "*** BEGIN TRIBUTE TESTING ***\n");

	for (test_counter = 0; test_counter < NUMBER_OF_TESTS; test_counter++)
	{
		// Randomize game values
		num_players = rand() % 3 + 2;
		random_seed = rand();

		// Initialize game
		initializeGame(num_players, kingdom_cards, random_seed, G);
		current_player = G->whoseTurn;
		next_player = (current_player + 1) % G->numPlayers;
		num_minions = num_coppers = num_estates = 0;
		num_next_player_cards = 2;

		// Randomize other values
		num_actions = rand() % 9 + 1;
		num_coins = rand() % 10;
		num_cards_in_hand = rand() % 9 + 1;
		for (i = 0; i < 2; i++)
		{
			switch (rand() % 4)
			{
			case 0:
				next_player_cards[i] = minion;
				num_minions++;
				break;
			case 1:
				next_player_cards[i] = copper;
				num_coppers++;
				break;
			case 2:
				next_player_cards[i] = estate;
				num_estates++;
				break;
			case 3:
				next_player_cards[i] = -1;
				num_next_player_cards--;
				break;
			}
		}

		// Apply them
		// Player variables
		G->numActions = num_actions;
		G->coins = num_coins;
		G->handCount[current_player] = num_cards_in_hand;
		G->hand[current_player][0] = tribute;
		for (i = 1; i < num_cards_in_hand; i++)
			G->hand[current_player][G->handCount[current_player]] = kingdom_cards[rand() % 10];

		// Next player variables
		G->deckCount[next_player] = 0;
		G->discardCount[next_player] = 0;
		for (i = 0; i < 4; i++)
		{
			if (next_player_cards[i] == -1)
				continue;

			if (rand() % 2 == 0)
				G->deck[next_player][G->deckCount[next_player]++] = next_player_cards[i];
			else
				G->discard[next_player][G->discardCount[next_player]++] = next_player_cards[i];
		}

		// Play Tribute
		tributeCardEffect(G, 0, next_player, current_player, &revealed_cards);

		// Soft asserts

		// 1) Next player had no cards
			// +0 Actions
			// +0 Coins
			// -1 Cards
		// 2) Next player had one action
			// +2 Actions
			// +0 Coins
			// -1 Cards
		// 3) Next player had one treasure
			// Actions same
			// +2 Coins
			// -1 Cards
		// 4) Next player had one victory
			// +0 Actions
			// +0 Coins
			// +1 Cards
		// 5) Next player had action & treasure
			// +2 Actions
			// +2 Coins
			// -1 Cards
		// 6) Next player had action & victory
			// +2 Actions
			// +0 Coins
			// +1 Cards
		// 7) Next player had treasure & victory
			// +0 Actions
			// +2 Coins
			// +1 Cards
		
		soft_assert(test_counter, G->numActions == num_actions + 2 * num_minions, "+2 Actions");
		soft_assert(test_counter, G->coins == num_coins + 2 * num_coppers, "+2 Coins");
		soft_assert(test_counter, G->handCount[current_player] == num_cards_in_hand - 1 + 2 * num_estates, "+2 Cards");
	}

	fprintf(stderr, "*** END TRIBUTE TESTING ***\n");

	return 0;
}
