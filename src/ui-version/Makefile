FLAGS = `pkg-config --cflags gtk+-2.0 gmodule-2.0`
LIBS =  `pkg-config --libs gtk+-2.0 gmodule-2.0`
 


ann-enbp.o: ann-enbp.c
	gcc -c ann-enbp.c -lm

lectura.o: lectura.c
	gcc -c lectura.c



entrena-ann: ann-enbp.o lectura.o entrena-enbp.c
	gcc -o entrena-ann ann-enbp.o lectura.o entrena-enbp.c -lm
	
clasifica-ann: ann-enbp.o lectura.o clasifica-enbp.c
	gcc -o clasifica-ann ann-enbp.o lectura.o clasifica-enbp.c -lm

interfaz-ann: interfaz.c
	gcc -Wall -g interfaz.c -o interfaz-ann $(FLAGS) $(LIBS)



all: entrena-ann clasifica-ann interfaz-ann
	
clean:
	rm *.o
