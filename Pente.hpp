/* 
 * File:   Pente.hpp
 * Author: mab015
 * Errors introduced by Shaun Meyer
 *
 * Created on February 28, 2012, 11:27 PM
 */

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <stdexcept>
#include "wlm.hpp"
#include "game.hpp"

using namespace std;

enum {
    W, NW, N, NE, E, SE, S, SW
}; // Directions


class PenteLinearAI: public PenteGame {
public:

    struct cell {
        int color;
        int r, c;
        cell * neighbors[8];
        
        cell(int r = 0, int c = 0, int color = EMPTY) {
            this->r = r;
            this->c = c;
            this->color = color;
        }
    };

    vector<cell*> Board;

    /* Data Members */
    int board_size;
    int turn; // turn number.
    string players[2];
    int blkCaps;
    int whtCaps;
    vector<cell*> gametrace;
    vector< vector<cell*> > captureBuffer;

    /* Member Functions */
    PenteLinearAI() {
        players[0] = players[1] = "WAITING";
        board_size = 19;
        turn = 0;
        _initBoard_();
        blkCaps = whtCaps = 0;
    }

    ~PenteLinearAI() {
        _killBoard_();
    }

    void reset() {
        _killBoard_();
        _initBoard_();
        captures(WHITE) = captures(BLACK) = 0;
    }
    
    int& captures(int color) {
        // Return an integer reference to the appropriate capture counter for `color`.
        if (color == WHITE)
            return whtCaps;
        else if (color == BLACK)
            return blkCaps;
        else
            throw new logic_error("Captures must be WHITE or BLACK.");
    }

    char opposite_color(char color) {
        return (color == WHITE) ? BLACK : WHITE;
    }

    void _initBoard_();
    void _killBoard_();
    bool isValidCoords(int r, int c);
    bool isValidColor(int color);
    int lastRow() {
        return gametrace.back()->r;
    }

    int lastCol() {
        return gametrace.back()->c;
    }
    
    cell *getCell(int r, int c);
    cell *getCell(int i);

    bool isEmpty(int r, int c);
    void fillCell(int r, int c, int color);
    void clearCell(int r, int c);
    vector<cell*> getFilled(int color);
    int getPossible(int &possD, int &possT, int &possQ, int &possWins, int color);
    int getAllBlocks(int &D, int &T, int &Q, int &P, int color);
    int getCertainSpaces(int &D, int &T, int &Q, int &P, int color);
    int getCertain(int &certD, int &certT, int &certQ, int &certP, int color);
    int getPossibleCaptures(int color);
    int chkTotalCapture(int color);
    int chkCapture(int r, int c, int color, bool remove = false);
    int chkTotalBlocks(int &Block3, int &Block4, int &Block5, int color);
    int chkBlocks(int &Block3, int &Block4, int &Block5, int color = EMPTY);
    double getProximity(int color, int radius);
    string toString();
    string serialize();
    void deserialize(ifstream &f);
    vector<cell*> getEmpty();

    int playerNumber(string sessionid);
    int playerColor(string sessionid);
    string playerTurn();
    int nInARow(int n, int color);
    void playToken(int r, int c, int color);
    void unPlayToken();
    int gameOutcome(int color);


    State toState();    
    State toStateOld();
    State tryMove(int r, int c, int color);
    void makeMove(Weight &weight);



};


void PenteLinearAI::_initBoard_() {
    cell *tCell; //zombies!
    int row, col;

    for (int r = 0; r < 19; r++) {
        for (int c = 0; c < 19; c++) {
            tCell = new cell(r, c, EMPTY);
            Board.push_back(tCell);
        }
    }

    for (int i = 0; i < Board.size(); i++) {
        tCell = Board[i];
        for (int dir = 0; dir < 8; dir++) {
            row = tCell->r;
            col = tCell->c;
            switch (dir) {
            case W:
                col--;
                break;
            case NW:
                row--;
                col--;
                break;
            case N:
                row--;
                break;
            case NE:
                row--;
                col++;
                break;
            case E:
                col++;
                break;
            case SE:
                row++;
                col++;
                break;
            case S:
                row++;
                break;
            case SW:
                row++;
                col--;
                break;
            }
            if ((row < 0) || (row >= 19) || (col < 0) || (col >= 19))
                tCell->neighbors[dir] = NULL;
            else
                tCell->neighbors[dir] = getCell(row, col);
        }
    }
}

