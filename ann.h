#ifndef ANN_H
#define ANN_H 1

#include "io/input.h"

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



void error_mem();

ann *ann_crear_red(int *capas, int n);
int ann_escribir_red(char *dir, ann *red, int actual);
void ann_pesos_aleatorios(ann *red);
ann *ann_leer_red(FILE *archivo_red);
void ann_libera(ann *red);

void ann_imprime_red(ann *red);

void vector_sub(float *v, float *datos, int datos_n);
void vector_paa(float *v, float *datos, int datos_n);

ann *entrena_x_epocas(ann *red, bd_t *d, float learning_rate, int epocas_max);
ann *entrena_x_epocas_paa(ann *red, bd_t *d, float learning_rate, int epocas_max);
ann *entrena_x_epocas_sub(ann *red, bd_t *d, float learning_rate, int epocas_max);

void ann_calcular_salida(ann *red);

int ann_identicos(float *a, float *b, int n);

int ann_tam_bytes(ann *red);



void entrenamiento_init_matrices(ann *red);

void entrenamiento_bp(float *lr, ann *red, float *patron);

void entrenamiento_pesos_a_red(ann *red, float ***pesos, float **bias);

//~ ann *entrena_entrena_red(ann *red, info *d, float learning_rate);

#endif
