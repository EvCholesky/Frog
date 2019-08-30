#pragma once
#include "Common.h"

u32 Frog_NSuperFastHashLower (const char * data, size_t len);

class FrStringTable;

inline u32 Frog_HvFromPchz(const char * pChz, size_t cB = 0)
	{
		if (!pChz)
			return 0;
		if (cB == 0)
			cB = CBCoz(pChz);

		return Frog_NSuperFastHashLower(pChz, cB);
	}

inline u32 Frog_HvFromAB(const void * aB, size_t cB)
	{
		return Frog_NSuperFastHashLower((const char *)aB, cB);
	}

// string hash class tracks a (possibly invalid) pointer to the source string
#define FR_SHASH_DEBUG_POINTER 1

// string hash class stores a full copy of the string for debug purposes
#define FR_SHASH_DEBUG_COPY	 0

struct FrStringHash // tag=shash
{
public:

	u32 m_hv;

#if FR_SHASH_DEBUG_POINTER 
	const char * m_pChzDebugSource; // for debug only! may be invalid!
#elif FR_SHASH_DEBUG_COPY
	char m_aDebugSourceString[256];
#endif
};

inline FrStringHash ShashCreate()
	{
		FrStringHash shash;
		shash.m_hv = 0;
	#if FR_SHASH_DEBUG_POINTER
		shash.m_pChzDebugSource = nullptr;
	#elif FR_SHASH_DEBUG_COPY
		shash.m_aDebugSourceString[0] = '\0';
	#endif
		return shash;
	}

inline FrStringHash ShashCreate(const char * pChz)
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

inline FrStringHash ShashCreateLen(const char * pChz, size_t cB)
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

inline const char * PChzDebug(FrStringHash * pShash)
{
#if FR_SHASH_DEBUG_POINTER
	return pShash->m_pChzDebugSource;
#elif FR_SHASH_DEBUG_COPY
	return pShash->m_aDebugSourceString;
#else
	return "UnknownHash";
#endif
}


