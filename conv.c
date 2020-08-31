#include "conv.h"
#include "supportutils.c"
#include <openssl/sha.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

#define TRUE 1
#define FALSE 0
const int mat_size = 3;



struct kernel *r_kernelize(char inp[10]){
    srand(time(0));
    int rnd[9], kernel[9]; int flags[10]={0};
    int x=0, x_coord=0, y_coord=0, lin_coord=0, lb=0;
    struct kernel *kern = (struct kernel*)malloc(sizeof(struct kernel));

    for(int i=0;i<mat_size;i++)
        for(int j=0;j<mat_size;j++)
                kern->W[i][j]=1;

    //create a random ordered key for inserting characters from input at random coordinates in kernel.
    while(TRUE){
        if(x==(mat_size*mat_size)) break;
        int num = (rand() % (((mat_size*mat_size)-1) - lb + 1)) + lb;
        if(flags[num]) continue;
        else{
            flags[num]=1;
            rnd[x] = num;
            x = x + 1;
        }
    }
    
    //create a 3x3 kernel with input characters arranged
    //randomly based on randomized indices list obtained
    //in previous step
    for(int p=0; p<strlen(inp); p++){
        y_coord = rnd[p]%mat_size;
        x_coord = rnd[p]/mat_size;
        kern->W[x_coord][y_coord] = inp[p];
    }
    return kern;
}




struct filedata *filetomatrix(char* fileloc)
{
    long xsize=0,ysize=0;
    struct filedata *thisptr = (struct filedata*)malloc(sizeof(struct filedata));
    FILE* file = fopen(fileloc, "r");
    int c=0;
    if(file==NULL){
        printf("\nFilenotfound exception");
        return NULL;
      }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    printf("\nFilesize: %ld\n", size);

    if(size<10)
    {
        printf("\n Filesize must be greater than 10bytes\n");
        return NULL;
    }
    /*minimize blank spaces in file matrix by rounding
    square root of filesize to nearest integer and adding a row if required*/
    xsize = round(sqrt(size)), ysize=0;
    if(size>(xsize*xsize)) ysize=xsize+1; 
    else ysize=xsize;

    int **frame = (int **)malloc(xsize * sizeof(int *));
    for (int i=0; i<xsize; i++)
        frame[i] = (int *)malloc(ysize * sizeof(int));

    printf("creating file matrix of dimensions: %lux%lu \n", xsize,ysize);
    int bfr = 0;
    for(int i=0;i<xsize;i++)
        for(int j=0;j<ysize;j++){
            if(bfr!=EOF){
                fread(&bfr, sizeof(bfr)+1, 1, file); 
                frame[i][j]=bfr;
            }

            else
                frame[i][j]=0;
        }
    thisptr->dataframe = frame;
    thisptr->x = xsize;
    thisptr->y = ysize;
    thisptr->filesize = size;

    fclose(file);
    printf("\n Filematrix: \n");
    for(int i=0;i<thisptr->x;i++){
        for(int j=0;j<thisptr->y;j++){
            printf("%d ", thisptr->dataframe[i][j]);
        }
        printf("\n");
    }


    return thisptr;
}

int writedatatofile(struct kernel *krn, struct filedata *f, char *filename) //write encrypted matrix to file along with other file data
{
    FILE* filenew = fopen(filename, "w");
    if(filenew==NULL) {
        printf("\nError opening file");
        return 0;
    }
    int counter=0;
    for(int i=0;i<f->x;i++){
        for(int j=0;j<f->y;j++){
            if(counter<=f->filesize){
                int x = f->dataframe[i][j];
                fwrite(&x, sizeof(x), 1, filenew); //dump encrypted data to file
                counter++;
            }
        }
    }
    fclose(filenew);
    printf("\ncombining kernel to create password........");
    unsigned char pswbuf[20], buffer[65],ordbuf[10]={0}, ord[10]={0};
    unsigned char *psw=(unsigned char*)strdup("");
    int tempindex = 0;
    for(int i=0;i<3;i++){
        for(int j=0;j<3;j++){
            char c = krn->W[i][j];
            if(c!=1) pswbuf[tempindex++] = c;
        }
    }

    sha256(pswbuf, buffer);
    char *sizestring = (char *)malloc((sizeof(char *))*20);
    sprintf(sizestring, "%016ld", f->filesize);

    FILE *file = fopen(filename, "a"); 
      char *newfilename = (char *)malloc(sizeof(char *)*20);
      if(strstr(filename, ".")==NULL){
        newfilename = strcat(filename, ".msencrptd");
        fputs("none", file);
      }
      else{
        char *ext = strrchr(filename, '.');
        newfilename = extchng(filename, ".msencrptd");
        fputs(ext, file);
      }

      fputs((const char*)buffer, file);  //append hash
      for(unsigned long int x=0;x<((sizeof(krn->ordarray))/(sizeof(krn->ordarray[0])));x++){
         sprintf((char *)ordbuf, "%d", krn->ordarray[x]); //append secret string
         strcat(ord, ordbuf);
      }

      fputs(sizestring, file);           //append filesize string
      fputs(ord, file);
      
    int rn = rename(filename, newfilename);
    if(rn!=0){
      printf("\nCould not rename file\n");
      return 0;
    }
    fclose(file);

    return 1;
}




