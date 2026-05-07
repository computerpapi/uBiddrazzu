#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>


/*
RISORSE:
  BITBOARD:
  https://www.youtube.com/watch?v=MzfQ8H16n0M
  https://www.youtube.com/watch?v=4ohJQ9pCkHI
  https://youtu.be/w4FFX_otR-4?si=TPPAsuq10Gsf0chV

  ENCODING MOSSE:
  https://www.youtube.com/watch?v=gyf3mr1LI7A
*/



// SCACCHIERA
/* 
  Rappresentare la scacchiera in questo modo è l'opzione migliore perché è più flessibile e non c'è bisogno di calcolare l'esadecimale di ogni posizione
  ogni volta perché viene eseguita automaticamente dal compilatore facendo lo shift di un ULL intero con la posizione sulla scacchiera.
*/
enum Checkboard {
  a1, b1, c1, d1, e1, f1, g1, h1,
  a2, b2, c2, d2, e2, f2, g2, h2,
  a3, b3, c3, d3, e3, f3, g3, h3,
  a4, b4, c4, d4, e4, f4, g4, h4,
  a5, b5, c5, d5, e5, f5, g5, h5,
  a6, b6, c6, d6, e6, f6, g6, h6,
  a7, b7, c7, d7, e7, f7, g7, h7,
  a8, b8, c8, d8, e8, f8, g8, h8,
};


// GIOCATORI
enum { WHITE, BLACK };



// ====================| COSTANTI |====================
typedef uint64_t U64; 
#define C64(n) UINT64_C(n) // Creo una macro per far in modo che il compilatore tratta gli ULL allo stesso modo su tutte le macchine (standard C) 

// VALORE DEI PEZZI
const int KNIGHT_VALUE = 3, BISHOP_VALUE = 3;
const int ROOK_VALUE = 5;
const int QUEEN_VALUE =  9;

  
// MASCHERE DEI PEZZI
U64 whiteKing = C64(1) << e1;
U64 whiteQueen = C64(1) << d1;
U64 whiteKnights = (C64(1) << b1) | (C64(1) << g1);
U64 whiteBishops = (C64(1) << c1) | (C64(1) << f1);
U64 whiteRooks = (C64(1) << a1) | (C64(1) << h1);
U64 whitePawns = C64(0xFF) << a2;
U64 whitePieces;

U64 blackKing = C64(1) << e8;
U64 blackQueen = C64(1) << d8;
U64 blackKnights = (C64(1) << b8) | (C64(1) << g8);
U64 blackBishops = (C64(1) << c8) | (C64(1) << f8);
U64 blackRooks = (C64(1) << a8) | (C64(1) << h8);
U64 blackPawns = C64(0xFF) << a7;
U64 blackPieces;

// MASCHERE DELLE RIGHE E DELLE COLONNE
U64 rank_1 = C64(0xFF) << a1;
U64 rank_2 = C64(0xFF) << a2;
U64 rank_3 = C64(0xFF) << a3;
U64 rank_4 = C64(0xFF) << a4;
U64 rank_5 = C64(0xFF) << a5;
U64 rank_6 = C64(0xFF) << a6;
U64 rank_7 = C64(0xFF) << a7;
U64 rank_8 = C64(0xFF) << a8;

U64 file_A = C64(0x0101010101010101);
U64 file_B;
U64 file_C;
U64 file_D;
U64 file_E;
U64 file_F;
U64 file_G;
U64 file_H;

U64 emptySquares;
U64 board; 

// REGOLE DEL GIOCO
bool whiteCheckmate = false;
bool blackCheckmate = false;
bool stall = false;

// EVALUATION
int evaluation;



void initBoard() {
  /* In C le variabili globali non possono essere definite usando altre variabili globali, quindi le inizializzo non appena viene avviato il programma */
  U64 whitePieces = whiteKing | whiteQueen | whiteKnights | whiteBishops | whiteRooks | whitePawns;
  U64 blackPieces = blackKing | blackQueen | blackKnights | blackBishops | blackRooks | blackPawns;
  U64 board = whitePieces | blackPieces;
  U64 emptySquares = ~board; 

  U64 file_B = file_A << 1;
  U64 file_C = file_A << 2;
  U64 file_D = file_A << 3;
  U64 file_E = file_A << 4;
  U64 file_F = file_A << 5;
  U64 file_G = file_A << 6;
  U64 file_H = file_A << 7;
}



