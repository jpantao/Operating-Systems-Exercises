/******************************************************************************
 * ASC II 05/06
 * FSO 17/18
 * 
 * copiar ficheiros usando stdio: fopen, fread, fwrite...
 *
 * © Março de 2006 - 2017
 * Vitor Duarte
 * Departamento de Informática
 * Faculdade de Ciências e Tecnologia/Universidade Nova de Lisboa
 *****************************************************************************/

#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>


char *buf; /* dinamico! */


void aborta( char *msg )
/* aborta o programa após afixar uma mensagem no STDERR */
{
  if (msg != NULL) fprintf(stderr, "%s\n", msg);
  exit(1);
}



int main( int argc, char *argv[] )
{
  int nr, nw, bsize;
  FILE *fin, *fout;

  if ( argc != 4 )
	aborta( "Numero errado de argumentos [copia tamanho f1 f2]" );
  
  if ( (bsize = atoi( argv[1] )) <= 0 ) aborta("Tamanho do buffer mal");

  if ( (buf = malloc( bsize )) == NULL ) aborta("Falta de memoria");
  
  fin = fopen( argv[2], "r" );
  if ( fin == NULL ) aborta( "Erro no primeiro ficheiro" );

  fout = fopen( argv[3], "w" );
  if ( fout == NULL ) aborta( "Erro no segundo ficheiro" );

  while( (nr = fread( buf, 1, bsize, fin )) > 0 )    /* A COPIA...*/
  {
    nw = fwrite( buf, 1, nr, fout );
    if ( nw < nr ) aborta( "Erro na escrita" );
  }
  if ( nr < 0 ) aborta( "Erro na leitura" );

  return 0;  /* terminou bem */
}
