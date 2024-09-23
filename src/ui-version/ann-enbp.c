#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>


#include "ann-enbp.h"

#define F_SIGMOID(x) (float)(1/ (1 + expf(-x)))
#define RANDOMIZE	 ((float)rand() / (float)RAND_MAX)


//#define DEBUG 1

/* Red para el entrenamiento */
float ***pesos_nuevos = NULL;
float **bias_nuevos = NULL;
	
void ann_imprime_red(ann *red){
	int i,j,k, nodos_anterior;
	float **pesos;
	
	fprintf(stdout, "## Info: Capas de la red: %d\n", red->n_capas);
	
	nodos_anterior = red->entrada->n_nodos;
	for(i = 1; i < red->n_capas; i++){
		fprintf(stdout, "## Capa %d, Nodos %d\n", i, red->capas[i-1].n_nodos);
		
		pesos = red->capas[i-1].pesos;
		
		fprintf(stdout, "## PESOS:\n");
		for(j = 0; j < red->capas[i-1].n_nodos; j++){
			
			for(k = 0; k < nodos_anterior; k++)
				fprintf(stdout, "%f ", pesos[j][k]);
						
			fprintf(stdout, "\n");
		}
		
		
		fprintf(stdout, "## SALIDAS:\n");
		for(j = 0; j < red->capas[i-1].n_nodos; j++)
			fprintf(stdout, "%f ", red->capas[i-1].salidas[j]);		
		
		fprintf(stdout, "\n");
		
		nodos_anterior = red->capas[i-1].n_nodos;
	}
}

void error_mem(){
	fprintf(stderr, "No hay suficiente memoria para crear la red\n");
	exit(1);
}

void ann_calcular_salida(ann *red){
	int i,j,k, nodos_entrada, nodos_salida;
	float *entradas;
	float **pesos;
	float *salidas;
	float *bias;
	
	float salida_i;
	
	/* Establecer las entradas para el primer ciclo */
	entradas = red->entrada->salidas;
	nodos_entrada = red->entrada->n_nodos;
		
	for(k = 0; k < red->n_capas-1; k++){			
		/* Datos necesarios para el cálculo de los pesos */
		pesos = red->capas[k].pesos;
		salidas = red->capas[k].salidas;
		nodos_salida = red->capas[k].n_nodos;
		bias = red->capas[k].bias;
					
		/*Calcular la salida de la capa #indice_capa */
		for(i = 0; i < nodos_salida; i++){
			
			salida_i = red->entrada->x0 * bias[i];
			for(j = 0; j < nodos_entrada; j++)
				salida_i += pesos[i][j] * entradas[j];		
			
			salidas[i] = F_SIGMOID(salida_i);
			
			#ifdef DEBUG
			fprintf(stdout, "Salida %d %d %f\n", k, i, salidas[i]);
			#endif
		}
		
		entradas = red->capas[k].salidas;
		nodos_entrada = red->capas[k].n_nodos;
	}	
}

int ann_identicos(float *a, float *b, int n){
	int identicos = 1;
	int i;
	
	char buff1[10];
	char buff2[10];
	
	for(i = 0; i < n && identicos; i++){
		memset(buff1, '\0', 10);
		memset(buff2, '\0', 10);
		
		sprintf(buff1, "%f", a[i]);
		sprintf(buff2, "%f", b[i]);
		
		if(strcmp(buff1, buff2) != 0) identicos = 0;
	}
	
	return identicos;
}

int ann_escribir_red(ann *red, char *archivo){
	int i,j,k, r, total;
	int *capas;
	FILE *archivo_red;
	float **pesos;
	
	archivo_red = fopen(archivo, "w");
	
	capas = (int *) malloc(sizeof(int) * red->n_capas);
	capas[0] = red->entrada->n_nodos;
	for(i = 1; i < red->n_capas; i++)
		capas[i] = red->capas[i-1].n_nodos;
	
	/* Escribe la cantidad de capas y la longitud 
	 * de nodos de cada una */
	fwrite(&red->n_capas, sizeof(int), 1, archivo_red);
	fwrite(capas, sizeof(int), red->n_capas, archivo_red);
	
	/* Escribe las matrices con los pesos */
	for(i = 1; i < red->n_capas; i++){
		pesos = red->capas[i-1].pesos;
		
		for(j = 0; j < capas[i]; j++)
			fwrite(pesos[j], sizeof(float), capas[i-1], archivo_red);
		
	}
		
	fclose(archivo_red);
	return 1;
}

