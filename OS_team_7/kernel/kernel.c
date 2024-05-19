#include "include_main.h"




int main()
{
    print_minios("[team 7 top command] Hello, World!");

    char *input;
    int system(const char *str);

    while(1) 
    {
        input = readline("커맨드를 입력하세요(종료:q) : ");
        
        if (strcmp(input,"q")==0)
        {
        	break;
        }
        if (strcmp(input,"t")==0)
        {
        	top();
        }
        else system(input);
        

    }
        free(input);
        print_minios("[team 7 top command] TOP command Shutdown........");
        return(1);
}



void print_minios(char* str) 
{
        printf("%s\n",str);
}