void PenteLinearAI::_killBoard_() {
    for (int i = 0; i < Board.size(); i++) {
        delete Board[i];
        Board[i] = NULL;
    }
    Board.clear();
    gametrace.clear();
}

bool PenteLinearAI::isValidCoords(int r, int c) {
    return ((r >= 0) && (r < 19) && (c >= 0) && (c < 19));
}

bool PenteLinearAI::isValidColor(int color) {
    return (toupper(color) == BLACK || toupper(color) == WHITE);
}

bool PenteLinearAI::isEmpty(int r, int c) {
    if (!isValidCoords(r, c))
        return false;
    return getCell(r, c)->color == EMPTY;
}

PenteLinearAI::cell *PenteLinearAI::getCell(int r, int c) {
    // Get cell by coords

    if (!isValidCoords(r, c))
        throw out_of_range("getCell(r,c): row or col out_of_range error.");

    return Board[r * 19 + c];
}

PenteLinearAI::cell *PenteLinearAI::getCell(int i) {
    // Get cell by index.

    if (i < 0 || i > Board.size())
        throw out_of_range("getCell(i): out_of_range.");

    return Board[i];
}

void PenteLinearAI::fillCell(int r, int c, int color) {
    color = toupper(color);

    if (!isValidCoords(r, c))
        throw runtime_error("Cell or Row out of bounds.");

    if (isValidColor(color)) {
        getCell(r, c)->color = color;
    }
}

void PenteLinearAI::clearCell(int r, int c) {

    if (!isValidCoords(r, c))
        throw runtime_error("Cell or Row out of bounds.");

    getCell(r, c)->color = EMPTY;
}

vector<PenteLinearAI::cell*> PenteLinearAI::getFilled(int color) {
    cell *t;
    vector<cell*> filled;

    if (!isValidColor(color))
        throw runtime_error("Invalid color.");

    for (int i = 0; i < Board.size(); i++) {
        t = Board[i];
        if (t->color == color)
            filled.push_back(t);
    }

    return filled;
}

int PenteLinearAI::getPossible(int &possD, int &possT, int &possQ, int &possWins, int color) {
    cell *tCell, *nxt;
    vector<cell*> filled = getFilled(color);
    int count = 1, blank = 0;

    if (filled.size() == 0)
        return 0;


    for (int i = 0; i < filled.size(); i++) {
        tCell = filled[i];
        for (int j = 4; j < 8; j++) {
            nxt = tCell->neighbors[j];
            while ((nxt != NULL) && (blank < 2)) {
                if (nxt->color == color)
                    count++;
                else if (nxt->color == EMPTY)
                    blank++;
                else if (nxt->color != color)
                    break;
                nxt = nxt->neighbors[j];
            }
            if ((tCell->neighbors[j - 4] != NULL) && (tCell->neighbors[j - 4]->color)) {
                ;
            } else if ((tCell->neighbors[j - 4] == NULL) || (tCell->neighbors[j - 4]->color == EMPTY)) {
                switch (count) {
                case 2:
                    possT++;
                    break;
                case 3:
                    possQ++;
                    break;
                case 4:
                    possWins++;
                    break;
                default:
                    break;
                }
            }
            count = 1;
            blank = 0;
        }
    }

    return 0;

}