ann *ann_leer_red(FILE *archivo_red){
	ann *red = NULL;
	int *capas = NULL;
	int n_capas, r, total;
	float **pesos;
	
	int i,j,k;
	
	/* Lee la cantidad de capas */
	fread(&n_capas, sizeof(int), 1, archivo_red);
	
	/* Vector para crear la red */
	capas = (int *) malloc(sizeof(int) * n_capas);
	if(capas == NULL) error_mem();
	fread(capas, sizeof(int), n_capas, archivo_red);
	
	/* Crea la red con el vector*/
	red = ann_crear_red(capas, n_capas);
	
	/* Inicializa los pesos con el archivo */
	for(i = 1; i < n_capas; i++){
		pesos = red->capas[i-1].pesos;
		
		for(j = 0; j < capas[i]; j++)
			fread(pesos[j], sizeof(float), capas[i-1], archivo_red);
			
	}
	
	return red;
}

ann *ann_crear_red(int *capas, int n){
	int i,j;
	ann *red = NULL;
	float **pesos_i = NULL;
	
	red = (ann *) malloc(sizeof(ann));
	if(red == NULL) error_mem();
	
	red->n_capas = n;
	red->entrada = (capa_entrada *) malloc(sizeof(capa_entrada));
	if(red->entrada == NULL) error_mem();
	
	red->entrada->n_nodos = capas[0];
	red->entrada->salidas = (float *) malloc(sizeof(float) * capas[0]);
	if(red->entrada->salidas == NULL) error_mem();
	
	red->capas = (capa_oculta *) malloc(sizeof(capa_oculta) * n-1);
	if(red->capas == NULL) error_mem();
	
	for(i = 1; i < n; i++){
		red->capas[i-1].n_nodos = capas[i];
		
		red->capas[i-1].salidas = (float *) malloc(sizeof(float) * capas[i]);
		if(red->capas[i-1].salidas == NULL) error_mem();
		
		red->capas[i-1].bias = (float *) malloc(sizeof(float) * capas[i]);
		if(red->capas[i-1].bias == NULL) error_mem();
		
		pesos_i = (float **) malloc(sizeof(float *) * capas[i]);
		if(pesos_i == NULL) error_mem();
		
		for(j = 0; j < capas[i]; j++){
			pesos_i[j] = (float *) malloc(sizeof(float) * capas[i-1]);
			if(pesos_i[j] == NULL) error_mem();
		}
		
		red->capas[i-1].pesos = pesos_i;
	}
	
	return red;
}

void ann_pesos_aleatorios(ann *red){
	int i,j,k, nodos_anterior;
	float **pesos;
	float *bias;
	
	srand(time(NULL));
	
	red->entrada->x0 = 1;
	nodos_anterior = red->entrada->n_nodos;
	for(i = 1; i < red->n_capas; i++){
		pesos = red->capas[i-1].pesos;
		bias = red->capas[i-1].bias;
		
		for(j = 0; j < red->capas[i-1].n_nodos; j++){
			red->capas[i-1].bias[j] = RANDOMIZE;		
			
			for(k = 0; k < nodos_anterior; k++)
				pesos[j][k] = RANDOMIZE;
		}
		
		nodos_anterior = red->capas[i-1].n_nodos;
	}
}




