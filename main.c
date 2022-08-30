#include <gtk/gtk.h>
#include <cairo/cairo.h>
#include <stdlib.h>
#include <omp.h>



gboolean draw_callback (GtkWidget *widget, cairo_t *cr, gpointer data)
{
    guint width, height;
    
    width = gtk_widget_get_allocated_width (widget);
    height = gtk_widget_get_allocated_height (widget);

   

  	int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width);
    
    guchar *data_buffer;
    data_buffer = calloc(sizeof(guchar),stride * height);

    cairo_set_source_rgba(cr, 0,0,0,0.85);
    cairo_paint(cr);


    #pragma omp parallel for 
    for(long int i=0; i<stride * height; i+=4){
      data_buffer[i] = ((i/4) % width) *    256 / width ;  // bleu
      data_buffer[i+1] = ((i/4) / width) * 256 / height ;  //vert
      data_buffer[i+2] = (data_buffer[i] + data_buffer[i+1])/1;  //rouge
      
      data_buffer[i+3] = rand() * 256 / RAND_MAX; //alpha
    }


    cairo_surface_t *full_surface;
    full_surface = cairo_image_surface_create_for_data((unsigned char *)data_buffer, CAIRO_FORMAT_ARGB32, width, height, stride);


    cairo_set_source_surface(cr, full_surface, 0, 0);
		cairo_paint(cr);



    return FALSE;
}

gint main(int argc,char *argv[])
{

    GtkWidget *window, *drawing_area;
    
    gtk_init (&argc, &argv);


    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
    
    drawing_area = gtk_drawing_area_new();
    gtk_container_add (GTK_CONTAINER (window), drawing_area);
    gtk_widget_set_size_request (drawing_area, 200, 100);
    g_signal_connect (G_OBJECT (drawing_area), "draw", G_CALLBACK (draw_callback), NULL);


    gtk_widget_show_all (window);
    gtk_main ();
    
    
    return 0;
}