// copyright (C) 2014 Evan Christensen

#include "FrogString.h"
#include <ctype.h>
#include <cstdarg>
#include <stdio.h>
#include <string.h>


/* By Paul Hsieh (C) 2004, 2005.  Covered under the Paul Hsieh derivative 
   license. See: 
   http://www.azillionmonkeys.com/qed/weblicense.html for license details.

   http://www.azillionmonkeys.com/qed/hash.html */

/*
#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
					   +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif
*/

FR_FORCE_INLINE u16 NGet16bits ( const void * p )
{
  return *(const u16*)p;
}

u32 Frog_FNSuperFastHash (const char * pB, size_t cB) 
{
	u32 hash = 0, tmp;
	int cBRemain;

	if (cB <= 0 || pB == nullptr) return 0;

	cBRemain = cB & 3;
	cB >>= 2;

	// Main loop
	for (;cB > 0; --cB)
	{
		hash  += NGet16bits (pB);
		tmp    = (NGet16bits (pB+2) << 11) ^ hash;
		hash   = (hash << 16) ^ tmp;
		pB  += 2*sizeof(u16);
		hash  += hash >> 11;
	}

	// Handle end cases
	switch (cBRemain) 
	{
	case 3:
		hash += NGet16bits (pB);
		hash ^= hash << 16;
		hash ^= pB[sizeof(u16)] << 18;
		hash += hash >> 11;
		break;
	case 2:
		hash += NGet16bits (pB);
		hash ^= hash << 11;
		hash += hash >> 17;
		break;
	case 1:
		hash += *pB;
		hash ^= hash << 10;
		hash += hash >> 1;
	}

	// Force "avalanching" of final 127 bits
	hash ^= hash << 3;
	hash += hash >> 5;
	hash ^= hash << 4;
	hash += hash >> 17;
	hash ^= hash << 25;
	hash += hash >> 6;

	return hash;
}

FR_FORCE_INLINE u16 NGet16bitsLower ( const void * p )
{
	union Bits
	{
		u16		m_u16;
		char	m_aC[2];
	};

	Bits bits;
	bits.m_u16 = *(const u16*)p;
	bits.m_aC[0] = tolower(bits.m_aC[0]);
	bits.m_aC[1] = tolower(bits.m_aC[1]);
	return bits.m_u16;
}

u32 Frog_NSuperFastHashLower (char * pB, size_t cB) 
{
	u32 hash = 0, tmp;
	int cBRemain;

	if (cB <= 0 || pB == nullptr) 
		return 0;

	cBRemain = cB & 3;
	cB >>= 2;

	// Main loop
	for (;cB > 0; --cB) 
	{
		hash  += NGet16bitsLower (pB);
		tmp    = (NGet16bitsLower (pB+2) << 11) ^ hash;
		hash   = (hash << 16) ^ tmp;
		pB  += 2*sizeof(u16);
		hash  += hash >> 11;
	}

	// Handle end cases
	switch (cBRemain) 
	{
	case 3:
		hash += NGet16bitsLower (pB);
		hash ^= hash << 16;
		hash ^= tolower(pB[sizeof(u16)]) << 18;
		hash += hash >> 11;
		break;
	case 2:
		hash += NGet16bitsLower (pB);
		hash ^= hash << 11;
		hash += hash >> 17;
		break;
	case 1:
		hash += tolower(*pB);
		hash ^= hash << 10;
		hash += hash >> 1;
	}

	// Force "avalanching" of final 127 bits
	hash ^= hash << 3;
	hash += hash >> 5;
	hash ^= hash << 4;
	hash += hash >> 17;
	hash ^= hash << 25;
	hash += hash >> 6;

	return hash;
}


void Freb_init(FrEditBuffer * pFreb, char * pChMin, char * pChMax)
{
	pFreb->m_pChzMin = pChMin;
	pFreb->m_pChzMax = pChMax;

	pFreb->m_pChzAppend = pChMin;
	Freb_Clear(pFreb);
}

void Freb_Clear(FrEditBuffer * pFreb)
{
	pFreb->m_pChzAppend = pFreb->m_pChzMin;
	if (pFreb->m_pChzMin != pFreb->m_pChzMax)
	{
		pFreb->m_pChzMin[0] = '\0';
	}
}

void Freb_AppendCh(FrEditBuffer * pFreb, const char * pChz, size_t cB)
{
	size_t cBAvail = pFreb->m_pChzMax - pFreb->m_pChzAppend;

	if (cB < cBAvail)
	{
		// expects pChz[cB] is the null terminator

		memcpy(pFreb->m_pChzAppend, pChz, cB);
		pFreb->m_pChzAppend += cB - 1;
	}
	else if (cBAvail > 1)
	{
		size_t cBWrite = cBAvail-1;
		memcpy(pFreb->m_pChzAppend, pChz, cBWrite);
		pFreb->m_pChzAppend[cBWrite] = '\0';
		pFreb->m_pChzAppend += cBWrite;
	}
}

void Freb_AppendChz(FrEditBuffer * pFreb, const char * pChz)
{
	char * pChzDest = pFreb->m_pChzAppend;
	char * pChzDestEnd = pFreb->m_pChzMax;
	if (pChzDest == pChzDestEnd)
		return;

	for ( ; (*pChz != '\0') & ((pChzDest+1) != pChzDestEnd); ++pChz, ++pChzDest)
	{
		*pChzDest = *pChz;
	}

	pFreb->m_pChzAppend = pChzDest;
	*pChzDest = '\0';
}

void Freb_AppendChzWithPad(FrEditBuffer * pFreb, const char * pChz, char ch, size_t cChPad)
{

}

void Freb_Printf(FrEditBuffer * pFreb, const char * pChzFormat, ...)
{
	size_t cBAvail = pFreb->m_pChzMax - pFreb->m_pChzAppend;

	va_list ap;
	va_start(ap, pChzFormat);
	int cCh = vsprintf_s(pFreb->m_pChzAppend, cBAvail, pChzFormat, ap);
	if (cCh > 0)
	{
		pFreb->m_pChzAppend += cCh;
	}
}

size_t Freb_CCh(FrEditBuffer * pFreb)
{
	return pFreb->m_pChzAppend - pFreb->m_pChzMin;
}
