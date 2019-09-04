#pragma once

#include <new.h>
#include <stdint.h>
#include <stdbool.h>

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef size_t uSize;
typedef ptrdiff_t sSize;



inline int Frog_NMin(int a, int b)	{ return (a < b) ? a : b; }
inline int Frog_NMax(int a, int b)	{ return (a > b) ? a : b; }

enum EDGES 	// EDGE State
{
	EDGES_Off,
	EDGES_Release,
	EDGES_Hold,
	EDGES_Press	
};

#define FR_DIM(arr) (sizeof(arr) / sizeof(*arr))
#define FR_PMAX(arr) &arr[FR_DIM(arr)]

#if defined( __GNUC__ )
	#define		FR_FORCE_INLINE	inline __attribute__((always_inline))
	#define		FR_ALIGN(CB)		__attribute__((aligned(CB)))
	#define 	FR_ALIGN_OF(T) 	__alignof__(T)
	#define		FR_IS_ENUM(T)		__is_enum(T)
	#define		FR_DEBUG_BREAK()	asm ("int $3")
#elif defined( _MSC_VER )
	#define		FR_FORCE_INLINE	__forceinline
	#define		FR_ALIGN(CB)		__declspec(align(CB))
	#define 	FR_ALIGN_OF(T) 	__alignof(T)
	#define		FR_IS_ENUM(T)		__is_enum(T)
	#define		FR_DEBUG_BREAK()	__debugbreak()
#elif defined( __clang__)
	#define		FR_FORCE_INLINE	inline __attribute__((always_inline))
	#define		FR_ALIGN(CB)		__attribute__((aligned(CB)))
	#define 	FR_ALIGN_OF(T) 	__alignof__(T)
	#define		FR_IS_ENUM(T)		__is_enum(T)
	#define		FR_DEBUG_BREAK()   asm("int $3")
#endif

void FrAssertHandler( const char* pChzFile, u32 line, const char* pChzCondition, const char* pChzMessage, ...);

#define FR_VERIFY( PREDICATE, ... ) \
	do { if (!(PREDICATE)) { \
		FrAssertHandler(__FILE__, __LINE__, #PREDICATE, __VA_ARGS__); \
		FR_DEBUG_BREAK(); \
	} } while (0)

#define FR_ASSERT( PREDICATE, ... ) FR_VERIFY(PREDICATE, __VA_ARGS__);


#if defined( _MSC_VER )
#define FR_FVERIFY_PROC( PREDICATE, ASSERTPROC, FILE, LINE, ... )\
(\
  ( ( PREDICATE ) ? \
	true :\
	(\
	  ASSERTPROC( FILE, LINE, #PREDICATE, __VA_ARGS__ ),\
	  FR_DEBUG_BREAK(), \
	  false\
	)\
  )\
)
#else
// use a goofy expression statement to play nice with clang
#define FR_FVERIFY( PREDICATE, ... )\
(\
  ( ( PREDICATE ) ? \
	true :\
	({\
	  ASSERTPROC( FILE, LINE, #PREDICATE, __VA_ARGS__ );\
	  FR_DEBUG_BREAK(); \
	  false;\
	})\
  )\
)
#endif

#define FR_FVERIFY(PREDICATE, ...) \
	FR_FVERIFY_PROC (PREDICATE, FrAssertHandler, __FILE__, __LINE__, __VA_ARGS__ )

#define FR_FASSERT( PREDICATE, ... ) FR_FVERIFY(PREDICATE, __VA_ARGS__)

#define FR_TRACE(PREDICATE, ...) \
do { if (PREDICATE) \
	{ printf(__VA_ARGS__); } \
} while (0)



inline s32 S32Coerce(s64 n)		{ s32 nRet = (s32)n;	FR_ASSERT((s64)nRet == n, "S32Coerce failure"); return nRet; }
inline s16 S16Coerce(s64 n)		{ s16 nRet = (s16)n;	FR_ASSERT((s64)nRet == n, "S16Coerce failure"); return nRet; }
inline s8 S8Coerce(s64 n)		{ s8 nRet = (s8)n;		FR_ASSERT((s64)nRet == n, "S8Coerce failure");  return nRet; }
inline u32 U32Coerce(u64 n)		{ u32 nRet = (u32)n;	FR_ASSERT((u64)nRet == n, "u32Coerce failure"); return nRet; }
inline u16 U16Coerce(u64 n)		{ u16 nRet = (u16)n;	FR_ASSERT((u64)nRet == n, "u16Coerce failure"); return nRet; }
inline u8 U8Coerce(u64 n)		{ u8 nRet = (u8)n;		FR_ASSERT((u64)nRet == n, "u8Coerce failure");  return nRet; }



void ZeroAB(void * pDest, size_t cB);
void FillAB(u8 b, void * pDest, size_t cB);
void CopyAB(const void * pSource, void * pDest, size_t cB);

void ShowErrorV(const char * pChzFormat, va_list ap);
void ShowError(const char * pChzFormat, ...);
void ShowInfoV(const char * pChzFormat, va_list ap);
void ShowInfo(const char * pChzFormat, ...);

u32 HvFromPBFVN(const void * pV, size_t cB);
u32 HvConcatPBFVN(u32 hv, const void * pV, size_t cB);

void SplitFilename(const char * pChzFilename, size_t * piBFile, size_t * piBExtension, size_t * piBEnd);
void ConcatPChz(const char* pChzA, const char * pChzB, char * pChOut, size_t cChOutMax);

inline size_t CBCodepoint(const char * pCoz)
{
	if ((*pCoz & 0xF8) == 0xF0)	return 4;
	if ((*pCoz & 0xF0) == 0xE0)	return 3;
	if ((*pCoz & 0xE0) == 0xC0)	return 2;
	return 1;
}

size_t	CBCoz(const char * pCoz);
size_t	CBCopyCoz(const char * pCozSource, char * aCoDest, size_t cBDest);