int conv(struct kernel *krn, char *filename)
{
    struct filedata *f = filetomatrix(filename);
    if(f==NULL){
      printf("\nError converting file to dataframe\n");
      return 0;
    }


    if(strstr(filename,".msencrptd")){
      printf("File already encrypted");
      return 0;
    }

    int nC = (f->y -3+1), nR = (f->x -3+1);
    printf("# of convolutions: horizontal- %d, Vertical- %d\n", nC, nR);
    int ptrX = 0, ptrY = 0;
    for(int i=0;i<nR;i++){
        ptrY=0;
        for(int j=0;j<nC;j++)
        {
            for(int k=ptrX;k<ptrX+3;k++){
                for(int l=ptrY;l<ptrY+3;l++){
                   // f->dataframe[k][l] = (krn->W[k-ptrX][l-ptrY])*(f->dataframe[k][l]);
                    f->dataframe[k][l] = rotateleft((f->dataframe[k][l]), (krn->W[k-ptrX][l-ptrY]));        //add any reversible encryption function here...
                }
            }
            ptrY++;
        }
        ptrX++;
    }
    int x = writedatatofile(krn, f, filename);
    if(!x){
      printf("\nError occured while writing data to file");
      return 0;
    }

    /*printf("\npost encryption:\n");
    for(int i=0;i<f->x;i++){
      for(int j=0;j<f->y;j++){
        printf("%d ", f->dataframe[i][j]);
      }
      printf("\n");
    }*/
    return 1;
}





int verify(char *pass, char *filename)
{           
    char extension[20], hashstring[65]="",buff[10]="", shabuff[65]= "", sha[65]="", psw[50]="";
    int x_coord =0, y_coord=0; char rnd[9];
    int kern[3][3];
    for(int x=0;x<3;x++)
        for(int y=0;y<3;y++)
            kern[x][y]=1;

    FILE *file = fopen(filename, "r");
    if(file==NULL)
    {
        printf("\nFilenotfound exception");
        return 0;
    }
    
    fseek(file, 0, SEEK_END);
    long size = ftell(file);            //encrypted file size for tracking purposes
    fseek(file, 0, SEEK_SET);

    fseek(file, size-25-64, SEEK_SET);  //set offset to start of filedata appended while encryption
    int coord= ftell(file);  
    for(int i=0;i<64;i++)   hashstring[i]= fgetc(file);

    char *sizestring= (char*)malloc(sizeof(char*)*16);
    for(int i=0;i<16;i++)   sizestring[i]= fgetc(file);

    long ogsize = atoi(sizestring);
    printf("\noriginal filesize: %ld\n", ogsize);
    for(int x=0;x<9;x++) rnd[x]=fgetc(file);

    for(int i=0;i<strlen(pass);i++){
        y_coord = (rnd[i]-48)%mat_size;
        x_coord = (rnd[i]-48)/mat_size;
        kern[x_coord][y_coord] = pass[i];
    }

    char pswbuff[20]= "",temp[10]; int tempindex=0;
    for(int x=0;x<3;x++){
        for(int y=0;y<3;y++){
            int t = kern[x][y];
            if(t!=1) pswbuff[tempindex++] = kern[x][y];
        }
    }

    struct kernel *cur = (struct kernel*)malloc(sizeof(struct kernel));
    memset(cur->W, 0, sizeof(kern[0][0])*9);
    memcpy(cur->W, kern, sizeof(kern));
    sha256(pswbuff, shabuff);
    
    if(strcmp(shabuff, hashstring)!=0)
        return -1;

    else
    {
        int data = 0;
        printf("\npasswords match. Decrypting file...\n");
        fseek(file, 0, SEEK_SET);
        FILE *tempfileptr = fopen("temp", "w");
        for(int i=0;i<ogsize;i++){
            fread(&data, sizeof(data), 1, file); 
            fwrite(&data, sizeof(data), 1, tempfileptr); 
        }
        fclose(tempfileptr);
        int check = deconv(cur, "temp");
       // remove("temp");
        //if(check==1){}
           // rename
    }
    fclose(file);
    return 0;
}


int deconv(struct kernel *krn, char *filename)
{
    struct filedata *f = filetomatrix(filename);

    int nC = (f->y -2), nR = (f->x -2); //filematrix axis size + kernel size - 1
    printf("# of convs: horizontal: %d, Vertical: %d\n", nC, nR);
    int ptrX = f->x-3, ptrY = f->y-3;
    for(int i=nR-1;i>=0;i--){
        ptrY=f->y-3;
        for(int j=nC-1;j>=0;j--){
            for(int k=ptrX;k<ptrX+3;k++){
                for(int l=ptrY;l<ptrY+3;l++){
                    //f->dataframe[k][l] = ((f->dataframe[k][l])/(krn->W[k-ptrX][l-ptrY]));
                   f->dataframe[k][l] = rotateright((f->dataframe[k][l]),(krn->W[k-ptrX][l-ptrY]));
                }
            }
            ptrY--;
        }
        ptrX--;
    }
    return 1;
}