int PenteLinearAI::getAllBlocks(int &D, int &T, int &Q, int &P, int color) {
    // getCertainSpaces with a twist: we require zero space on either end.
    cell *tCell, *nxt;
    vector<cell*> filled = getFilled(color);

    D = T = Q = P = 0; // Initialize the values

    if (!isValidColor(color))
        throw runtime_error("Invalid color");

    if (filled.size() == 0)
        return 0;

    for (int i = 0; i < filled.size(); i++) {
        tCell = filled[i];
        for (int j = 4; j < 8; j++) {
            int count = 1;
            bool has_beginning_space = false;
            bool has_ending_space = false;

            // Check for leading space
            if (tCell->neighbors[j-4] && !tCell->neighbors[j-4]->color)
                has_beginning_space = true;

            // Skip cells we've already visited.
            if ((tCell->neighbors[j - 4] != NULL)
                && (tCell->neighbors[j - 4]->color == color)) {
                continue;
            }

            nxt = tCell->neighbors[j];
            while ((nxt != NULL) && (nxt->color)) {
                if (nxt->color == color)
                    count++;
                else {
                    has_ending_space = false;
                    break;
                }
                nxt = nxt->neighbors[j];
            }
            
            if (nxt && !nxt->color)
                has_ending_space = true;
            
            // no beginning and no ending spaces:
            if (has_beginning_space && has_ending_space)
                continue;

            if (nxt && (!nxt->color || nxt->color != color))
                switch (count) {
                case 5:
                    P += has_beginning_space + has_ending_space;
                    Q += has_beginning_space && has_ending_space; 
                case 4:
                    Q += has_beginning_space + has_ending_space;
                    T += has_beginning_space && has_ending_space; 
                case 3:
                    T += has_beginning_space + has_ending_space;
                    D += has_beginning_space && has_ending_space; 
                case 2:
                    D += has_beginning_space + has_ending_space;
                default:
                    break;
                }

        }
    }

    return 0;
}

int PenteLinearAI::getCertainSpaces(int &D, int &T, int &Q, int &P, int color) {
    // getCertain with a twist: we require space on either end.
    cell *tCell, *nxt;
    vector<cell*> filled = getFilled(color);

    D = T = Q = P = 0; // Initialize the values

    if (!isValidColor(color))
        throw runtime_error("Invalid color");

    if (filled.size() == 0)
        return 0;

    for (int i = 0; i < filled.size(); i++) {
        tCell = filled[i];
        for (int j = 4; j < 8; j++) {
            int count = 1;
            bool has_beginning_space = false;
            bool has_ending_space = false;

            // Check for leading space
            if (tCell->neighbors[j-4] && !tCell->neighbors[j-4]->color)
                has_beginning_space = true;

            // Skip cells we've already visited.
            if ((tCell->neighbors[j - 4] != NULL)
                && (tCell->neighbors[j - 4]->color == color)) {
                continue;
            }

            nxt = tCell->neighbors[j];
            while ((nxt != NULL) && (nxt->color)) {
                if (nxt->color == color)
                    count++;
                else {
                    has_ending_space = false;
                    break;
                }
                nxt = nxt->neighbors[j];
            }
            
            if (nxt && !nxt->color)
                has_ending_space = true;
            
            // no beginning and no ending spaces:
            if (!has_beginning_space && !has_ending_space)
                continue;

            if (nxt && (!nxt->color || nxt->color != color))
                switch (count) {
                case 5:
                    P += has_beginning_space + has_ending_space;
                    Q += has_beginning_space && has_ending_space;
                case 4:
                    Q += has_beginning_space + has_ending_space;
                    T += has_beginning_space && has_ending_space;
                case 3:
                    T += has_beginning_space + has_ending_space;
                    D += has_beginning_space && has_ending_space;
                case 2:
                    D += has_beginning_space + has_ending_space;
                default:
                    break;
                }

        }
    }

    return 0;
}

int PenteLinearAI::getCertain(int &certD, int &certT, int &certQ, int &certP, int color) {
    cell *tCell, *nxt;
    vector<cell*> filled = getFilled(color);
    int count = 1;

    certD = certT = certQ = certP = 0; // Initialize the values

    if (!isValidColor(color))
        throw runtime_error("Invalid color");

    if (filled.size() == 0)
        return 0;

    for (int i = 0; i < filled.size(); i++) {
        tCell = filled[i];
        for (int j = 4; j < 8; j++) {
            nxt = tCell->neighbors[j];
            while ((nxt != NULL) && (nxt->color)) {
                if (nxt->color == color)
                    count++;
                else
                    break;
                nxt = nxt->neighbors[j];
            }

            // Skip cells we've already visited.
            if ((tCell->neighbors[j - 4] != NULL)
                && (tCell->neighbors[j - 4]->color == color)) {
                count = 1;
                continue;
            }

            switch (count) {
            case 2:
                certD++;
                break;
            case 3:
                certT++;
                break;
            case 4:
                certQ++;
                break;
            case 5:
                certP++;
                break;
            default:
                break;
            }

            count = 1;
        }
    }

    return 0;
}

