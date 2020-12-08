#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<openssl/sha.h>
#include <stdint.h>
#include <limits.h>

#define INTSIZE 8 //bits


char newfilename[10];
char *extchng(char *initname, char *newExt)
{
    int dotfound=0, dotloc = 0;
    for(int i=strlen(initname)-1; i>=0;i--)
    {
        if(initname[i]=='.')
         {
             dotloc = i;
             dotfound = 1;
             break;
         }   
    }

    if(dotfound)
    {
        for(int i=0;i<dotloc;i++) newfilename[i]=initname[i];
        strcat(newfilename, newExt);
        printf("\nNewfilename: %s\n", newfilename);
        return newfilename;
    }
    else
        return initname;
}

char *strrev(char *str)
{
      char *p1, *p2;

      if (! str || ! *str)
            return str;
      for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
      {
            *p1 ^= *p2;
            *p2 ^= *p1;
            *p1 ^= *p2;
      }
      return str;
}

int letterscaledown(char c)
{
    if(c>=97)return c-96; 
    if(c>=65 && c<=90) return c-64;
    return c;
}
int fileintegritycheck(char *fll, struct filedata *fd)
{
    FILE* curfile = fopen(fll, "rb");
    int counter = 0; char c;
    for(int i=0;i<fd->x;i++){
        for(int j=0;j<fd->y;j++){
            c = fgetc(curfile);
            if(c!=EOF){
                if(c!=fd->dataframe[i][j])
                {
                    printf("\nBit %c does not match\n", c);
                    fclose(curfile);
                    return 0;
                }
            }
            if(c==EOF)
                break;
            else
                continue;
        }
    }
    printf("\nFiles match\n");
    fclose(curfile);
    return 1;
}

void sha256(unsigned char *pass, unsigned char outbuff[65])
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, pass, strlen((const char*)pass));
    SHA256_Final(hash, &sha256);
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        sprintf((char*)outbuff + (i * 2), "%02x", hash[i]);
    outbuff[64] = 0;
}

int rotateleft (int value, int count) {
    int buf[INTSIZE]={0};
    int x = 0; 
    while (value > 0) { 
        buf[INTSIZE-(x++)-1] = value % 2;
        value = value / 2;
    }

    for(int p=0;p<count;p++){
        int temp = buf[0], i; 
        for (i = 0; i < INTSIZE-1; i++) 
            buf[i] = buf[i + 1]; 
        buf[i] = temp; 
    }

    x=0;
    for(int i=0;i<INTSIZE;i++) x = x + (buf[INTSIZE-i-1] * pow(2, i));
    return x;
}


int rotateright (int value, int count) {
    int buf[INTSIZE]={0}, bits[INTSIZE]={0};
    int x = 0; 
    while (value > 0) { 
        bits[INTSIZE-(x++)-1] = value % 2;
        value = value / 2;
    }

    for(int p=0;p<count;p++){
        int temp = bits[INTSIZE-1];
        for (int j = INTSIZE-1;j>0; j--)
            bits[j]=bits[j-1];
        bits[0] = temp;
    }

    x=0;
    for(int i=0;i<INTSIZE;i++) x = x + (bits[INTSIZE-i-1] * pow(2, i));
    return x;
}
