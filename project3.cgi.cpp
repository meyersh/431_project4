/*
 * Handle the webapp junk.
 * Shaun Meyer, Mar 2012
 */

// TODO: Game serialization for saving + loading
// TODO: game_id persistance
// TODO: game_id timeouts
// TODO: ?

#include <iostream>
#include <sstream>
#include <vector>
#include "session.hpp"
#include "game.hpp"
#include "Pente.hpp"

using namespace std;

#define MAX_GAME_AGE (60*5)

void die(string msg) {
	cout << "ERROR_CAUSED_SHUTDOWN" << endl
		 << msg << endl;
	exit(1);
}

int main() {
	string game_id;
	string session_id;

	string instr; // initial POST instruction

	string param; // a place holder for reading in the parameters

	vector<string> params; // subsequent POSTS.

	getline(cin, instr); // all requests are by POST.
   
    // Initialize `params`, skipping empty lines.
	while (getline(cin, param))
		if (param != "")
			params.push_back(param);

	cout << "Content-Type: text/plain\n\n";

	// Filter out blank instructions (initial instruction).
	if (instr == "")
		die("msg: Expected parameter.");

	/*
	 * S E T U P
	 */

	if (instr == "SETUP")
        {
            // check for four characters, noting all options
            // have size() == 4, begin with h and end with 1 or 2.
            
            // Housekeeping: delete all expired games.
            vector<string> games = list_games();
            for (int i = 0; i < games.size(); i++)
                if (gameid_age(games[i]) > MAX_GAME_AGE) 
                    remove_game(games[i]);

            bool human_only;
            bool creator_is_white;
            string gameid;
            string sessionkey;
	  
            string game_type = params[0];

            if (game_type.size() != 3 || game_type[0] != 'h')
                die("Invalid parameter: " + param);

            if (game_type[1] == 'h')
                human_only = true;
            else if (game_type[1] == 'c')
                human_only = false;
            else
                die("2nd character must be h or c.");

            if (game_type[2] == '1')
                creator_is_white = true;
            else if (game_type[2] == '2')
                creator_is_white = false;
            else
                die("3rd character must be 1 or 2.");

            // Generate a unique game id and session key.
            for (int i = 0; i < NUM_WORDS; i++)
                {
                    gameid = get_word(i);
                    sessionkey = get_word(++i);
                    if (!gameid_exists(gameid))
                        break;
                }

            ofstream game_file(gameid_file_path(gameid));

            if (!game_file.good())
                die("Unable to create new game file at '" + 
                    string(gameid_file_path(gameid)) + "'.");

            PenteGame *game;

            if (human_only) 
                {

                    if (creator_is_white) // hh1
                        game->players[0] = sessionkey;
		  
                    else // hh2
                        game->players[1] = sessionkey;

                } 
            else 
                {
		
                    if (creator_is_white) // hc1
                        {
                            game->players[0] = sessionkey;
                            game->players[1] = "COMPUTER";
                        } 
                    else // hc2
                        {
                            game->players[0] = "COMPUTER";
                            game->players[1] = sessionkey;
                        }
                }

            // Finally, place the white piece on the board

            game->playToken(9,9,WHITE);


            // return the details to the client
            cout << "SETUP" << endl
                 << gameid << endl
                 << sessionkey << endl
                 << "9" << endl
                 << "9" << endl;
		
            if (game_type == "hh1")
                cout << "WAITING" << endl;
            else if (game_type == "hh2")
                cout << "MOVE" << endl;
            else if (game_type == "hc1")
                {
                    // generate a computer move and return it.
                    Weight weights(WEIGHTS_FILE);
                    weights.load();
                    game->makeMove(weights);
                    cout << game->lastRow() << endl
                         << game->lastCol() << endl
                         << "MOVE" << endl;
                }
            else if (game_type == "hc2")
                cout << "MOVE";

            // Save the file.
            game_file << game->serialize();
            game_file.close();
        }

	/*
	 * J O I N
	 */

	else if (instr == "JOIN")
        {

            // TODO: Assign a valid session ID otherwise.
            // TODO: Or report that the game is full.

            /*
              JOIN\n<gameid>	JOIN\n<gameid>\n<sessionid>	
              JOIN\n<gameid>	JOIN\n<gameid>\nGAME_UNDERWAY	
            */


            string gameid = params[0];

            if (params.size() != 1)
                die("Invalid number of parameters.");

            if (!gameid_exists(gameid))
                die("No game identified by '" + gameid + "'.");

            PenteGame *game;
		
            ifstream game_file( gameid_file_path(gameid) );
            game->deserialize(game_file);
            game_file.close();

            cout << "JOIN" << endl
                 << gameid << endl;

            // Both players have joined (are != WAITING)
            if (game->players[0] != "WAITING" && game->players[1] != "WAITING")
                cout << "GAME_UNDERWAY" << endl;

            // The player has joined as player2
            if (game->players[0] == "WAITING") {
                game->players[0] = generate_sessionid();
                cout << game->players[0] << endl
                     << "hh2" << endl
                     << "9" << endl
                     << "9" << endl
                     << "WAITING" << endl;
            }

            // The player has joined as player1
            else if (game->players[1] == "WAITING") {
                game->players[1] = generate_sessionid();
                cout << game->players[1] << endl
                     << "hh1" << endl 
                     << "9" << endl
                     << "9" << endl
                     << "MOVE" << endl;
            }

            // Save our game.
            ofstream ogame_file( gameid_file_path(gameid) );
            ogame_file << game->serialize() << endl;

            ogame_file.close();

        }

	/* 
	 * M O V E 
	 */
   
	else if (instr == "MOVE")
        {
            /* POST: MOVE\n<gameid>\n<sessionid>\n<row>\n<col>
               RESPONSE: 
               MOVE\n<gameid>\n<sessionid>
               MOVE\n<gameid>\n<sessionid>\nWIN
            */

            // TODO: Validate gameid, sessionid, and feed the computer a move.
            if (params.size() != 4)
                die("Invalid number of parameters.");

            string gameid    = params[0];
            string sessionid = params[1];
            int row          = atoi(params[2].c_str());
            int col          = atoi(params[3].c_str());

            if (!gameid_exists(gameid))
                die("No such gameid '" + gameid + "'.");

            PenteGame *game;
            ifstream infile(gameid_file_path(gameid));
            game->deserialize( infile );
            infile.close();

            // Die for invalid sessionid
            if (game->playerNumber(sessionid) == -1)
                die("'" + sessionid + "' is not a valid sessionid.");

            // Die for invalid coords
            if (!game->isValidCoords(row, col))
                die("Invalid coordinates."); // this should never happen
		 
            // Die for piece already layed.
            if(!game->isEmpty(row, col)) {
                stringstream ss;
                ss << "Cell already has a piece at (" << row << ',' << col << ")";
                die(ss.str());
            }

            // Die if the game is already completed. 
            if (game->gameOutcome(WHITE) != 0)
                die("This game has already been decided.");

            // Validate that it is, in fact, our turn.
            if (game->playerTurn() != sessionid)
                die("It's not even your turn to move!");

            // go ahead and lay the piece.
            game->playToken(row, col, game->playerColor(sessionid));

            // Save the game
            ofstream game_file(gameid_file_path(gameid));
            game_file << game->serialize();
            game_file.close();

            cout << "MOVE" << endl
                 << gameid << endl
                 << sessionid << endl;

            if (game->gameOutcome(game->playerColor(sessionid)) == 1)
                cout << "WIN" << endl;

        }

	/*
	 * C H E C K
	 */

	else if (instr == "CHECK")
        {

            // TODO: Return computer (or other player) move.

            /* POST:
               CHECK\n<gameid>\n<sessionid>

               RESPONSE:
               CHECK\n<gameid>\n<sessionid>\nWAITING
               CHECK\n<gameid>\n<sessionid>\n<row>\n<col>
               CHECK\n<gameid>\n<sessionid>\n<row>\n<col>\n{WIN|LOSE}
               CHECK\n<gameid>\n<sessionid>\n<row>\n<col>\nTIMEOUT
            */

            if (params.size() != 2)
                die("Invalid number of CHECK parameters.");

            string gameid    = params[0];
            string sessionid = params[1];

            cout << "CHECK" << endl
                 << gameid << endl
                 << sessionid << endl;

            if (!gameid_exists(gameid) || gameid_age(gameid) > MAX_GAME_AGE) {
                cout << "9" << endl    // bogus row + col because it can't matter.
                     << "9" << endl
                     << "TIMEOUT" << endl;

                // remove the old game.
                remove_game(gameid);

                exit(1);
            }

            // Load the game for more checks
            PenteGame *game;
            ifstream game_file( gameid_file_path(gameid) );
            game->deserialize(game_file);
            game_file.close();
	

            // Validate sessionid
            if (game->playerNumber(sessionid) == -1)
                die("Invalid sessionid: '" + sessionid + "'");

            // If the computer is playing and we're waiting on them,
            // go ahead and initiate a computer move.

            if (game->playerTurn() == "COMPUTER") {
                // Generate a computer move.
                Weight weights(WEIGHTS_FILE);
                weights.load();
                game->makeMove(weights);
                
                // Save the computer move to file.
                ofstream ogame_file( gameid_file_path(gameid) );
                ogame_file << game->serialize() << endl;
                ogame_file.close();
            }

            // Stop here if it is not the requesting players' turn.
            else if (game->playerTurn() != sessionid) {
                cout << "WAITING" << endl;
                exit(1);
            }

            // Output row + col of last move:
            cout << game->lastRow() << endl
                 << game->lastCol() << endl;

            // Report a possible victory.
            if (game->gameOutcome( game->playerColor(sessionid) ) == 1)
                cout << "WIN" << endl;

            else if (game->gameOutcome( game->playerColor(sessionid) ) == -1)
                cout << "LOSE" << endl;

        }
   

	return 0;

}
