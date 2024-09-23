/* CONSIDERAR:
 * Los datos que este algoritmo recibe,
 * no están asignados a ninguna clase,
 * y todos los atributos serán utilizados.*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ann-enbp.h"
#include "lectura.h"

void ayuda(char *programa){
	fprintf(stdout, "USO:\n");
	fprintf(stdout, "\t Imprime ayuda:\n");
	fprintf(stdout, "\t %s -h\n", programa);
	fprintf(stdout, "\n");
	fprintf(stdout, "\t Clasifica:\n");
	fprintf(stdout, "\t %s archivo.csv archivo.tmt ann.bin\n", programa);
}

int main(int argc, char **argv){
	relacion *r;
	esquema_t *e;
	int esquema_l;
	
	FILE *a_red, *a_csv, *a_tmt;
	
	ann *red;
	int n_salidas;
	
	float **datos;
	int i,j,k;
	
	if(strcmp(argv[1], "-h") == 0){
		ayuda(argv[0]);
		exit(1);
	}
	
	a_csv = fopen(argv[1], "r");
	a_tmt = fopen(argv[2], "r");
	a_red = fopen(argv[3], "r");
	/* TO-DO: 
	 * Validar que se abrieron los archivos */
	
	e = lectura_obtener_esquema(a_tmt, &esquema_l);
	r = lectura_obtener_relacion(a_csv, e, esquema_l);
	
	
	/* Pasar los datos a una matriz de float */
	datos = (float **) malloc(sizeof(float *) * r->tuplas_len);
	for(i = 0; i < r->tuplas_len; i++){
		
		datos[i] = (float *) malloc(sizeof(float) * esquema_l);
		for(j = 0; j < esquema_l; j++)
			datos[i][j] = r->tuplas[j][i].v.valor_f;
		
	}
	
	/* Construye la red */
	red = ann_leer_red(a_red);	
	n_salidas = red->capas[red->n_capas-2].n_nodos;
	
	/* Realiza la clasificación */
	for(i = 0; i < r->tuplas_len; i++){
		red->entrada->salidas = datos[i];
		ann_calcular_salida(red);
		
		fprintf(stdout, "Intancia %d  Clase: ", i);
		for(j = 0; j < n_salidas; j++)
			fprintf(stdout, "%f %f ## ", 
			((red->capas[red->n_capas-2].salidas[j] <= 0.5) ? 0.0 : 1.0),
			red->capas[red->n_capas-2].salidas[j]);
		
		
		fprintf(stdout, "\n");
	}
	
	ann_imprime_red(red);
	
	return 0;
}
