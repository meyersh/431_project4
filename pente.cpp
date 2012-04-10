#include <iostream>
#include <vector>
#include "assert.h"
#include "Pente.hpp"

using namespace std;

int main() {
    PenteLinearAI *p = new PenteLinearAI();

    p->players[0] = "COMPUTER";
    p->players[1] = "TesterMcTester";


    // Test playerNumber functionality
    assert (p->playerNumber("COMPUTER") == 0);
    assert (p->playerNumber("TesterMcTester") == 1);
    assert (p->playerNumber("Wrong!") == -1);

    // Test playerColor functionality
    assert (p->playerColor("COMPUTER") == WHITE);
    assert (p->playerColor("TesterMcTester") == BLACK);
    assert (p->playerColor("Wrong!") == EMPTY);

    // Test coords functionality
    assert (p->isValidCoords(-1,0) == false);
    assert (p->isValidCoords(0,-1) == false);
    assert (p->isValidCoords(19,0) == false);
    assert (p->isValidCoords(0,19) == false);

    assert (p->isValidCoords(0,0) == true);

    // Test color functionality
    assert (p->isValidColor(BLACK) == true);
    assert (p->isValidColor(WHITE) == true);

    assert (p->isValidColor('x') == false);

    // Test fillCell, getFilled, and getEmpty functionality
    p->fillCell(2,12, WHITE);
    assert (p->getFilled(WHITE).size() == 1);
    assert (p->getFilled(BLACK).size() == 0);
    assert (p->getEmpty().size() == 361-1);

    assert (p->isEmpty(2,12) == false);
    assert (p->isEmpty(2,13) == true);

    // test clearCell.
    p->clearCell(2, 12);
    assert (p->isEmpty(2, 12) == true);

    // Test nInARow() - simple case
    p->fillCell(0,0, WHITE);
    p->fillCell(0,1, WHITE);
    assert (p->nInARow(2, WHITE) == 1);
    p->fillCell(0,2, WHITE);
    assert (p->nInARow(2, WHITE) == 0); // verify that 3-in-a-row is not also 2-in-a-row
    assert (p->nInARow(3, WHITE) == 1);
    p->fillCell(0,3, WHITE);
    assert (p->nInARow(4, WHITE) == 1);
    p->fillCell(0,4, WHITE); 
    assert (p->nInARow(5, WHITE) == 1);

    p->reset();
    assert(p->getEmpty().size() == 361);

    // Try more complicated example (and this proved an error)
    p->fillCell(0,0, WHITE);
    p->fillCell(0,1, WHITE);
    p->fillCell(0,2, BLACK);
    p->fillCell(1,2, BLACK);
    p->fillCell(0,3, WHITE);
    p->fillCell(0,4, WHITE);
    assert (p->nInARow(2, WHITE) == 2);
    assert (p->nInARow(2, BLACK) == 1);

    // Test getPossibleCaptures

    assert (p->getPossibleCaptures(BLACK) == 1);
    assert (p->getPossibleCaptures(WHITE) == 0);

    p->fillCell(1, 4, BLACK);
    p->fillCell(2, 5, BLACK);

    assert (p->getPossibleCaptures(BLACK) == 1);
    assert (p->getPossibleCaptures(WHITE) == 1);

    // Test getPossibleCaptures()
    int certD, certT, certQ, certP;
    p->getCertain(certD, certT, certQ, certP, BLACK);

    // Verify chkCapture()

    // Assert that there are two pieces TO capture:
    assert (p->getCell(1, 4)->color != EMPTY);
    assert (p->getCell(2, 5)->color != EMPTY);

    p->playToken(3, 6, WHITE); // This would be a capture.

    // Assert that we've captured them
    assert (p->getCell(1, 4)->color == EMPTY);
    assert (p->getCell(2, 5)->color == EMPTY);

    // Assert that we're scoring them..
    assert (p->whtCaps == 1); 
    assert (p->blkCaps == 0);

    // Assert that there are two pieces to capture (by black):
    assert (p->getCell(0, 3)->color != EMPTY);
    assert (p->getCell(0, 4)->color != EMPTY);
   
    p->playToken(0, 5, BLACK); // This would be a capture.

    // Assert that we've captured them
    assert (p->getCell(0, 3)->color == EMPTY);
    assert (p->getCell(0, 4)->color == EMPTY);

    // Assert that we've scored correctly.
    assert (p->whtCaps == 1);
    assert (p->blkCaps == 1);


    // Reset the board to factory settings
    p->reset();
    assert(p->getEmpty().size() == 361);
   
    // Test that getPossibleCaptures()
    p->fillCell(0,1, BLACK);
    p->fillCell(0,2, BLACK);
    p->fillCell(0,3, WHITE);
    assert(p->getPossibleCaptures(WHITE) == 1);
    assert(p->getPossibleCaptures(BLACK) == 0);
   

    // Test playToken(), unPlayToken();
    p->reset();
    p->playToken(0,1, BLACK);
    p->playToken(0,2, BLACK);
    p->playToken(0,3, WHITE);

    // Check that our captures scores are initialized OK
    assert(p->captures(WHITE) == 0);
    assert(p->captures(BLACK) == 0);

    p->playToken(0,0, WHITE); // CAPTURE!

    assert(p->captures(WHITE) == 1);
    assert(p->captures(BLACK) == 0);

    // Assert missing pieces.
    assert(p->getCell(0,1)->color == EMPTY);
    assert(p->getCell(0,2)->color == EMPTY);

    p->unPlayToken(); // undo last move

    // verify that pieces were properly replaced.
    assert(p->getCell(0,1)->color == BLACK);
    assert(p->getCell(0,2)->color == BLACK);

    // Verify that we re-played the score properly.
    assert(p->captures(WHITE) == 0);
    assert(p->captures(BLACK) == 0);

    /* Test our with spaces stuff */
    p->reset();
    p->playToken(0,0, WHITE);
    p->playToken(0,1, BLACK);
    p->playToken(0,2, BLACK);
    p->playToken(0,3, BLACK);
    p->playToken(0,4, WHITE);

    int D, T, Q, P;
    p->getCertainSpaces(D, T, Q, P, BLACK);
    assert(T == 0);

    p->clearCell(0,0);
    p->getCertainSpaces(D, T, Q, P, BLACK);
    assert(T == 1);
   
    p->clearCell(0,4);
    p->getCertainSpaces(D, T, Q, P, BLACK);
    assert(T == 2);

    cout << p->toString() << endl;


    // If we made it this far, we're golden.
    cout << "Ok." << endl;
    return 0;

}
