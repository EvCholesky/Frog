#include "commonCpp.h"

#include <cstdarg>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FROG_CALL void FrAssertHandler(const char* pChzFile, u32 line, const char* pChzCondition, const char* pChzMessage, ... )
{
	printf("Assertion failed: \"%s\" at %s:%u\n", pChzCondition, pChzFile, line);

	if (pChzMessage)
	{
		va_list ap;
		va_start(ap, pChzMessage);
		vprintf(pChzMessage, ap);
		va_end(ap);
		printf("\n");
	}
}

FROG_CALL void ZeroAB(void * pDest, size_t cB)
{
	memset(pDest, 0, cB);
}

FROG_CALL void FillAB(u8 b, void * pDest, size_t cB)
{
	memset(pDest, b, cB);
}

FROG_CALL void CopyAB(const void * pSource, void * pDest, size_t cB)
{
	memcpy(pDest, pSource, cB);
}

void ShowErrorV(const char * pChzFormat, va_list ap)
{
	printf("Error: ");
	vprintf(pChzFormat, ap);
}

void ShowError(const char* pChzFormat, ...)
{
	va_list ap;
	va_start(ap, pChzFormat);
	ShowErrorV(pChzFormat, ap);
	va_end(ap);
}

void ShowInfoV(const char * pChzFormat, va_list ap)
{
	printf("Error: ");
	vprintf(pChzFormat, ap);
}

void ShowInfo(const char * pChzFormat, ...)
{
	va_list ap;
	va_start(ap, pChzFormat);
	ShowInfoV(pChzFormat, ap);
	va_end(ap);
}
// simple hash function with meh avalanching, works for now. replace with MUM hash or xxHash
u32 HvFromPBFVN(const void * pV, size_t cB)
{
	auto pB = (u8*)pV;
    u32 hv = 2166136261;

	for (size_t iB=0; iB < cB; ++iB)
    {
        hv = (hv * 16777619) ^ pB[iB];
    }

    return hv;
}

u32 HvConcatPBFVN(u32 hv, const void * pV, size_t cB)
{
	auto pB = (u8*)pV;
	for (size_t iB=0; iB < cB; ++iB)
    {
        hv = (hv * 16777619) ^ pB[iB];
    }

    return hv;
}

void SplitFilename(const char * pChzFilename, size_t * piBFile, size_t * piBExtension, size_t * piBEnd)
{
	ptrdiff_t iBFile = 0;
	ptrdiff_t iBExtension = -1;
	const char * pChIt = pChzFilename;
	for ( ; *pChIt != '\0'; pChIt += CBCodepoint(pChIt))
	{
		if (*pChIt == '\\')
		{
			iBFile = (pChIt+1) - pChzFilename;
			iBExtension = -1;	// only acknowledge the first '.' after the last directory
		}
		else if (iBExtension < 0 && *pChIt == '.')
		{
			iBExtension = pChIt - pChzFilename;
		}
	}

	*piBFile = iBFile;
	*piBEnd = pChIt - pChzFilename;
	*piBExtension = (iBExtension < 0) ? *piBEnd : iBExtension;
}

void ConcatPChz(const char* pChzA, const char * pChzB, char * pChOut, size_t cChOutMax)
{
	char* pChzOutIt = pChOut;
	char* pChzOutEnd = &pChOut[cChOutMax-1];

	const char* pChzInIt = pChzA;
	while((*pChzInIt != '\0') & (pChzOutIt != pChzOutEnd))
		*pChzOutIt++ = *pChzInIt++;

	pChzInIt = pChzB;
	while((*pChzInIt != '\0') & (pChzOutIt != pChzOutEnd))
		*pChzOutIt++ = *pChzInIt++;

	*pChzOutIt = '\0';
}

size_t CBCoz(const char * pCoz)
{
	// bytes needed for this string (including the null terminator)
	if (!pCoz)
		return 0;

	auto pCozIt = pCoz;
	while (*pCozIt != '\0')
		++pCozIt;

	++pCozIt;
	return pCozIt - pCoz;
}

struct SStringBuffer // tag=strbuf
{
			SStringBuffer()
			:m_pCozBegin(nullptr)
			,m_pCozAppend(nullptr)
			,m_cBMax(0)
				{ ; }

			SStringBuffer(char * pCoz, size_t cBMax)
			:m_pCozBegin(pCoz)
			,m_pCozAppend(pCoz)
			,m_cBMax(cBMax)
				{ ; }

	char *	m_pCozBegin;
	char *	m_pCozAppend;
	size_t	m_cBMax;
};

inline bool FIsValid(const SStringBuffer & strbuf)
{
	return strbuf.m_pCozBegin != nullptr && strbuf.m_pCozAppend != nullptr;
}

static inline bool FIsBasicChar(char ch)	{ return (ch & 0x80) == 0;}
static inline bool FIsStarterChar(char ch)	{ return (ch & 0xC0) == 0xC0;}

void EnsureTerminated(SStringBuffer * pStrbuf, char ch)
{
	size_t iB = pStrbuf->m_pCozAppend - pStrbuf->m_pCozBegin;

	if (pStrbuf->m_cBMax <= 0)
		return;

	iB =  Frog_NMin(iB, pStrbuf->m_cBMax -1);

	auto pCozEnd = &pStrbuf->m_pCozBegin[iB];
	auto pCozBackup = pCozEnd; // - 1;	// skip the spot our terminator will go

	while (pCozBackup != pStrbuf->m_pCozBegin)
	{
		--pCozBackup;
		if (FIsBasicChar(*pCozBackup) || FIsStarterChar(*pCozBackup))
			break;
	}

	size_t cBBackup;
	if ((*pCozBackup & 0xF8) == 0xF0)		cBBackup = 4;
	else if ((*pCozBackup & 0xF0) == 0xE0)	cBBackup = 3;
	else if ((*pCozBackup & 0xE0) == 0xC0)	cBBackup = 2;
	else									cBBackup = 1;

	if (cBBackup > size_t(pCozEnd - pCozBackup))
	{
		*pCozBackup = ch;
	}
	else
	{
		*pCozEnd = ch;
	}
}


void AppendCoz(SStringBuffer * pStrbuf, const char *pCozSource)
{
	if (!FR_FVERIFY(pCozSource && FIsValid(*pStrbuf), "Null pointer passed to CCoCopy"))
		return;

	char * pCozDest = pStrbuf->m_pCozAppend;
	char * pCozDestEnd = &pStrbuf->m_pCozBegin[pStrbuf->m_cBMax-1];
	for ( ; (*pCozSource != '\0') & (pCozDest != pCozDestEnd); ++pCozSource, ++pCozDest)
	{
		*pCozDest = *pCozSource;
	}

	pStrbuf->m_pCozAppend = pCozDest;
	EnsureTerminated(pStrbuf, '\0');
}


size_t	CBCopyCoz(const char * pCozSource, char * aCoDest, size_t cBDest)
{
	SStringBuffer strbuf(aCoDest, cBDest);
	AppendCoz(&strbuf, pCozSource);
	return strbuf.m_pCozAppend - strbuf.m_pCozBegin + 1;
}
