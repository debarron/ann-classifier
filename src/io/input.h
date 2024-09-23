#ifndef INPUT_H
#define INPUT_H

typedef struct{
	int n;
	int d;
	float **datos;
	char * clases;
} bd_t;

bd_t *input_crear_bd100(FILE *bds, int n);
char input_obtener_clase(const char *archivo);
bd_t *input_leer_bdbin(FILE *bd_archivo);
int input_tam_bd(bd_t *bd);

#endif
