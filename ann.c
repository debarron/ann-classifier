#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "ann.h"

#define F_SIGMOID(x) (float)(1/ (1 + expf(-x)))
#define RANDOMIZE	 ((float)rand() / (float)RAND_MAX)

#define RED_NUEVA "red.bin"
#define RED_ACTUAL "red-actual.bin"

#define PAA_N 45

float ***pesos_nuevos = NULL;
float **bias_nuevos = NULL;
float *salida = NULL;
float *patron = NULL;

void error_mem(){
	fprintf(stderr, "ANN: No hay suficiente memoria para crear la red\n");
	exit(1);
}

int ann_tam_bytes(ann *red){
	int tam = 0;
	int nodos_anterior = 0;
	int i;
	
	tam = sizeof(ann);
	tam += sizeof(capa_entrada);
	
	nodos_anterior = red->entrada->n_nodos;
	for(i = 0; i < red->n_capas -1; i++){
		/* Salidas y bias */
		tam += (red->capas[i].n_nodos * sizeof(float)) * 2;
		
		/* Pesos */
		tam += sizeof(float) * red->capas[i].n_nodos * nodos_anterior;
		nodos_anterior = red->capas[i].n_nodos;
	}
	
	return tam;
}

void entrenamiento_init_matrices(ann *red){
	int i, j;
	int nodos_anteriores;
	capa_oculta *capas;
	
	pesos_nuevos = (float ***) malloc(sizeof(float **) * red->n_capas-1);
	bias_nuevos = (float **) malloc(sizeof(float *) * red->n_capas - 1);
	capas = red->capas;
	
	for(i = 0; i < red->n_capas-1; i++){
		bias_nuevos[i] = (float *) malloc(sizeof(float) * capas[i].n_nodos);
		pesos_nuevos[i] = (float **) malloc(sizeof(float *) * capas[i].n_nodos);
		
		if(i == 0) nodos_anteriores = red->entrada->n_nodos;
		else nodos_anteriores = capas[i-1].n_nodos;
			
		for(j = 0; j < capas[i].n_nodos; j++)
			pesos_nuevos[i][j] = (float *) malloc(sizeof(float) * nodos_anteriores);
		
	}
}

void entrenamiento_bp(float *lr, ann *red, float *patron){
	float learning_rate = *lr;
	float *r_ant_aux, *r_anteriores;
	float r_i, sum, delta_i;
	int i,j,k;
		
	float *salidas;
	capa_oculta *capas = red->capas;
	int nodos_anteriores;
	
	/* Asigna la memoria solo cuando es necesario */
	if(pesos_nuevos == NULL)
		entrenamiento_init_matrices(red);
	
	/* El algoritmo ajusta los pesos de la capa i+1 
	 * baias de la capa i es actualizado */
	r_anteriores = NULL;
	for(i = red->n_capas-2; i >= 0; i--){
		
		r_ant_aux = (float *) malloc(sizeof(float) * capas[i].n_nodos);
		salidas = capas[i].salidas;
				
		for(j = 0; j < capas[i].n_nodos; j++){
			
			/* Si es capa de salida */
			if(i == red->n_capas-2){
				r_i = salidas[j] * (1 - salidas[j]) * (patron[j] - salidas[j]);
			
			}
			else{ 
				/* Si no es nodo de salida, calcula la responsabilidad
				 * de acuerdo con las capas siguientes */
				sum = 0.0;
				for(k = 0; k < capas[i+1].n_nodos; k++)
					sum += capas[i+1].pesos[k][j] * r_anteriores[k];
			
				r_i = salidas[j] * (1 - salidas[j]) * sum;				
				
				/* Responsabilidad por salida */
				for(k = 0; k < capas[i+1].n_nodos; k++){
					delta_i = learning_rate * r_anteriores[k] * salidas[j];
					pesos_nuevos[i+1][k][j] = capas[i+1].pesos[k][j] + delta_i;
				}			
			}
			
			/* Calculo de bias */
			delta_i = learning_rate * r_i * red->entrada->x0;
			bias_nuevos[i][j] = capas[i].bias[j] + delta_i;
				
			//fprintf(stdout, "Valor del nuevo bias %f\n", bias_nuevos[i][j])		;
				
			r_ant_aux[j] = r_i;
		}
		if(r_anteriores != NULL) free(r_anteriores);
		
		r_anteriores = r_ant_aux;
	}
	
	
	salidas = red->entrada->salidas;
	for(j = 0; j < red->capas[0].n_nodos; j++){
		
		for(i = 0; i < red->entrada->n_nodos; i++){
			delta_i = learning_rate * r_anteriores[j] * salidas[i];
			pesos_nuevos[0][j][i] = capas[0].pesos[j][i] + delta_i;
		}
	}
	
	entrenamiento_pesos_a_red(red, pesos_nuevos, bias_nuevos);
	free(r_ant_aux);
}

