#include <iostream>
#include "assert.h"
#include "neural.hpp"
#include "wlm.hpp"
#include <fann.h>

using namespace std;

int main(int argc, char **argv) {

	PenteNeuralAI *p1, *p2;
    p1 = new PenteNeuralAI();
    p2 = new PenteNeuralAI();

    Weight weights(WEIGHTS_FILE); /* Vhat() loads the weight file automatically, but
                                     we want to keep it up to date between calculations
                                     so I'll use this has a pointer. */


	p1->players[0] = p2->players[1] = "COMPUTER";
	p1->players[1] = p2->players[0] = "OTHER";

	p1->fillCell(9,9, WHITE);
	p2->fillCell(9,9, WHITE);

    // for init, don't read in the file but init to 0's.
    if (argc > 1 && string(argv[1]) == "init") {
        const unsigned int num_layers = 3;
        const unsigned int num_input = 20;
        const unsigned int num_output = 1;
        
        const unsigned int num_neurons_hidden_a = 30;

        const float desired_error = (const float) 0.001;
        
        struct fann *ann = fann_create_standard(num_layers,
                                                num_input,
                                                num_neurons_hidden_a,
                                                num_output);

        fann_set_activation_function_hidden(ann, FANN_SIGMOID_SYMMETRIC);
        fann_set_activation_function_output(ann, FANN_SIGMOID_SYMMETRIC);

        fann_save(ann, "pente.net");
        fann_destroy(ann);

        exit(0);
        
        /* weights.w.resize(p1->toState().size() + 1);
        for (int i = 1; i < weights.size(); i++)
            weights[i] = 2;
        
        weights.save();
        cout << "reinitializing weights..." << endl;*/
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
    exit(1); // not yet.

	/* Vtrain(b) <- V(Successor(b)) */
	/* Adjust weights with wi <- wi + n(Vtrain(b) - V(b))*xi */
	

	PenteNeuralAI *games[] = {p1, p2};
	int colors[] = {WHITE, BLACK};

    int computer_color; 

    fann *ann = fann_create_from_file("pente.net"); // Our neural network
    

	for (int i = 0; i < 2; i++) {
		cout << "Analyzing P" << i << endl;
        cout << "eta: " << weights.eta << endl;
		PenteNeuralAI *game = games[i];
        computer_color = game->playerColor("COMPUTER");


        

		// Our end-game state
		State b = game->toState();
		State successor = game->toState(); 

		double error = 
            game->gameOutcome(computer_color)*100 - weights.Vhat(b);

        vector<double> desired_output(1, weights.Vhat(b));

        // Push in game->toState() as input to the neural net and 
        // use the vhat() as the expected output to train the network. 

        fann_train(ann,
                   (fann_type*)&b.x[0], /* The state values */
                   (fann_type*)&desired_output[0]);

		weights.adjust(b, error);


        cout << "Initial error: (" << 
            game->gameOutcome(computer_color)*100 << " - " << 
            weights.Vhat(b) << ") = " << error << endl;

        cout << "Errors => ";
		while (game->gametrace.size()) {
			successor = game->toState();
		
			// remember + remove the last move.
			PenteNeuralAI::cell* tCell = game->gametrace.back();
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
    fann_save(ann, "pente.net");
    fann_print_connections(ann);
    fann_destroy(ann);
	return 0;

}
