all:
	jsonModeler.sh sampleOne.model
	gcc -g -O0 -o sampleOne sampleOneMain.c sampleOne.c -lprojewski-tools -I/usr/local/include/projewski -ljansson

clean:
	rm -f sampleOne.c sampleOne.h *~ .*~ jsonModel.h sampleOne