int PenteLinearAI::getPossibleCaptures(int color) {
    // Check for the number of captures available to `color` on the board.
    // (WBB_ is one for white, for instance.)

    int caps = 0;

    int opposite_color = (color == WHITE) ? BLACK : WHITE;

    for (int i = 0; i < Board.size(); i++) {
        cell *tCell = getCell(i);

        // Skip the empty cells and cells which arn't `color`.
        if (tCell->color != color || !tCell->color)
            continue;

        // Search all directions for our CNN_ pattern
        for (int dir = W; dir <= SW; dir++) {
            cell * cellSet[] = {tCell, NULL, NULL, NULL};
            bool setOK = true;

            for (int c = 1; c < 4 && setOK; c++) {
                if (cellSet[c - 1])
                    cellSet[c] = cellSet[c - 1]->neighbors[dir];

                if (cellSet[c] == NULL) {
                    setOK = false;
                    break;
                }
            }

            if (!setOK)
                continue;

            // Require the inner cells to have pieces
            if (!cellSet[1]->color || !cellSet[2]->color)
                continue;

            // Require the inner cells to be matching colors
            if (cellSet[1]->color != cellSet[2]->color)
                continue;

            // Require the inner cells to be the opposite color from tCell.
            if (cellSet[1]->color != opposite_color)
                continue;

            // Require the end cell to be open.
            if (cellSet[3]->color)
                continue;

            // By now, we have a situtation where `color` can capture.
            caps++;

        }

    }

    return caps;

}

int PenteLinearAI::chkTotalCapture(int color) {
    int n = 0;

    vector<PenteLinearAI::cell*> filled = getFilled(color);
    
    for (int i = 0; i < filled.size(); i++) {
        n += chkCapture(filled[i]->r, filled[i]->c, color, false);
    }

    return n;
}

int PenteLinearAI::chkCapture(int r, int c, int color, bool remove) {
    /* placing a piece in r,c of color `color`, if this would
       result in any capture, record the number. If `remove` is true,
       update the scores and remove the captured pieces.

       always return the number of captures at r,c for `color`. */

    cell *tCell, *one, *two, *end;
    int eColor = ((color == BLACK) ? WHITE : BLACK);
    tCell = getCell(r,c);
    int caps = 0;

    // Add a slot onto the end of captureBuffer.
    captureBuffer.push_back( vector<cell*>() ); 

    for (int j = 0; j < 8; j++) {
        one = tCell->neighbors[j];
        if ((one == NULL) || (one->color == EMPTY) || (one->color != eColor))
            continue;
        two = one->neighbors[j];
        if ((two == NULL) || (two->color == EMPTY) || (two->color != eColor))
            continue;
        end = two->neighbors[j];
        if ((end == NULL) || (end->color == EMPTY))
            continue;
        else if ((end->color) && (end->color == color)) {
            caps++;
            if(remove) {

                // Note these pieces in our buffer.
                captureBuffer[turn-1].push_back(one);
                captureBuffer[turn-1].push_back(two);

                one->color = two->color = EMPTY;
            }
        }
    }

    if (remove) 
        captures(color) += caps;

    return caps;
}

int PenteLinearAI::chkTotalBlocks(int& Block3, int& Block4, int& Block5, int color) {
    // Check to see how many 3, 4, and 5 in-a-row's have been blocked.

    int fdir, bdir, f=0, b=0, tot;
    vector<cell*> filled = getFilled(color);
    cell *tCell;
    cell *nxt;

    Block3 = Block4 = Block5 = 0;

    for (int i = 0; i < filled.size(); i++) {
        tCell = getCell(i);
        
        bool skip = false;
        for (int dir=W;dir<E;dir++)
            if (tCell->neighbors[dir] 
                && tCell->neighbors[dir]->color == color)
                skip = true;

        if (skip)
            continue;

        for(fdir=E;fdir<=SW;fdir++) {
            bdir = fdir-4;
            //check forwards
            nxt = tCell->neighbors[fdir];
            while(nxt && (nxt->color == color)) {
                f++;
                nxt = nxt->neighbors[fdir];
            }
            //check backwards
            nxt = tCell->neighbors[bdir];
            while(nxt && (nxt->color == color)) {
                b++;
                nxt = nxt->neighbors[bdir];
            }
        }

        tot = f+b;  tot = (tot>5)?5:tot;
        switch(tot) {
        case 5:
            Block5++;
        case 4:
            Block4++;
        case 3:
            Block3++;
        default:
            break;
        }
    }
    return 0;
}


