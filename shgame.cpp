// Show a given game file.
#include <iostream>
#include <fstream>
#include <string>

#include "session.hpp"
#include "Pente.hpp"

using namespace std;

int main(int argc, char** argv)
{

    if (argc != 2) {
        cout << "Please specify a gameid." << endl;
    }

    string gameid = argv[1];
    ifstream game_file(gameid.c_str());

    if (!game_file) {
        cout << "Unable to open file at " 
             << gameid << "." << endl;
        exit(1);
    }

    Pente p;

    p.deserialize(game_file);

    
    cout << "Reading " << gameid << endl
         << p.toString() << endl
         << "Sessions:" << endl
         << "   players[0] => " << p.players[0] << endl
         << "   players[1] => " << p.players[1] << endl
         << "Captures:" << endl
         << "   White: " << p.whtCaps << endl
         << "   Black: " << p.blkCaps << endl
         << "GameOutcome(WHITE) => " << p.gameOutcome(WHITE) << endl
         << "turn => " << p.turn << endl;

    
    game_file.close();
    
    return 0;
}
