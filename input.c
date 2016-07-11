#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "input.h"

#define DIR_IN_BD "/home/daniel/Documents/Master/Tesis/experimentos/programas/red-neuronal/datos/ecg-db/patrones/patrones_bin_float"

#define D 90

int input_tam_bd(bd_t *bd){
	int tam = 0;
	
	tam += sizeof(bd_t);
	tam += bd->d * bd->n * sizeof(float);
	tam += bd->n * sizeof(char);
	
	return tam;
}

char input_obtener_clase(const char *archivo){
	int indice, len, cg;
	char clase;
	
	clase = -1;
	
	len = strlen(archivo);
	indice = 0;
	cg = 0;
	while(cg < 3 && indice < len){
		if(archivo[indice] == '-') cg++;
		
		indice++;
	}
	
	/* El nombre del archivo no es correcto */
	if(cg != 3) return clase;
	
	
	switch(archivo[indice + 2]){
		case '1': /* Puede ser CL1, CL13, CL14, CL15, ... */
			switch(archivo[indice + 3]){
				case '-': clase = 1; break;
				case '3': clase = 10; break;
				case '4': clase = 11; break;
				case '5': clase = 12; break;
				case '6': clase = 13; break;
				case '7': clase = 14; break;
				case '8': clase = 15; break;
			}
			break;
		case '2': clase = 2; break;
		case '3': clase = 3; break;
		case '4': clase = 4; break;
		case '5': clase = 5; break;
		case '6': clase = 6; break;
		case '7': clase = 7; break;
		case '8': clase = 8; break;
		case '9': clase = 9; break;
	}
	
	return clase;
}

bd_t *input_leer_bdbin(FILE *bd_archivo){
	bd_t *bd;
	int i;
	
	bd = (bd_t *) malloc(sizeof(bd_t));
	fread(&bd->n, sizeof(int), 1, bd_archivo);
	fread(&bd->d, sizeof(int), 1, bd_archivo);
	
	bd->datos = (float **) malloc(sizeof(float *) * bd->n);
	bd->clases = (char *) malloc(sizeof(char) * bd->n);
	for(i = 0; i < bd->n; i++){
		bd->datos[i] = (float *) malloc(sizeof(float) * bd->d);
		fread(bd->datos[i], sizeof(float), bd->d, bd_archivo);
	}
	fread(bd->clases, sizeof(char), bd->n, bd_archivo);
	
	return bd;
}

bd_t *input_crear_bd100(FILE *bds, int n){
	int i, sig_dir_len, sig_d;
	FILE *sig_archivo;
	char *sig_dir;
	char *sig;
	
	bd_t *bd;
	
	bd = (bd_t *) malloc(sizeof(bd_t));
	bd->d = D;
	bd->n = n;
	bd->datos = (float **) malloc(sizeof(float *) * n);
	bd->clases = (char *) malloc(sizeof(char) * n);
	if(bd->datos == NULL){
		fprintf(stderr, "No se pudo asignar memoria para bd->datos\n");
		exit(1);
	}
	
	for(i = 0; i < n; i++){
		bd->datos[i] = (float *) malloc(sizeof(float) * D);
		if(bd->datos[i] == NULL){
			fprintf(stderr, "No se pudo asignar memoria para bd->datos[%d]\n", i);
			exit(1);
		}
		
		fscanf(bds, "%s\n", sig);
		sig_dir_len = strlen(DIR_IN_BD) + strlen(sig) + 1;
		sig_dir = (char *) malloc(sizeof(char) * sig_dir_len);
		sprintf(sig_dir, "%s/%s", DIR_IN_BD, sig);
		
		sig_archivo = fopen(sig_dir, "r");
		if(sig_archivo == NULL){
			fprintf(stderr, "Error al abrir el archivo %s\n", sig_archivo);
			exit(1);
		}
		
		bd->clases[i] = input_obtener_clase(sig);
		fread(bd->datos[i], sizeof(float), D, sig_archivo);
		
		fclose(sig_archivo);
		free(sig_dir);
		
		//fprintf(stdout, "El archivo es de clase %d\n", bd->clases[i]);	
		//fprintf(stdout, "%s %d\n", sig_dir, strlen(sig_dir));	
	}
	
	return bd;
}

