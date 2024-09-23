#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "io/input.h"
#include "ann.h"

#define CLASE_LEN 15
#define PATRON_LEN 90 

void ayuda(char *p){
	fprintf(stdout, "## CLASIFICACION DE LA RED NEURONAL\n");
	fprintf(stdout, "%s RED DATOS MODO\n", p);
	fprintf(stdout, "\t RED archivo de la red neuronal.\n", );
	fprintf(stdout, "\t DATOS archivo de datos de prueba.\n");
	fprintf(stdout, "\t MODO N (normal) P (PAA) S (Submuestreo).\n");
}

int main_(){
	int n_capas = 3;
	int capas [3] = {3,2,1};
	
	float patron [1] = {0.8};
	float lr = 0.9;
	
	ann *red = ann_crear_red(capas, n_capas);
	
	fprintf(stdout, "Empieza\n");
	
	red->entrada->x0 = 1;
	red->entrada->salidas[0] = 0.4;
	red->entrada->salidas[1] = 0.2;
	red->entrada->salidas[2] = 0.7;
	
	fprintf(stdout, "Entradas OK\n");
	
	red->capas[0].bias[0] = 0.5;
	red->capas[0].pesos[0][0] = 0.6;
	red->capas[0].pesos[0][1] = 0.8;
	red->capas[0].pesos[0][2] = 0.6;
	
	fprintf(stdout, "Capa Oculta, Nodo 1 OK\n");
	
	red->capas[0].bias[1] = 0.7;
	red->capas[0].pesos[1][0] = 0.9;
	red->capas[0].pesos[1][1] = 0.8;
	red->capas[0].pesos[1][2] = 0.4;
	
	fprintf(stdout, "Capa Oculta, Nodo 2 OK\n");
	
	red->capas[1].bias[0] = 0.5;
	red->capas[1].pesos[0][0] = 0.9;
	red->capas[1].pesos[0][1] = 0.9;
	
	fprintf(stdout, "Capa de salida OK\n");
	
	ann_calcular_salida(red);
	//ann_imprime_red(red);
	
	while(!ann_identicos(red->capas[1].salidas, patron, 1)){
		fprintf(stdout, "Salida: %f\n", red->capas[1].salidas[0]);
		fprintf(stdout, "Patron: %f\n", patron[0]);
		
		fprintf(stdout, "###########\n");
		entrenamiento_bp(&lr, red, patron);
		
		ann_calcular_salida(red);
		ann_imprime_red(red);
		getchar();
	}
	
	ann_imprime_red(red);
}

