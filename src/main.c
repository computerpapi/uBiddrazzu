#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <immintrin.h>
#include <string.h>


/*
RISORSE:
  BITBOARD:
  https://www.youtube.com/watch?v=MzfQ8H16n0M
  https://www.youtube.com/watch?v=4ohJQ9pCkHI
  https://youtu.be/w4FFX_otR-4?si=TPPAsuq10Gsf0chV

  ENCODING MOSSE:
  https://www.youtube.com/watch?v=gyf3mr1LI7A
  https://www.strydr.net/articles/devlog-0x8
*/


// ENCODING TABLE: https://www.chessprogramming.org/Encoding_Moves



// SCACCHIERA
/* 
  Rappresentare la scacchiera in questo modo è l'opzione migliore perché è più flessibile e non c'è bisogno di calcolare l'esadecimale di ogni posizione
  ogni volta perché viene eseguita automaticamente dal compilatore facendo lo shift di un ULL intero con la posizione sulla scacchiera.
*/
typedef enum {
  a1, b1, c1, d1, e1, f1, g1, h1,
  a2, b2, c2, d2, e2, f2, g2, h2,
  a3, b3, c3, d3, e3, f3, g3, h3,
  a4, b4, c4, d4, e4, f4, g4, h4,
  a5, b5, c5, d5, e5, f5, g5, h5,
  a6, b6, c6, d6, e6, f6, g6, h6,
  a7, b7, c7, d7, e7, f7, g7, h7,
  a8, b8, c8, d8, e8, f8, g8, h8,
} Checkboard;


typedef enum {
  K, Q, R, N, B, P, 
  k, q, r, n, b, p
} Piece;


// GIOCATORI
enum { BLACK, WHITE };



// ====================| COSTANTI |====================
typedef uint64_t U64; 
typedef uint32_t U32;
#define C64(n) UINT64_C(n) // Creo una macro per far in modo che il compilatore tratta gli ULL allo stesso modo su tutte le macchine (standard C) 

// VALORE DEI PEZZI
const int KNIGHT_VALUE = 3, BISHOP_VALUE = 3;
const int ROOK_VALUE = 5;
const int QUEEN_VALUE = 9;

  
// MASCHERE DEI PEZZI
U64 whiteKing = C64(0);
U64 whiteQueen = C64(0);
U64 whiteKnights = C64(0);
U64 whiteBishops = C64(0);
U64 whiteRooks = C64(0);
U64 whitePawns = C64(0);
U64 whitePieces = C64(0);

U64 blackKing = C64(0);
U64 blackQueen = C64(0);
U64 blackKnights = C64(0);
U64 blackBishops = C64(0);
U64 blackRooks = C64(0);
U64 blackPawns = C64(0);
U64 blackPieces = C64(0);

// MASCHERE DELLE RIGHE E DELLE COLONNE
U64 rank_1 = C64(0xFF) << a1;
U64 rank_2 = C64(0xFF) << a2;
U64 rank_3 = C64(0xFF) << a3;
U64 rank_4 = C64(0xFF) << a4;
U64 rank_5 = C64(0xFF) << a5;
U64 rank_6 = C64(0xFF) << a6;
U64 rank_7 = C64(0xFF) << a7;
U64 rank_8 = C64(0xFF) << a8;

// Inizializzo le variabili perché altrimenti avrebbero contenuto random che potrebbe interferire con il funzionamento del motore
U64 file_A = C64(0x0101010101010101);
U64 file_B = C64(0);
U64 file_C = C64(0);
U64 file_D = C64(0);
U64 file_E = C64(0);
U64 file_F = C64(0);
U64 file_G = C64(0);
U64 file_H = C64(0);

U64 emptySquares = C64(0);
U64 board = C64(0); 


// REGOLE DEL GIOCO
bool whiteCheckmate = false;
bool blackCheckmate = false;
bool draw = false;
bool whiteLongCastle = false;
bool whiteShortCastle = false;
bool blackLongCastle = false;
bool blackShortCastle = false;
int enPassantSquare = 0;
int turn = WHITE;

int moveNumber = 0;
int semiMoves = 0;
int evaluation = 0;

// LISTA DELLE MOSSE PSEUDO-LEGALI
U64 knightAttacks[64] = {0};
U64 kingAttacks[64] = {0};
U64 whitePawnsAttacks[64] = {0};
U64 blackPawnsAttacks[64] = {0};
U64 rookAndBishopAttacks[107648] = {0}; // Contiene tutte le mosse di rook e bishop. Invece di creare due array diversi così si risparmia overhead
U64 rookMask [64] = {0};
U64 bishopMask[64] = {0};
U32 rookBase [64] = {0};
U32 bishopBase[64] = {0};



