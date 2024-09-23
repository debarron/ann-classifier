/* Minería de Datos
 * Programa para leer archivos de datos en formato CSV.
 *
 * Este programa requiere dos archivos, archivo de datos (CSV)
 * y el archivo de configuración (TMT). El archivo de configuración
 * obedece el formato descrito en "lectura.h".
 *
 * Daniel Ernesto Lopez-Barron
 * */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "lectura.h"

#define TIPO_1 "NOMINAL"
#define TIPO_2 "ENTERO"
#define TIPO_3 "REAL"
#define TIPO_4 "ORDINAL"
#define TIPO_5 "PERIODO"

#define BUFF 512


/* TODO:
 * 	-> Implementar el uso de periodos
 * 	-> Implementar el uso de ordinales
 * 	-> Idear que hacer con situaciones de error
 * 	   (más atributos de lo establecido)
 * */



esquema_t* lectura_obtener_esquema(FILE *archivo_tmt, int *esquema_len){
	char *str1, *str2, *tok;
	char *token_tmt = " \t\n";

	int i, len;
	int bytes_archivo;
	int variables_len;

// Actuallización 14-OCT-2012	
	int k, tok_len, elementos_len;
	int inicio_elemento, fin_elemento;
	int indice_elemento;
	char **elementos;

	esquema_t *esquema;

	/* Obtener la cantidad de bytes del archivo y asignar memoria*/
	fseek(archivo_tmt, 0, SEEK_END);
	bytes_archivo = ftell(archivo_tmt);
	rewind(archivo_tmt);

	str1 = (char *) malloc(sizeof(char) * bytes_archivo);
	str2 = (char *) malloc(sizeof(char) * bytes_archivo);
	fread(str1, sizeof(char), bytes_archivo, archivo_tmt);
	memcpy(str2, str1, sizeof(char) * bytes_archivo);


	/* Contar el numero de variables por los datos leidos, sigue el esquema
	 * del archivo TMT */
	variables_len = 0;
	tok = strtok(str1, token_tmt);
	while(tok != NULL){
		tok = strtok(NULL, token_tmt);
		variables_len++;
	}

	variables_len = variables_len/3;
	*esquema_len = variables_len;


	/* Leer el archivo TMT */
	esquema = (esquema_t *) malloc(sizeof(esquema_t) * variables_len);
	if(esquema == NULL){
	    fprintf(stdout, "lectura_obtener_esquema: Sin memoria para asignar <esquema>\n");
	    exit(1);
	}

	for(i = 0; i < variables_len; i++){

		tok = strtok(((i==0)?str2:NULL), token_tmt);

		/* Leer el nombre de la variable */
		len = strlen(tok);
		esquema[i].nombre = (char *) malloc(sizeof(char) * len);
		strcpy(esquema[i].nombre, tok);

		/* Ubicar el tipo de la variable */
		tok = strtok(NULL, token_tmt);
		if(strcmp(tok, TIPO_1) == 0)
			esquema[i].tipo = 1;
		else if(strcmp(tok, TIPO_2) == 0)
			esquema[i].tipo = 2;
		else if(strcmp(tok, TIPO_3) == 0)
			esquema[i].tipo = 3;
		else if(strcmp(tok, TIPO_4) == 0)
			esquema[i].tipo = 4;
		else
			esquema[i].tipo = 5;

		esquema[i].elementos_len = 0;
		/* Parte de los ordinales */
		tok = strtok(NULL, token_tmt);
		
//** Actuallización 14-OCT-2012
/*** RECONOCE LAS CLASES DE LOS NOMINALES ****/			
		/* Ver si viene como X */
		if(tok[0] == 'X') continue;
		
			
		/* Contar los elementos */
		/* Se considera que los valores vienen 
		 * de la forma {,,}, sin espacios entre 
		 * las variables */
		elementos_len = 0;
		tok_len = strlen(tok);
		for(k = 0; k < tok_len - 1; k++)
			if(tok[k] == ',') elementos_len++;
		
		elementos_len++;
		
		elementos = (char **) malloc(sizeof(char *) * elementos_len);
		if(elementos == NULL){
			fprintf(stdout, "lectura_obtener_esquema: Error, no se pudo asignar memoria\n");
			exit(1);			
		}
		
		
		indice_elemento = 0;
		inicio_elemento = 1;	
		k = 1;
		
		while(indice_elemento < elementos_len - 1){
			
			fin_elemento = k;
			if(tok[fin_elemento] == ','){
				elementos[indice_elemento] = (char *) malloc(sizeof(char) * (fin_elemento - inicio_elemento));
					
				memcpy(elementos[indice_elemento],
					   &tok[inicio_elemento],
					   sizeof(char) * (fin_elemento - inicio_elemento)
					  );
				
				inicio_elemento = fin_elemento + 1;
				
				indice_elemento++;
			}
			k++;
		}
		
		elementos[indice_elemento] = (char *) malloc(sizeof(char) * strlen(tok) - inicio_elemento - 1);
		memcpy(elementos[indice_elemento], 
			   &tok[inicio_elemento],
			   sizeof(char) * strlen(tok) - inicio_elemento - 1
			   );
		
		
		esquema[i].elementos = elementos;
		esquema[i].elementos_len = elementos_len;
		
	}

	free(str1);
	free(str2);

	return esquema;
}

