/*
  Analyze a specific gametrace from the server logs and apply the learning
  mechanism to this file. */

#include <iostream>
#include "assert.h"
#include "Pente.hpp"
#include "wlm.hpp"

using namespace std;

int main(int argc, char **argv) {

	Pente p1, p2;
    Weight weights(WEIGHTS_FILE); /* Vhat() loads the weight file automatically, but
                                     we want to keep it up to date between calculations
                                     so I'll use this has a pointer. */


	p1.players[0] = p2.players[1] = "COMPUTER";
	p1.players[1] = p2.players[0] = "OTHER";

	p1.fillCell(9,9, WHITE);
	p2.fillCell(9,9, WHITE);

    // for init, don't read in the file but init to 0's.
    if (argc > 1 && string(argv[1]) == "init") {
        weights.w.resize(p1.toState().size() + 1);
        for (int i = 1; i < weights.size(); i++)
            weights[i] = 2;
        
        weights.save();
        cout << "reinitializing weights..." << endl;
    }
    else
        weights.load();


    // Read in our game
    if (argc != 2) {
        cout << "SYNTAX: " << argv[0] << " <gamefile>" << endl;
        exit(0);
    }

    ifstream game_file(argv[1]);

    if (!game_file) {
        cout << "Unable to open file at '" << argv[1] << "'" << endl;
        exit(1);
    }

    p1.deserialize(game_file);
    game_file.close();

    // computers only.
    if (p1.playerNumber("COMPUTER") == -1) {
        cout << "A computer did not play in this game." << endl;
        exit(0);
    }

    // Print the summary...
	cout << endl;
	cout << "p1.gameOutcome(COMPUTER) -> " 
         << p1.gameOutcome(p1.playerColor("COMPUTER")) << endl
		 << "whtCaps -> " << p1.whtCaps << endl
		 << "blkCaps -> " << p1.blkCaps << endl
		 << p1.toString() << endl << endl;

	// Analyze the game and adjust weights accordingly.


	/* Vtrain(b) <- V(Successor(b)) */
	/* Adjust weights with wi <- wi + n(Vtrain(b) - V(b))*xi */

	Pente *games[] = {&p1, &p2};
	char colors[] = {WHITE, BLACK};

    char computer_color; 

	for (int i = 0; i < 1; i++) {
		cout << "Analyzing P" << i << endl;
        cout << "eta: " << weights.eta << endl;
		Pente *game = games[i];
        computer_color = game->playerColor("COMPUTER");
        if (game->gameOutcome(WHITE) == 0) {
            cout << "Game is undecided." << endl;
            break;
        }
            

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
			Pente::cell* tCell = game->gametrace.back();
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
