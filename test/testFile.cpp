#include <cassert>
#include <cstring>
#include <iostream>
#include <stdio.h>

#include "../src/file.h"

int main()
{
     File file;
     int s1=file.mkdir("/test");
     printf("s1=%d\n",s1);

     int s2=file.mkdir("/test/test1");
     printf("s2=%d\n",s2);

     int s3=file.mkdir("/os/oss");
     printf("s3=%d\n",s3);

     int s4=file.unlink( "/test/test1");
      printf("s4=%d\n",s4);

     file.tree("/");
}