int PenteLinearAI::chkBlocks(int& Block3, int& Block4, int& Block5, int color) {
    // Check the last move to see how many 3, 4, and 5 in-a-row's it blocked.
    if(gametrace.empty())
        return 0;
    color = (color == EMPTY) ? gametrace.back()->color  : color;
    int fdir, bdir, f=0, b=0, tot;
    cell *tCell = gametrace.back();
    cell *nxt;

    Block3 = Block4 = Block5 = 0;

    for(fdir=E;fdir<=SW;fdir++) {
        bdir = fdir-4;
        //check forwards
        nxt = tCell->neighbors[fdir];
        while(nxt && (nxt->color == color)) {
            f++;
            nxt = nxt->neighbors[fdir];
        }
        //check backwards
        nxt = tCell->neighbors[bdir];
        while(nxt && (nxt->color == color)) {
            b++;
            nxt = nxt->neighbors[bdir];
        }
    }

    tot = f+b;  tot = (tot>5)?5:tot;
    switch(tot) {
    case 3:
        Block3++;
        break;
    case 4:
        Block4++;
        break;
    case 5:
        Block5++;
        break;
    default:
        break;
    }

    return 0;
}

double PenteLinearAI::getProximity(int color, int radius) {
	cell *tCell, *nxt;
	vector<PenteLinearAI::cell*> filled = getFilled(color);
	int c = 0, prox = 0;
	
	for(int i=0;i<filled.size();i++) {
		tCell = filled[i];
		for(int dir=E;dir<=SW;dir++) {
			nxt = tCell->neighbors[dir];
			while(nxt && (c<radius)) {
				if(nxt->color == color)
					prox++;
				nxt = nxt->neighbors[dir];
				c++;
			}
			c = 0;
		}
	}
	
	return (double)(prox/(filled.size()+1));
}

string PenteLinearAI::toString() {
    stringstream ss;

    for (int r = 0; r < board_size; r++) {
        for (int c = 0; c < board_size; c++) {
            int contents = getCell(r, c)->color;

            if (contents == EMPTY)
                ss << "_";
            else if (contents == WHITE)
                ss << "W";
            else if (contents == BLACK)
                ss << "B";

            ss << " ";
        }
        ss << endl;
    }

    return ss.str();

}

string PenteLinearAI::serialize() {
    // A simple string serialization of the board.
    // All of the occupied cells in "<row> <col>" format seems fine.

    stringstream ss;

    ss << players[0] << endl;
    ss << players[1] << endl;

    for (vector<cell*>::iterator c = gametrace.begin(); c != gametrace.end(); c++) {
        ss << " " << (*c)->r 
           << " " << (*c)->c << endl;
    }

    return ss.str();
}

void PenteLinearAI::deserialize(ifstream &f) {
    // read the file `f` (already open) for our serial format:
    // <row> <col>

    string line;
    int color;
    int row, col;

    f >> players[0]; // first line is session id (or COMPUTER) of the white player.
    f >> players[1]; // second line is the session id (or COMPUTER) of the black player.

    while (getline(f, line)) {
        // Skipping any empty lines
        if (line == "")
            continue;

        color = (turn % 2 == 0) ? WHITE : BLACK; 

        stringstream ss(line);

        ss >> row >> col; // space separated...

        if (!isValidColor(color))
            continue; // invalid row.

        if (!isValidCoords(row, col))
            continue;

        // Everything is good. Insert the piece.
        playToken(row, col, (color == 'W' ? BLACK : WHITE));

    }

}

vector<PenteLinearAI::cell*> PenteLinearAI::getEmpty() {
    vector<cell*> emT;
    cell *tmp;

    for (int i = 0; i < Board.size(); i++) {
        tmp = Board[i];
        if (tmp->color == EMPTY) {
            emT.push_back(tmp);
        }
    }

    return emT;
}

int PenteLinearAI::playerNumber(string sessionid) {
    // Given a playerid return the index of players[]
    // or -1 for invalid id.
    for (int i = 0; i < 2; i++)
        if (players[i] == sessionid)
            return i;

    return -1;
}

int PenteLinearAI::playerColor(string sessionid) {
    // Return the color char of a given player
    // (or empty for none.)
    switch (playerNumber(sessionid)) {
    case -1:
        return EMPTY;
    case 0:
        return WHITE;
    case 1:
        return BLACK;
    }
}

