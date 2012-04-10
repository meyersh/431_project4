#ifndef __GAME_HPP__
#define __GAME_HPP__

#include "wlm.hpp"
#include <vector>
#include <string>
#include <fstream>

enum {EMPTY, WHITE, BLACK};

class PenteGame {
public:
    
    string players[2];

    PenteGame() {
        players[0] = players[1] = "WAITING";
    }

    // The API requires us to query the last move played.
    virtual int lastRow() = 0;
    virtual int lastCol() = 0;

    virtual State toState() = 0;
    
    virtual std::string serialize() = 0;
    virtual void deserialize(std::ifstream &) = 0;

    // -1 for lost, 0 for neither, 1 for win.
    virtual int gameOutcome(int color) = 0;

    // Allow the CGI to do some basic error checking
    virtual bool isValidCoords(int r, int c) = 0;
    virtual bool isEmpty(int r, int c) = 0;

    // Allow the CGI to validate some informations.
    virtual int playerNumber(std::string sessionid) = 0; // DEPRECATED?
    virtual int playerColor(std::string sessionid) = 0;

    /* playerTurn() should return either the sessionid, 
       WAITING, or COMPUTER. */
    virtual std::string playerTurn() = 0;

    /* Place a piece on the board, calculate captures,
       remove captures, and increment lastRow() and lastCol() */
    virtual void playToken(int r, int c, int color) = 0;
    
    // Given a Weight reference, (with ::Vhat()) make a move.
    virtual void makeMove(Weight &weight) = 0;

    // Return the number of captures for color()
    virtual int& captures(int color) = 0;

};

#endif
