#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ann-enbp.h"
#include "lectura.h"

#define ARCHIVO "red.bin"

//#define DEBUG 1

int limite;

/*
 * <Opción> Archivo.csv Archivo.tmt "Patrones" Learning-rate ["Topologia" | Archivo-red] 
 * Opciones:
 * -a Actualizar los pesos
 * 		-a Archivo.csv Archivo.tmt "Patrones" Learning-rate Archivo-red
 * 
 * -n Crear la red
 * 		-n Archivo.csv Archivo.tmt "Patrones" Learning-rate "Topologia" 
 * 
 * */

void ann_e_ayuda(char **argv){
	fprintf(stdout,"USO:\n");
	fprintf(stdout,"\t Imprime ayuda\n");
	fprintf(stdout,"\t %s -h \t Imprime ayuda.\n\n", argv[0]);
	
	fprintf(stdout,"\t Nueva red y entrenamiento\n");
	fprintf(stdout,"\t %s -n datos.csv datos.tmt \"indices_var_objetivo\" learning-rate \"topologia\"\n\n", argv[0]);
	
	fprintf(stdout,"\t Actualizar red existente \n");
	fprintf(stdout,"\t %s -a datos.csv datos.tmt \"indices_var_objetivo\" learning-rate red.bin\n\n", argv[0]);
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
	ann_imprime_red(red);
	
	while(!ann_identicos(red->capas[1].salidas, patron, 1)){
		fprintf(stdout, "Salida: %f\n", red->capas[1].salidas[0]);
		fprintf(stdout, "Patron: %f\n", patron[0]);
		
		fprintf(stdout, "###########\n");
		ann_bp_en(&lr, red, patron);
		
		ann_calcular_salida(red);
		ann_imprime_red(red);
		getchar();
	}
	
	ann_imprime_red(red);
}

void ann_e_entrena(ann *red, info parametros, float lr){
	float *capa_salida;
	float *entrada;
	
	int epocas, ajustes;
	
	int i, j, k, patron_len;
	
	int indice_salida = red->n_capas-2;
	
	capa_salida = (float *) malloc(sizeof(float) * red->capas[indice_salida].n_nodos);
	patron_len = red->capas[indice_salida].n_nodos * sizeof(float);
	
	if(capa_salida == NULL) error_mem();
	
	epocas = 0;
	ajustes = 0;
	
	
	do{
		ajustes = 0;
		epocas++;
		
		/* ## PROBLEMA 
		 * CHECAR LA VARIABLE patron_v ES LA 
		 * QUE CAUSA RUIDO EN LA EJECUCIÓN */
		
		/* Ciclo de los datos */
		for(i = 0; i < parametros.n_datos; i++){
			
			red->entrada->salidas = parametros.datos[i];			
			ann_calcular_salida(red);
			memcpy(capa_salida, red->capas[indice_salida].salidas, patron_len);
			
			#ifdef DEBUG
				fprintf(stdout, "\n\nESTO ME INTERESAAAA\n");
				for(k = 0; k < red->capas[indice_salida].n_nodos; k++)
					fprintf(stdout, "%f %f\n", capa_salida[k], red->capas[indice_salida].salidas[k]);
				
				fprintf(stdout, "\n");
				getchar();
			#endif
			
			/* Arregla los pesos con el UMBRAL */
			for(k = 0; k < red->capas[indice_salida].n_nodos; k++)		
				capa_salida[k] = (capa_salida[k] <= UMBRAL) ? 0.0 : 1.0;
			
			
						
			//while(memcmp(parametros.patrones[i], capa_salida, patron_len)){
			while(!ann_identicos(parametros.patrones[i], capa_salida, red->capas[indice_salida].n_nodos)){
				ajustes++;
								
				ann_bp_en(&lr, red, parametros.patrones[i]);			
				ann_calcular_salida(red);
				memcpy(capa_salida, red->capas[indice_salida].salidas, patron_len);
				
				/* Arregla los pesos con el UMBRAL */
				for(k = 0; k < red->capas[indice_salida].n_nodos; k++)		
					capa_salida[k] = (capa_salida[k] <= UMBRAL) ? 0.0 : 1.0;
				
				#ifdef DEBUG				
					fprintf(stdout, "Salida Nueva \n");
					for(k = 0; k < red->capas[indice_salida].n_nodos; k++){
						fprintf(stdout, "%f \t %f \t %f\n", 
								red->capas[indice_salida].salidas[k],
								capa_salida[k],
								parametros.patrones[i][k]);
					}
					fprintf(stdout, "\n\n");
				#endif
			}
		
			#ifdef DEBUG
				fprintf(stdout, "## EPOCA: %d Instancias : %d de %d\n", epocas, i, parametros.n_datos);
			#endif
		}
		
		fprintf(stdout, "## EPOCA: %d\n", epocas);
		
	}while(ajustes > 0 && limite > epocas);
	
}

