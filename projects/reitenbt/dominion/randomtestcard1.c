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

// Random tests for cardEffectBaron()
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

	// Baron specific
	int num_estates_in_supply;
	int num_cards_in_hand;
	int num_estates_in_hand;
	int num_coins;
	int num_cards_in_discard;
	int num_buys;

	int baron_position;
	int choice;		// 0 = gain, 1 = discard
	for (test_counter = 0; test_counter < NUMBER_OF_TESTS; test_counter++)
	{
		// Randomize game values
		num_players = rand() % 3 + 2;
		random_seed = rand();
		
		// Initialize game
		initializeGame(num_players, kingdom_cards, random_seed, G);
		// Randomize other values
		num_estates_in_supply = rand() % 11;
		num_cards_in_hand = rand() % 5 + 1;
		if (num_cards_in_hand > 1)
			num_estates_in_hand = rand() % (num_cards_in_hand - 1);
		else
			num_estates_in_hand = 0;
		num_coins = rand() % 11;
		num_cards_in_discard = rand() % 6;
		choice = rand() % 2;
		num_buys = rand() % 5;
		
		// Apply them
		current_player = G->whoseTurn;
		G->supplyCount[estate] = num_estates_in_supply;
		G->handCount[current_player] = num_cards_in_hand;
		G->coins = num_coins;
		G->discardCount[current_player] = num_cards_in_discard;
		for (i = 0; i < num_estates_in_hand; i++)
		{
			G->hand[current_player][i] = estate;
		}
		baron_position = i;
		G->hand[current_player][baron_position] = baron;
		while (i < num_cards_in_hand)
		{
			G->hand[current_player][i] = kingdom_cards[rand() % 10];
			i++;
		}
		G->numBuys = num_buys;

		// Play Baron
		while (cardEffectBaron(baron_position, choice, G) != 0)
		{
			soft_assert(test_counter, 0, "Swapping choice; should occur once, else cannot play Baron.");
			choice = !choice;
		}

		// Soft asserts
		if (choice == 0) // choice is to discard an estate
		{
			// +4 Coins, estate in discard, discard count + 1, hand count - 2
			soft_assert(test_counter, G->coins == num_coins + 4,
				"+4 Coins");
			soft_assert(test_counter, G->discardCount[current_player] == num_cards_in_discard + 1,
				"+1 Card in Discard");
			soft_assert(test_counter, G->discard[current_player][G->discardCount[current_player]] == estate,
				"Discarded Estate in Discard");
			soft_assert(test_counter, G->handCount[current_player] == num_cards_in_hand - 2,
				"-2 Cards in Hand");
		}
		else // choice is to gain an estate
		{
			if (num_estates_in_supply == 0)
			{
				soft_assert(test_counter, G->supplyCount[estate] == 0,
					"Estates in Supply Remains 0");
				soft_assert(test_counter, G->discardCount[current_player] == num_cards_in_discard,
					"Discard Count Unchanged");
			}
			else
			{
				soft_assert(test_counter, G->supplyCount[estate] == num_estates_in_supply - 1,
					"-1 Estates in Supply");
				soft_assert(test_counter, G->discardCount[current_player] == num_cards_in_discard - 1,
					"+1 Card in Discard");
				soft_assert(test_counter, G->discard[current_player][G->discardCount[current_player]] == estate,
					"Gained Estate in Discard");
			}

			soft_assert(test_counter, G->handCount[current_player] == num_cards_in_hand - 1,
				"-1 Cards in Hand");
		}
	}
	return 0;
}
