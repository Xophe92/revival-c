#include <gtk/gtk.h>
#include <math.h>

void set_black_zone(guchar* start_of_data, int length){
  for(int i=0; i<length; i++){
    start_of_data[4*i] = 0;
    start_of_data[4*i+1] = 0;
    start_of_data[4*i+2] = 0;
    start_of_data[4*i+3] = 255;
  }
}
#ifdef DEBUG
void show_data_in_console(guchar* start_of_data, int length){

  guchar* current_position;
  current_position = start_of_data;

 printf("--------------%d bytes starting from %#x-------------------", length, (unsigned int)start_of_data);
  for(int i=0; i<length; i++, current_position++)
  {
    
    if((i%4==0) && (i%32!=0))
      printf(" ");
    if(i%32==0)
      printf("\n[%#x] ", (unsigned int) current_position);


    printf("%#x ", *current_position);  
  }
  fflush(NULL);
  printf("--------------------------------------------------------");
}

void draw_gradient(guchar* start_of_data, int width, int height){
    for(int i=0; i<height; i++) for(int j=0; j<width; j++){
        start_of_data[4*(i*width + j)] = 256*i/height;
        start_of_data[4*(i*width + j)+1] = 256*j/width;
        start_of_data[4*(i*width + j)+2] = 0;
        start_of_data[4*(i*width + j)+3] = 255;
    }
    
}

#endif

#define clip(n, range) (n<0 ? 0 : (n>range ? range : n))

#define position(x,y,c, width, height) (4*(x*width + y)+c)
#define position_clipped(x,y,c, width, height) position(clip(x, width), clip(y, height), c, width, height)

#define position_tensor(x,y,c,d, width, height) (d*4*width*height+4*(x*width + y)+c)


#define forX(width) for(int x=0;x<width;x++)
#define forY(height) for(int y=0;y<height;y++)
#define ford4 for(int d=0;d<4;d++)

// we do not compute anything on the alpha layer and will put it at 255;
#define forC for(int c=0;c<3;c++)

guchar* compute_gradient(guchar* start_of_data, int width, int height){
    guchar *result;

    result = malloc(4 * width * height * sizeof(guchar) * 2);

    forX(width) forY(height) forC
    {
        result[position_tensor(x, y, c, 0, width, height)] =  (start_of_data[position_clipped(x+1, y, c, width, height)]  - start_of_data[position_clipped(x-1, y, c, width, height)])/2 + 128;
        result[position_tensor(x, y, c, 1, width, height)] =  (start_of_data[position_clipped(x, y+1, c, width, height)]  - start_of_data[position_clipped(x, y-1, c, width, height)])/2 + 128;
    }

    //set alpha opaque
    forX(width) forY(height) for(int d=0;d<2;d++) 
        result[position_tensor(x, y, 3, d, width, height)] = 255;

    return(result);
}


guchar* compute_G(guchar* start_of_data, int width, int height){
    // in this tensor, the dumention (4th argument of the macro position_tensor is the coordinate in the G matrix for a given point x,y,c)

    guchar *gradient;
    gradient = compute_gradient(start_of_data, width, height);

    guchar *result;
    result = malloc(4 * width * height * sizeof(guchar) * 4); //4 for the number of colors, 4 for the size of the tensor

    forX(width) forY(height) forC
    {
        result[position_tensor(x, y, c, 0, width, height)] =  (gradient[position_tensor(x, y, c, 0, width, height)] -128 ) *  (gradient[position_tensor(x, y, c, 0, width, height)] -128 ) * 16 / 256 + 128;
        result[position_tensor(x, y, c, 1, width, height)] =  (gradient[position_tensor(x, y, c, 0, width, height)] -128 ) *  (gradient[position_tensor(x, y, c, 1, width, height)] -128 ) * 16 / 256 + 128;
        result[position_tensor(x, y, c, 2, width, height)] =  (gradient[position_tensor(x, y, c, 1, width, height)] -128 ) *  (gradient[position_tensor(x, y, c, 0, width, height)] -128 ) * 16 / 256 + 128;
        result[position_tensor(x, y, c, 3, width, height)] =  (gradient[position_tensor(x, y, c, 1, width, height)] -128 ) *  (gradient[position_tensor(x, y, c, 1, width, height)] -128 ) * 16 / 256 + 128;
    }

    // G is a monocgrome calculation, the components according to the differnet colors are added
    // to display, we will make it grey and hence make redundent data for all colors
    forX(width) forY(height) ford4 {
        int current_sum = 0;

        forC
            current_sum += result[position_tensor(x, y, c, d, width, height)] / 3;
        
        forC
            result[position_tensor(x, y, c, d, width, height)] = current_sum;
    }

    //set alpha opaque
    forX(width) forY(height) ford4 
        result[position_tensor(x, y, 3, d, width, height)] = 255;

    free(gradient);
    return(result);
}

double* compute_eigen(guchar* start_of_data, int width, int height){
    guchar *tensor_G;
    tensor_G = compute_G(start_of_data,  width,  height);


    // I keep 4 colors because I want to represent it 
    // 6 dimensions : lamba_1, lambda_2, u_1, v_1, u_2, v_2
    double *result;
    result = malloc(4 * width * height * sizeof(double) * 6); 


    forX(width) forY(height){
        int trace, determinant;
        double sqrt_delta;
        //polynome charactéristrique
        //(M_a - lambda)(M_d - lambda) - M_b * M_c = 0
        // lambda² - lambda * traceM + determianntM = 0



        trace = tensor_G[position_tensor(x, y, 0, 0, width, height)] + tensor_G[position_tensor(x, y, 0, 3, width, height)];
        
        determinant = tensor_G[position_tensor(x, y, 0, 0, width, height)] * tensor_G[position_tensor(x, y, 0, 3, width, height)]; // a*d
        determinant -= tensor_G[position_tensor(x, y, 0, 1, width, height)] * tensor_G[position_tensor(x, y, 0, 2, width, height)]; // - b*c
        

        sqrt_delta = sqrt(trace*trace - 4*determinant);
        
        forC
        {
            //let's compute thigs 3 times ?
            // lambda 1 and 2
            result[position_tensor(x, y, c, 0, width, height)] = (trace + sqrt_delta)/4;
            result[position_tensor(x, y, c, 1, width, height)] = (trace - sqrt_delta)/4;
        }
    }


    forX(width) forY(height) for(int d=0;d<6;d++)
            result[position_tensor(x, y, 3, d, width, height)] = 255;


    free(tensor_G);
    return(result);

}