U64 setCellState(int index) {
  /* Imposta il valore di una determinata cella. Usa shift a sinistra per comparare i bit */

  U64 newSquare = 1ULL << index;
  return (board |= newSquare);
}


bool getSquareState(int index) {
  /*
    Restituisce il valore di una determinata cella: true se è occupata, false se è libera.
    Faccio lo shift a sinistra del bit che voglio comparare e poi uso il bitwise END per verificare se i bit sono uguali o no
  */

  U64 square = 1ULL << index;
  return (bool) (board & square) != 0;
}


char getPieceTypeFromSquare(int square) {
  U64 piece = C64(1) << square; // Creo una bitboard in cui l'indice square è 1 così la posso comparare con le altre bitboard

  if (whiteKing & piece) {
    return 'K';
  } else if (whiteQueen & piece) {
    return 'Q';
  } else if (whiteBishops & piece) {
    return 'B';
  } else if (whiteKnights & piece) {
    return 'N';
  } else if (whiteRooks & piece) {
    return 'R';
  } else if (whitePawns & piece) {
    return 'P';
  } else if (blackKing & piece) {
    return 'k';
  } else if (blackQueen & piece) {
    return 'q';
  } else if (blackBishops & piece) {
    return 'b';
  } else if (blackKnights & piece) {
    return 'n';
  } else if (blackRooks & piece) {
    return 'r';
  } else if (blackPawns & piece) {
    return 'p';
  } else {
    return '.';
  }
}



// ====================| CALCOLO PEZZI BIANCHI E NERI |====================
int calculatePieces(int color) {
  /* __builtin_popcountll() è una funzione integrata di C che calcola il numero di bit impostati a 1 di una particolare sequenza di bit */
  int pawns = __builtin_popcountll((color == WHITE) ? whitePawns : blackPawns);
  int bishops = __builtin_popcountll((color == WHITE) ? whiteBishops : blackBishops) * BISHOP_VALUE;
  int knights = __builtin_popcountll((color == WHITE) ? whiteKnights : blackKnights) * KNIGHT_VALUE;
  int rooks = __builtin_popcountll((color == WHITE) ? whiteRooks : blackRooks) * ROOK_VALUE;
  int queen = __builtin_popcountll((color == WHITE) ? whiteQueen : blackQueen) * QUEEN_VALUE;
  
  return pawns + bishops + knights + rooks + queen;
}



// ====================| EVALUAZIONE |====================
void calculateEvaluation() {
  /* Calcola l'evaluazione. Se è maggiore di 0 il bianco ha il vantaggio; se è minore di 0 il nero ha il vantaggio. Se 0 allora sitauzione di parità */

  if (whiteCheckmate) { evaluation = 100; }
  if (blackCheckmate) { evaluation = -100; }
  if (stall) { evaluation = 0; }

  evaluation = calculatePieces(WHITE) - calculatePieces(BLACK);

  // Aggiungere il calcolo relativo dei pezzi sulla scacchiera
}



// ====================| RENDERING SCACCHIERA NEL TERMINALE |====================
void showBoard() {
  int rank;

  for (int startingSquare = a8; startingSquare >= 0; startingSquare-=8) {
    rank = startingSquare / 8 + 1;
    printf("\n   +---+---+---+---+---+---+---+---+\n %d |", rank);

    for (int square = startingSquare; square <= startingSquare + 7; square++) {
      printf(" %c |", getPieceTypeFromSquare(square));
    }
  }

  printf("\n   +---+---+---+---+---+---+---+---+\n     A   B   C   D   E   F   G   H\n");
}



// ====================| MAIN |====================
int main() {
  initBoard();
  calculateEvaluation();
  printf("\nuBiddrazzu - Chess Engine\n");
  showBoard();
  printf("\n\nEvaluation: %d", evaluation);

  return 0;
}