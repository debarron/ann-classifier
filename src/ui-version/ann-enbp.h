#ifndef ANN_H
#define ANN_H 1

#define UMBRAL 0.5

typedef struct{
	int n_nodos;
	float *salidas;
	float x0;
} capa_entrada;


typedef struct{
	int n_nodos;
	
	float **pesos;
	float *salidas;
	float *bias;
} capa_oculta;

typedef struct{
	capa_entrada *entrada;
	capa_oculta *capas;
	
	int n_capas;
} ann;


typedef struct{
	int n_datos;
	float **datos;
	float **patrones;
}info;


void ann_pesos_aleatorios(ann *red);
ann *ann_crear_red(int *capas, int n);
ann *ann_leer_red(FILE *archivo_red);
int ann_escribir_red(ann *red, char *archivo);
void error_mem();
void pesos2red(ann *red, float ***pesos, float **bias);
#endif
