#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#pragma pack(push, 1)

typedef struct {
	unsigned short bType;
	unsigned int bfSize;
	unsigned short bfReserved1;
	unsigned short bfReseved2;
	unsigned int bfOffBits;
} BitmapFileHeader;


typedef struct {
	unsigned int biSize;
	unsigned int biWidth;
	unsigned int biHeight;
	unsigned short biPlanes;
	unsigned short biBitCount;
	unsigned int biCompression;
	unsigned int biSizeImage;
	unsigned int biXPelsPerMeter;
	unsigned int biYpelsPerMeter;
	unsigned int biClrUsed;
	unsigned int biClrImportant;
} BitmapInfoHeader;

typedef struct {
	unsigned char r;
	unsigned char g;
	unsigned char b;
} RGB;

typedef struct {
	BitmapFileHeader bmfh;
	BitmapInfoHeader bmih;
	RGB** data;
} BMP;

#pragma pack(pop)

typedef struct {
    int x_left_top, y_left_top, x_right_bottom, y_right_bottom;
    int x_centre, y_centre;
    int radius;
    int thickness;
    int isFilled;
    int r, g, b;
    int fr, fg, fb;
    char* component;
    int value;
    int xParts, yParts;
    int height, width;
    char* path;
    char* output;
    int help, info;
    int DrawCirlceByRadius, DrawCirlceBySquare, rgbFilter, DrawParts, SaveParts,outFill;    
} configs;


int readBMP(BMP* img, const char* path)
{  
    if (path[strlen(path)-1] != 'p' || path[strlen(path)-2] != 'm' || path[strlen(path)-3] != 'b' 
    || path[strlen(path)-4] != '.'){
        puts("Wrong format, maybe it`s not bmp-file.");
        return 0;
    }


    FILE* file = fopen(path, "rb");

    if(file == NULL){
        puts("There is no such file.");
        return 0;
    }

    fread(&img->bmfh, 1, sizeof(BitmapFileHeader), file);
    fread(&img->bmih, 1, sizeof(BitmapInfoHeader), file);
    
    if(img->bmfh.bType != 0x4d42){
        puts("Invalid format.");
        return 0;
    }

    if(img->bmih.biSize != 40){
        puts("Invalid version.");
        return 0;
    }
    
    if(img->bmih.biCompression != 0){
        puts("Your file shouldn`t be compressed");
        return 0;
    }
    
    if(img->bmih.biBitCount != 24){
        puts("Your picture doesn`t have 24 pixels per bit.");
        return 0;
    }
    
    unsigned int H = img->bmih.biHeight;
    unsigned int W = img->bmih.biWidth;
    unsigned int offset = (4 - (W * 3) % 4) % 4;
    unsigned int w = sizeof(RGB)*W + offset;
    img->data = malloc(sizeof(RGB*)*H);
    if(img->data == NULL){
        puts("Memory is not available.");
        return 0;
    }
    for(int i = 0; i < H; i++)
    {
        img->data[i] = malloc(w);
        if (img->data[i] == NULL){
            puts("Memory is not available.");
            return 0;
        }
        fread(img->data[i], 1, w, file);
    }

    fclose(file);

    return 1;
}

int writeBMP(const char* path, BMP img)
{

    if (path[strlen(path)-1] != 'p' || path[strlen(path)-2] != 'm' || path[strlen(path)-3] != 'b' 
    || path[strlen(path)-4] != '.'){
        puts("Wrong format.");
        return 0;
    }

    FILE* file = fopen(path, "wb");

    if(file == NULL){
        puts("Can`t write this file.");
        return 0;
    }

    fwrite(&img.bmfh, 1, sizeof(BitmapFileHeader), file);
    fwrite(&img.bmih, 1, sizeof(BitmapInfoHeader), file);

    unsigned int H = img.bmih.biHeight;
    unsigned int W = img.bmih.biWidth;
    unsigned int offset = (4 - (W * 3) % 4) % 4;
    unsigned int w = sizeof(RGB)*W + offset;

    for(int i = 0; i < H; i++)
    {
        fwrite(img.data[i], 1, w, file);
    }

    fclose(file);

    return 1;
}    

