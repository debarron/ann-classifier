#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ann.h"
#include "io/input.h"


void ayuda(char *p){
	fprintf(stdout, "ENTRENAMIENTO DE LA RED NEURONAL\n## Uso:\n");
	fprintf(stdout, "## Ayuda\n%s -h\n", p);
	fprintf(stdout, "## Crear\n%s -c TOPOLOGIA DIR\n", p);
	fprintf(stdout, "## Entrenar\n%s -e RED datos_entrenamiento lerning-rate (Error | Max_Epocas) MODO\n", p);
	fprintf(stdout, "-------------------------------------------------------------------\n");
	fprintf(stdout, "\t TOPOLOGIA Secuencia de numeros que caracteriza la estrucutra de la red.\n");
	fprintf(stdout, "\t \t Ej. %s -c 3 5 4 3 => Tres capas 5 nodos en C1, 4 en C2 y 3 en C3.\n", p);
	fprintf(stdout, "\t RED Archivo binario resultado de -c\n");
	fprintf(stdout, "\t datos_entrenamiento Archivo con los datos de entrenamiento\n");
	fprintf(stdout, "\t learning-rate Coeficiente de aprendizaje\n");
	fprintf(stdout, "\t Error que debe alcanzar la red\n");
	fprintf(stdout, "\t Max_Epocas Número máximo de epocas.\n");
}


int *entrena_obtener_topologia(char *topologia, int *n_capas){
	int *capas;
	char *nodo;
	int i;
	
	*n_capas = 1;
	for(i = 0; i < strlen(topologia); i++)
		if(topologia[i] == ',') *n_capas = *n_capas + 1;
	
	capas = (int *) malloc(sizeof(int) * *n_capas);
	if(capas == NULL){
		fprintf(stderr, "ENTRENAMIENTO: Error al generar las capas\n");
		exit(1);
	}
	
	nodo = (char *) strtok(topologia, ",");
	capas[0] = atoi(nodo);
	for(i = 1; i < *n_capas; i++){
		nodo = (char *) strtok(NULL, ",");
		capas[i] = atoi(nodo);
	}
	
	return capas;
}


int main(int argc, char **argv){
	char *op, *red_dir, *datos_dir, *arg_parada;
	int *capas, n_capas;
	int i, op_indice;
	
	ann *red;
	
	float learning_rate, error_max;
	int parada_len, op_parada, epocas_max;
	
	FILE *p_red, *p_datos;
	bd_t *bd_entrenamiento;
	
	op = argv[1];
	
	/* Validación de la entrada */
	if(strcmp(op, "-c") == 0) op_indice = 0;
	else if(strcmp(op, "-e") == 0) op_indice = 1;
	else op_indice = 2;
	
	/* Mal comando */
	if(op_indice == 2){
		ayuda(argv[0]);
		exit(1);
	}
	
	/*################################################################*/
	/* Crear la red */
	if(op_indice == 0){ 
		n_capas = atoi(argv[2]);
		capas = (int *) malloc(sizeof(int) * n_capas);
		
		/* Casting de cada entrero en la secuencia */
		for(i = 0; i < n_capas; i++) capas[i] = atoi(argv[i + 3]);
		
		red = ann_crear_red(capas, n_capas);
		ann_pesos_aleatorios(red);
		ann_escribir_red(argv[argc-1], red, 0);
		
		return 0;
	}
	
	
	/*################################################################*/
	/* Actualizar los pesos (ENTRENAR) */
	
	red_dir = argv[2];
	datos_dir = argv[3];
	learning_rate = atof(argv[4]);
	arg_parada = argv[5];
	
	/* Validar si es error o epocas */
	op_parada = 0;
	parada_len = strlen(arg_parada);
	for(i = 0; i < parada_len; i++){
		if(arg_parada[i] == '.'){
			op_parada = 1;
			break;
		}
	}
	if(op_parada == 0) epocas_max = atoi(arg_parada);
	else error_max = atof(arg_parada);
	
	p_datos = fopen(datos_dir, "r"); /* Lee los datos */
	bd_entrenamiento = input_leer_bdbin(p_datos);
	if(p_datos == NULL){
		fprintf(stderr, "Error al abrir el archivo: %s\n", red_dir);
		exit(1);
	}
	
	p_red = fopen(red_dir, "r"); /* Lee la red */
	if(p_red == NULL){
		fprintf(stderr, "Error al abrir el archivo: %s\n", red_dir);
		exit(1);
	}
	red = ann_leer_red(p_red);
	
	fclose(p_red);
	fclose(p_datos);
	
	if(op_parada == 0) 
		entrena_x_epocas(red, bd_entrenamiento, learning_rate, epocas_max);
	
	ann_escribir_red(red_dir, red, 1);
	
	return 0;	
}

int main_(){
	int n_capas = 3;
	int capas [3] = {3,2,1};
	
	float patron [1] = {0.8};
	float lr = 0.9;
	
	ann *red = ann_crear_red(capas, n_capas);
	red->entrada->salidas = (float *) malloc(sizeof(float) * capas[0]);
	
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
		entrenamiento_bp(&lr, red, patron);
		//~ ann_bp(&lr, red, patron);
		
		ann_calcular_salida(red);
		ann_imprime_red(red);
		getchar();
	}
	
	ann_imprime_red(red);
}
