
cp lixo lixo2

echo "fcopia +++++++++++++++++++++++++++++++++++++++++++++++++++++++"
for s in 1 100 256 511 512 513 1023 1024 1025 4095 4096 4097 1048576
do
  echo "================ $s ==============="
   sync
  time fcopia $s lixo lixo2
   sync
  time fcopia $s lixo lixo2
   sync
  time fcopia $s lixo lixo2
   sync

  echo ""
done

echo "copia +++++++++++++++++++++++++++++++++++++++++++++++++++++++"
for s in 1 100 256 511 512 513 1023 1024 1025 4095 4096 4097 1048576
do
  echo "================ $s ==============="
   sync
  time copia $s lixo lixo2
   sync
  time copia $s lixo lixo2
   sync
  time copia $s lixo lixo2
   sync

  echo ""
done

