#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define GUI "interfaz.glade"

#define ENTRENAMIENTO "./entrena-enbp"
#define CLASIFICACION "./clasifica-enbp"

typedef struct _data Data;

struct _data{
	GtkWidget *rbt_clasificacion;
	GtkWidget *rbt_entrenamiento;
	GtkWidget *fc_csv;
	GtkWidget *fc_tmt;
	GtkWidget *fc_red;
	GtkWidget *txt_indices;
	GtkWidget *txt_lr;
	GtkWidget *txt_capas;
	GtkWidget *txt_epocas;
	GtkWidget *gp_entrenamiento;
	GtkWidget *btn_guardar;
	GtkWidget *btn_ejecutar;
	GtkWidget *btn_salir;
	GtkWidget *txv_resultado;
};

char *archivo_tmt = NULL;
char *archivo_csv = NULL;
char *archivo_red = NULL;

int entrenamiento = 1;
char *comando = NULL;


/* Crear el buffer para pasar al textview todo de stdout */
GtkTextBuffer *txv_resultado_buffer;
int fds[2];

void stdout_to_txv(gpointer data, gint source, GdkInputCondition condition){
	gchar internal_buffer[1024];
	gint internal_buffer_len;
	GtkTextIter iterator;
	
	internal_buffer_len = 1024;
	gtk_text_buffer_get_end_iter(txv_resultado_buffer, &iterator);

	while (internal_buffer_len == 1024){
		internal_buffer_len = read(fds[0], internal_buffer, 1024);
		gtk_text_buffer_insert(txv_resultado_buffer, &iterator, internal_buffer, internal_buffer_len);
	}
}


int gui_validar_parametros(Data *gui){
	unsigned short lr, desc, indices;
	int valido = 1;
	
	lr = (unsigned short) gtk_entry_get_text_length(GTK_ENTRY(gui->txt_lr));
	indices = (unsigned short) gtk_entry_get_text_length(GTK_ENTRY(gui->txt_indices));
	desc = (unsigned short) gtk_entry_get_text_length(GTK_ENTRY(gui->txt_capas));
	
	if(lr == 0 || indices == 0 || desc == 0)
		valido = 0;
		
	return valido;
}

int gui_validar_archivos(Data *gui){
	char *tmt, *red, *csv;
	int valido = 1;
	
	tmt = (char *) gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(gui->fc_tmt));
	csv = (char *) gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(gui->fc_csv));
	red = (char *) gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(gui->fc_red));
	
	if(entrenamiento){		
		/* Valida que haya archivos */
		if(tmt == NULL || csv == NULL)
			valido = 0;
	}
	else{
		if(tmt == NULL || csv == NULL || red == NULL)
			valido = 0;
	}
	
	return valido;
}	


G_MODULE_EXPORT void rbt_clicked(GtkWidget *rbt, Data *interfaz){
		
	if(GTK_RADIO_BUTTON(rbt) == GTK_RADIO_BUTTON(interfaz->rbt_clasificacion)){		
		gtk_widget_set_sensitive(interfaz->gp_entrenamiento, FALSE);
		entrenamiento = 0;
	}
	else{	
		gtk_widget_set_sensitive(interfaz->gp_entrenamiento, TRUE);
		entrenamiento = 1;
	}
	
}


G_MODULE_EXPORT void btn_ejecutar_clicked(GtkWidget *boton, Data *interfaz){
	int comando_len = 0;
	
	GtkTextIter ei;
	GtkTextIter si;
	
	char *tmt, *csv, *red, *indices, *epocas, *capas, *lr;	
	
	/* Realiza el pipe con stdout */
	pipe(fds);
	dup2(fds[1], 1);
	
	/* Crea y vincula el buffer con stdout */
	txv_resultado_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(interfaz->txv_resultado));
	gtk_text_buffer_get_end_iter(txv_resultado_buffer, &ei);
	gtk_text_buffer_get_start_iter(txv_resultado_buffer, &si);
	gtk_text_buffer_delete(txv_resultado_buffer, &si, &ei);
		
	gdk_input_add(fds[0], GDK_INPUT_READ, stdout_to_txv, NULL);
	
	
	/* apunta a los textos de los widgets */
	csv = (char *) gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(interfaz->fc_csv));
	tmt = (char *) gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(interfaz->fc_tmt));
	red = (char *) gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(interfaz->fc_red));
	
	indices = (char *) gtk_entry_get_text(GTK_ENTRY(interfaz->txt_indices));
	capas = (char *) gtk_entry_get_text(GTK_ENTRY(interfaz->txt_capas));
	lr = (char *) gtk_entry_get_text(GTK_ENTRY(interfaz->txt_lr));
	epocas = (char *) gtk_entry_get_text(GTK_ENTRY(interfaz->txt_epocas));
	
		
	if(entrenamiento){
		if(gui_validar_archivos(interfaz) && gui_validar_parametros(interfaz)){
			
			comando_len = strlen(csv) + strlen(tmt) + strlen(indices) + 
						  strlen(capas) + strlen(lr) + strlen(epocas) + 30;

			comando = (char *) malloc(sizeof(char) * comando_len);
			memset(comando, '\0', comando_len * sizeof(char));
			
			sprintf(comando, "%s -n %s %s \"%s\" %s \"%s\" %s",			
					ENTRENAMIENTO, csv, tmt, indices, lr, capas, epocas);
						
			system(comando);
		}
		
	}
	else{
			comando_len = strlen(csv) + strlen(tmt) + strlen(red) + 30;
			comando = (char *) malloc(sizeof(char) * comando_len);
			memset(comando, '\0', comando_len * sizeof(char));
			
			sprintf(comando, "%s %s %s %s", CLASIFICACION, csv, tmt, red);
			system(comando);
			
		}
}