relacion* lectura_obtener_relacion(FILE *archivo_datos, esquema_t *esquema, int esquema_len){
	char *buffer, *valor;

	char c;
	int instancias_len;
	int i, j, len;

	fpos_t inicio;

	atributo **tuplas;
	relacion *rel;

	/* Contar las lineas del archivo de datos por los saltos de linea */
	instancias_len = 0;
	fgetpos(archivo_datos, &inicio);
	do{
		c = fgetc(archivo_datos);
		if( c == '\n') instancias_len++;
	}while(c != EOF);
	fsetpos(archivo_datos, &inicio);


	/* Asignar memoria para los datos */
	tuplas = (atributo **) malloc(sizeof(atributo *) * esquema_len);
	if(tuplas == NULL){
	    fprintf(stdout, "lectura_obtener_relacion: Sin memoria para asignar <tuplas>\n");
	    exit(1);
	}

	for(i = 0; i < esquema_len; i++){
		tuplas[i] = (atributo *) malloc(sizeof(atributo) * instancias_len);
		if(tuplas[i] == NULL){
            fprintf(stdout, "lectura_obtener_relacion: Sin memoria para asignar <tuplas[i]>\n");
            exit(1);
        }
	}


	/* Lectura de datos del archivo CSV */
	buffer = (char *) malloc(sizeof(char) * BUFF);
	if(buffer == NULL){
	    fprintf(stdout, "lectura_obtener_relacion: Sin memoria para asignar <buffer>\n");
	    exit(1);
	}
	memset(buffer, '\0', BUFF);

	for(i = 0; i < instancias_len; i++){
		for(j = 0; j < esquema_len; j++){

			fgetpos(archivo_datos, &inicio);
			len = 0;

			/* Leer todo aquello que este entre una , o un salto de linea */
			do{
				c = fgetc(archivo_datos);
				len++;
			}while(c != ',' && c != '\n');

			/* Asignar memoria para guardar el patron
			 * Posicionar al inicio del patron encontrado y leer
			 * Leer otra vez para tomar la , o el \n ignorado */
			valor = (char *) malloc(sizeof(char) * len);
			if(valor == NULL){
                fprintf(stdout, "lectura_obtener_relacion: Sin memoria para asignar <valor>\n");
                exit(1);
            }
			memset(valor, '\0', sizeof(char) * len);

			fsetpos(archivo_datos, &inicio);
			fread(valor, sizeof(char), len-1, archivo_datos);

			fgetc(archivo_datos);


			/* Asignar el valor de acuerdo al tipo del atributo j */
			if(esquema[j].tipo == 2){
				memset(buffer, '\0', BUFF);
				memcpy(buffer, valor, len-1);

				sscanf(buffer, "%d", &tuplas[j][i].v.valor_i);
			}
			else if(esquema[j].tipo == 3){
				memset(buffer, '\0', BUFF);
				memcpy(buffer, valor, len-1);

				sscanf(buffer, "%f", &tuplas[j][i].v.valor_f);
			}else{
				tuplas[j][i].cadena = (char *) malloc(strlen(valor) + 1);
				memcpy(tuplas[j][i].cadena, valor, strlen(valor));
				tuplas[j][i].cadena[strlen(valor)] = '\0';
			}

			free(valor);
		}
	}

	fprintf(stdout, "Leyo los datos\n");

	free(buffer);

	rel = (relacion *) malloc(sizeof(relacion));
	rel->esquema = esquema;
	rel->tuplas = tuplas;
	rel->esquema_len = esquema_len;
	rel->tuplas_len = instancias_len;

	return rel;
}


//~ int main(int argc, char **argv){
	//~ char *archivo_datos;
	//~ char *archivo_tmt;
//~ 
	//~ FILE * datos;
	//~ FILE * tmt;
//~ 
	//~ int variables_len, i, j;
	//~ esquema_t *esquema;
	//~ relacion *r;
//~ 
	//~ archivo_datos = argv[1];
	//~ archivo_tmt = argv[2];
//~ 
	//~ tmt = fopen(archivo_tmt, "r");
	//~ if(tmt == NULL){
		//~ fprintf(stderr, "Error al abrir el archivo .tmt\n");
		//~ exit(1);
	//~ }
//~ 
	//~ esquema = lectura_obtener_esquema(tmt, &variables_len);
	//~ fclose(tmt);
	//~ fprintf(stdout, "Leyo el esquema sin problemas\n");
//~ 
	//~ /* Lectura por lineas */
	//~ datos = fopen(archivo_datos, "r");
	//~ r = lectura_obtener_relacion(datos, esquema, variables_len);
	//~ fclose(datos);
	//~ fprintf(stdout, "Leyo el los datos sin problemas sin problemas\n");
//~ 
	//~ fprintf(stdout, "DATOS:\n ");
	//~ for(i = 0; i < r->tuplas_len; i++){
		//~ for(j = 0; j < r->esquema_len; j++){
//~ 
			//~ if(r->esquema[j].tipo == 2)
				//~ fprintf(stdout, "%d ", r->tuplas[j][i].v.valor_i);
			//~ else if(r->esquema[j].tipo == 3)
				//~ fprintf(stdout, "%f ", r->tuplas[j][i].v.valor_f);
			//~ else
				//~ fprintf(stdout, "%s ", r->tuplas[j][i].cadena);
		//~ }
		//~ fprintf(stdout, "\n ");
	//~ }
	//~ fprintf(stdout, "\n ");
//~ 
	//~ return 0;
//~ }
