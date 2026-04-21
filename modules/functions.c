#include <stdio.h>


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