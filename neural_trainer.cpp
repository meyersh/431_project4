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

        const unsigned int num_input = 20; // plus ours and theirs captures.
        const unsigned int num_output = 1;
        
        const unsigned int num_neurons_hidden_a = 45;
        const unsigned int num_neurons_hidden_b = 35;

        const float desired_error = (const float) 0.001;
        
        struct fann *ann = fann_create_standard(num_layers,
                                                num_input,
                                                num_neurons_hidden_a,
                                                //num_neurons_hidden_b,
                                                num_output);

        fann_set_activation_function_hidden(ann, FANN_SIGMOID_SYMMETRIC);
        fann_set_activation_function_output(ann, FANN_LINEAR);

        fann_save(ann, "pente.net");
        fann_destroy(ann);

        exit(0);
        
    }

    weights.load();
    cout << "Weights: " <<
        weights.toString() << endl;

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
        if (getenv("VERBOSE"))
            cout << p2->toString() << endl;
        else
            cout << ".";

		cout.flush();
   
	}

	// Print the summary...
	cout << endl;
	cout << "p1->gameOutcome(WHITE) -> " << p1->gameOutcome(WHITE) << endl
		 << "whtCaps -> " << p1->whtCaps << endl
		 << "blkCaps -> " << p1->blkCaps << endl
         << p2->toState().toString() << endl
		 << p2->toString() << endl << endl;

    if (getenv("DEBUG")) {
        for (int i = 0; i < p2->Board.size(); i++) {
            if (p2->getCell(i)->color == EMPTY)
                continue;
            
            cout << "p->playToken(" << p2->getCell(i)->r << "," << 
                p2->getCell(i)->c << ",";
            
            if (p2->getCell(i)->color == WHITE)
                cout << "WHITE);" << endl;
            else
                cout << "BLACK);" << endl;
        }
    }



	// Analyze the game and train the neural network based on its error compared to Vhat()

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

        double weightsVhat = weights.Vhat(b);
        
        // Exit without computing error if weights is 0...
        if (weightsVhat == 0)
            continue;

		double error = 
            game->toValue() - weightsVhat;

        fann_type desired_output[1] = {weightsVhat};

        // Push in game->toState() as input to the neural net and 
        // use the vhat() as the expected output to train the network. 

        fann_train(ann,
                   game->neural_inputs(), /* The game inputs */
                   desired_output /* the expected outputs */);

        cout << "Initial error: (" << 
            game->toValue() << "(value) - " << 
            weights.Vhat(b) << "(vhat(b) )) = " << error << endl;

        cout << "Errors => ";
		while (game->gametrace.size()) {
			// remember + remove the last move.
			PenteNeuralAI::cell* tCell = game->gametrace.back();
			game->gametrace.pop_back();
			game->clearCell(tCell->r, tCell->c);

			b = game->toState();
            weightsVhat = weights.Vhat(b);

            if (getenv("VERBOSE") != NULL) {
                //error = weights.Vhat(b) - game->toValue();
                cout << weightsVhat << ":" << game->toValue() << " ";
                cout << b.toString() << endl;
            }

            if (weightsVhat == 0) {
                cout << ".";
                cout.flush();
                continue;
            }

            desired_output[0] = weightsVhat;

            fann_train(ann,
                       game->neural_inputs(),
                       desired_output);

		}
        cout << endl;
	   
	}


    // Save the neural network back to disk.
    fann_save(ann, "pente.net");

    //fann_print_connections(ann);
    fann_destroy(ann);
	return 0;

}