void entrenamiento_pesos_a_red(ann *red, float ***pesos, float **bias){
	int i, j, k;
	
	for(i = 1; i < red->n_capas; i++){
		if(i == 1) k = red->entrada->n_nodos;
		else k = red->capas[i-2].n_nodos; /* Nodos anteriores */
		
		memcpy(red->capas[i-1].bias, bias[i-1], sizeof(float) * red->capas[i-1].n_nodos);
		
		for(j = 0; j < red->capas[i-1].n_nodos; j++)
			memcpy(red->capas[i-1].pesos[j], pesos[i-1][j], sizeof(float) * k);
			
	}
}

/* Utiliza la constante de PAA_N como la dimension del vector característico */
void vector_paa(float *v, float *datos, int datos_n){
	int gap, offset, i, k;

	offset = 0;
	gap = datos_n / PAA_N;

	for(i = 0; i < PAA_N; i++){
		for(k = offset; k < offset + gap; k++) v[i] += datos[k];

		v[i] = v[i] / gap;
		offset += gap;
	}
}

/* Requiere de un arreglo con longitud (datos_n / 2) */
void vector_sub(float *v, float *datos, int datos_n){
	int i, f, index;

	f = 0;
	index = 0;
	for(i = 0; i < datos_n; i++){
		if(f == 0){
			v[index++] = datos[i];
			f = 1;
		}
		else f = 1;
	}
}


ann *entrena_x_epocas_sub(ann *red, bd_t *d, float learning_rate, int epocas_max){
	int epocas, n_salida, i, j;
	int error, s;
	
	float *v_sub;
	
	epocas = 0;
	n_salida = red->capas[red->n_capas-2].n_nodos; /* El índice de la capa de salida */
	
	if(salida == NULL){
		salida = (float *) malloc(sizeof(float) * n_salida);
		patron = (float *) malloc(sizeof(float) * n_salida);
	}

	v_sub = (float *) malloc(sizeof(float) * PAA_N);
	
	if(salida == NULL || patron == NULL){
		fprintf(stdout, "Sin memoria\n");
		exit(1);
	}
		
	fprintf(stdout, "#Epoca Error\n");
	for(epocas = 0; epocas < epocas_max; epocas++){
		
		for(i = 0; i < d->n; i++){ /* Entrenamiento */

			/**********************************************************/
			/* USO DE PAA PARA GENERAR EL VECTOR CARACTERISTICO */
			memset(v_sub, 0, sizeof(float) * PAA_N);
			vector_sub(v_sub, d->datos[i], d->d);

			red->entrada->salidas = v_sub;
			ann_calcular_salida(red);
			
			/* Obtener las salidas y redondear con el umbral */
			memcpy(salida, red->capas[red->n_capas-2].salidas, sizeof(float) * n_salida);
			for(j = 0; j < n_salida; j++)
				salida[j] = (salida[j] > UMBRAL) ?  1.0 :  0.0;
			
			/* Obtener el patron */
			memset(patron, 0, sizeof(float) * n_salida);
			patron[d->clases[i] - 1] = 1.0;
			
			/* Preguntar si hubo o no un error */
			error = 0;
			for(j = 0; j < n_salida; j++){
				if(patron[j] != salida[j]){
					error = 1;
					break;
				}
			}
			
			if(error) /* Si hay error se debe entrenar */
				entrenamiento_bp(&learning_rate, red, patron);						
		}
		
		/* Sacar el error por epoca */
		error = 0;
		for(i = 0; i < d->n; i++){

			/**********************************************************/
			/* USO DE PAA PARA GENERAR EL VECTOR CARACTERISTICO */
			memset(v_sub, 0, sizeof(float) * 45);
			vector_sub(v_sub, d->datos[i], d->d);

			red->entrada->salidas = v_sub;
			ann_calcular_salida(red);
			
			/* Obtener el patron */
			memset(patron, 0, sizeof(float) * n_salida);
			patron[d->clases[i] - 1] = 1.0;
			
			/* Obtener las salidas y redondear con el umbral */
			memcpy(salida, red->capas[red->n_capas-2].salidas, sizeof(float) * n_salida);
			for(j = 0; j < n_salida; j++){
				salida[j] = (salida[j] > UMBRAL) ?  1.0 :  0.0;
				
				if(patron[j] != salida[j]){
					error += 1;
					break;
				}
			}
			
		}
		fprintf(stdout, "%d %f\n", epocas, ((float)error / (float)d->n) * 100.0);
	}
			
	free(v_sub);
	
	return red;
}