int main(int argc, char **argv){
	FILE *datos_csv, *datos_tmt, *archivo_red;
	
	ann *red;
	int n_capas, *capas;
	
	/* Patrones y topología*/
	char *objetivos, *obj;
	char *topologia, *nodo;
	
	int i, j;
	float *capa_salida;
	
	/* Lectura de los datos */
	relacion *rel;
	esquema_t *e;
	int e_len;
	
	float **datos;
	int n_datos;
	float learning_rate = 0.001;
	
	int n_indices;
	int *indices;
	int j_d, j_p, k;
	int encontrado;
	float **patrones;
	
	info parametros;
	
	/* Validación de parámetros */
	int opcion;
	if(strcmp(argv[1], "-h") == 0 || argc < 2){
		ann_e_ayuda(argv);
		exit(1);
	}
	
	limite = atoi(argv[argc-1]);
	
	/* Identificar la opción */
	if(strcmp(argv[1], "-n") == 0) /* Crear la red nueva */
		opcion = 2;
	else if(strcmp(argv[1], "-a") == 0) /* Actualizar los pesos */
		opcion = 3;
	else
		opcion = 1;
	
	/* Parámetros de entrada */
	datos_csv = fopen(argv[2], "r");
	datos_tmt = fopen(argv[3], "r");
	objetivos = argv[4];
	learning_rate = atof(argv[5]);
	
	/* Obtener los datos */
	e = lectura_obtener_esquema(datos_tmt, &e_len);
	rel = lectura_obtener_relacion(datos_csv, e, e_len);
	fclose(datos_tmt);
	fclose(datos_csv);
	
	/* Obtener los índices de los patrones */		
	n_indices = 1;
	for(i = 0; i < strlen(objetivos); i++)
		if(objetivos[i] == ',') n_indices++;
	
	indices = (int *) malloc(sizeof(int) * n_indices);
	
	obj = strtok(objetivos, ",");
	indices[0] = atoi(obj);
	for(i = 1; i < n_indices; i++){
		obj = strtok(NULL, ",");
		indices[i] = atoi(obj);
	}
	
	
	/* Pasa los datos a la matriz */
	n_datos = rel->tuplas_len;
	datos = (float **) malloc(sizeof(float *) * n_datos);
	patrones = (float **) malloc(sizeof(float *) * n_datos);
	
	for(i = 0; i < n_datos; i++){
		j_d = j_p = 0;
		
		datos[i] = (float *) malloc(sizeof(float) * e_len);
		patrones[i] = (float *) malloc(sizeof(float) * n_indices);
		
		for(j = 0; j < e_len; j++){
			encontrado = 0;
			
			/* Buscar la columna en los indices del patron*/
			for(k = 0; k < n_indices; k++)
				if(j == indices[k]) encontrado = 1;
			
			/* Lo ubica como patron o como dato */
			if(encontrado)
				patrones[i][j_p++] = rel->tuplas[j][i].v.valor_f;
			else
				datos[i][j_d++] = rel->tuplas[j][i].v.valor_f;
				
		}
	}
	
	#ifdef DEBUG
	for(i = 0; i < n_datos; i++){
		for(j = 0; j < n_indices; j++){
			fprintf(stdout, "%f ", patrones[i][j]);
		}
		fprintf(stdout, "\n");
	}
	getchar();
	#endif
	
	#ifdef DEBUG
	for(i = 0; i < n_datos; i++){
		for(j = 0; j < e_len; j++){
			fprintf(stdout, "%f ", datos[i][j]);
		}
		fprintf(stdout, "\n");
	}
	getchar();
	#endif
	
	/* Variables que ya no serán utilizadas */
	free(indices);
	free(e);
	free(rel);	
	
	/* Centraliza la información [cuestiones de diseño] */
	parametros.n_datos = n_datos;
	parametros.datos = datos;
	parametros.patrones = patrones;
	
	/* Lectura de la entrada de datos */
	switch(opcion){
		case 2:
			
			topologia = argv[argc-2];
			n_capas = 1;
			for(i = 0; i < strlen(topologia); i++)
				if(topologia[i] == ',') n_capas++;
			
			capas = (int *) malloc(sizeof(int) * n_capas);
			
			nodo = strtok(topologia, ",");
			capas[0] = atoi(nodo);
			for(i = 1; i < n_capas; i++){
				nodo = strtok(NULL, ",");
				capas[i] = atoi(nodo);
			}
			
			/* Imprime la topología de la red */
			#ifdef DEBUG
			fprintf(stdout, "Topologia:\n");
			for(i = 0; i < n_capas; i++)
				fprintf(stdout, "[%d] %d\n", i, capas[i]);
			
			getchar();
			#endif
			
			red = ann_crear_red(capas, n_capas);
			ann_pesos_aleatorios(red);
			
			ann_escribir_red(red, ARCHIVO);
			break;
			
		case 3:
					
			archivo_red = fopen(argv[argc-2], "r");
			if(archivo_red == NULL){
				fprintf(stderr, "Error al abrir el archivo %s\n", argv[argc-1]);
				exit(1);
			} 
			
			red = ann_leer_red(archivo_red);
			fclose(archivo_red);
			break;
			
		case 1:
			ann_e_ayuda(argv);
			exit(1);
	}
	
	ann_e_entrena(red, parametros, learning_rate);
	ann_escribir_red(red, ARCHIVO);
	
	ann_imprime_red(red);
	
	
	return 0;
}