void setPixel(BMP img, int x, int y, int r, int g ,int b){
    if(x>=0 && x < img.bmih.biWidth && y >= 0 && y < img.bmih.biHeight){
        img.data[y][x].r = r;
        img.data[y][x].g = g;
        img.data[y][x].b = b;
    } else {
        puts("Your pixel is out of picture.");
        return;
    }
}

void drawCircle(BMP img, int x0, int y0, int radius,int thickness, int Fill, int r, int g, int b, int fR, int fG, int fB){
     
    if(x0 < 0 || x0 >= img.bmih.biWidth || y0 >= img.bmih.biHeight || y0 < 0 || x0+((radius+(thickness)/2)) >= img.bmih.biWidth || x0-((radius+(thickness)/2)) < 0 
    || y0+radius+((thickness)/2) >= img.bmih.biHeight || y0-(radius+((thickness)/2)) < 0 || (Fill != 1 && Fill != 0)){
        puts("Invalid value.");
        return;
    }
    
    if ( thickness < 1 || thickness > radius){
        puts("Invalid thickness value.");
        return;
    }
    if (r > 255 || r < 0 || g > 255 || g < 0 || b > 255 || b < 0 || fR < 0 || fR > 255 || fG < 0 || fG > 255 || fB < 0 || fB > 255){
        puts("Invalid color value.");
        return;
    }

     if(Fill == 1){
        double inRadius = radius-(double)((double)thickness/2);
        double x1 = x0 - inRadius;
        double y1 = y0 - inRadius;
        double x2 = x0 + inRadius;
        double y2 = y0 + inRadius;
        for(int i = y1; i < y2; i++){
            for(int j = x1; j < x2; j++){
                if ((sqrt(((j-x0)*(j-x0)) + ((i-y0)*(i-y0))) <= inRadius)){
                    setPixel(img, j, i, fR, fG, fB);
                }
            }
        }   
    }

    if (thickness == 1){
        int x = 0;
        int y = radius;
        int D = 3-2*radius;
        while (x<=y)
        {
                setPixel(img, x0+x, y0+y, r, g, b);
                setPixel(img, x0+x, y0-y, r, g, b);
                setPixel(img, x0-x, y0+y, r, g, b);
                setPixel(img, x0-x, y0-y, r, g, b);
                setPixel(img, x0+y, y0+x, r, g, b);
                setPixel(img, x0+y, y0-x, r, g, b);
                setPixel(img, x0-y, y0+x, r, g, b);
                setPixel(img, x0-y, y0-x, r, g, b);
                if (D <= 0){
                    D += 4*x+6;
                    x+=1;
                } else {
                    x+=1;
                    y-=1;
                    D += 4*(x-y)+10;
                }          
        }
        
    }
    if (thickness > 1){
        double inRadius = radius-(double)((double)thickness/2);
        double outRadius = radius+(double)((double)thickness/2);
        double x1 = x0-outRadius;
        double x2 = x0+outRadius;
        double y1 = y0-outRadius;
        double y2 = y0+outRadius;
        for(int i = y1; i < y2; i++){
            for(int j = x1; j<x2; j++){
                if((sqrt(((j-x0)*(j-x0)) + ((i-y0)*(i-y0))) < outRadius) && (sqrt(((j-x0)*(j-x0)) + ((i-y0)*(i-y0))) >= inRadius)){
                    setPixel(img, j, i, r, g, b);
                }
            }
        }
    }
      
}

void drawcircleSq(BMP img, int x1, int y1, int x2, int y2,int thickness, int Fill, int r, int g, int b, int fR, int fG, int fB){
    if(x1 < 0 || x1 >= img.bmih.biWidth || x2 >= img.bmih.biWidth || x2 < 0){
        puts("Your coordinates go outside the picture.");
        return;
    }
    
    if(y1 < 0 || y1 >= img.bmih.biHeight || y2 >= img.bmih.biHeight || y2 < 0){
        puts("Invalid coordinates value to draw cirlce.");
        return;
    }

    if (x2-x1 != y1-y2){
        puts("Your values are not symmetrical.");
        return;
    }

    int radius = (x2-x1+1)/2;
    int x0 = x2-radius;
    int y0 = y1-radius;
    if((x2-x1+1)%2==0 && (x1-1<0 || y1-1<0)){
        drawCircle(img, x0+1, y0+1, radius, thickness, Fill, r, g, b, fR, fB, fG);
        return;
    }
    drawCircle(img, x0, y0, radius, thickness, Fill, r, g, b, fR, fB, fG);
}

