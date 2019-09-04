#pragma once

#include "Common.h"

template <typename T> T frMin(T a, T b)						{ return a < b ? a : b; }
template <typename T> T frMax(T a, T b)						{ return a > b ? a : b; }
template <typename T> T frSwap(T & a, T & b)				{ T temp = a; a = b; b = temp; }



// template traits
template <typename T> struct SStripConst				{ typedef T Type;	enum { F_STRIPPED= false }; };
template <typename T> struct SStripConst<const T>		{ typedef T Type;	enum { F_STRIPPED= true }; };

template <typename T> struct SStripReference			{ typedef T Type; 	enum { F_STRIPPED= false }; };
template <typename T> struct SStripReference<T&>		{ typedef T Type; 	enum { F_STRIPPED= true }; };

template <typename T> struct SStripPointer				{ typedef T Type; 	enum { F_STRIPPED= false }; };
template <typename T> struct SStripPointer<T*>			{ typedef T Type; 	enum { F_STRIPPED= true }; };

template <typename T> struct SIsReference				{ enum { V = false }; };
template <typename T> struct SIsReference<T&>			{ enum { V = true }; };

template <typename T> struct SIsPointer					{ enum { V = false }; };
template <typename T> struct SIsPointer<T&>				{ enum { V = true }; };

template <typename T> struct SIsSignedInt				{ enum { V = false }; };
template <typename T> struct SIsUnsignedInt						{ enum { V = false }; };

template <> struct SIsUnsignedInt<u8>							{ enum { V = true }; };
template <> struct SIsUnsignedInt<u16>							{ enum { V = true }; };
template <> struct SIsUnsignedInt<u32>							{ enum { V = true }; };
template <> struct SIsUnsignedInt<u64>							{ enum { V = true }; };
 
template <typename T> struct SIsInt								{ enum { V  = SIsSignedInt<T>::V  || SIsUnsignedInt<T>::V  }; };

template <typename T> struct SIsFloat							{ enum { V = false }; };
template <> struct SIsFloat<f32>								{ enum { V = true }; };
template <> struct SIsFloat<f64>								{ enum { V = true }; };

template <typename T> struct SIsBool							{ enum { V = false }; };
template <> struct SIsBool<bool>								{ enum { V = true }; };

template <typename T> struct SIsVoid							{ enum { V = false }; };
template <> struct SIsVoid<void>								{ enum { V = true }; };

template <typename T> struct SVoidSafeSizeof					{ enum { V = sizeof(T) }; };
template <> struct SVoidSafeSizeof<void>						{ enum { V = 0 }; };

// NOTE: can't just check static_cast<T>(-1) because it doesn't work for custom types
template <typename T, bool IS_ENUM> struct SIsSignedSelector	{ enum { V = SIsFloat<T>::V || SIsSignedInt<T>::V }; };
template <typename T> struct SIsSignedSelector<T, true>			{ enum { V = static_cast<T>(-1) < 0 }; };
template <typename T> struct SIsSigned							{ enum { V = SIsSignedSelector<T, FR_IS_ENUM(T)>::V }; };

template <typename T> struct SIsFundamentalType					{ enum { V  = 
																		SIsPointer<T>::V  || 
																		SIsReference<T>::V  || 
																		SIsInt<T>::V  || 
																		SIsFloat<T>::V  || 
																		SIsBool<T>::V  || 
																		FR_IS_ENUM(T) 
																	}; 
															};
template <typename T>
struct SArrayTraits
{
	typedef T Element;
	enum { C_ELEMENTS = -1 };
	enum { F_IS_ARRAY = false };
};

template <typename A, int C>
struct SArrayTraits<A[C]>
{
	typedef A Element;
	enum { C_ELEMENTS = C };
	enum { F_IS_ARRAY = true };
};

template <typename T> struct SHasTrivialConstructor		{ enum { V  = SIsFundamentalType<T>::V  }; };
template <typename T> struct SHasTrivialCopy			{ enum { V  = SIsFundamentalType<T>::V  }; };
template <typename T> struct SHasTrivialDestructor		{ enum { V  = SIsFundamentalType<T>::V  }; };

template <typename T, bool TRIVIAL_CONSTRUCT>
struct SConstructSelector
{
	static void Construct(T * p)
	{
		FR_ASSERT( ((uintptr_t)p & (FR_ALIGN_OF(T)-1)) == 0, "trying to construct misaligned object" );
		new (p) T;
	}

