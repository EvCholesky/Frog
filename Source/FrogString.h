#pragma once
#include "Common.h"

u32 Frog_NSuperFastHashLower (char * data, size_t len);

inline u32 Frog_HvFromPchz(char * pChz, size_t cB)
	{
		if (!pChz)
			return 0;
		if (cB == 0)
			cB = CBCoz(pChz);

		return Frog_NSuperFastHashLower(pChz, cB);
	}

inline u32 Frog_HvFromAB(void * aB, size_t cB)
	{
		return Frog_NSuperFastHashLower((char *)aB, cB);
	}

// string hash class tracks a (possibly invalid) pointer to the source string
#define FR_SHASH_DEBUG_POINTER 1

// string hash class stores a full copy of the string for debug purposes
#define FR_SHASH_DEBUG_COPY	 0

typedef struct FrStringHash_tag // tag=shash
{
	u32 m_hv;

#if FR_SHASH_DEBUG_POINTER 
	char * m_pChzDebugSource; // for debug only! may be invalid!
#elif FR_SHASH_DEBUG_COPY
	char m_aDebugSourceString[256];
#endif
} FrStringHash;

inline FrStringHash ShashCreateEmpty()
	{
		FrStringHash shash;
		shash.m_hv = 0;
	#if FR_SHASH_DEBUG_POINTER
		shash.m_pChzDebugSource = NULL;
	#elif FR_SHASH_DEBUG_COPY
		shash.m_aDebugSourceString[0] = '\0';
	#endif
		return shash;
	}

inline FrStringHash ShashCreate(char * pChz)
	{
		FrStringHash shash;
		shash.m_hv = Frog_HvFromPchz(pChz, 0);
	#if FR_SHASH_DEBUG_POINTER
		shash.m_pChzDebugSource = pChz;
	#elif FR_SHASH_DEBUG_COPY
		CBCopyCoz(m_aDebugSourceString, pChz, FR_DIM(m_aDebugSourceString));
	#endif
		return shash;
	}

inline FrStringHash ShashCreateLen(char * pChz, size_t cB)
	{
		FrStringHash shash;
		shash.m_hv = Frog_HvFromPchz(pChz, cB);
	#if FR_SHASH_DEBUG_POINTER
		shash.m_pChzDebugSource = pChz;
	#elif FR_SHASH_DEBUG_COPY
		CBCopyCoz(m_aDebugSourceString, pChz, FR_DIM(m_aDebugSourceString));
	#endif
		return shash;
	}

inline char * PChzDebug(FrStringHash * pShash)
{
#if FR_SHASH_DEBUG_POINTER
	return pShash->m_pChzDebugSource;
#elif FR_SHASH_DEBUG_COPY
	return pShash->m_aDebugSourceString;
#else
	return "UnknownHash";
#endif
}


typedef struct FrEditbuffer_t // tag = freb
{
	char *		m_pChzMin;
	char *		m_pChzMax;
	char *		m_pChzAppend;
} FrEditBuffer;

FROG_CALL void Freb_init(FrEditBuffer * pFreb, char * pChMin, char * pChMax);
FROG_CALL void Freb_Clear(FrEditBuffer * pFreb);
FROG_CALL void Freb_AppendCh(FrEditBuffer * pFreb, const char * pChz, size_t cB);
FROG_CALL void Freb_AppendChz(FrEditBuffer * pFreb, const char * pChz);
FROG_CALL void Freb_Printf(FrEditBuffer * pFreb, const char * pChzFormat, ...);
FROG_CALL void Freb_AppendChzWithPad(FrEditBuffer * pFreb, const char * pChz, char ch, size_t cChPad);
FROG_CALL size_t Freb_CCh(FrEditBuffer * pFreb);



