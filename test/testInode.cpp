#include <cassert>
#include <cstring>
#include <iostream>
#include <stdio.h>

#include "../src/inode.h"

using namespace std;

void testBasic()
{
    Inode in;
    in.logger().beginOP();
    inode &i = in.ialloc(InodeStructure::FILE);
    in.ilock(i);
    uint bn0 = in.bmap(i, 0);
    uint bn1 = in.bmap(i, 1);
    buf &b = in.logger().read(bn0);
    std::strcpy((char *)b.data, "hello world");
    in.logger().write(b);
    in.logger().relse(b);
    in.iupdate(i);

    in.iunlock(i);
    in.iput(i);

    in.logger().endOP();

    buf &cb = in.logger().read(bn0);
    assert(std::strcmp((char *)cb.data, "hello world") == 0);
    in.logger().relse(cb);
}

void testRW()
{
    Inode in;
    in.logger().beginOP();
    inode &i = in.ialloc(InodeStructure::FILE);
    in.ilock(i);

    int success=in.writei(i, "hello world", 1, 12);
    printf ("write: %d\n", success);
    char str[100];
    in.readi(i, str, 1, 12);
    printf("read: %s\n", str);
    // assert(std::strcmp(str, "hello world hhhhhhhhhhhhhhhh") == 0);

    in.iunlock(i);
    in.iput(i);

    in.logger().endOP();
}

int main()
{
    testRW();
    return 0;
}