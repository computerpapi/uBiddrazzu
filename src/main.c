#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>


/*
RISORSE:
  BITBOARD:
  https://www.youtube.com/watch?v=MzfQ8H16n0M
  https://www.youtube.com/watch?v=4ohJQ9pCkHI
  https://youtu.be/w4FFX_otR-4?si=TPPAsuq10Gsf0chV
*/



// ====================| COSTANTI |====================
typedef uint64_t U64; 
#define C64(n) UINT64_C(n)

// VALORE DEI PEZZI
const int KNIGHT_VALUE = 3, BISHOP_VALUE = 3;
const int ROOK_VALUE = 5;
const int QUEEN_VALUE =  9;

  
// MASCHERE DEI PEZZI
U64 whiteKing    = C64(0x10);
U64 whiteQueen   = C64(0x08);
U64 whiteKnights = C64(0x42);
U64 whiteBishops = C64(0x24);
U64 whiteRooks   = C64(0x81);
U64 whitePawns   = C64(0xFF00);
U64 whitePieces;

U64 blackKing    = C64(0x10) << 56;
U64 blackQueen   = C64(0x08) << 56;
U64 blackKnights = C64(0x42) << 56;
U64 blackBishops = C64(0x24) << 56;
U64 blackRooks   = C64(0x81) << 56;
U64 blackPawns   = C64(0xFF00) << 40;
U64 blackPieces;

U64 occupiedSquares;
U64 emptySquares;
U64 board; 

// REGOLE DEL GIOCO
bool whiteCheckmate = false;
bool blackCheckmate = false;
bool stall = false;

// EVALUATION
int evaluation = 0;



void initBoard() {
  /* In C le variabili globali non possono essere definite usando altre variabili globali, quindi le inizializzo non appena viene avviato il programma */
  U64 blackPieces = whiteKing | whiteQueen | whiteKnights | whiteBishops | whiteRooks | whitePawns;
  U64 blackPieces = blackKing | blackQueen | blackKnights | blackBishops | blackRooks | blackPawns;
  U64 occupiedSquares = whitePieces | blackPieces;
  U64 emptySquares = ~occupiedSquares;
  U64 board = occupiedSquares & emptySquares; 
}



int calculateIndex(int row, int column) {
  /* 
    Normalmente, per accedere alla posizione di un determinato elemento in una matrice bidimensionale (come la scacchiera) 
    usiamo le coordinate X (riga) e Y (colonna) con una sintassi simile a matrice[x][y].
    Per evitare questo e ottenere direttamente l'indice in maniera più veloce si usa una proprietà (Row Major Order) 
    che consente di trasformare la matrice bidimensionale in un vettore (che ha una sola dimensione).

    ROW MAJOR ORDER = riga * numero_di_colonne + colonna
  */

  return row * 8 + column;
}



U64 setCellState(int index) {
  /* Imposta il valore di una determinata cella. Usa shift a sinistra per comparare i bit */

  U64 newSquare = 1ULL << index;
  return (board |= newSquare);
}


bool getCellState(int index) {
  /*
    Restituisce il valore di una determinata cella: true se è occupata, false se è libera.
    Faccio lo shift a sinistra del bit che voglio comparare e poi uso il bitwise END per verificare se i bit sono uguali o no
  */

  U64 square = 1ULL << index;
  return (bool) (board & square) != 0;
}



// ====================| CALCOLO PEZZI BIANCHI E NERI |====================
int calculateWhitePieces() {
  int pawns = __builtin_popcountll(whitePawns);
  int bishops = __builtin_popcountll(whiteBishops) * BISHOP_VALUE;
  int knights = __builtin_popcountll(whiteKnights) * KNIGHT_VALUE;
  int rooks = __builtin_popcountll(whitePawns) * ROOK_VALUE;
  int queen = __builtin_popcountll(whitePawns) * QUEEN_VALUE;
  
  return pawns + bishops + knights + rooks + queen;
}


int calculateBlackPieces() {
  int pawns = __builtin_popcountll(blackPawns);
  int bishops = __builtin_popcountll(blackBishops) * BISHOP_VALUE;
  int knights = __builtin_popcountll(blackKnights) * KNIGHT_VALUE;
  int rooks = __builtin_popcountll(blackPawns) * ROOK_VALUE;
  int queen = __builtin_popcountll(blackQueen) * QUEEN_VALUE;
  
  return pawns + bishops + knights + rooks + queen;
}



// ====================| EVALUAZIONE |====================
void calculateEvaluation() {
  /* 
    Calcola l'evaluazione. Se è maggiore di 0 il bianco ha il vantaggio; se è minore di 0 il nero ha il vantaggio. 
    Se 0 allora sitauzione di parità 
  */

  if (whiteCheckmate) { evaluation = 100; }
  if (blackCheckmate) { evaluation = -100; }
  if (stall) { evaluation = 0; }

  evaluation += calculateWhitePieces();
  evaluation -= calculateBlackPieces();

  // Aggiungere il calcolo relativo dei pezzi sulla scacchiera
}



// ====================| RENDERING SCACCHIERA NEL TERMINALE |====================
void showBoard() {

}



// ====================| MAIN |====================
int main() {
  initBoard();
  calculateEvaluation();
  printf("uBiddrazzu - Chess Engine\n");
  return 0;
}