void rgbFilter(BMP img, int value,const char* component){
    if(value != 0 && value != 255){
        puts("Invalid value.");
        return;
    }
    if(strcmp(component, "red") != 0 && strcmp(component, "green") != 0 && strcmp(component, "blue") != 0){
        puts("Invalid color-component.");
        return;
    }
    for(unsigned int i = 0; i < img.bmih.biHeight; i++){
        for(unsigned int j = 0; j < img.bmih.biWidth; j++){
            if (strcmp("red", component) == 0){
                img.data[i][j].r = value;
            }
            if (strcmp("green", component) == 0){
                img.data[i][j].g = value;
            }
            if (strcmp("blue", component) == 0){
                img.data[i][j].b = value;
            }
        }
    }
}

void parts(BMP img, int xParts, int yParts, int thickness, int r, int g, int b){
    if(thickness < 1){
        puts("Invalid thickness value.");
        return;
    }
    if(xParts < 1 || yParts < 1){
        puts("Invalid some part value.");
        return;
    }

    unsigned int H = img.bmih.biHeight;
    unsigned int W = img.bmih.biWidth;
    if ((xParts * thickness)+xParts > W  || (yParts * thickness)+yParts > H ) {
        puts("Invalid values to divide into parts.");
        return;
    }
    int x = 0;
    int y = 0;
    unsigned int yP = H/yParts;
    unsigned int xP = W/xParts;

    while (x<=W)
    {
        for(int i = 0; i < thickness; i++){
            for(int j = 0; j < H; j++){
                if(x+(thickness/2-i) >= 0 && x+(thickness/2-i)<W){
                    img.data[j][x+(thickness/2-i)].r = r;
                    img.data[j][x+(thickness/2-i)].g = g;
                    img.data[j][x+(thickness/2-i)].b = b;
                }
            }
        }
        x+=xP;
    }
    while (y<=H)
    {
        for(int i = 0; i < thickness; i++){
            for(int j = 0; j < W; j++){
                if(y+(thickness/2-i) >= 0 && y+(thickness/2-i)<H){
                    img.data[y+(thickness/2-i)][j].r = r;
                    img.data[y+(thickness/2-i)][j].g = g;
                    img.data[y+(thickness/2-i)][j].b = b;
                }
            }
        }
        y+=yP;
    }    
}

void save(BMP img, int x1, int y1, int x2, int y2, char* path){
    img.bmih.biHeight = y2-y1;
    img.bmih.biWidth = x2-x1;
    if(path[strlen(path) - 1] != 'p' || path[strlen(path) - 2] != 'm'
    || path[strlen(path) - 3] != 'b' || path[strlen(path) - 4] != '.'){
        puts("Wrong format of output file.");
        return;
    }
    FILE* file = fopen(path, "wb");
    if (file == NULL){
        puts("Can`t save this file.");
        return;
    }
    fwrite(&img.bmfh, 1, sizeof(BitmapFileHeader), file);
    fwrite(&img.bmih, 1, sizeof(BitmapInfoHeader), file);

    size_t H = img.bmih.biHeight;
    size_t W = img.bmih.biWidth;
    size_t offset = (4 - (W * 3) % 4) % 4;
    size_t w = sizeof(RGB)*W + offset;

    RGB** tmpData = malloc(sizeof(RGB*)*H);
    if(tmpData == NULL){
        puts("Memory is not available.");
        return;
    }
    for(int i = 0; i < H; i++){
        tmpData[i] = malloc(sizeof(RGB)*W);
        if(tmpData[i] == NULL){
            puts("Memory is not available.");
            return;
        }
    }
    
    for(int j = y1; j < y2; j++){
        for(int k = x1; k < x2; k++){
            tmpData[j-y1][k-x1] = img.data[j][k];
        }
    }

    for(int i = 0; i < H; i++){
        fwrite(tmpData[i], 1, w, file);
    }

    fclose(file);

}


