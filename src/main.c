#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>


/*
RISORSE:
  BITBOARD:
  https://www.youtube.com/watch?v=MzfQ8H16n0M
  https://www.youtube.com/watch?v=4ohJQ9pCkHI
*/



// ====================| COSTANTI |====================

/* 
  Per rappresentare la scacchiera uso una Bitboard che consente di fare operazioni più velocemente.
  Non uso una sola Bitboard per rappresentare il gioco, ma più di una. Una Bitboard è la scacchiera in sè, ma
  le altre sono tutte maschere per controllare lo stato e la posizione dei pezzi su più livelli 
*/

typedef uint64_t U64; 
#define C64(n) UINT64_C(n)
#define BOARD_WIDTH 8


U64 board = C64(0000000000000000000000000000000000000000000000000000000000000000); // Scacchiera
  
// Maschere dei pezzi bianchi (posizioni)
U64 whiteKing;
U64 whiteQueen;
U64 whiteKnights;
U64 whiteBishops;
U64 whiteRooks;
U64 whitePawns;

// Maschere dei pezzi neri (posizioni)
U64 blackKing;
U64 blackQueen;
U64 blackKnights;
U64 blackBishops;
U64 blackRooks;
U64 blackPawns;

// REGOLE DEL GIOCO
bool whiteTurn;
bool castlingRights;
bool enPassantSquare;
bool whiteCheckMate;
bool blackCheckMate;
bool stall;

// EVALUATION
int evaluation;

// VALORE DEI PEZZI
#define PAWN_VALUE 1
#define KNIGHT_VALUE 3
#define BISHOP_VALUE 3
#define ROOK_VALUE 5
#define QUEEN_VALUE 9 

// Maschere delle colonne

// Maschere delle righe





int calculateIndex(int row, int column) {
  /* 
    Normalmente, per accedere alla posizione di un determinato elemento in una matrice bidimensionale (come la scacchiera) 
    usiamo le coordinate X (riga) e Y (colonna) con una sintassi simile a matrice[x][y].
    Per evitare questo e ottenere direttamente l'indice (utile nelle bitboards) usiamo una proprietà (Row Major Order) 
    che consente di trasformare la matrice bidimensionale in un vettore (che ha una sola dimensione).

    ROW MAJOR ORDER = riga * numero_di_colonne + colonna
  */

  return row * BOARD_WIDTH + column;
}



U64 setCellState(int index) {
  /*
    Imposta il valore di una determinata cella e restituisce la scacchiera 
  */

  U64 newCell = board << index;
  return (board |= newCell);
}


bool getCellState(int index) {
  /*
    Restituisce il valore di una determinata cella: true se è occupata, false se è libera.
    Faccio lo shift a sinistra del bit che voglio comparare e poi uso il bitwise END per verificare se i bit sono uguali o no
  */

  U64 cell = board << index;

  bool result;
  if (board & cell) {
    result = true;
  } else {
    result = false;
  }

  return result;
}


// ====================| CALCOLO PEZZI BIANCHI E NERI |====================
int calculateWhitePieces() {
  return 0;
}


int calculateBlackPieces() {
  return 0;
}




// ====================| EVALUAZIONE |====================
void calculateEvaluation() {
  /*
    Calcola l'evaluazione  
  */

  if (whiteCheckMate) { evaluation = 1000; }
  if (blackCheckMate) { evaluation = -1000; }
  if (stall) { evaluation = 0; }

  evaluation += calculateWhitePieces();
  evaluation -= calculateBlackPieces();
}



// ====================| MAIN |====================

int main() {
  calculateEvaluation();
  printf("uBiddrazzu - Chess Engine\n");
  printf("\nEvaluation: %d", evaluation);
  return 0;
}