string PenteLinearAI::playerTurn() {
    return players[ turn % 2 ];
}

int PenteLinearAI::nInARow(int n, int color) {
    // Return how instances a given color has of n-length in a row.

    int found = 0;

    vector<cell*> filled = getFilled(color);

    for (int i = 0; i < filled.size(); i++) {
        PenteLinearAI::cell *tCell = filled[i];

        for (int dir = 4; dir < 8; dir++) {
            int count = 1;
            PenteLinearAI::cell *nxt = tCell->neighbors[dir];

            while (nxt && nxt->color) {
                if (nxt->color == color)
                    count++;
                else
                    break;

                nxt = nxt->neighbors[dir];
            }

            if (tCell->neighbors[dir - 4]
                && tCell->neighbors[dir - 4]->color == color) 
                continue;

            if (count == n)
                found++;

        }
    }

    return found;

}

void PenteLinearAI::playToken(int r, int c, int color) {

    turn++; // increment the move counter.

    fillCell(r, c, color);
    gametrace.push_back(getCell(r, c)); 

    // this will put cell* in captureBuffer[turn] for each capture.
    chkCapture(r, c, color, true); 

}

void PenteLinearAI::unPlayToken() {
    // Undo the last move.
    int last_turn = turn - 1;

    PenteLinearAI::cell *last_piece = gametrace.back();
    
    // This is the color of the most recent move.
    int last_move_color = last_piece->color;

    // remove the piece.
    clearCell(gametrace.back()->r, gametrace.back()->c);

    // replace captured pieces.
    // TODO: change capture count BACKwards.
    if (!captureBuffer[last_turn].empty())
        {
            PenteLinearAI::cell *tCell;

            // This is the color of the pieces which were captured.
            int capture_color = (last_move_color == WHITE) ? BLACK : WHITE;

            // Reduce the capture count.
            captures(last_move_color) -= captureBuffer[last_turn].size()/2;

            if (captures(last_move_color) < 0)
                throw logic_error("captures have gone negative.");

            for (int c = 0; c < captureBuffer[last_turn].size(); c++)
                {
                    tCell = captureBuffer[last_turn][c];
                                        fillCell(tCell->r, tCell->c, capture_color);
                    
                }
        }

    // undo our memory.
    gametrace.pop_back();

    // undo that the turn ever happened...
    captureBuffer.pop_back();

    turn = last_turn;

}

int PenteLinearAI::gameOutcome(int color) {
    // Has a given color won, lost, or neither (yet)?
    // Return 1, -1, 0 accordingly.

    enum {
        LOST = -1, UNDETERMINED = 0, WON = 1
    };

    int theirColor = opposite_color(color);

    if (captures(theirColor) >= 5)
        return LOST;

    if (captures(color) >= 5)
        return WON;

    if (nInARow(5, color) > 0)
        return WON;
    if (nInARow(5, theirColor) > 0)
        return LOST;

    return UNDETERMINED;
}

State PenteLinearAI::toState() {
    int D, T, Q, P;

    State s;

    int ours, theirs;

    ours = playerColor("COMPUTER");
    theirs = (ours == WHITE) ? BLACK : WHITE;

    // Register all sets that are free to develop further
    getCertainSpaces(D, T, Q, P, ours);
    s.insert(D);
    s.insert(T);
    s.insert(Q);
    s.insert(P);

    getCertainSpaces(D, T, Q, P, theirs);
    s.insert(D);
    s.insert(T);
    s.insert(Q);
    s.insert(P);

    // Register blocked sets (that is, sets which are blocked on each end)
    getAllBlocks(D, T, Q, P, ours);
    s.insert(D);
    s.insert(T);
    s.insert(Q);
    s.insert(P);

    getAllBlocks(D, T, Q, P, theirs);
    s.insert(D);
    s.insert(T);
    s.insert(Q);
    s.insert(P);

    // Register the potential captures OTT_ setups.
    s.insert(getPossibleCaptures(ours));
    s.insert(getPossibleCaptures(theirs));

    // Note the number of captures. 
    s.insert(captures(ours));
    s.insert(captures(theirs));
    
    /*
    // Register interruptions to a line such as OTOO
    chkTotalBlocks(T, Q, P, ours);
    s.insert(T);
    s.insert(Q);
    s.insert(P);
    
    chkTotalBlocks(T, Q, P, theirs);
    s.insert(T);
    s.insert(Q);
    s.insert(P);
    */

    return s;

}

