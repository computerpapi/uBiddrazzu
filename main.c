#include <stdio.h>


/*
RISORSE:
  BITBOARD:
  https://www.youtube.com/watch?v=MzfQ8H16n0M
  https://www.youtube.com/watch?v=4ohJQ9pCkHI
  
*/


// DEFINIZIONE DELLE COSTANTI
#define BOARD_WIDTH 8


int calculateIndex(int row, int column) {
  /* 
  Normalmente, per accedere alla posizione di un determinato elemento in una matrice bidimensionale (come la scacchiera) 
  usiamo le coordinate X (riga) e Y (colonna) con una sintassi simile a matrice[x][y].
  Per evitare questo e ottenere direttamente l'indice (utile nelle magic bitboards) usiamo una proprietà (Row Major Order) 
  che ci consente di trasformare la matrice bidimensionale in un vettore (che ha una sola dimensione).

  ROW MAJOR ORDER: riga * numero_di_colonne + colonna
  */
  return row * BOARD_WIDTH + column;
}


int getCellState(long board, int row, int column) {
  /* Restituisce 1 se la cella (ricavata dal bitboard con riga e colonna) è occupata, altrimenti 0 */

}


int main() {
  long board; // Scacchiera
  long white_mask; // Caselle bianche
  long black_mask; // Caselle nere
  long white_position; // Posizione dei pezzi bianchi sulla scacchiera
  long black_position; // Posizione dei pezzi neri sulla scacchiera


  return 0;
}