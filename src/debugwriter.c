#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#define LOGDIR "../log"
#define DEF_STR_LOGSUFFIX ".dbg"


/*
* example: vdebugwrite(__FILE__, __LINE__, "没有出错 %s,%d", "aaaa", 502);
*/
void vdebugwrite(const char* szfile, int line, const char* msgFormat, ...)
{
	va_list ap;
	int d;
	unsigned int u;
	double f;
	char c, *s, cp;	
	int sub;
	char msg[512];	
	char path[256];
	char *ptr;
	time_t t;
	struct tm* lt;
	char tbuf[128];
	int suffixLen;
	char file[strlen(szfile)+1];
	strcpy( file, szfile );
	
	time( &t ); 	//获取Unix时间戳
	lt = localtime( &t );	//转化时间结构
	memset(tbuf, 0, sizeof(tbuf));
	sprintf(tbuf, "%d-%02d-%02d %02d:%02d:%02d",lt->tm_year+1900,lt->tm_mon,
	lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec);
		
	memset(msg, 0, sizeof(msg));
	sprintf(msg, " at line:%d -->", line);
	
	va_start(ap, msgFormat);
	sub = 0;
	while (*msgFormat)
	{
		cp = *msgFormat++;
		if (cp != '%')
		{
			sub = strlen(msg);
			msg[sub] = cp;
			continue;
		}
		else
		{
			cp = *msgFormat++;
		}
		switch (cp)
		{
			case 's':
				s = va_arg(ap, char*);
				strcat(msg, s);
				break;
			case 'c':
				c = (char)va_arg(ap, int);
				sub = strlen(msg);
				msg[sub] = c;
				break;
			case 'd':
				d = va_arg(ap, int);
				sprintf(msg + strlen(msg), "%d", d);
				break;
			case 'u':
				u = va_arg(ap, unsigned int);
				sprintf(msg + strlen(msg), "%u", u);
				break;
				
			case 'f':
				f = va_arg(ap, double);
				sprintf(msg + strlen(msg), "%f", f);
				break;
			default:
				break;
		}
	}
	va_end(ap);
	
	
	
	memset(path, 0, sizeof(path));
	memcpy(path, LOGDIR, sizeof(LOGDIR));
	
	ptr = strrchr(file, '/');
	
	
	if(ptr)
		memcpy(path + strlen(path), ptr, strlen(ptr));  //最后一个/后面是文件名: xxx.cpp|xxx.pc|xxx
	else
	{
		strcat(path, "/");
		memcpy(path + strlen(path), file, strlen(file));
	}		
	
	suffixLen = 0;		//后缀名长度
	ptr = strrchr(file, '.');
	if(ptr)
	{
		suffixLen = strlen(ptr);
	}	

	memcpy(path + strlen(path) - suffixLen, DEF_STR_LOGSUFFIX, sizeof(DEF_STR_LOGSUFFIX));
	
//	printf ("%s      \n", path);
//	printf("file = %s\n", file);
	FILE* fp = fopen(path, "a+");
	if(fp)
	{	
		fwrite(tbuf, sizeof(char), strlen(tbuf), fp);		
		fwrite(msg, sizeof(char), strlen(msg), fp);	
		fputs("\n", fp);
		
		fclose(fp);
	}else{
		char buf[128];
	//	getcwd(buf, sizeof(buf));
		printf("file open failed, file = %s\n", path );
	}	
}


void vWriteMsg2File(const char *pBuf, int iLen, const char *pSource)
{
	char path[256] = "/home/atmsvr/log/";
	char *pEnv;
	char szFileNameBuf[256];
	memset(szFileNameBuf, 0, 256);
	sprintf( szFileNameBuf, path);
	sprintf( szFileNameBuf + strlen(path), pSource);
	sprintf( szFileNameBuf + strlen(szFileNameBuf),".msg");

	FILE* fp = fopen( szFileNameBuf, "a+" );
	if( !fp )
	{
		return;
	}
	int iSeq = 0;
	char __str[ 1024 ];
	time_t __tmNow = time( NULL );
	struct tm* __pNow = localtime( &__tmNow );
	sprintf( __str, "%04d-%02d-%02d %02d:%02d:%02d %s[%d]\n=========================================================\n", 
	__pNow->tm_year+1900, __pNow->tm_mon + 1, __pNow->tm_mday, __pNow->tm_hour, __pNow->tm_min, __pNow->tm_sec, pSource, iLen );
	unsigned int iIndex;
	for( iIndex = 0; iIndex < iLen;  )
	{
		//000001: 12 34 56 78 AB CD EF 12 34 56 78 12 12 12 12 12 ...............
		const int BYTES_PER_LINE = 16;
		const int SEQ_LEN = 3;

		char szWkBuf[ 128 ], szCharBuf[ BYTES_PER_LINE+1 ];

		sprintf( szWkBuf, "%0*X: ", SEQ_LEN, iSeq++ );
		strcat( __str, szWkBuf );

		strcpy( szCharBuf, "; " );
		unsigned int i;
		for( i = 0; i < BYTES_PER_LINE && iIndex < iLen; ++i )
		{
			unsigned char __by = pBuf[iIndex];
			sprintf( szWkBuf, "%02X ", __by );
			strcat( __str, szWkBuf );
			sprintf( szCharBuf+strlen(szCharBuf), "%c", (__by < 0x20 || __by == 0xff) ? '.' : __by );
			++iIndex;
		}
		
		strcat( szCharBuf, "   \n" );       
		fwrite( __str, sizeof(char), strlen(__str), fp );
		
		__str[ 0 ] = 0;
		if( iIndex % 16 )
		{
			for( i = 0; i < 3 * ( 16 - iIndex % 16 ); ++i )
			{
				fputc( ' ', fp );       
			}
		}
		fwrite( szCharBuf, sizeof(char), strlen(szCharBuf), fp );
	}
	fputs( "\n", fp );
	fclose( fp );
}

// int main()
// {
	// printf("当前源代码函数名：__FUNCTION__==%s\n",__FUNCTION__);
    // printf("当前源代码行号：__LINE__==%d\n",__LINE__);
    // printf("当前源代码文件名：__FILE__==%s\n",__FILE__);
    // printf("当前编译日期〔注意和当前系统日期区别开来〕:__DATE__==%s\n",__DATE__);
    // printf("当前编译时间〔注意和当前系统日期区别开来〕:__TIME__==%s\n",__TIME__);
    // printf("当前系统时间戳：__TIMESTAMP__==%s\n",__TIMESTAMP__);
    // printf("当要求程序严格遵循ANSIC标准时该标识符被赋值为1:__STDC__==%d\n",__STDC__);
    // printf("当用C++编译程序编译时，标识符__cplusplus就会被定义:__cplusplus==%d\n",__cplusplus);
	// return 0;
// }