State PenteLinearAI::toStateOld() {
    /* X variables:
       0: doubles (ours)
       1: triples (ours)
       2: quads (ours)
       3: pentes (ours) // useful when looking at potential moves
       4: possible captures (ours)
       5: Resulting captures from the last move (ours).
       6: Resulting 3x blocks (ours)
       7: Resulting 4x blocks (ours)
       8: Resulting 5x blocks (ours)
       9: Proximity(ours, 2)
       
       10: doubles (theirs)
       11: triples (theirs)
       12: quads (theirs)
       13: pentes (theirs) // useful when looking at potential moves
       14: possible captures (theirs)
       15: resulting captures from the last move (theirs)
       16: Resulting 3x blocks (ours)
       17: Resulting 4x blocks (ours)
       18: Resulting 5x blocks (ours)
       19: Proximity(theirs, 2)

       
    */

    // Construct and return the state of the game.
    State s(20);

    int ours, theirs;

    if (playerNumber("COMPUTER") == 0) {
        ours = WHITE;
        theirs = BLACK;
    } else {
        ours = BLACK;
        theirs = WHITE;
    }
            

    /*    if(playerNumber("COMPUTER")==0) {
     */
       
    // Figure for our pieces...
    getCertain(s[0], s[1], s[2], s[3], ours);
    s[4] = getPossibleCaptures(ours);
    s[5] = (gametrace.empty()) ? 0 : 
        chkCapture( gametrace.back()->r, gametrace.back()->c,
                    ours );

    chkBlocks(s[6], s[7], s[8], ours);

    s[9] = getProximity(ours, 2);
    
    // Figure for their pieces.
    getCertain(s[10], s[11], s[12], s[13], theirs);
    s[14] = getPossibleCaptures(theirs);
    s[15] = (gametrace.empty()) ? 0 :
        chkCapture( gametrace.back()->r, gametrace.back()->c,
                    theirs );

    chkBlocks(s[16], s[17], s[18], theirs);

    s[19] = getProximity(theirs, 2);

    /*
      } else if(playerNumber("COMPUTER")==1) {
      // Figure for black pieces...
      getCertain(s[0], s[1], s[2], s[3], BLACK);
      s[4] = getPossibleCaptures(BLACK);
      s[5] = (gametrace.empty()) ? 0 : 
      chkCapture( gametrace.back()->r, gametrace.back()->c,
      gametrace.back()->color );
      // Figure for white pieces.
      getCertain(s[6], s[7], s[8], s[9], WHITE);
      s[10] = getPossibleCaptures(WHITE);
      s[11] = gametrace.empty() ? 0 :
      chkCapture( gametrace.back()->r, gametrace.back()->c,
      gametrace.back()->color );
      } 
        
      //s[8] = (playerColor("COMPUTER"));
      */
    return s;
}

State PenteLinearAI::tryMove(int r, int c, int color) {

    // Try a move
    playToken(r, c, color);
    
    // Record the state
    State s = toState();

    //Undo the fantasy.
    unPlayToken();


    return s;
}

void PenteLinearAI::makeMove(Weight &weight) {
    // Make a computerized move.
    srand(weight.random());
    vector<cell*> possible_moves = getEmpty();
    random_shuffle(possible_moves.begin(), possible_moves.end());
    cell* best_move = possible_moves[0];
    // Figure out our color
    int computer_color = playerColor("COMPUTER");

    // Assume the best
    double best_state = weight.Vhat(tryMove(best_move->r, 
                                            best_move->c, 
                                            computer_color));

    // Compare every other move
    for (int i = 0; i < possible_moves.size(); i++) {
        // Try a move
        State fantasy_move = tryMove(possible_moves[i]->r,
                                     possible_moves[i]->c,
                                     computer_color);

        // Remember a better move
        if (weight.Vhat(fantasy_move) > best_state) {
            best_move = possible_moves[i];
            best_state = weight.Vhat(fantasy_move);
        }

    } // end for

    // Make the _BEST_ move.
    playToken(best_move->r, best_move->c, computer_color);
    
}
