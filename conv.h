#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <Windows.h>
#include <stdbool.h>
#include <fcntl.h>
#include <time.h>
#include <math.h>
#include <stdint.h>

struct kernel   //fixed kernel size for demo purposes. Create a struct similar to filedata struct for variable length kernel.
{
    int W[3][3];
    char ordarray[9];
};

struct filedata
{
    int x;
      unsigned long filesize;
    int y;
      int **dataframe;    //pointer to a 2d matrix of size (X x Y) for converting file to matrix
};

struct kernel *r_kernelize(char inp[10]);
struct filedata *filetomatrix(char* fileloc, char*optype);
int conv(struct kernel *cur, char *filename);
int deconv(struct kernel *krn, char *filename);
int verify(char *pass, char *filename);
int fileintegritycheck(char *fll, struct filedata *fd);