void saveParts(BMP img, int xParts, int yParts, char* path){
    unsigned int H = img.bmih.biHeight;
    unsigned int W = img.bmih.biWidth;
    if(xParts < 1|| yParts < 1){
        puts("Invalid some part value.");
        return;
    }
    if(xParts > W || yParts > H){
        puts("Invalid some part value.");
        return;
    }
    int x = 0;
    int y = 0;
    int count = 1;
    unsigned int yP = H/yParts;
    unsigned int xP = W/xParts;
    for(int i = y; i < H-yParts; i+= yP){
        for(int j = x; j < W-xParts; j+=xP){
            char* sym = malloc(sizeof(char)*20);
            char* pathN = malloc(sizeof(char)*(strlen(path)+20));
            sprintf(sym, "%d", count);
            strcpy(pathN, path);
            pathN = strcat(pathN, sym);
            pathN = strcat(pathN, ".bmp");
            save(img, j, i, j+xP, i+yP, pathN);
            count++;
        }
    }
}

void freeMem(BMP* img){
    for(unsigned int i = 0; i < img->bmih.biHeight; i++){
        free(img->data[i]);
    }
    free(img->data);
}

void printImageInfo(BMP img){
    printf("Btype:          \t%x\t(%hu)\n", img.bmfh.bType, img.bmfh.bType);
    printf("bfSize:         \t%x\t(%u)\n", img.bmfh.bfSize, img.bmfh.bfSize);
    printf("bfReserved1:    \t%x\t(%hu)\n", img.bmfh.bfReserved1, img.bmfh.bfReserved1);
    printf("bfReserved2:    \t%x\t(%hu)\n", img.bmfh.bfReseved2, img.bmfh.bfReseved2);
    printf("bfOffBits:      \t%x\t(%u)\n", img.bmfh.bfOffBits, img.bmfh.bfOffBits);
    printf("biSize:         \t%x\t(%u)\n", img.bmih.biSize, img.bmih.biSize);
    printf("biWidth:        \t%x\t(%u)\n", img.bmih.biWidth, img.bmih.biWidth);
    printf("biHeight:       \t%x\t(%u)\n", img.bmih.biHeight, img.bmih.biHeight);
    printf("biPlanes:       \t%x\t(%u)\n", img.bmih.biPlanes, img.bmih.biPlanes);
    printf("biBitCount:     \t%x\t(%hu)\n", img.bmih.biBitCount, img.bmih.biBitCount);
    printf("biCompression:  \t%x\t(%u)\n", img.bmih.biCompression, img.bmih.biCompression);
    printf("biSizeImage:    \t%x\t(%u)\n", img.bmih.biSizeImage, img.bmih.biSizeImage);
    printf("biXPelsPerMeter:\t%x\t(%u)\n", img.bmih.biXPelsPerMeter, img.bmih.biXPelsPerMeter);
    printf("biYpelsPerMeter:\t%x\t(%u)\n", img.bmih.biYpelsPerMeter, img.bmih.biYpelsPerMeter);
    printf("biClrUsed:      \t%x\t(%u)\n", img.bmih.biClrUsed, img.bmih.biClrUsed);
    printf("biClrImportant: \t%x\t(%u)\n", img.bmih.biClrImportant, img.bmih.biClrImportant);
}

