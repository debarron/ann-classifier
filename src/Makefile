CC=gcc
OUTPUT=/bin

OBJS= ann.o input.o
OBJS_CLASIFICACION= ann.o input.o
LIBS= -lm

.PHONY: entrena-ann clasifica-ann clean all


entrena-ann: entrena-ann.o $(OBJS)
	$(CC) -o $(OUTPUT)/$@ $@.o $(OBJS) $(LIBS)


clasifica-ann: $(OBJS)
	$(CC) -o $@ clasifica-ann.c $(OBJS) $(LIBS)



input.o: io/input.c
	$(CC) -c io/input.c
	
entrena-ann.o: entrena-ann.c
	$(CC) -c entrena-ann.c

ann.o: ann.c $(LIBS)
	$(CC) -c ann.c


clean:
	rm *.o

all: entrena-ann clasifica-ann


#~ prueba: ann.o entrena-ann.o prueba.c
	#~ $(CC) -o $@ ann.o entrena-ann.o prueba.c $(LIBS)
#~ 
#~ clasifica-ann-p: $(OBJS_CLASIFICACION)
	#~ $(CC) -o $@ clasifica-ann.c $(OBJS_CLASIFICACION) $(LIBS)
