#include "stdafx.h"
#include "Library.h"

#ifdef WIN32
#include <atlstr.h>//For CString
#endif

RLibrary::RLibrary(const std::string &strFilename)
: m_strFilename(strFilename)
{
}

RLibrary::RLibrary( const CString& strFilename )
{
   CT2CA pszConvertedAnsiString( strFilename );
   m_strFilename = std::string( pszConvertedAnsiString );
}


bool RLibrary::Load()
{
#ifdef WIN32
	m_hModule = LoadLibraryW(CStringW(m_strFilename.c_str()));
#else
	m_hModule = dlopen(m_strFilename.c_str(), RTLD_LAZY);
#endif

	return m_hModule != NULL;
}

void* RLibrary::Resolve(const char* pstrExport)
{
	void *ptrRet;

#ifdef WIN32
	ptrRet = GetProcAddress(m_hModule, pstrExport);
#else
	ptrRet = dlsym(m_hModule, pstrExport);
#endif

	return ptrRet;
}