ann *entrena_x_epocas_paa(ann *red, bd_t *d, float learning_rate, int epocas_max){
	int epocas, n_salida, i, j;
	int error, s;
	
	float *paa;
		
	epocas = 0;
	n_salida = red->capas[red->n_capas-2].n_nodos; /* El índice de la capa de salida */
	
	if(salida == NULL){
		salida = (float *) malloc(sizeof(float) * n_salida);
		patron = (float *) malloc(sizeof(float) * n_salida);
	}

	paa = (float *) malloc(sizeof(float) * PAA_N);
	
	if(salida == NULL || patron == NULL){
		fprintf(stdout, "Sin memoria\n");
		exit(1);
	}
	
		
	fprintf(stdout, "#Epoca Error\n");
	for(epocas = 0; epocas < epocas_max; epocas++){
		
		for(i = 0; i < d->n; i++){ /* Entrenamiento */
			
			/**********************************************************/
			/* Calcular PAA */
			memset(paa, 0, sizeof(float) * PAA_N);
			vector_paa(paa, d->datos[i], d->d); 
			
			red->entrada->salidas = paa;
			ann_calcular_salida(red);
				
			/* Obtener las salidas y redondear con el umbral */
			memcpy(salida, red->capas[red->n_capas-2].salidas, sizeof(float) * n_salida);
			for(j = 0; j < n_salida; j++)
				salida[j] = (salida[j] > UMBRAL) ?  1.0 :  0.0;
			
			/* Obtener el patron */
			memset(patron, 0, sizeof(float) * n_salida);
			patron[d->clases[i] - 1] = 1.0;
			
			/* Preguntar si hubo o no un error */
			error = 0;
			for(j = 0; j < n_salida; j++){
				if(patron[j] != salida[j]){
					error = 1;
					break;
				}
			}

			if(error)
				entrenamiento_bp(&learning_rate, red, patron);						
		}
		
		/* Sacar el error por epoca */
		error = 0;
		for(i = 0; i < d->n; i++){

			/**********************************************************/
			/* Calcular PAA */
			memset(paa, 0, sizeof(float) * PAA_N);
			vector_paa(paa, d->datos[i], d->d);

			red->entrada->salidas = paa;
			ann_calcular_salida(red);
			
			/* Obtener el patron */
			memset(patron, 0, sizeof(float) * n_salida);
			patron[d->clases[i] - 1] = 1.0;
			
			/* Obtener las salidas y redondear con el umbral */
			memcpy(salida, red->capas[red->n_capas-2].salidas, sizeof(float) * n_salida);
			for(j = 0; j < n_salida; j++){
				salida[j] = (salida[j] > UMBRAL) ?  1.0 :  0.0;
				
				if(patron[j] != salida[j]){
					error += 1;
					break;
				}
			}
			
		}
		fprintf(stdout, "%d %f\n", epocas, ((float)error / (float)d->n) * 100.0);
	}
	
	free(paa);
			
	return red;
}

