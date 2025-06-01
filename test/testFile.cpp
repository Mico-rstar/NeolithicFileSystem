#include <cassert>
#include <cstring>
#include <iostream>
#include <stdio.h>

#include "../src/file.h"

int main()
{
     File f;
     int s1=f.mkdir("/test");
     printf("s1=%d\n",s1);
    



    file *op=f.open("/test/test.t",true,true);
    if(op!=nullptr)
    {
       f.write(op, "Helloc o World!",  strlen("Helloc o World!"));
       char str[100];
       memset(str, 0, 100);
       op->off=0;
       f.read(op, str,  strlen("Helloc o World!"));
       printf("read=%s\n",str);
       //assert(strcmp(str,"Helloc o World!")==0);


    }
    int s2=f.mkdir("/os");
     printf("s2=%d\n",s2);

     f.tree("/");

     fsstat fstat=f.fstat();

     
     printf("cnt=%d\n", fstat.nfb);
}