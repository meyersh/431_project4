#include <iostream>
#include "assert.h"
#include "Pente.hpp"
#include "wlm.hpp"

using namespace std;

int main(int argc, char **argv) {

	PenteLinearAI *p1, *p2;
    p1 = new PenteLinearAI();
    p2 = new PenteLinearAI();

    Weight weights(WEIGHTS_FILE); /* Vhat() loads the weight file automatically, but
                                     we want to keep it up to date between calculations
                                     so I'll use this has a pointer. */


	p1->players[0] = p2->players[1] = "COMPUTER";
	p1->players[1] = p2->players[0] = "OTHER";

	p1->fillCell(9,9, WHITE);
	p2->fillCell(9,9, WHITE);

    // for init, don't read in the file but init to 0's.
    if (argc > 1 && string(argv[1]) == "init") {
        weights.w.resize(p1->toState().size() + 1);
        for (int i = 1; i < weights.size(); i++)
            weights[i] = 2;
        
        weights.save();
        cout << "reinitializing weights..." << endl;
    }
    else
        weights.load();

	// Play a game with 0 players.
	while (p1->gameOutcome(WHITE) == 0) {
		// Black moves
		p2->makeMove(weights);

		// Copy the black move into p1
		p1->playToken(p2->gametrace.back()->r, p2->gametrace.back()->c, BLACK);

        if (p1->gameOutcome(WHITE) != 0)
            break;
        
		// White moves
		p1->makeMove(weights);

		// Copy the white move into p2
		p2->playToken(p1->gametrace.back()->r, p1->gametrace.back()->c, WHITE);

		// Show some progress to the screen.
		cout << ".";
		cout.flush();
   
	}

	// Print the summary...
	cout << endl;
	cout << "p1->gameOutcome(WHITE) -> " << p1->gameOutcome(WHITE) << endl
		 << "whtCaps -> " << p1->whtCaps << endl
		 << "blkCaps -> " << p1->blkCaps << endl
		 << p2->toString() << endl << endl;

	// Analyze the game and adjust weights accordingly.


	/* Vtrain(b) <- V(Successor(b)) */
	/* Adjust weights with wi <- wi + n(Vtrain(b) - V(b))*xi */
	

	PenteLinearAI *games[] = {p1, p2};
	int colors[] = {WHITE, BLACK};

    int computer_color; 

	for (int i = 0; i < 2; i++) {
		cout << "Analyzing P" << i << endl;
        cout << "eta: " << weights.eta << endl;
		PenteLinearAI *game = games[i];
        computer_color = game->playerColor("COMPUTER");

		// Our end-game state
		State b = game->toState();
		State successor = game->toState(); 

		double error = 
            game->gameOutcome(computer_color)*100 - weights.Vhat(b);
		weights.adjust(b, error);
        cout << "Initial error: (" << 
            game->gameOutcome(computer_color)*100 << " - " << 
            weights.Vhat(b) << ") = " << error << endl;

        cout << "Errors => ";
		while (game->gametrace.size()) {
			successor = game->toState();
		
			// remember + remove the last move.
			PenteLinearAI::cell* tCell = game->gametrace.back();
			game->gametrace.pop_back();
			game->clearCell(tCell->r, tCell->c);

			b = game->toState();

			error = weights.Vhat(successor) - weights.Vhat(b);

			weights.adjust(game->toState(), error);

            cout << error << " ";
		}
        cout << endl;
	   
	}
    cout << "Weights: " << weights.toString() << endl;

    weights.save();

	return 0;

}
