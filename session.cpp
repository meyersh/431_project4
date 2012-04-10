#include <iostream>

#include <vector>
#include <string>

#include "session.hpp"

using namespace std;

int main() {

   cout << "gameid_exists(\"nonexistant\") -> " 
		<< gameid_exists("nonexistant") << endl; 
   cout << "gameid_age(\"nonexistant\") -> " << gameid_age("nonexistant") << endl;

   vector<string> games = list_games();
   for (int i = 0; i < games.size(); i++)
	  cout << games[i] << endl
		   << "   gameid_exists(\"" << games[i] << "\") -> " 
		   << gameid_exists(games[i]) << endl
		   << "   gameid_age(\"" << games[i] << "\") -> " 
		   << gameid_age(games[i]) 
		   << endl;

   if (gameid_exists("DELETEME"))
	  {
	  cout << "Detected DELETEME game, testing remove_game." << endl;
	  remove_game("DELETEME");
	  }

   return 0;
}
