#include <stdlib.h>
#include <string.h>
#include <stdio.h>
struct abc{
	char as1[10];
	char as2[5];
}abc1;
void tst(char *s);

int main( int argc, char* argv[])
{
	ERR_EXIT("ABC");
	struct abc *pabc;
	pabc = &abc1;
	char s1[10];
	s1[9] = 0;
	memset( &s1, 'a', 9 );
	printf( "%s\n", s1);
	char s2[5];
	s2[4] = 0;
	memset( s2, 'b', 4);
	memcpy( s1, s2, strlen(s2) );
	printf( "s1=%s, strlen(s2) = %d\n", s1, strlen(s2));
	printf( "sizeof(abc1.as1) = %d\n",sizeof(abc1.as1));  
	printf( "sizeof(pabc->as1) = %d\n",sizeof(pabc->as1));  
	tst(s1);
}

void tst(char *s)
{
	printf ("sizeof s= %d\n", sizeof(s));
}
