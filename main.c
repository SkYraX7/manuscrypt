#include "conv.h"
//Edgecases yet to be handled:
//  filesize less than a threshold or smaller than kernel


int main(int argc, char** argv)
{
    char abs [100], inp[10], pin[10], rel[100];
    struct kernel *cur;
    int flag=1;

    char choice='\0';
    while(flag)
    {   
        printf("\n1. Encrypt   2. Decrypt   3. RevertFile   0. exit\n");
        scanf(" %c", &choice);
        
        if(choice!='1'&&choice!='2'&&choice!='3'&&choice!='0'){
            printf("\nInvalid choice\n");
            continue;
        }
        if(choice=='3') //TEST FUNCTION!! REMOVE DURING FINAL COMMIT
        {
            system("cp 16bytecp.txt 16byte.msencrptd");
            system("mv 16byte.msencrptd 16byte.txt");
            continue;
        }
        if(choice=='0')
            exit(0);

        printf("\nEnter name of the file: \n");
        scanf(" %s", rel);
        printf("Filename: %s\n",rel);

        if(choice=='1'){
            printf("\nEnter numerical pin between 5-9 characters: ");
            scanf(" %s",inp);
            cur = r_kernelize(inp);

            printf("Kernel: \n");
            for(int i=0;i<3;i++){
                for(int j=0;j<3;j++){
                    printf("%d ", cur->W[i][j]);
                }
                printf("\n");
            }

            int con = conv(cur, rel);
            if(con)
                printf("\n File successfully encrypted");
            else
                printf("Conv() returned with error code");
            continue;
        }
        if(choice=='2'){
            if(strstr(rel,".msencrptd")==NULL){
                printf("File not encrypted");
                continue;
            }   
            else{
                printf("\nEnter pin:\n");
                scanf("%s", pin);
                int dec = verify(pin,rel);
                if(dec==-1)
                {
                    printf("\nPasswords dont match\n");
                    continue;
                }
            }
        }

        return 1;
    }
    printf("\nThe End!");
    exit(0);
}
