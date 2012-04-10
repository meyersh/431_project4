/*
 * session.hpp
 * Store sessions and gameids
 *
 * Shaun Meyer, Mar 2012
 */

#ifndef __SESSION_HPP__
#define __SESSION_HPP__

#include <fstream>
#include <stdexcept>
#include <vector>
#include <string>
#include <ctime>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>



using namespace std;

// The trailing slash is required.
#define GAMEID_DIRECTORY "games/"
#define SESSION_WORDS_FILE "session-words.txt"
#define NUM_WORDS 235724

// ProtoTypes
const char* gameid_file_path(const string);
void remove_game(string);
vector<string> list_games();
bool gameid_exists(string);
int gameid_age(string);
template<class T> int save_game(string gameid, T game);
string get_word(string index);
int sessionid_turn(string gameid, string sessionid);

// 
// Definitions here:
//

const char* gameid_file_path(const string gameid) {
   // Render the file path for a gameid. 
   
   string path = string(GAMEID_DIRECTORY) + gameid;
   return path.c_str();
}

void remove_game(string gameid) {
   // delete a gameid file.
   
   if (gameid_exists( gameid ))
	  unlink(gameid_file_path(gameid));

}

vector<string> list_games() {
   // List the games in GAMEID_DIRECTORY
 
   vector<string> ret;

   DIR *dp;
   struct dirent *ep;     
   dp = opendir(GAMEID_DIRECTORY);
   
   if (dp == NULL)
	  throw runtime_error("Couldn't open the directory");

   while (ep = readdir (dp))
	  {
	  string file = ep->d_name;
	  if (file != "." 
		  && file != "..")
		 ret.push_back(file);
	  }
   (void) closedir (dp);

   return ret;
}

bool gameid_exists(string gameid) {
   // Check if a game exists.
   ifstream gameid_file( gameid_file_path(gameid) );
   return gameid_file; // nz for ok to write.

}

int gameid_age(string gameid) {
   // how old is a file in seconds?

   struct stat s;

   stat( gameid_file_path(gameid), &s);

   std::time_t t   = s.st_mtime;
   std::time_t now = std::time(NULL);
   
   return now-t;
}

template<class T> 
int save_game(string gameid, T game) {
   // Serialize and save a game.

   ofstream gameid_file(gameid_file_path(gameid));
   
   if (!gameid_file.good())
	  return 1;

   gameid_file << game.serialize() << endl;
   gameid_file.close();

   return 0;
}

string get_word(int index) {

   ifstream dict(SESSION_WORDS_FILE);
   string word;

   index = index % NUM_WORDS; // the number of words

   for (int i = 0; i < index; i++)
	  getline(dict, word);

   dict.close();

   return word;

}

string generate_sessionid() {
   return get_word((list_games().size() + 12) % NUM_WORDS);
}

#endif
