#include <gtk/gtk.h>
#include <cairo/cairo.h>
#include <stdlib.h>
#include <omp.h>
#include <gdk/gdk.h>
#include <stdbool.h>  
#include "utils.h"

gboolean draw_callback_image (GtkWidget *widget, cairo_t *cr, gpointer data)
{
  
  GdkPixbuf *pixbuf_lena;
  GError *err=NULL;
  pixbuf_lena = gdk_pixbuf_new_from_file("../lena.png", &err);
  
  if(err)
  {
      printf("Error : %s\n", err->message);
      g_error_free(err);
      fflush(NULL);
      return FALSE;
  }

  guchar* lena_data = gdk_pixbuf_get_pixels (pixbuf_lena);
/* works and is beautiful
  set_black_zone(lena_data, 550*552/2);
  draw_gradient(lena_data, 550,552/4);
*/

  //guchar* gradient_tensor;
  //gradient_tensor = compute_gradient(lena_data, 550,552); => works :)

  guchar* G_tensor; // https://hal.archives-ouvertes.fr/hal-00332800/document
  G_tensor = compute_G(lena_data, 550,552);

  GdkPixbuf* new_pixbuf_from_lena_data;
  new_pixbuf_from_lena_data = gdk_pixbuf_new_from_data  (G_tensor, GDK_COLORSPACE_RGB, true, 8, 550, 552*2,550*4, NULL, NULL);


  gdk_cairo_set_source_pixbuf(cr, new_pixbuf_from_lena_data, 0, 0);
  cairo_paint(cr);


    return FALSE;
}

gboolean draw_callback_data (GtkWidget *widget, cairo_t *cr, gpointer data)
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
    full_surface =  cairo_image_surface_create_for_data((unsigned char *)data_buffer, CAIRO_FORMAT_ARGB32, width, height, stride);


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
    gtk_widget_set_size_request (drawing_area, 550, 552*2);
    g_signal_connect (G_OBJECT (drawing_area), "draw", G_CALLBACK (draw_callback_image), NULL);


    gtk_widget_show_all (window);
    gtk_main ();
    
    
    return 0;
}