ann *entrena_x_epocas(ann *red, bd_t *d, float learning_rate, int epocas_max){
	int epocas, n_salida, i, j;
	int error, s;
	
	epocas = 0;
	
	n_salida = red->capas[red->n_capas-2].n_nodos; /* El índice de la capa de salida */
	
	if(salida == NULL){
		salida = (float *) malloc(sizeof(float) * n_salida);
		patron = (float *) malloc(sizeof(float) * n_salida);
	}
	
	if(salida == NULL || patron == NULL){
		fprintf(stdout, "Sin memoria\n");
		exit(1);
	}
		
	fprintf(stdout, "#Epoca Error\n");
	for(epocas = 0; epocas < epocas_max; epocas++){
		
		/* Entrenamiento */
		for(i = 0; i < d->n; i++){
			red->entrada->salidas = d->datos[i];
			ann_calcular_salida(red);
			
			/* Obtener las salidas y redondear con el umbral */
			memcpy(salida, red->capas[red->n_capas-2].salidas, sizeof(float) * n_salida);
			for(j = 0; j < n_salida; j++)
				salida[j] = (salida[j] > UMBRAL) ?  1.0 :  0.0;
			
			/* Obtener el patron */
			memset(patron, 0, sizeof(float) * n_salida);
			patron[d->clases[i] - 1] = 1.0;
			
			/* Preguntar si hubo o no un error */
			error = 0;
			for(j = 0; j < n_salida; j++){
				if(patron[j] != salida[j]){
					error = 1;
					break;
				}
			}
			
			if(error)
				entrenamiento_bp(&learning_rate, red, patron);						
		}
		
		/* Sacar el error por epoca */
		error = 0;
		for(i = 0; i < d->n; i++){
			red->entrada->salidas = d->datos[i];
			ann_calcular_salida(red);
			
			/* Obtener el patron */
			memset(patron, 0, sizeof(float) * n_salida);
			patron[d->clases[i] - 1] = 1.0;
			
			/* Obtener las salidas y redondear con el umbral */
			memcpy(salida, red->capas[red->n_capas-2].salidas, sizeof(float) * n_salida);
			for(j = 0; j < n_salida; j++){
				salida[j] = (salida[j] > UMBRAL) ?  1.0 :  0.0;
				
				if(patron[j] != salida[j]){
					error += 1;
					break;
				}
			}
			
		}
		fprintf(stdout, "%d %f\n", epocas, ((float)error / (float)d->n) * 100.0);
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
	red->entrada->x0 = 1.0;
	/*red->entrada->salidas = (float *) malloc(sizeof(float) * capas[0]);
	if(red->entrada->salidas == NULL) error_mem();*/
	
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

void ann_calcular_salida(ann *red){
	int i,j,k, nodos_entrada, nodos_salida;
	float *entradas;
	float **pesos;
	float *salidas_r;
	float *bias;
	
	float salida_i;
	
	/* Establecer las entradas para el primer ciclo */
	entradas = red->entrada->salidas;
	nodos_entrada = red->entrada->n_nodos;
		
	for(k = 0; k < red->n_capas-1; k++){			
		/* Datos necesarios para el cálculo de los pesos */
		pesos = red->capas[k].pesos;
		salidas_r = red->capas[k].salidas;
		nodos_salida = red->capas[k].n_nodos;
		bias = red->capas[k].bias;
					
		/*Calcular la salida de la capa #indice_capa */
		for(i = 0; i < nodos_salida; i++){
			
			salida_i = red->entrada->x0 * bias[i];
			for(j = 0; j < nodos_entrada; j++)
				salida_i += pesos[i][j] * entradas[j];		
				
			salidas_r[i] = F_SIGMOID(salida_i);
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

int ann_escribir_red(char *dir, ann *red, int actual){
	int i,j,k, r, total;
	int *capas;
	float **pesos;
	FILE *archivo_red;
	
	/* Pregunta si es red nueva o se actualizan los pesos */
	archivo_red = fopen(dir, "w");
	
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
		
		fwrite(red->capas[i-1].bias, sizeof(float), capas[i], archivo_red);
	}
	fclose(archivo_red);
	free(capas);
	
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
		
		fread(red->capas[i-1].bias, sizeof(float), capas[i], archivo_red);			
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