/* Mejorado! 3/Mazo/2012
/* Algoritmo "Backpropagation" */
void ann_bp_en(float *lr, ann *red, float *patron){
	
	/* Utiliza dos variables globales para asignar
	 * memoria y no utilizar más malloc durante el
	 * entrenamiento. Las variables son:
	 * ## float ***pesos_nuevos
	 * ## float **bias_nuevos 
	 */
	
	float learning_rate = *lr;
	float *r_ant_aux, *r_anteriores;
	float r_i, sum, delta_i;
	int i,j,k;
		
	float *salidas;
	capa_oculta *capas = red->capas;
	int nodos_anteriores;
	
	/* Asigna la memoria solo cuando es necesario */
	if(pesos_nuevos == NULL){
		
		pesos_nuevos = (float ***) malloc(sizeof(float **) * red->n_capas-1);
		bias_nuevos = (float **) malloc(sizeof(float *) * red->n_capas - 1);
		
		for(i = 0; i < red->n_capas-1; i++){
			bias_nuevos[i] = (float *) malloc(sizeof(float) * capas[i].n_nodos);
			pesos_nuevos[i] = (float **) malloc(sizeof(float *) * capas[i].n_nodos);
			
			if(i == 0) nodos_anteriores = red->entrada->n_nodos;
			else nodos_anteriores = capas[i-1].n_nodos;
				
			for(j = 0; j < capas[i].n_nodos; j++)
				pesos_nuevos[i][j] = (float *) malloc(sizeof(float) * nodos_anteriores);
			
		}
	}
		
	#ifdef DEBUG
		fprintf(stdout, "## Informe.\n");
		fprintf(stdout, "## Red: Learning Rate %f\n", learning_rate);
		fprintf(stdout, "## Red: Capas[ %d ]\n", red->n_capas);
		
		fprintf(stdout, "## (%d, %d) ", 1, red->entrada->n_nodos);
		for(j = 0; j < red->n_capas-1; j++)
			fprintf(stdout, "(%d, %d)", j+2, capas[j].n_nodos);
		fprintf(stdout, "\n");
		
		//getchar();
	#endif
	
	
	/* El algoritmo ajusta los pesos de la capa i+1 
	 * baias de la capa i es actualizado */
	for(i = red->n_capas-2; i >= 0; i--){
		
		r_ant_aux = (float *) malloc(sizeof(float) * capas[i].n_nodos);
		salidas = capas[i].salidas;
		
		#ifdef DEBUG
			fprintf(stdout, "## Informe.\n");
			fprintf(stdout, "## Red: Pesos nuevos (%d) %s\n", i, ((pesos_nuevos[i] == NULL) ? "Nulos" : "Asignados"));
			fprintf(stdout, "## Red: Bias (%d) %s\n", i, ((bias_nuevos[i] == NULL) ? "Nulos" : "Asignados"));
			fprintf(stdout, "## Red: Sigmas (%d) %s\n", i, ((r_ant_aux == NULL) ? "Nulos" : "Asignados"));
					
			//getchar();
		#endif
				
		for(j = 0; j < capas[i].n_nodos; j++){
			
			/* Si es capa de salida */
			if(i == red->n_capas-2){
				r_i = salidas[j] * (1 - salidas[j]) * (patron[j] - salidas[j]);
			
				delta_i = learning_rate * r_i * red->entrada->x0;
				bias_nuevos[i][j] = capas[i].bias[j] + delta_i;	
				
							
			}
			else{ 
				/* Si no es nodo de salida, calcula la responsabilidad
				 * de acuerdo con las capas siguientes */
				 
				sum = 0.0;
				for(k = 0; k < capas[i+1].n_nodos; k++)
					sum += capas[i+1].pesos[k][j] * r_anteriores[k];
			
				r_i = salidas[j] * (1 - salidas[j]) * sum;
				
				/* Solo si estoy en el último caso */
				//if(i == 0) salidas = red->entrada->salidas;
				
				/* Responsabilidad por salida */
				for(k = 0; k < capas[i+1].n_nodos; k++){
					delta_i = learning_rate * r_anteriores[k] * salidas[j];
					pesos_nuevos[i+1][k][j] = capas[i+1].pesos[k][j] + delta_i;
				}
				
				#ifdef DEBUG
					fprintf(stdout, "## Informe.\n");
					fprintf(stdout, "## r_i %f sum %f r_anterior %f\n", r_i, sum, r_anteriores[j]);
				#endif				
			}
			
			r_ant_aux[j] = r_i;
			
			#ifdef DEBUG
				fprintf(stdout, "## Informe.\n");
				fprintf(stdout, "## Red: Responsabilidad %d %d %f\n", i,j, r_i);
				fprintf(stdout, "## Red: Constante %d %d %f\n", i,j, c);
				fprintf(stdout, "## Red: %d %d LR %f \n", i,j, learning_rate);
										
				//getchar();
			#endif	
		}
		//if(r_anteriores != NULL) free(r_anteriores);
		r_anteriores = r_ant_aux;
	}
	
	
	salidas = red->entrada->salidas;
	for(j = 0; j < red->capas[0].n_nodos; j++){
		
		for(i = 0; i < red->entrada->n_nodos; i++){
			delta_i = learning_rate * r_anteriores[j] * salidas[i];
			pesos_nuevos[0][j][i] = capas[0].pesos[j][i] + delta_i;
		}
	}
	
	pesos2red(red, pesos_nuevos, bias_nuevos);
	free(r_ant_aux);
}

void pesos2red(ann *red, float ***pesos, float **bias){
	int i, j, k;
	
	for(i = 1; i < red->n_capas; i++){
		if(i == 1) k = red->entrada->n_nodos;
		else red->capas[i-2].n_nodos; /* Nodos anteriores */
		
		memcpy(red->capas[i-1].bias, bias[i-1], sizeof(float) * red->capas[i-1].n_nodos);
		
		for(j = 0; j < red->capas[i-1].n_nodos; j++)
			memcpy(red->capas[i-1].pesos[j], pesos[i-1][j], sizeof(float) * k);
			
	}
}
