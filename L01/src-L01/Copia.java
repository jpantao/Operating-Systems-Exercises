/******************************************************************************
 * FSO 17/18
 * 
 * copiar ficheiros usando File*Stream
 *
 * © 2017
 * Vitor Duarte
 * Departamento de Informática
 * Faculdade de Ciências e Tecnologia/Universidade Nova de Lisboa
 *****************************************************************************/

import java.io.*;

public class Copia {

    public static void main(String[] args) throws Exception {
        if ( args.length != 3 ) {
            System.err.println("Numero de argumentos errado [Copia tamanho fich1 fich2]");
            return;
        }
		
	int bufSz;		
	FileInputStream f;
	FileOutputStream g;
	try {
		bufSz = Integer.parseInt(args[0]);
		f = new FileInputStream(args[1]);
		g = new FileOutputStream(args[2]);
	} catch (Exception e) {
		System.err.println("Erro nos argumentos. "+e);
		return;
	}
		
	try {
		byte[] buff = new byte[bufSz];
		int n;
		///System.out.println("available "+f.available());
		while ( (n=f.read(buff)) > 0 )
		   g.write(buff,0,n);
	} catch (Exception e) {
		System.err.println("Erro na copia. "+e);
		return;
	}
    }
}
