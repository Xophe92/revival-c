#ifndef UTILS_H
#define UTILS_H
 


#include <gtk/gtk.h>

void set_black_zone(guchar* start_of_data, int length);

#ifdef DEBUG
void show_data_in_console(guchar* start_of_data, int length);
void draw_gradient(guchar* start_of_data, int width, int height);
#endif

guchar* compute_gradient(guchar* start_of_data, int width, int height);
guchar* compute_G(guchar* start_of_data, int width, int height);




#endif  