void printHelp(){

    puts("\tThis program works with BMP files version 3.");
    puts("\tThe program supports files with depth of 24 bits per pixel.");
    puts("\tFiles must not be compressed.");
    puts("\tThe file must match version 3 of the BMP file");
    puts("\tInput examples: ./a.out [input file] [name of function] [key 1] [argument 1], ...., [argument N] ... \n"
         "\t[key N] [argument 1], ... ,[argument N] [name of output file]\n");

    puts("This program has the following functionality:");
    puts("(\t 1-st) Draw cirlce by radius -D(--drawCircleByRadius)");
    puts("\t\tThis function can draw circle by centre coordinates and radius of the circle");
    puts("\t\tFunction needs:");
    puts("\t\t -r(--radius), -t(--thickness), -c(--centre_coordinates), -z(--isFilled), -c(color) -f(--fillColor) keys.");
    puts("(\t 2-nd) Draw circle by square -S(--drawCircleBySquare)");
    puts("\t\tFunction draw circle by square coordinates");
    puts("\t\tFunction needs:");
    puts("\t\t -l(--left_top_coordinates), -b(--right_bottom_coordinates), -t(--thickness), -z(--isFilled), -e(--color), -f(--fillColor) keys.");
    puts("(\t3-th)-F(--rgbFilter):");
    puts("\t\tThis function changes the value of one of the components to 0 or 255.");
    puts("\t\tThis function needs:");
    puts("\t\t-a(--axis) and -v(--value_of_component) keys.");
    puts("(\t4-th)-P(--drawParts)");
    puts("\t\tThis function divides the image with lines.");
    puts("\t\tFunction needs:");
    puts("\t\t-x(--x_Parts), -y(--y_Parst), -t(--thickness), -e(--color) keys.");
    puts("\t5-th)-V(--saveParts)");
    puts("\t\tThis function splits the image saving each part in a separate file.");
    puts("\t\tFunction needs:");
    puts("\t\t-x(--x_Parts), -y(--y_Parst), -w(--path)");
    puts("\t6-th)-L(--outFill)");
    puts("\t\tThis function fill pixels out of rectangle.");
    puts("\t\tFunction needs:");
    puts("\t\t-l(--left_top_coordinates), -u(--width), -p(--height), -e(--color)"); 


    puts("All keys:");
    puts("\t-r(--radius):");
    puts("\t\tGets radius of the cirle.");
    puts("\t-z(--isFilled):");
    puts("\t\tGets fill information.");
    puts("\t-t(--thickness):");
    puts("\t\tGets thickness information.");
    puts("\t-e(--color)");
    puts("\t\tGets information about border color.");
    puts("\t-f(--fillColor)");
    puts("\t\tGets information about fill color.");
    puts("\t-c(--centre_coordinates):");
    puts("\t\tGets information about circle center coordinates.");
    puts("\t-l(--left_top_coordinates):");
    puts("\t\tGets information about left-top square coordinates.");
    puts("\t-b(--right_bottom_coordinates):");
    puts("\t\tGets information about right-bottom coordinates.");
    puts("\t-v(--value_of_component):");
    puts("\t\tGets information about the value to be changed.");
    puts("\t-a(--axis):");
    puts("\t\tGets information about the component to be changed.");
    puts("\t-x(--x_Parts):");
    puts("\t\tGets information about how many parts to divide along the X axis.");
    puts("\t-y(--y_Parst):");
    puts("\t\tGets information about how many parts to divide along the Y axis.");
    puts("\t-w(--path):");
    puts("\t\tGets information about the path where to save parts.");
    puts("\t-h(--help)");
    puts("\t\tGets information about calling a help function.");
    puts("\t-i(--info)");
    puts("\t\tGets information about function-information call.");
    puts("\t-D(--drawCircleByRadius)");
    puts("\t\tGets information about calling the circle-drawing function.");
    puts("\t-S(--drawCircleBySquare)");
    puts("\t\tGets information about calling the circle-drawing function.");
    puts("\t-F(--rgbFilter)");
    puts("\t\tGets information about the rgbFilter function call.");
    puts("\t-P(--drawParts)");
    puts("\t\tGets information about a split function call.");
    puts("\t-V(--saveParts)");
    puts("\t\tGets information about a split function call.");
    puts("\t-L(--outFill)");
    puts("\t\tFill all pixels out of rectangle.");
    puts("Examples:");
    puts("\t\t./a.out example.bmp -F -v 0 - a blue exmp.bmp");
    puts("\t\t./a.out example.bmp -D -r 50 -c 300,300 -o exm.bmp -t 3");
}