G_MODULE_EXPORT void btn_guardar_clicked(GtkWidget *boton, Data *interfaz){
}

G_MODULE_EXPORT void btn_salir_clicked(GtkWidget *boton, Data *interfaz){
	gtk_main_quit();
}

/*
G_MODULE_EXPORT void buscar_archivo(GtkWidget *boton, Data *datos){
	
	char *archivo;
	GtkWidget *fc_archivo;
	
	Crear la ventana para buscar archivos 
	fc_archivo =  gtk_file_chooser_dialog_new("Abrir archivo...",
                    NULL, GTK_FILE_CHOOSER_ACTION_OPEN,
                    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_OK, NULL);
    
    Si se seleccionó un archivo 
    if(gtk_dialog_run(GTK_DIALOG(fc_archivo)) == GTK_RESPONSE_OK){
		archivo = (char *) gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(fc_archivo));
		
		if(boton == GTK_WIDGET(datos->btn_csv)){
			archivo_csv = archivo;
			gtk_label_set_text(GTK_LABEL(datos->lbl_csv), (const gchar *)archivo);
		}
		else{
			archivo_tmt = archivo;
			gtk_label_set_text(GTK_LABEL(datos->lbl_tmt), (const gchar *)archivo);
		}
	}
		
	gtk_widget_destroy(fc_archivo);
}
*/

int main(int argc, char **argv){
	GtkBuilder *builder;
	GtkWidget *window;
	Data interfaz;
	
	gtk_init(&argc, &argv);
	
	/* Crear el constructor */
	builder = gtk_builder_new();
	gtk_builder_add_from_file(builder, GUI, NULL);
	
	
	/* Obtener los componentes del formulario */
	window = GTK_WIDGET(gtk_builder_get_object(builder, "w_principal"));
	
	interfaz.rbt_clasificacion = GTK_WIDGET(gtk_builder_get_object(builder, "rbt_clasificacion"));
	interfaz.rbt_entrenamiento = GTK_WIDGET(gtk_builder_get_object(builder, "rbt_entrenamiento"));
	
	interfaz.txt_lr = GTK_WIDGET(gtk_builder_get_object(builder, "txt_lr"));
	interfaz.txt_indices = GTK_WIDGET(gtk_builder_get_object(builder, "txt_indices"));
	interfaz.txt_capas = GTK_WIDGET(gtk_builder_get_object(builder, "txt_capas"));
	interfaz.txt_epocas = GTK_WIDGET(gtk_builder_get_object(builder, "txt_epocas"));
	
	interfaz.txv_resultado = GTK_WIDGET(gtk_builder_get_object(builder, "txv_resultado"));
	
	
	interfaz.gp_entrenamiento = GTK_WIDGET(gtk_builder_get_object(builder, "gp_entrenamiento"));
	
	
	interfaz.fc_csv = GTK_WIDGET(gtk_builder_get_object(builder, "fc_csv"));
	interfaz.fc_tmt = GTK_WIDGET(gtk_builder_get_object(builder, "fc_tmt"));
	interfaz.fc_red = GTK_WIDGET(gtk_builder_get_object(builder, "fc_red"));
	/*data.lbl_csv = GTK_WIDGET(gtk_builder_get_object(builder, "lbl_csv"));
	data.btn_csv = GTK_WIDGET(gtk_builder_get_object(builder, "btn_csv"));
	
	data.lbl_tmt = GTK_WIDGET(gtk_builder_get_object(builder, "lbl_tmt"));
	data.btn_tmt = GTK_WIDGET(gtk_builder_get_object(builder, "btn_tmt"));
	
	data.btn_aceptar = GTK_WIDGET(gtk_builder_get_object(builder, "btn_aceptar"));
	data.btn_cancelar = GTK_WIDGET(gtk_builder_get_object(builder, "btn_cancelar"));
	*/
	
	/* Vincular las señales */
	gtk_builder_connect_signals(builder, &interfaz);
	
	
	/* Liberar el Builder */
	g_object_unref(G_OBJECT(builder));
	
	gtk_widget_show(window);
	gtk_main();
	
	return 0;
}
