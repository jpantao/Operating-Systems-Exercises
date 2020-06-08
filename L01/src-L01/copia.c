/******************************************************************************
 * ASC II 05/06
 * FSO 17/18
 * 
 * copiar ficheiros usando SO (Posix/Unix): open, read, write...
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
  int fin, fout, nr, nw, bsize;

  if ( argc != 4 )
	aborta( "Numero errado de argumentos [copia tamanho f1 f2]" );
  
  if ( (bsize = atoi( argv[1] )) <= 0 ) aborta("Tamanho do buffer mal");

  if ( (buf = malloc( bsize )) == NULL ) aborta("Falta de memoria");
  
  fin = open( argv[2], O_RDONLY );
  if ( fin < 0 ) aborta( "Erro no primeiro ficheiro" );

  fout = creat( argv[3], 0666 );  
  /* equivalente a open(argv[3], O_CREAT|O_WRONLY|O_TRUNC, 0666 ) */
  if ( fout < 0 ) aborta( "Erro no segundo ficheiro" );

  while( (nr = read( fin, buf, bsize )) > 0 )    /* A COPIA...*/
  {
    nw = write( fout, buf, nr );
    if ( nw < nr ) aborta( "Erro na escrita" );
  }
  if ( nr < 0 ) aborta( "Erro na leitura" );

  return 0;  /* terminou bem */
}