int main(int argc, char* argv[]){
    BMP img;
    char infile[100];
    char outfile[100];
    


    if(argc < 2){
        printHelp();
        return 0;
    }

    strcpy(infile, argv[1]);
    strcpy(outfile, argv[argc-1]);

    if(strcmp(infile, "-h") == 0 || strcmp(infile, "--help") == 0){
        printHelp();
        return 0;
    }

    if(argc < 3){
        puts("The program received too few arguments.");
        printHelp();
        return 0;
    }

    char* short_opt = " r:z:t:e:f:c:l:b:v:a:x:y:w:o:p:u:hiDSFPVL";

    const struct option long_opts[] = {
        {"radius", no_argument, NULL, 'r'},
        {"isFilled", required_argument, NULL, 'z'},
        {"thickness", required_argument, NULL, 't'},
        {"color", required_argument, NULL, 'e'},
        {"fillColor", required_argument, NULL, 'f'},
        {"centre_coordinates", required_argument, NULL, 'c'},
        {"left_top_coordinates", required_argument, NULL, 'l'},
        {"right_bottom_coordinates", required_argument, NULL, 'b'},
        {"value_of_component", required_argument, NULL, 'v'},
        {"axis", required_argument, NULL, 'a'},
        {"x_Parts", required_argument, NULL, 'x'},
        {"y_Parst", required_argument, NULL, 'y'},
        {"path", required_argument, NULL, 'w'},
        {"output", required_argument, NULL, 'o'},
        {"height", required_argument, NULL, 'p'},
        {"width", required_argument, NULL, 'u'},
        {"help", no_argument, NULL, 'h'},
        {"info", no_argument, NULL, 'i'},
        {"drawCircleByRadius", no_argument, NULL, 'D'},
        {"drawCircleBySquare", no_argument, NULL, 'S'},
        {"rgbFilter", no_argument, NULL, 'F'},
        {"drawParts", no_argument, NULL, 'P'},
        {"saveParts", no_argument, NULL, 'V'},
        {"outFill", no_argument, NULL, 'L'},
        {NULL, 0, NULL, 0}
    };

    int opt;
    int long_index;
    opt = getopt_long(argc, argv, short_opt, long_opts, &long_index);

    configs config = {-1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, NULL ,-1, -1, -1, -1, -1, NULL, outfile, 0, 0, 0, 0, 0, 0, 0, 0 };
     if(readBMP(&img, infile) == 0){
        puts("Failed to open BMP file.");
        return 0;
    }

    if (opt == -1){
        puts("Invalid input format.");
        return 0;
    }

    int count;
    while (opt != -1){
        switch (opt){
            case 'o':
                config.output = optarg;
                break;
            case 'r':
                count = sscanf(optarg, "%d", &config.radius);
                if(count < 1){
                    puts("Your radius wasn`t read or you didn`t set it.");
                }
                break;
            case 'z':
                count = sscanf(optarg, "%d", &config.isFilled);
                if(count < 1){
                    puts("Your fill-info wasn`t read or you didn`t set it.");
                }
                break;
            case 't':
                count = sscanf(optarg, "%d", &config.thickness);
                if (count < 1){
                    puts("Your thickness wasn`t read or you didn`t set it.");
                }
                break;
            case 'c':
                count = sscanf(optarg, "%d,%d", &config.x_centre, &config.y_centre);
                if(count < 2){
                    puts("Your centre coordinates weren`t read or you didn`t set them.");
                }
                break;
            case 'l':
                count = sscanf(optarg, "%d,%d", &config.x_left_top, &config.y_left_top);
                if(count < 2){
                    puts("Your top coordinates weren`t read or you didn`t set them.");
                }
                break;
            case 'e':
                count = sscanf(optarg, "%d,%d,%d", &config.r, &config.g, &config.b);
                if(count < 3){
                    puts("Your color arguments weren`t read or you didn`t set them, so they would be set as default\n"
                    "(0, 0, 0)");
                }
                break;
            case 'f':
                count = sscanf(optarg, "%d,%d,%d", &config.fr, &config.fg, &config.fb);
                if(count < 3){
                    puts("Your color arguments weren`t read or you didn`t set them, so they would be set as default\n"
                    "(0, 0, 0)");
                }
                break;
            case 'b':
                count = sscanf(optarg, "%d,%d", &config.x_right_bottom, &config.y_right_bottom);
                if(count < 2){
                    puts("Your bottom coordinates weren`t read or you didn`t set them.");
                }
                break;
            case 'v':
                count = sscanf(optarg, "%d", &config.value);
                if(count < 1){
                    puts("Your value wasn`t read or you didn`t set it.");
                }
                break;
            case 'a':
                config.component = malloc(15*sizeof(char));
                count = sscanf(optarg, "%s", config.component);
                if(count == 0){
                    puts("Your axis wasn`t read or you didn`t set it.");
                }
                break;
            case 'x':
                count = sscanf(optarg, "%d", &config.xParts);
                if(count < 1){
                    puts("Your 'X parts' weren`t read or you didn`t set them.");
                }
                break;
            case 'y':
                count = sscanf(optarg, "%d", &config.yParts);
                if(count < 1){
                    puts("Your 'Y parts' weren`t read or you didn`t set them.");
                }
                break;
            case 'w':
                config.path = malloc(140*sizeof(char));
                count = sscanf(optarg, "%s", config.path);
                if(count == 0){
                    puts("Your path wasn`t read or you didn`t set it.");
                }
                break;
            case 'p':
                count = sscanf(optarg, "%d", &config.height);
                if (count < 1){
                    puts("You set wrong height");
                }
                break;
            case 'u':
                count = sscanf(optarg, "%d", &config.width);
                if (count < 1){
                    puts("You set wrong width");
                }
                break;
            case 'h':
                config.help = 1;
                break;
            case 'i':
                config.info = 1;
                break;
            case 'D':
                config.DrawCirlceByRadius = 1;
                break;
            case 'S':
                config.DrawCirlceBySquare = 1;
                break;
            case 'F':
                config.rgbFilter = 1;
                break;
            case 'P':
                config.DrawParts = 1;
                break;
            case 'V':
                config.SaveParts = 1;
                break;
            default:
                puts("No such key.");
                printHelp();
                return 0;
            }
            opt = getopt_long(argc, argv, short_opt, long_opts, &long_index);
    }

    if(config.help + config.info + config.DrawCirlceByRadius + config.DrawCirlceBySquare + config.rgbFilter + config.DrawParts + config.SaveParts > 1){
        puts("You can use only 1 function.");
        return 0;
    }
    if(config.help == 1){
        printHelp();
        return 0;
    }

    if(config.info == 1){
        printImageInfo(img);
        return 0;
    }
    
    if(config.DrawCirlceByRadius == 1){
        if(config.radius == -1){
            puts("Radius was set incorrectly.");
            return 0;
        }
        if(config.x_centre == -1 || config.y_centre == -1){
            puts("Centre coordinates were set incorrectly.");
            return 0;
        }
        if(config.thickness == -1){
            puts("Thicness was set incorrectly.");
            return 0;
        }
        if(config.isFilled == -1){
            puts("Filled argument was set incorrectly.");
            return 0;
        }
        if(config.r == -1 || config.g == -1 || config.b == -1){
            puts("Border collor was set incorrectly");
            return 0;
        }
        drawCircle(img, config.x_centre, config.y_centre, config.radius, config.thickness,config.isFilled ,config.r, config.g, config.b, config.fr, config.fg, config.fb);
    }

    if(config.DrawCirlceBySquare == 1){
        if(config.x_left_top == -1 || config.y_left_top == -1 || config.x_right_bottom == -1 || config.y_right_bottom == -1){
            puts("Cordinates were set incorrectly.");
            return 0;
        } 
        if(config.thickness == - 1){
            puts("Thicness was set incorrectly.");
            return 0;
        }
        if(config.isFilled == -1){
            puts("Filled argument was set incorrectly.");
            return 0;
        }
        if(config.r == -1 || config.g == -1 || config.b == -1){
            puts("Border collor was set incorrectly");
            return 0;
        }
        drawcircleSq(img, config.x_left_top, config.y_left_top, config.x_right_bottom, config.y_right_bottom, config.thickness, config.isFilled, config.r, config.g, config.b, config.fr, config.fg, config.fb);
    }

    if(config.rgbFilter == 1){
        if(config.component == NULL){
            puts("Color component was set incorrectly.");
            return 0;
        }
        if(config.value == -1){
            puts("Value was set incorrectly.");
            return 0;
        }
        rgbFilter(img, config.value, config.component);
    }
    if(config.DrawParts == 1){
        if(config.xParts == -1 || config.yParts == -1){
            puts("xParts or yParts were set incorrectly.");
            return 0;
        }
        if(config.thickness == -1){
            puts("Thicness was set incorrectly.");
            return 0;
        }
        parts(img, config.xParts, config.yParts, config.thickness, config.r, config.g, config.b);
    }

    if(config.SaveParts == 1){
        if(config.xParts == -1 || config.yParts == -1){
            puts("xParts or yParts were set incorrectly.");
            return 0;
        }
        if(config.path == NULL){
            puts("Color component was set incorrectly.");
            return 0;
        }
        saveParts(img, config.xParts, config.yParts, config.path);
        return 0;
    }

    writeBMP(config.output, img);

    return 0;
}  
