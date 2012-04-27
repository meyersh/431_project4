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
#include <fann.h>
#include <assert.h>
#include "wlm.hpp"
#include "game.hpp"

using namespace std;

enum {
    W, NW, N, NE, E, SE, S, SW
}; // Directions


class PenteNeuralAI: public PenteGame {
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

    int blkCaps;
    int whtCaps;
    vector<cell*> gametrace;
    vector< vector<cell*> > captureBuffer;

    /* Member Functions */
    PenteNeuralAI() {

        board_size = 19;
        turn = 0;
        _initBoard_();
        blkCaps = whtCaps = 0;
    }

    ~PenteNeuralAI() {
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
    int oppositeColor(int color) {
        return (color == WHITE) ? BLACK : WHITE;
    }
    int playerNumber(string sessionid);
    int playerColor(string sessionid);
    string playerTurn();
    int nInARow(int n, int color);
    void playToken(int r, int c, int color);
    void unPlayToken();
    int gameOutcome(int color);


    State toState();    

    State tryMove(int r, int c, int color);
    int RowColToIndex(int r, int c);
    fann_type *neural_inputs();
    float toValue();

    void makeMove(Weight &weight);



};


void PenteNeuralAI::_initBoard_() {
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

void PenteNeuralAI::_killBoard_() {
    for (int i = 0; i < Board.size(); i++) {
        delete Board[i];
        Board[i] = NULL;
    }
    Board.clear();
    gametrace.clear();
}

bool PenteNeuralAI::isValidCoords(int r, int c) {
    return ((r >= 0) && (r < 19) && (c >= 0) && (c < 19));
}

bool PenteNeuralAI::isValidColor(int color) {
    return (color == BLACK || color == WHITE);
}

bool PenteNeuralAI::isEmpty(int r, int c) {
    if (!isValidCoords(r, c))
        return false;
    return getCell(r, c)->color == EMPTY;
}

PenteNeuralAI::cell *PenteNeuralAI::getCell(int r, int c) {
    // Get cell by coords

    if (!isValidCoords(r, c))
        throw out_of_range("getCell(r,c): row or col out_of_range error.");

    return Board[r * 19 + c];
}

PenteNeuralAI::cell *PenteNeuralAI::getCell(int i) {
    // Get cell by index.

    if (i < 0 || i > Board.size())
        throw out_of_range("getCell(i): out_of_range.");

    return Board[i];
}

void PenteNeuralAI::fillCell(int r, int c, int color) {
    color = toupper(color);

    if (!isValidCoords(r, c))
        throw runtime_error("Cell or Row out of bounds.");

    if (isValidColor(color)) {
        getCell(r, c)->color = color;
    }
}

void PenteNeuralAI::clearCell(int r, int c) {

    if (!isValidCoords(r, c))
        throw runtime_error("Cell or Row out of bounds.");

    getCell(r, c)->color = EMPTY;
}

vector<PenteNeuralAI::cell*> PenteNeuralAI::getFilled(int color) {
    vector<cell*> filled;

    if (!isValidColor(color)) {
        stringstream msg;
        msg << "Invalid color: '" << color << "'";
        throw runtime_error(msg.str());
    }

    for (int i = 0; i < Board.size(); i++) {
        if (getCell(i)->color == color)
            filled.push_back(getCell(i));
    }

    return filled;
}

int PenteNeuralAI::getPossible(int &possD, int &possT, int &possQ, int &possWins, int color) {
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

int PenteNeuralAI::getAllBlocks(int &D, int &T, int &Q, int &P, int color) {
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

            //if (nxt && (!nxt->color || nxt->color != color)) // What is this even doing?
                switch (count) {
                case 5:
                    P += !has_beginning_space + !has_ending_space;
                    Q += !has_beginning_space && !has_ending_space; 
                case 4:
                    Q += !has_beginning_space + !has_ending_space;
                    T += !has_beginning_space && !has_ending_space; 
                case 3:
                    T += !has_beginning_space + !has_ending_space;
                    D += !has_beginning_space && !has_ending_space; 
                case 2:
                    D += !has_beginning_space + !has_ending_space;
                default:
                    break;
                }

        }
    }

    return 0;
}

int PenteNeuralAI::getCertainSpaces(int &D, int &T, int &Q, int &P, int color) {
    // getCertain with a twist: we require space on either end.
    cell *tCell, *nxt;
    vector<cell*> filled = getFilled(color);

    D = T = Q = P = 0; // Initialize the values

    if (!isValidColor(color))
        throw runtime_error("Invalid color");

    if (filled.empty())
        return 0;

    for (int i = 0; i < filled.size(); i++) {
        tCell = filled[i];
        for (int j = 4; j < 8; j++) {
            int count = 1;
            bool has_beginning_space = false;
            bool has_ending_space    = false;

            // Look behind for a few tests.
            if (tCell->neighbors[j-4]) {
            
                // Check for leading space
                if (tCell->neighbors[j-4]->color == EMPTY) 
                    has_beginning_space = true;
            
                // Skip cells we've already visited.
                else if (tCell->neighbors[j-4]->color == color) 
                    continue;

            } else
                has_beginning_space = false;
            
            
            nxt = tCell->neighbors[j];

            // Count up the number of `color` in a row. 
            while (nxt && nxt->color == color) {
                count++;
                nxt = nxt->neighbors[j];
            }
            
            // Check for a trailing space
            if (nxt && nxt->color == EMPTY)
                has_ending_space = true;
            
            // no beginning and no ending spaces:
            if (!has_beginning_space && !has_ending_space)
                continue;

            /*            if (nxt && (!nxt->color || nxt->color != color))*/ // What is this even doing?
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

int PenteNeuralAI::getCertain(int &certD, int &certT, int &certQ, int &certP, int color) {
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

int PenteNeuralAI::getPossibleCaptures(int color) {
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

int PenteNeuralAI::chkTotalCapture(int color) {
    int n = 0;

    vector<PenteNeuralAI::cell*> filled = getFilled(color);
    
    for (int i = 0; i < filled.size(); i++) {
        n += chkCapture(filled[i]->r, filled[i]->c, color, false);
    }

    return n;
}

int PenteNeuralAI::chkCapture(int r, int c, int color, bool remove) {
    /* placing a piece in r,c of color `color`, if this would
       result in any capture, record the number. If `remove` is true,
       update the scores and remove the captured pieces.

       always return the number of captures at r,c for `color`. */

    cell *tCell, *one, *two, *end;
    int eColor = oppositeColor(color);
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

int PenteNeuralAI::chkTotalBlocks(int& Block3, int& Block4, int& Block5, int color) {
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


int PenteNeuralAI::chkBlocks(int& Block3, int& Block4, int& Block5, int color) {
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

double PenteNeuralAI::getProximity(int color, int radius) {
	cell *tCell, *nxt;
	vector<PenteNeuralAI::cell*> filled = getFilled(color);
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

string PenteNeuralAI::toString() {
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

string PenteNeuralAI::serialize() {
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

void PenteNeuralAI::deserialize(ifstream &f) {
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

vector<PenteNeuralAI::cell*> PenteNeuralAI::getEmpty() {
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

int PenteNeuralAI::playerNumber(string sessionid) {
    // Given a playerid return the index of players[]
    // or -1 for invalid id.
    for (int i = 0; i < 2; i++)
        if (players[i] == sessionid)
            return i;

    return -1;
}

int PenteNeuralAI::playerColor(string sessionid) {
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

string PenteNeuralAI::playerTurn() {
    return players[ turn % 2 ];
}

int PenteNeuralAI::nInARow(int n, int color) {
    // Return how instances a given color has of n-length in a row.

    int found = 0;

    vector<cell*> filled = getFilled(color);

    for (int i = 0; i < filled.size(); i++) {
        PenteNeuralAI::cell *tCell = filled[i];

        for (int dir = 4; dir < 8; dir++) {
            int count = 1;
            PenteNeuralAI::cell *nxt = tCell->neighbors[dir];

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

void PenteNeuralAI::playToken(int r, int c, int color) {

    turn++; // increment the move counter.

    fillCell(r, c, color);
    gametrace.push_back(getCell(r, c)); 

    // this will put cell* in captureBuffer[turn] for each capture.
    chkCapture(r, c, color, true); 

}

void PenteNeuralAI::unPlayToken() {
    // Undo the last move.
    int last_turn = turn - 1;

    PenteNeuralAI::cell *last_piece = gametrace.back();
    
    // This is the color of the most recent move.
    int last_move_color = last_piece->color;

    // remove the piece.
    clearCell(gametrace.back()->r, gametrace.back()->c);

    // replace captured pieces.
    // TODO: change capture count BACKwards.
    if (!captureBuffer[last_turn].empty())
        {
            PenteNeuralAI::cell *tCell;

            // This is the color of the pieces which were captured.
            int capture_color = oppositeColor(last_move_color);

            // Reduce the capture count if there were captures.
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

int PenteNeuralAI::gameOutcome(int color) {
    // Has a given color won, lost, or neither (yet)?
    // Return LOST,  UNDETERMINED, or  WON.
    enum {LOST = -1, UNDETERMINED = 0, WON = 1};

    int theirColor = oppositeColor(color);

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

State PenteNeuralAI::toState() {
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

State PenteNeuralAI::tryMove(int r, int c, int color) {

    // Try a move
    playToken(r, c, color);
    
    // Record the state
    State s = toState();

    //Undo the fantasy.
    unPlayToken();


    return s;
}
 
int PenteNeuralAI::RowColToIndex(int r, int c) {
    return (r*19)+c;
}

fann_type *PenteNeuralAI::neural_inputs() {
    fann_type *res = (fann_type*)malloc(20 * sizeof(fann_type));

    State b = toState();

    for (int i = 0; i < b.size(); i++) {
        res[i] = b[i];
        
    }

    return res;
}

float PenteNeuralAI::toValue() {
    // Return the neural network output for the current board configuration.

    fann *ann = fann_create_from_file("pente.net");
    fann_type *output = fann_run(ann, neural_inputs());

    return (float)output[0];

}

void PenteNeuralAI::makeMove(Weight &weight) {
    // Make a computerized move.
    srand(weight.random());
    vector<cell*> possible_moves = getEmpty();
    random_shuffle(possible_moves.begin(), possible_moves.end());

    cell* best_move = NULL;
    float best_value = 0;

    // Figure out our color
    int computer_color = playerColor("COMPUTER");

    // Load up our Neural Network.
    fann *ann = fann_create_from_file("pente.net");
    
    // Our input array: -1 for opponent piece, 0 for none, 1 for ours. 
    fann_type *input = neural_inputs();

    // Find the best move we can make:
    for (int i = 0; i < possible_moves.size(); i++) {

        // Mark our imaginary move
        playToken(possible_moves[i]->r, possible_moves[i]->c, computer_color);

        // process the board with the neural net:    
        fann_type *output = fann_run(ann, neural_inputs());

        // Remember this move if it is the best we've seen so far.
        // Also: short-circuit for pentes. 
        if (output[0] > best_value || best_move == NULL) {
            best_value = output[0];
            best_move = possible_moves[i];
            }

        // Unmark our imaginary move.
        unPlayToken();
    }

    // Clean up our Neural Network
    fann_destroy(ann);

    //cout << best_value << endl;

    // Make the _BEST_ move.
    playToken(best_move->r, best_move->c, computer_color);
    
}
