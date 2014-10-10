/*************************************************************************
    > File Name: test.c
    > Author: ma6174
    > Mail: ma6174@163.com 
    > Created Time: 2014年10月09日 星期四 16时34分15秒
 ************************************************************************/

#include <stdio.h>
#include "image.h"
unsigned int tt(const unsigned char image[])
{
	printf("%d\n",sizeof(image));

}

int main(void)
{	
	tt(gImage_1_1);
	return 0;

}
