SHELL=/bin/bash
CC=gcc
CFLAGS=-Wall -Wextra -pedantic -g
VFLAGS=--leak-check=full --show-leak-kinds=all --track-origins=yes -s
.PHONY: clean run prepfile

default: bibserver bibclient

all: bibserver bibclient

prepfile:
	chmod +x ./fileCleaner.sh &
	./fileCleaner.sh 1> /dev/null


test: bibserver bibclient
	./bibserver bb1 ./bibData/bib1.txt 4 &
	./bibserver bb2 ./bibData/bib2.txt 2 &
	./bibserver bb3 ./bibData/bib3.txt 3 &
	./bibserver bb4 ./bibData/bib4.txt 2 &
	./bibserver bb5 ./bibData/bib5.txt 3 &
	sleep 1
	./bibclient --autore="di ciccio, antonio" --titolo="manuale di architettura pisana" --scaffale="a.west.2" --editore="palestro" -p &
	./bibclient --titolo="Parallel Computer Architecture" &
	./bibclient --autore="Pike, Rob Kernighan" --titolo="Programmazione nella Pratica" -p &
	./bibclient --titolo="La soledad del manager" -p &
	./bibclient --titolo="Hijacking the World - The dark side of Microsoft" &
	sleep 1
	./bibclient --luogo_pubblicazione="barcelona" -p &
	./bibclient --nota="copia del vescovo" &
	./bibclient --autore="Aho" &
	./bibclient --editore="The Turing Digital archive" --anno="1951" --luogo_pubblicazione="http://www.turingarchive.org" -p &
	./bibclient --autore="Dick, Philip K." &
	sleep 1
	./bibclient --editore="CRC Press, Taylor and Francis Group" -p &
	./bibclient --nota="nota: Copia M. Fillo" --collocazione="QA76 58" &
	./bibclient --collocazione="XXdfg" &
	./bibclient --editore="The Turing Digital archive" --anno="1951" --luogo_pubblicazione="http://www.turingarchive.org" -p &
	./bibclient --autore="Lam, Monica S." &
	sleep 1
	./bibclient --autore="Luccio, Fabrizio" &
	./bibclient --titolo="The Design and Evolution of C++" --luogo_pubblicazione="Reading MA" -p &
	./bibclient --collocazione="df.563" &
	./bibclient --editore="The Turing Digital archive" --anno="1951" --luogo_pubblicazione="http://www.turingarchive.org" &
	./bibclient --nota="On line publishing" -p &
	sleep 1
	./bibclient --descrizione_fisica="359 p., softcov, 13 cm" -p &
	./bibclient --nota="Chapman & Hall/CRC Applied Algorithms and Data Structures series" &
	./bibclient --autore="Lucentini, Franco" -p &
	./bibclient --luogo_pubblicazione="Upper Saddle River, NY" -p &
	./bibclient --collocazione="Q446 58" --descrizione_fisica="239 + xi pp." &
	sleep 1
	./bibclient --luogo_pubblicazione="antartide" -p &
	./bibclient --nota="Edizione Francese" -p &
	./bibclient --autore="Singh, Jaswinder Pal Culler, David E. Gupta,Anoop" &
	./bibclient --editore="The Turing Digital archive" --anno="1951" &
	./bibclient --luogo_pubblicazione="Boston" &
	sleep 1
	./bibclient --descrizione_fisica="294 p., softcov, 13 cm" -p &
	./bibclient --nota="Chapman & Hall/CRC Applied Algorithms and Data Structures series" -p &
	./bibclient --autore="Jeffrey D." -p &
	./bibclient --luogo_pubblicazione="Reading MA" &
	./bibclient --collocazione="molto in alto" --descrizione_fisica="239 + xi pp." &
	sleep 1
	./bibclient --descrizione_fisica="813 p., softcov, 13 cm" -p &
	./bibclient --nota="Edizione Francese" -p &
	./bibclient --collocazione="WW.54" &
	./bibclient --editore="Prentice Hall PTR" --anno="1951" &
	./bibclient --luogo_pubblicazione="Ragusa" &
	sleep 5
	pkill bibserver --signal 2
	sleep 10
	chmod +x ./bibaccess.sh
	./bibaccess.sh --query bb1.log bb2.log bb3.log bb4.log bb5.log
	sleep 1
	./bibaccess.sh --loan bb1.log bb2.log bb3.log bb4.log bb5.log

bibserver: bibserver.o ./lib/queue.o ./lib/linkedList.o ./lib/book.o ./lib/util.o
	$(CC) $(CFLAGS) $^ -o $@ -pthread

bibclient: bibclient.o ./lib/linkedList.o ./lib/util.o
	$(CC) $(CFLAGS) $^ -o $@ -pthread

bibserver.o: bibserver.c ./lib/linkedList.h ./lib/queue.h ./lib/util.h
queue.o: ./lib/queue.c ./lib/queue.h ./lib/util.h
linkedlist.o: ./lib/linkedList.c ./lib/linkedList.h
book.o: ./lib/book.c ./lib/book.h
bibclient.o: bibclient.c ./lib/linkedList.h
util.o: ./lib/util.c ./lib/util.h

cleanrun:
	rm ./conf/bib.conf && touch ./conf/bib.conf && ./bibserver bb1 ./bibData/bib1.txt 2

runsingle: bibserver
	./bibserver bb1 ./bibData/bib1.txt 4 &
	sleep 1
	./bibclient --autore="di ciccio, antonio" --titolo="manuale di architettura pisana" -p &
	sleep 3
	pkill bibserver --signal 2



runserver: bibserver
	./bibserver bb1 ./bibData/bib1.txt 2

runserver1: bibserver
	./bibserver bb2 ./bibData/bib2.txt 3

runserver2: bibserver
	./bibserver bb3 ./bibData/bib3.txt 4

runclient: bibclient
	./bibclient --autore="Luccio, Fabrizio"

runclient1: bibclient
	./bibclient --titolo="manuale di architettura pisana" -p

clean:
	rm -f *.o bibserver ./lib/*.o
	rm -f *.o bibclient
	rm ./conf/addr.conf && touch ./conf/addr.conf
	clear

stop:
	pkill bibserver --signal 2

hardstop: 
	pkill bibserver --signal 9

valgrindserver: bibserver
	valgrind $(VFLAGS) --log-file=./log/bibserver_valgrind.log ./bibserver bb1 ./bibData/bib1.txt 3

valgrindclient: bibclient
	valgrind $(VFLAGS) --log-file=./log/bibclient_valgrind.log ./bibclient --autore="ciccio" --titolo="labestia" --editore="io" --anno="1983" --nota="ciaoo" -p
