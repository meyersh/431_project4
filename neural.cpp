#include <iostream>
#include <vector>
#include <fann.h>
#include "assert.h"
#include "neural.hpp"

using namespace std;

int main() {
    PenteNeuralAI *p = new PenteNeuralAI();

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
    // We have WBBBW
    p->getCertainSpaces(D, T, Q, P, BLACK);
    assert(D == 0);
    assert(T == 0);
    assert(Q == 0);
    assert(P == 0);

    // Verify that white is still empty.
    p->getCertainSpaces(D, T, Q, P, WHITE);
    assert(D == 0);
    assert(T == 0);
    assert(Q == 0);
    assert(P == 0);
    
    // Remove white, leaving _BBBW
    p->clearCell(0,0);
    p->getCertainSpaces(D, T, Q, P, BLACK);
    assert(D == 1);
    assert(T == 1);
    assert(Q == 0);
    assert(P == 0);


    // Verify that white is still empty.
    p->getCertainSpaces(D, T, Q, P, WHITE);
    assert(D == 0);
    assert(T == 0);
    assert(Q == 0);
    assert(P == 0);
    
    // Remove other white, leaving _BBB_
    p->clearCell(0,4);
    p->getCertainSpaces(D, T, Q, P, BLACK);
    assert(D == 3); // ?? weird, but consistent, behavior.
    assert(T == 2);
    assert(Q == 0);
    assert(P == 0);

    // Verify that white is still empty.
    p->getCertainSpaces(D, T, Q, P, WHITE);
    assert(D == 0);
    assert(T == 0);
    assert(Q == 0);
    assert(P == 0);

    
    /* With spaces, different colors */
    p->reset();
    p->playToken(0,0, BLACK);
    p->playToken(0,1, WHITE);
    p->playToken(0,2, WHITE);
    p->playToken(0,3, WHITE);
    p->playToken(0,4, BLACK);

    // We have BWWWB
    p->getCertainSpaces(D, T, Q, P, WHITE);
    assert(T == 0);

    // Remove white, leaving _WWWB
    p->clearCell(0,0);
    p->getCertainSpaces(D, T, Q, P, WHITE);
    assert(T == 1);
    
    // Remove other white, leaving _WWW_
    p->clearCell(0,4);
    p->getCertainSpaces(D, T, Q, P, WHITE);
    assert(T == 2);

    /* Test our with spaces stuff for problems on the right edge*/
    p->reset();
    p->playToken(0,18, WHITE);
    p->playToken(0,17, BLACK);
    p->playToken(0,16, BLACK);
    p->playToken(0,15, BLACK);
    p->playToken(0,14, WHITE);

    // We have WBBBW
    p->getCertainSpaces(D, T, Q, P, BLACK);
    assert(D == 0);
    assert(T == 0);
    assert(Q == 0);
    assert(P == 0);


    /* Check getAllBlocks() */
    p->reset();
    p->playToken(0,0, BLACK);
    p->playToken(0,1, WHITE);
    p->playToken(0,2, WHITE);
    p->playToken(0,3, WHITE);
    p->playToken(0,4, BLACK);

    p->getAllBlocks(D, T, Q, P, WHITE);
    assert(D == 3);
    assert(T == 2);
    assert(Q == 0);
    assert(P == 0);

    /* 
     * Test weird state our game is coming up with... 
     */

    p->reset();
    p->playToken(0,0,BLACK);
    p->playToken(0,1,WHITE);
    p->playToken(0,2,BLACK);
    p->playToken(0,3,WHITE);
    p->playToken(0,4,BLACK);
    p->playToken(0,5,WHITE);
    p->playToken(0,6,BLACK);
    p->playToken(0,7,WHITE);
    p->playToken(0,8,BLACK);
    p->playToken(0,9,WHITE);
    p->playToken(0,10,BLACK);
    p->playToken(0,11,WHITE);
    p->playToken(0,12,BLACK);
    p->playToken(0,13,WHITE);
    p->playToken(0,14,BLACK);
    p->playToken(0,15,WHITE);
    p->playToken(0,16,BLACK);
    p->playToken(0,17,WHITE);
    p->playToken(0,18,BLACK);
    p->playToken(1,0,WHITE);
    p->playToken(1,18,WHITE);
    p->playToken(2,0,BLACK);
    p->playToken(2,2,BLACK);
    p->playToken(2,3,WHITE);
    p->playToken(2,4,BLACK);
    p->playToken(2,5,WHITE);
    p->playToken(2,6,BLACK);
    p->playToken(2,7,WHITE);
    p->playToken(2,8,BLACK);
    p->playToken(2,9,WHITE);
    p->playToken(2,10,BLACK);
    p->playToken(2,11,WHITE);
    p->playToken(2,12,BLACK);
    p->playToken(2,13,WHITE);
    p->playToken(2,14,BLACK);
    p->playToken(2,15,WHITE);
    p->playToken(2,16,BLACK);
    p->playToken(2,17,BLACK);
    p->playToken(2,18,BLACK);
    p->playToken(3,0,WHITE);
    p->playToken(3,17,WHITE);
    p->playToken(3,18,WHITE);
    p->playToken(4,0,BLACK);
    p->playToken(4,2,WHITE);
    p->playToken(4,3,BLACK);
    p->playToken(4,4,WHITE);
    p->playToken(4,5,BLACK);
    p->playToken(4,6,WHITE);
    p->playToken(4,7,BLACK);
    p->playToken(4,8,WHITE);
    p->playToken(4,9,BLACK);
    p->playToken(4,10,WHITE);
    p->playToken(4,11,BLACK);
    p->playToken(4,12,WHITE);
    p->playToken(4,13,BLACK);
    p->playToken(4,14,WHITE);
    p->playToken(4,15,BLACK);
    p->playToken(4,16,BLACK);
    p->playToken(4,17,BLACK);
    p->playToken(4,18,BLACK);
    p->playToken(5,0,WHITE);
    p->playToken(5,16,WHITE);
    p->playToken(5,17,WHITE);
    p->playToken(5,18,WHITE);
    p->playToken(6,0,BLACK);
    p->playToken(6,2,WHITE);
    p->playToken(6,3,BLACK);
    p->playToken(6,4,WHITE);
    p->playToken(6,5,BLACK);
    p->playToken(6,6,WHITE);
    p->playToken(6,7,BLACK);
    p->playToken(6,8,WHITE);
    p->playToken(6,9,BLACK);
    p->playToken(6,10,WHITE);
    p->playToken(6,11,BLACK);
    p->playToken(6,12,WHITE);
    p->playToken(6,13,BLACK);
    p->playToken(6,14,WHITE);
    p->playToken(6,15,BLACK);
    p->playToken(6,16,BLACK);
    p->playToken(6,17,BLACK);
    p->playToken(6,18,BLACK);
    p->playToken(7,0,WHITE);
    p->playToken(7,16,WHITE);
    p->playToken(7,17,WHITE);
    p->playToken(7,18,WHITE);
    p->playToken(8,0,BLACK);
    p->playToken(8,2,WHITE);
    p->playToken(8,3,BLACK);
    p->playToken(8,4,WHITE);
    p->playToken(8,5,BLACK);
    p->playToken(8,6,WHITE);
    p->playToken(8,7,BLACK);
    p->playToken(8,9,BLACK);
    p->playToken(8,11,WHITE);
    p->playToken(8,12,BLACK);
    p->playToken(8,13,WHITE);
    p->playToken(8,14,BLACK);
    p->playToken(8,15,BLACK);
    p->playToken(8,16,BLACK);
    p->playToken(8,17,BLACK);
    p->playToken(8,18,BLACK);
    p->playToken(9,0,WHITE);
    p->playToken(9,9,WHITE);
    p->playToken(9,15,WHITE);
    p->playToken(9,16,WHITE);
    p->playToken(9,17,WHITE);
    p->playToken(9,18,WHITE);


    cout << p->toString() << endl;
    cout << p->getFilled(WHITE).size() << " white pieces." << endl;
    cout << p->toState().toString() << endl;

    // If we made it this far, we're golden.
    cout << "Ok." << endl;
    return 0;

}
