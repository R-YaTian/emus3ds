#ifndef	__CAPP_INCLUDED__
#define	__CAPP_INCLUDED__

#define	WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <stdlib.h>
#include "Typedef.h"


#define	ERRORSTRING_MAX	32

class CApp {
public:
	static	void	LoadErrorString();
	static	CHAR*	GetErrorString( INT nID );

protected:
	
	static	INT	m_ErrorStringTableID[ERRORSTRING_MAX];
	static	CHAR	m_ErrorString[ERRORSTRING_MAX][256];
private:
};

// �G���[���b�Z�[�W�p�e���|����
extern	CHAR	szErrorString[256];

#endif	// !__CAPP_INCLUDED__