int main(int argc, char **argv){
	FILE *p_red, *p_bd_prueba;
	char *dir_red, *dir_bd_prueba;
	ann *red;
	bd_t *bd_prueba;
	
	int i, j, capa_salida, entrenamiento_n;
	char s, opt;
	int errores;
	
	clock_t tiempo;
	float segs;
	int *errores_clase, *aciertos_clase;
	float pct_aciertos, pct_aciertos_clase;
	float *salidas_red, *patron;
	
	float *entrada, *paa, *sub;
	
	/* Mal ejecutado */
	if(argc != 4){
		ayuda(argv[0]);
		exit(1);
	}
	
	/* Leer los argumentos de la consola */
	dir_red = argv[1];
	dir_bd_prueba = argv[2];
	entrenamiento_n = atoi(argv[3]);
	
	p_red = fopen(dir_red, "r");
	p_bd_prueba = fopen(dir_bd_prueba, "r");
	if(p_red == NULL || p_bd_prueba == NULL){
		fprintf(stderr, "Error al abrir los archivos %s %s\n", dir_red, dir_bd_prueba);
		exit(1);
	}
	
	red = ann_leer_red(p_red);
	fclose(p_red);
	bd_prueba = input_leer_bdbin(p_bd_prueba);
	fclose(p_bd_prueba);
	
	
	/* Obtener el modo de clasificación */
	opt = argv[argc-1][0];
	switch(opt){
		case 'N': break;
		case 'S':
			sub = (float *) malloc(sizeof(float) * (bd_prueba->d / 2));
		break;

		case 'P':
			paa = = (float *) malloc(sizeof(float) * PAA_N);
		break;
		
		default:
			ayuda(argv[0]);
			exit(1);
		break;
	}
	
	capa_salida = red->n_capas-2; /* El índice de la capa de salida */
	
	/* Medir la eficiencia de la red */
	errores = 0;
	segs = 0.0;
	pct_aciertos = 0.0;
	pct_aciertos_clase = 0.0;
	
	errores_clase = (int *) malloc(sizeof(int) * red->capas[capa_salida].n_nodos);
	aciertos_clase = (int *) malloc(sizeof(int) * red->capas[capa_salida].n_nodos);
	patron = (float *) malloc(sizeof(float) * red->capas[capa_salida].n_nodos);
	
	memset(errores_clase, 0, sizeof(int) * red->capas[capa_salida].n_nodos);
	memset(aciertos_clase, 0, sizeof(int) * red->capas[capa_salida].n_nodos);
	
	for(i = 0; i < bd_prueba->n; i++){
		
		/* Establecer la entrada de la red */
		switch(opt){
			case 'S':
				memset(sub, 0, sizeof(float) * (bd_prueba / 2));
				vector_sub(sub, bd_prueba->datos[i], bd_prueba->d);
				entrada = sub;
			break;
				
			case 'P':
				memset(paa, 0, sizeof(float) * PAA_N);
				vector_paa(paa, bd_prueba->datos[i], bd_prueba->d);
				entrada = paa;
				break;
				
			case 'N':
				entrada = bd_prueba->datos[i]
			break;
		}
		
		/* Obtener el tiempo */
		tiempo = clock();
		
		//~ red->entrada->salidas = bd_prueba->datos[i];
		red->entrada->salidas = v_paa;
		ann_calcular_salida(red);
		
		tiempo = clock() - tiempo;
		segs += ((float) tiempo) / CLOCKS_PER_SEC;
		
		/* Crear el patron */
		memset(patron, 0, sizeof(float) * red->capas[capa_salida].n_nodos);
		patron[bd_prueba->clases[i] -1] = 1.0;
		
		/* Tomar las salidas y aplicarles el umbral */
		s = 0;
		salidas_red = red->capas[capa_salida].salidas;
		for(j = 0; j < red->capas[capa_salida].n_nodos; j++){
			salidas_red[j] = salidas_red[j] > 0.5 ? 1.0 : 0.0;
			
			if(salidas_red[j] != patron[j]){
				s = 1;
				break;
			}
		} 
		
		if(s == 0){
			pct_aciertos += 1.0;
			aciertos_clase[bd_prueba->clases[i] -1]++;
		}
		else{
			errores++;
			errores_clase[bd_prueba->clases[i] -1]++;
		}
	}
	//~ fprintf(stdout, "Total de errores %d %d\n", errores, bd_prueba->n);
	
	for(i = 0; i < red->capas[capa_salida].n_nodos; i++)
		pct_aciertos_clase += ((float)aciertos_clase[i] / (float)(aciertos_clase[i] + errores_clase[i])) * 100.0;
	
	pct_aciertos_clase = pct_aciertos_clase / (float)red->capas[capa_salida].n_nodos;
		
	//~ fprintf(stdout, "#registros bytes pct_aciertos1 pct_aciertos2 tiempo_segundos\n");
	fprintf(stdout, "%d %d %f %f %f\n", 
		entrenamiento_n, ann_tam_bytes(red), ((pct_aciertos / (float)bd_prueba->n) * 100), pct_aciertos_clase, segs);
	
	//~ fprintf(stdout, "Clases \"Aciertos\" \t \"Errores\" \n");	
	//~ for(i = 0; i < red->capas[capa_salida].n_nodos; i++)
		//~ fprintf(stdout, "\"C%d\" \t %d \t %d \n", i+1, aciertos_clase[i], errores_clase[i]);
				//~ 
	//~ fprintf(stdout, "------------------------------------------------------------------\n");
	//~ fprintf(stdout, "Prom \t %f \t %f \n", 
			//~ ((float)pct_aciertos / (float)bd_prueba->n) * 100.0,
			//~ ((float)errores / (float)bd_prueba->n) * 100.0 );
	
	return 0;
}