static inline U64 setCellState(int index) {
  /* Imposta il valore di una determinata cella. Usa shift a sinistra per comparare i bit */

  U64 newSquare = C64(1) << index;
  U64 temporaryBoard = board;
  return temporaryBoard | newSquare;
}


static inline bool getSquareState(int index) {
  /*
    Restituisce il valore di una determinata cella: true se è occupata, false se è libera.
    Faccio lo shift a sinistra del bit che voglio comparare e poi uso il bitwise END per verificare se i bit sono uguali o no
  */

  U64 square = C64(1) << index;
  return (bool) (board & square) != 0;
}


static inline char getPieceType(int square) {
  U64 piece = C64(1) << square; // Creo una maschera in cui l'indice square è 1 così la posso comparare con le maschere dei pezzi

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



// ==========================================================================
// ==========| CALCOLO DELLE MOSSE PSEUDO-LEGALI (LOOK-UP TABLES) |==========
// ==========================================================================
void calculateKnightMoves() {
  for (int i = a1; i <= h8; i++) {
    U64 knight = C64(1) << i;
    U64 attackedSquares = C64(0);

    attackedSquares |= (knight << 15) & ~file_H;             // topLeftSquare
    attackedSquares |= (knight << 17) & ~file_A;             // topRightSquare
    attackedSquares |= (knight >> 15) & ~file_A;             // bottomRightSquare
    attackedSquares |= (knight >> 17) & ~file_H;             // bottomLeftSquare
    attackedSquares |= (knight >> 10) & ~(file_G | file_H);  // rightTopSquare 
    attackedSquares |= (knight >> 6)  & ~(file_A | file_B);  // rightBottomSquare 
    attackedSquares |= (knight << 10) & ~(file_A | file_B);  // leftTopSquare 
    attackedSquares |= (knight << 6)  & ~(file_G | file_H);  // leftBottomSquare

    knightAttacks[i] = attackedSquares;
  }
}

void calculateKingMoves() {
  for (int i = a1; i <= h8; i++) {
    U64 king = C64(1) << i;
    U64 attackedSquares = C64(0);

    attackedSquares |= (king << 9) & ~file_A; // topLeftSquare
    attackedSquares |= (king << 8);           // topCenterSquare
    attackedSquares |= (king << 7) & ~file_H; // topRightSquare
    attackedSquares |= (king << 1) & ~file_A; // rightSquare
    attackedSquares |= (king >> 7) & ~file_A; // bottomRightSquare 
    attackedSquares |= (king >> 8);           // bottomCenterSquare 
    attackedSquares |= (king >> 9) & ~file_H; // bottomLeftSquare
    attackedSquares |= (king >> 1) & ~file_H; // leftSquare

    kingAttacks[i] = attackedSquares;
  }
}

void calculateWhitePawnsMoves() {
  for (int i = a2; i <= h7; i++) {
    U64 pawn = C64(1) << i;
    U64 squares = 0;

    squares |= (pawn << 9) & ~file_A; // topLeftSquare   (DIAGONALE)
    squares |= (pawn << 7) & ~file_H; // topRightSquare  (DIAGONALE)

    whitePawnsAttacks[i] = squares;
  }
}

void calculateBlackPawnsMoves() {
  for (int i = h7; i >= h2; i--) {
    U64 pawn = C64(1) << i;
    U64 squares = 0;

    squares |= (pawn >> 7) & ~file_A; // bottomRightSquare (DIAGONALE)
    squares |= (pawn >> 9) & ~file_H; // bottomLeftSquare  (DIAGONALE)

    blackPawnsAttacks[i] = squares;
  }
}

/* 
  Uso la funzione builtin _pext_u64() per la PEXT bitboard. Il vantaggio di usare una PEXT rispetto a una Magic Bitboard o una Black Magic Bitboard
  è le PEXT risparmiano memoria e operazioni CPU rendendo quindi l'algoritmo più veloce e meno pesante. 
  L'unico svantaggio è che non può essere usato su processori Intel precedenti al 2013 e processori AMD precedenti al 2020.
  Ques
*/
void calculateRookMoves() {
  for (int i = a1; i <= h8; i++) {
    
  }
}

void calculateBishopMoves() {
  for (int sq = a1; sq <= h8; sq++) {
    U64 index = _pext_u64(board, bishopMask[sq]); // Estraggo i bit (1) degli ostacoli sulla traiettoria dell'alfiere
    int finalIndex = bishopBase[sq] + index; // Ottengo l'indice della casella dove l'alfiere può spostarsi
  }
}



// =====================================================================
// ====================| ENCODING E DECODING MOSSE |====================
// =====================================================================
/* 
  Uso una static inline function perché sono lo standard nei compilatori C moderni e migliorano le performance.
  Faccio l'encoding a 32bit andando un po' a discapito della memoria (occupa più RAM) ma memorizzo informazioni utili evitando di
  chiamare le fuzioni per ottenere i pezzi ecc, migliorando performance e diminuendo anche il tempo necessario per cercare le mosse.
  La funzione restituisce un numero a 32bit contenente tutte le informazioni necessarie per rappresentare in maniera corretta una mossa.
  Lo shift sembra particolare ma in realtà è fatto apposta perché le informazioni vengono memorizzte ognuna con 6 bit.
*/

static inline U32 encodeMove(U32 startingSquare, U32 targetSquare, U32 piece, U32 promote, U32 capture, U32 doublePawnPush, U32 enpassant, U32 castling) {
  return (startingSquare) | (targetSquare << 6) | (piece << 12) | (promote << 16) | (capture << 20) | (doublePawnPush << 21) | (enpassant << 22) | (castling << 23);
}

static inline U32 decodeStartingSquare(U32 move) { return move & 0x3f;  }
static inline U32 decodeTargetSquare(U32 move) { return (move >> 6) & 0x3f;  }
static inline U32 decodePiece(U32 move) { return (move >> 12) & 0xf; }
static inline U32 decodePromoted(U32 move) { return (move >> 16) & 0xf;  }



// ================================================================
// ====================| CALCOLO VALORE PEZZI |====================
// ================================================================
static inline int calculatePieces(int color) {
  /* __builtin_popcountll() è una funzione integrata di C che calcola il numero di bit impostati a 1 di una particolare sequenza di bit */
  int pawns = __builtin_popcountll((color == WHITE) ? whitePawns : blackPawns);
  int bishops = __builtin_popcountll((color == WHITE) ? whiteBishops : blackBishops) * BISHOP_VALUE;
  int knights = __builtin_popcountll((color == WHITE) ? whiteKnights : blackKnights) * KNIGHT_VALUE;
  int rooks = __builtin_popcountll((color == WHITE) ? whiteRooks : blackRooks) * ROOK_VALUE;
  int queen = __builtin_popcountll((color == WHITE) ? whiteQueen : blackQueen) * QUEEN_VALUE;
  
  return pawns + bishops + knights + rooks + queen;
}


// =======================================================
// ====================| EVALUAZIONE |====================
// =======================================================
void calculateEvaluation() {
  /* Calcola l'evaluazione. Se è maggiore di 0 il bianco ha il vantaggio; se è minore di 0 il nero ha il vantaggio. Se 0 allora sitauzione di parità */

  evaluation = calculatePieces(WHITE) - calculatePieces(BLACK);

  if (whiteCheckmate) { evaluation = 10; }
  if (blackCheckmate) { evaluation = -10; }
  if (draw) { evaluation = 0; }

  // Aggiungere il calcolo relativo dei pezzi sulla scacchiera
}


// ==============================================================================
// ====================| RENDERING SCACCHIERA NEL TERMINALE |====================
// ==============================================================================
void showBoard() {
  int rank;

  for (int startingSquare = a8; startingSquare >= 0; startingSquare-=8) {
    rank = startingSquare / 8 + 1;
    printf("\n   +---+---+---+---+---+---+---+---+\n %d |", rank);

    for (int square = startingSquare; square <= startingSquare + 7; square++) {
      printf(" %c |", getPieceType(square));
    }
  }

  printf("\n   +---+---+---+---+---+---+---+---+\n     A   B   C   D   E   F   G   H\n");
  printf("\nMoves                   : %d", moveNumber);
  printf("\nSemimoves               : %d", semiMoves);
  printf("\nTurn      (1 = W; 0 = B): %d", turn);
  printf("\nWhite Long Castle  (WLC): %d", whiteLongCastle);
  printf("\nWhite Short Castle (WSC): %d", whiteShortCastle);
  printf("\nBlack Long Castle  (BLC): %d", blackLongCastle);
  printf("\nBlack Short Castle (BSC): %d\n\n", blackShortCastle);
}


// ============================================================
// ====================| INIZIALIZZAZIONE |====================
// ============================================================
void initEngine() {
  /* In C le variabili globali non possono essere definite usando altre variabili globali, quindi le inizializzo non appena viene avviato il programma */
  whitePieces = whiteKing | whiteQueen | whiteKnights | whiteBishops | whiteRooks | whitePawns;
  blackPieces = blackKing | blackQueen | blackKnights | blackBishops | blackRooks | blackPawns;
  board = whitePieces | blackPieces;
  emptySquares = ~board; 

  file_B = file_A << 1;
  file_C = file_A << 2;
  file_D = file_A << 3;
  file_E = file_A << 4;
  file_F = file_A << 5;
  file_G = file_A << 6;
  file_H = file_A << 7;
}


// ===============================================================
// ====================| IMPORT & EXPORT FEN |====================
// ===============================================================
void clearPieces() {
  // Resetta le maschere dei pezzi

  whiteKing = C64(0);
  whiteQueen = C64(0);
  whiteKnights = C64(0);
  whiteBishops = C64(0);
  whiteRooks = C64(0);
  whitePawns = C64(0);

  blackKing = C64(0);
  blackQueen = C64(0);
  blackKnights = C64(0);
  blackBishops = C64(0);
  blackRooks = C64(0);
  blackPawns = C64(0);
}



void parseBoard(char board[]) {
  int squareNumber;
  char element;
  int rank = 7;
  int file = 0;
  U64 pieceMask;
  char *pointer = board;

  while (*pointer != '\0') {
    element = *pointer; // Ottengo l'elemento dal suo indirizzo
    pieceMask = C64(1);
    squareNumber = rank * 8 + file;

    switch (element) {
      case 'K': whiteKing = pieceMask << squareNumber; file++; break;
      case 'Q': whiteQueen = pieceMask << squareNumber; file++; break;
      case 'R': whiteRooks |= pieceMask << squareNumber; file++; break;
      case 'N': whiteKnights |= pieceMask << squareNumber; file++; break;
      case 'B': whiteBishops |= pieceMask << squareNumber; file++; break;
      case 'P': whitePawns |= pieceMask << squareNumber; file++; break;
      case 'k': blackKing |= pieceMask << squareNumber; file++; break;
      case 'q': blackQueen |= pieceMask << squareNumber; file++; break;
      case 'r': blackRooks |= pieceMask << squareNumber; file++; break;
      case 'n': blackKnights |= pieceMask << squareNumber; file++; break;
      case 'b': blackBishops |= pieceMask << squareNumber; file++; break;
      case 'p': blackPawns |= pieceMask << squareNumber; file++; break;

      case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': file += (element - '0'); break;

      case '/': rank -= 1; file = 0; break;
    }

    pointer++;
  }
}



void parseCastlingRights(char rights[]) {
  /*
    Ci sono 16 possibili combinazioni di arrocco nella notazione FEN.
    Tutte queste combinazioni usano K, Q, k e q. A me interessa soltanto se sono presenti all'interno della stringa, non l'ordine in cui sono messi.
    (strchr(rihgts, LETTERA) != NULL) non fa che restituire un booleano se la lettera è presente nei diritti di arrocco.
    Sostanzialmente da 16 casi siamo passati a 4 casi. Questo è di gran lunga più efficiente.
  */

  whiteShortCastle = (strchr(rights, 'K') != NULL);
  whiteLongCastle = (strchr(rights, 'Q') != NULL);
  blackShortCastle = (strchr(rights, 'k') != NULL);
  blackLongCastle = (strchr(rights, 'q') != NULL);
}



void importFen(char fen[]) {
  // La funziona dà per scontato che il FEN sia corretto

  clearPieces();

  char *context = NULL;
  char *delimitator = " ";
  char *token = strtok_s(fen, delimitator, &context);

  int i = 0;

  while (token != NULL) {
    char element = *token; // Ottengo l'elemento dal suo indirizzo

    switch (i) {
      case 0: parseBoard(token); break;
      case 1: turn = (element == 'w') ? WHITE : BLACK; break;
      case 2: parseCastlingRights(token); break;
      case 3:    
        if (element != '-') {
          char column = element - 'a';
          int row = *(token + 1) - '0' - 1;
          enPassantSquare = row * 8 + column;
        } else {
          enPassantSquare = -1;
        }
        break;
      case 4: semiMoves = atoi(token); break; // Converte il numero da stringa (ASCII) in intero 
      case 5: moveNumber = atoi(token); break;
    }

    token = strtok_s(NULL, " ", &context);
    i++;
  }
}



void exportFen() {

}



void initPrecalculatedMoves() {
  calculateKnightMoves();
  calculateKingMoves();
  calculateWhitePawnsMoves();
  calculateBlackPawnsMoves();
  calculateRookMoves();
  calculateBishopMoves();
}



// ================================================
// ====================| MAIN |====================
// ================================================
int main() {
  printf("\nuBiddrazzu (uB) Chess Engine - Realized by @computerpapi\n");

  char position[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
  // "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
  // "r1bqkb1r/pppp1ppp/2n2n2/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4"
  // "r1bqk2r/ppppbppp/3n4/4R3/8/8/PPPP1PPP/RNBQ1BK1 b kq - 0 8"
  // "r1bq1rk1/ppppbppp/3n4/4R3/8/8/PPPP1PPP/RNBQ1BK1 w - - 1 9"
  // "8/8/8/3Q4/5P2/p1k4B/5KP1/8 w - - 1 61"

  importFen(position);
  initEngine();
  initPrecalculatedMoves();
  calculateEvaluation();
  showBoard();

  return 0;
}