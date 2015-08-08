CC=gcc
OBJS= ann.o entrenamiento.o input.o
OBJS_CLASIFICACION= ann.o input.o
LIBS= -lm

entrena-ann: entrena-ann.c $(OBJS)
	$(CC) -o $@ entrena-ann.c $(OBJS) $(LIBS)


clasifica-ann: $(OBJS)
	$(CC) -o $@ clasifica-ann.c $(OBJS) $(LIBS)



input.o: input.c
	$(CC) -c input.c
	
entrenamiento.o: entrenamiento.c
	$(CC) -c entrenamiento.c

ann.o: ann.c $(LIBS)
	$(CC) -c ann.c


clean:
	rm *.o

all: entrena-ann clasifica-ann


#~ prueba: ann.o entrenamiento.o prueba.c
	#~ $(CC) -o $@ ann.o entrenamiento.o prueba.c $(LIBS)
#~ 
#~ clasifica-ann-p: $(OBJS_CLASIFICACION)
	#~ $(CC) -o $@ clasifica-ann.c $(OBJS_CLASIFICACION) $(LIBS)
