#include<stdio.h>
#include<stdlib.h>
#include<string.h>


int main(int argc, char **argv)
{
    int x = 45, y = 46, z = 47;
    FILE *f = fopen("testfile.txt", "w");
    fwrite(&x,1,1,f);
    fwrite(&y,1,1,f);
    fwrite(&z,1,1,f);
}