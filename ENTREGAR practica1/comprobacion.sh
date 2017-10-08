clear
if [[ -x "mitar" ]]
then
true
else
	echo "Mitar no es ejecutable"
	exit 1
fi
if [ -d "tmp" ]
then
	rm -rf tmp
fi
mkdir tmp
cd tmp
echo "Hola Mundo!" >>./fich1.txt
head /etc/passwd>>./fich2.txt
if [ -f "/dev/random" ]
then
head -c 1024 /dev/random>>./fich3.dat
else
head -c 1024 /dev/urandom>>./fich3.dat
fi
../mitar -cf fichtar.mtar fich1.txt fich2.txt fich3.dat
if [ -d "out" ]
then
	rm -rf out
fi
mkdir out
cp ./fichtar.mtar ./out/fichtar.mtar
cd out
../../mitar -xf fichtar.mtar
if diff ../fich1.txt ./fich1.txt >/dev/null ; then
true
else
echo "Fich1.txt es diferente"
exit 1
fi
if diff ../fich2.txt ./fich2.txt >/dev/null ; then
true
else
echo "Fich2.txt es diferente"
exit 1
fi
if diff ../fich3.dat ./fich3.dat >/dev/null ; then
true
else
echo "Fich3.dat es diferente"
exit 1
fi
echo "Correcto"
exit 0