	static void ConstructN(T * p, size_t c)
	{
		FR_ASSERT( ((uintptr_t)p & (FR_ALIGN_OF(T)-1)) == 0, "trying to construct misaligned object" );
		for (size_t i = 0; i < c; ++i)
			new (p + i) T;
	}
};

template <typename T>
struct SConstructSelector<T, true> // trivial constructor
{
	static void Construct(T * p)					{ }
	static void ConstructN(T * p, size_t c)			{ }
};

template <typename T, bool TRIVIAL_COPY>
struct SCopySelector
{
	static void CopyConstruct(T * p, const T & orig)
	{
		FR_ASSERT( ((uintptr_t)p & (FR_ALIGN_OF(T)-1)) == 0, "trying to copy construct misaligned object" );
		new (p) T(orig);
	}

	static void CopyConstructN(T * p, size_t c, const T & orig)
	{
		FR_ASSERT( ((uintptr_t)p & (FR_ALIGN_OF(T)-1)) == 0, "trying to copy construct misaligned object" );
		for (size_t i = 0; i < c; ++i)
			new (p + i) T(orig);
	}

	static void CopyConstructArray(T * pTDst, size_t cT, const T * pTSrc)
	{
		FR_ASSERT( ((uintptr_t)pTDst & (FR_ALIGN_OF(T)-1)) == 0, "trying to copy construct misaligned object" );
		auto pTDstMax = pTDst + cT;
		for (auto pTDstIt = pTDst; pTDstIt != pTDstMax; ++pTDstIt, ++pTSrc)
			new (pTDstIt) T(*pTSrc);
	}
};

template <typename T>
struct SCopySelector<T, true> // trivial copy constructor
{
	static void CopyConstruct(T * p, const T & orig)					{ *p = orig; }
	static void CopyConstructN(T * p, size_t c, const T & orig)		
	{ 
		for (T * pEnd = &p[c]; p != pEnd; ++p)
			*p = orig;
	}

	static void CopyConstructArray(T * pTDst, size_t cT, const T * pTSrc)		
	{ 
		CopyAB(pTSrc, pTDst, sizeof(T) * cT);
	}
};

template <typename T, bool TRIVIAL_DESTRUCT>
struct SDestructSelector
{
	static void Destruct(T * p)
	{
		p->~T();
	}

	static void DestructN(T * p, size_t c)
	{
		for (size_t i = 0; i < c; ++i)
			(p + i)->~T();
	}
};

template <typename T>
struct SDestructSelector<T, true> // trivial destructor 
{
	static void Destruct(T * p)					{ }
	static void DestructN(T * p, size_t c)		{ }
};

template <typename T> void Construct(T * p)									{ SConstructSelector<T, SHasTrivialConstructor<T>::V >::Construct(p); }
template <typename T> void ConstructN(T * p, size_t c)						{ SConstructSelector<T, SHasTrivialConstructor<T>::V >::ConstructN(p,c); }

template <typename T> void CopyConstruct(T * p, const T & orig)				{ SCopySelector<T, SHasTrivialCopy<T>::V >::CopyConstruct(p, orig); }
template <typename T> void CopyConstructN(T * p, size_t c, const T & orig)	{ SCopySelector<T, SHasTrivialCopy<T>::V >::CopyConstructN(p, c, orig); }
template <typename T> void CopyConstructArray(T * pTDst, size_t cT, const T * pTSrc)	
																			{ SCopySelector<T, SHasTrivialCopy<T>::V >::CopyConstructArray(pTDst, cT, pTSrc); }

template <typename T> void Destruct(T * p)									{ SDestructSelector<T, SHasTrivialDestructor<T>::V >::Destruct(p); }
template <typename T> void DestructN(T * p, size_t c)						{ SDestructSelector<T, SHasTrivialDestructor<T>::V >::DestructN(p,c); }

// find index of element pointed to by 'pT' inside array 'aT' 
template <typename T>
size_t IFromP(const T * aT, size_t cT, const T * pT)
{ 
	size_t iT = ((uintptr_t)pT - (uintptr_t)aT) / sizeof(T); 
	FR_ASSERT((iT >= 0) & (iT < cT), "pointer not contained within array bounds");
	return iT; 
}
