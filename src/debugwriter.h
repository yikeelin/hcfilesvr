#ifndef _DEBUGWRITER_H_
#define _DEBUGWRTIER_H_

	void vdebugwrite(const char* szfile, int line, const char* msgFormat, ...);
	void vWriteMsg2File(const char *pBuf, int iLen, const char *pSource);

#endif
