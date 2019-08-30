#pragma once

#include "Common.h"
#include "FrogString.h"

typedef s32 FrDrawInt;		// tag = dri
typedef u32 FrDrawEnum;		// tag = dru
typedef s16 FrShaderHandle; // tag = shhand

enum { SHHAND_Nil = -1 };

struct FrVec2
{
	f32 m_x;
	f32 m_y;
};

struct FrVec3
{
	f32 m_x;
	f32 m_y;
	f32 m_z;
};

struct FrVec4
{
	f32 m_x;
	f32 m_y;
	f32 m_z;
	f32 m_w;
};

struct FrRect
{
	FrVec2		m_posMin;
	FrVec2		m_posMax;
};

typedef FrVec4 FrColorVec;

inline FrVec2 Frog_Vec2Create(f32 x, f32 y)					{ FrVec2 vec2 = {x, y}; return vec2; }
inline FrVec3 Frog_Vec3Create(f32 x, f32 y, f32 z)			{ FrVec3 vec3 = {x, y, z}; return vec3; }
inline FrVec4 Frog_Vec4Create(f32 x, f32 y, f32 z, f32 w)	{ FrVec4 vec4 = {x, y, z, 2}; return vec4; }

inline FrRect Frog_RectCreate(f32 xMin, f32 yMin, f32 xMax, f32 yMax)	
	{ 
		FrRect rect = {{xMin, yMin}, {xMax, yMax}};  
		return rect;
	}

inline FrRect Frog_RectCreate(FrVec2 posCenter, f32 dX, f32 dY)	
	{ 
		FrRect rect; 
		f32 dXHalf = dX * 0.5f;
		f32 dYHalf = dY * 0.5f;
		rect.m_posMin.m_x = posCenter.m_x - dXHalf;		rect.m_posMin.m_y = posCenter.m_y - dYHalf;
		rect.m_posMax.m_x = posCenter.m_x + dXHalf;		rect.m_posMax.m_y = posCenter.m_y + dYHalf;
		return rect;
	}

inline FrRect Frog_RectCreate(FrVec2 posMin, FrVec2 posMax)	
	{ 
		FrRect rect = {posMin, posMax};  
		return rect;
	}

inline FrVec2 Frog_PosCenter(const FrRect * pRect)
	{
		f32 dXHalf = (pRect->m_posMax.m_x - pRect->m_posMin.m_x) * 0.5f;
		f32 dYHalf = (pRect->m_posMax.m_y - pRect->m_posMin.m_y) * 0.5f;
		return Frog_Vec2Create(pRect->m_posMin.m_x + dXHalf, pRect->m_posMin.m_y + dYHalf);
	}

struct FrShader // tag = shad
{
	FrDrawInt	m_driProgram;
	FrDrawInt	m_driVertexShader;
	FrDrawInt	m_driFragmentShader;
};

enum CORESHK // core shader kind
{
	CORESHK_Sprite,
	CORESHK_Environment,	// like sprite, but with "perspective" distortion
	CORESHK_Max,
};

enum CORETEXK // core texture kind
{
	CORETEXK_Test,
	CORETEXK_Max
};

enum COREATLAS // core texture atlas
{
	COREATLAS_Tiles,
	COREATLAS_Max,
};

struct FrShaderManager // tag = shman
{
		FrShaderManager()
		{
			for (int iShad = 0; iShad < FR_DIM(m_aShad); ++iShad)
			{
				FrShader * pShad = &m_aShad[iShad];
				pShad->m_driProgram = 0;
				pShad->m_driVertexShader = 0;
				pShad->m_driFragmentShader = 0;
			}

			for (int iShhand = 0; iShhand < FR_DIM(m_mpCoreshkShhand); ++iShhand)
			{
				m_mpCoreshkShhand[iShhand] = SHHAND_Nil;
			}
		}

	FrShader		m_aShad[16];
	FrShaderHandle	m_mpCoreshkShhand[CORESHK_Max];
};


/*
class FrColor // tag = col
{
public:
					FrColor()
					:m_rgba(kBlack)
						{ ; }

					FrColor(u32 rgba)
					:m_rgba(rgba)
						{ ; }

					FrColor(u8 r, u8 g, u8 b, u8 a = 255)
					:m_r(r)
					,m_g(g)
					,m_b(b)
					,m_a(a)
						{ ; }
		CColorVec	Colvec() const
						{ 
							static const CScalar r = 1.0f / 255.0f;  
							return CColorVec((F32)m_r, (F32)m_g, (F32)m_b, (F32)m_a) * r; 
						}

	union
	{
		u32 m_rgba;
		struct 
		{
			u8 m_r;
			u8 m_g;
			u8 m_b;
			u8 m_a;
		};
	};
};
*/
enum { kFrBlack 	= (s32)0xFF000000 }; // funky casting to deal with clang error "doesn't fit in signed int"
enum { kFrWhite 	= (s32)0xFFFFFFFF };
enum { kFrRed	 	= (s32)0xFF1010FF };
enum { kFrGreen		= (s32)0xFF10FF10 };
enum { kFrBlue		= (s32)0xFFFF1010 };
enum { kFrYellow	= (s32)0xFF10FFFF };
enum { kFrMagenta	= (s32)0xFFFF10FF };
enum { kFrCyan 		= (s32)0xFFFFFF10 };

union FrColor
{
	u32 m_rgba;
	struct 
	{
		u8 m_r;
		u8 m_g;
		u8 m_b;
		u8 m_a;
	};
};

inline FrColor Frog_ColCreate(u32 rgba)					{ FrColor col; col.m_rgba = rgba;	return col;}
inline FrColor Frog_ColCreate(u8 r, u8 g, u8 b)			{ FrColor col; col.m_r = r; col.m_g = g; col.m_b = b; col.m_a = 255; return col; }
inline FrColor Frog_ColCreate(u8 r, u8 g, u8 b, u8 a)	{ FrColor col; col.m_r = r; col.m_g = g; col.m_b = b; col.m_a = a; return col; }
inline FrColorVec Frog_ColvecCreate(FrColor col)	
{
	static const f32 s_rCol = 1.0f / 255.0f;
	FrColorVec colvec = {f32(col.m_r) * s_rCol, f32(col.m_g) * s_rCol, f32(col.m_b) * s_rCol, f32(col.m_a) * s_rCol};
	return colvec;
}

enum ALIGNK
{
	ALIGNK_Nil,
	ALIGNK_Center,
	ALIGNK_SideMin,
	ALIGNK_SideMax,
	ALIGNK_Left		= ALIGNK_SideMin,
	ALIGNK_Bottom	= ALIGNK_SideMin,
	ALIGNK_Right	= ALIGNK_SideMax,
	ALIGNK_Top		= ALIGNK_SideMax,
};

enum FONTK
{
	FONTK_Regular,
	FONTK_Bold,

	FONTK_Max,
};

enum FONTSHK
{
	FONTSHK_Basic,

	FONTSHK_Max,
};



struct FrFontData // tag = fontd
{
	FrColor			m_colMain;
	FrColor			m_colShadow;
	f32				m_gCharSize;
	f32				m_rKerning;
	f32				m_uOpacity;
	FONTK			m_fontk;
	ALIGNK			m_alignkX;
	ALIGNK			m_alignkY;
	bool			m_fUseFixedWidth;
	bool			m_fUseWordWrap;

	FrVec2			m_posCursor;
};

struct FrDrawState // tag = dras
{
	FrFontData		m_fontd;
};


#define NEW_DRAW_STATE 1
#if NEW_DRAW_STATE 

#else //!NEW_DRAW_STATE
enum DRASK // draw state kind
{
	DRASK_Nil,
	DRASK_Min,

	/*
	DRASK_RectMin,
		DRASK_RectScissor		= DRASK_RectMin,
	DRASK_RectMax,

	DRASK_VecMin,
		DRASK_PosSortOriginWs	= DRASK_VecMin,		// this is the origin for tile "perspective" and sorting
		DRASK_VecScale,
		DRASK_PosCursor,
	DRASK_VecMax,
	*/

	DRASK_ColMin,
		DRASK_ColFontMain		= DRASK_ColMin,
		DRASK_ColFontShadow,
	DRASK_ColMax,

	DRASK_GMin,
		DRASK_GCharSize			= DRASK_GMin,
		DRASK_ROpacity,
	DRASK_GMax,

	DRASK_NMin,
		DRASK_IFont				= DRASK_NMin,
		DRASK_NAlignkX,
		DRASK_NAlignkY,
	DRASK_NMax,


	DRASK_FMin,
		DRASK_FFixedWidthFont	= DRASK_FMin,
		DRASK_FWordWrap,
		DRASK_FUseScissor,
		DRASK_FDrawSolid,
	DRASK_FMax,
	DRASK_Max
};

class FrDrawState // tag = dras
{
public:
			FrDrawState()
				{
					//SetRect(DRASK_RectScissor, CRect(0,0,0,0));

					//SetVec(DRASK_PosSortOriginWs, CVec2(KZERO));
					//SetVec(DRASK_VecScale, CVec2(1.0f, 1.0f));
					//SetVec(DRASK_PosCursor, CVec2(KZERO));

					SetCol(DRASK_ColFontMain, Frog_ColCreate(kFrWhite));
					SetCol(DRASK_ColFontShadow, Frog_ColCreate(kFrBlack));

					SetG(DRASK_GCharSize, 24.0f);
					SetG(DRASK_ROpacity, 1.0f);

					SetN(DRASK_IFont, 0);
					SetN(DRASK_NAlignkX, ALIGNK_Center);
					SetN(DRASK_NAlignkY, ALIGNK_Center);

					SetF(DRASK_FFixedWidthFont, false);
					SetF(DRASK_FWordWrap, false);
					SetF(DRASK_FUseScissor, false);
					SetF(DRASK_FDrawSolid, true);
				}

	void	SetF(DRASK drask, bool f)
				{ 
					FR_ASSERT((drask>=DRASK_FMin)&(drask<DRASK_FMax), "bad drask(F)");
					m_aF[drask-DRASK_FMin] = f; 
				}
	void	SetN(DRASK drask, s16 n)
				{ 
					FR_ASSERT((drask>=DRASK_NMin)&(drask<DRASK_NMax), "bad drask(N)");
					m_aN[drask-DRASK_NMin] = n;
				}
	void	SetG(DRASK drask, f32 g)
				{ 
					FR_ASSERT((drask>=DRASK_GMin)&(drask<DRASK_GMax), "bad drask(G)");
					m_aG[drask-DRASK_GMin] = g;
				}
	void	SetCol(DRASK drask, FrColor col)
				{ 
					FR_ASSERT((drask>=DRASK_ColMin)&(drask<DRASK_ColMax), "bad drask(Col)");
					m_aCol[drask-DRASK_ColMin] = col;
				}
	/*
	void	SetVec(DRASK drask, CVec2Arg vec)
				{ 
					FR_ASSERT((drask>=DRASK_VecMin)&(drask<DRASK_VecMax), "bad drask(Vec)");
					m_aVec[drask-DRASK_VecMin] = vec;
				}
	void	SetRect(DRASK drask, CRectArg rect)
				{ 
					FR_ASSERT((drask>=DRASK_RectMin)&(drask<DRASK_RectMax), "bad drask(Rect)");
					m_aRect[drask-DRASK_RectMin] = rect;
				} */
	bool	FGet(DRASK drask) const
				{
					FR_ASSERT((drask>=DRASK_FMin)&(drask<DRASK_FMax), "bad drask(F)");
					return m_aF[drask-DRASK_FMin];
				}
	s16		NGet(DRASK drask) const
				{
					FR_ASSERT((drask>=DRASK_NMin)&(drask<DRASK_NMax), "bad drask(N)");
					return m_aN[drask-DRASK_NMin];
				}
	f32		GGet(DRASK drask) const
				{
					FR_ASSERT((drask>=DRASK_GMin)&(drask<DRASK_GMax), "bad drask(G)");
					return m_aG[drask-DRASK_GMin];
				}
	FrColor	ColGet(DRASK drask) const
				{
					FR_ASSERT((drask>=DRASK_ColMin)&(drask<DRASK_ColMax), "bad drask(Col)");
					return m_aCol[drask-DRASK_ColMin];
				}
	/*
	CVec2	VecGet(DRASK drask) const
				{
					FR_ASSERT((drask>=DRASK_VecMin)&(drask<DRASK_VecMax), "bad drask(Vec)");
					return m_aVec[drask-DRASK_VecMin];
				}
	CRect	RectGet(DRASK drask) const
				{
					FR_ASSERT((drask>=DRASK_RectMin)&(drask<DRASK_RectMax), "bad drask(Rect)");
					return m_aRect[drask-DRASK_RectMin];
				}*/

	void	SetAlignk(ALIGNK alignkX, ALIGNK alignkY)
				{
					SetN(DRASK_NAlignkX, alignkX);
					SetN(DRASK_NAlignkY, alignkY);
				}

	void	FlushIfChanged(const FrDrawState & drasPrev)
				{ ; }

	void	PreDraw() const;
	void	PostDraw() const;

protected:
	//CRect		m_aRect[DRASK_RectMax - DRASK_RectMin];
	//CVec2		m_aVec[DRASK_VecMax - DRASK_VecMin];
	FrColor		m_aCol[DRASK_ColMax - DRASK_ColMin];
	f32			m_aG[DRASK_GMax - DRASK_GMin];
	s16			m_aN[DRASK_NMax - DRASK_NMin];
	bool		m_aF[DRASK_FMax - DRASK_FMin];
};
#endif



struct FrTextureMip // tag=texmip
{
	s32			m_dX;
	s32			m_dY;
	size_t		m_cB; // data size
	u8*			m_aB; // actual texel data
};

struct FrTexture // tag=tex
{
	s32				DX()
						{ return m_aTexmip[0].m_dX; }
	s32				DY()
						{ return m_aTexmip[0].m_dY; }
	
	FrStringHash	m_shashFilename;	// filename hash, for texture reuse
	u8				m_cTexmip;
	s16				m_iTex;				// index into our global texture array
	FrDrawInt		m_driId;			// GL id for this texture
	FrDrawInt		m_driComponents;	// number of components or GL-specific format	(GL_RGBA)
	FrDrawEnum		m_druFormat;		// format of color such as indexed or RGBA		(GL_RGBA)
	FrDrawEnum		m_druType;			// data type for each component					(GL_UNSIGNED_BYTE)
	FrDrawEnum		m_druTarget;		// gl target type (ie. GL_TEXTURE_2D)

	FrTextureMip	m_aTexmip[1];
};

class FrDrawStateStack	// tag = drasstk
{
public:
					FrDrawStateStack()
					: m_iDrasTop(0)
						{ ; }

	FrDrawState *	PDrasTop()
						{ return &m_aDras[m_iDrasTop]; }
	s16				IDrasTop() const
						{ return m_iDrasTop; }

	void			Push()
						{
							if(!FR_FVERIFY(m_iDrasTop + 1 < FR_DIM(m_aDras), "DrawStateStack Overflow"))
								return;
							m_aDras[m_iDrasTop + 1] = m_aDras[m_iDrasTop];
							++m_iDrasTop;
						}

	void			Pop()
						{
							if(!FR_FVERIFY(m_iDrasTop - 1 >= 0, "DrawStateStack Underflow"))
								return;
							--m_iDrasTop;
						}
protected:
	static const int s_cDrasDepth = 10;
	FrDrawState		m_aDras[s_cDrasDepth];
	s16				m_iDrasTop;
};





struct FrFontGlyph // tag=glyph
{
	u16		m_wch;					// UCS2 Codepoint for this glyph
	f32		m_dXPixels;
	f32		m_dYPixels;
	f32		m_xOffset;				// offset from the glyph origin to the top-left of the bitmap
	f32		m_yOffset;
	f32		m_dXKerningDefault;		// offset between letters
	u16 	m_iDxKerningMin;		// index into the kerning table 0xFFFF if none
	u16 	m_iDxKerningMax;		// index into the kerning table 0xFFFF if none
	f32		m_uMin;
	f32   	m_uMax;
	f32 	m_vMin;
	f32   	m_vMax;
};


struct FrFontGlyphFile //tag=glyphf
{
	s16					m_cGlyph;
	s32					m_iBaWchKerning; 		// byte offset from the start of this structure to n^2 list of kerning glyph pairs (UCS2)
	s32					m_iBaDxKerning;			// byte offset from the start of this structure to the kerning values (U8)

	float				m_dYAscent;				// y offset from the baseline to the height of tallest character
	float				m_dYDescent;			// y offset from the baseline to the bottom of the lowest descender
	float				m_dYLineGap;			// lineGap is the spacing between one row's descent and the next row's ascent...
												// you should advance the vertical position by "ascent - descent + lineGap"

	FrFontGlyph			m_aGlyph[1];			// sorted by UCS2 glyph (for binary search)
};

struct FrFont // tag=font
{
	FrTexture * 		m_pTex;
	FrFontGlyphFile *	m_pGlyphf;
	u16 * 				m_aWchKerning;
	f32 *				m_aDxKerning;
};

struct FrFontManager // tag=fontman
{
	FrFont 				m_aFont[FONTK_Max];
	FrShaderHandle		m_aShhand[FONTSHK_Max];
	s32					m_mpFontshkIParamTex[FONTSHK_Max];
};

struct FrDrawContext // tag=drac
{
	void		PushDras()	
					{
						m_drasstk.Push();
						m_pDras = m_drasstk.PDrasTop();
					}

	void		PopDras()	
					{
						m_drasstk.Pop();
						m_pDras = m_drasstk.PDrasTop();
					}

	FrShaderManager 	m_shman;
	FrFontManager 		m_fontman;
	FrDrawState *		m_pDras;
	FrDrawStateStack 	m_drasstk;
};

bool Frog_FTryStaticInitDrawContext(FrDrawContext * pDrac);
void Frog_SetupOrthoViewport(f64 xMin, f64 yMin, f64 xMax, f64 yMax);
void Frog_FlushFontVerts(FrDrawContext * pDrac);

void Frog_DrawChar(FrDrawContext * pDrac, u32 wCh, const FrRect * pRect, FrColor colFg, FrColor colBg);
void Frog_DrawTextRaw(FrDrawContext * pDrac, FrVec2 pos, const char * pCoz);



struct FrScreenTile // tag = tile
{
	u32				m_wch;
	FrColor			m_colFg;
	FrColor			m_colBg;
};

// Map from ascii to tile (w/ colors)
struct FrTileMap // tag = tmap
{
	FrScreenTile	m_mpChTile[255];
};

struct FrScreen // tag = scr
{
	int				m_dX;
	int				m_dY;

	FrScreenTile *	m_aTile;
	/*
	u32 *			m_aWchBase;			// base wide-character grid
	FrColor *		m_aColForeBase;		// base color grid
	FrColor *		m_aColBackBase;		// base color grid
	*/
};

FrScreen * Frog_AllocateScreen(int dX, int dY);
void Frog_FreeScreen(FrScreen * pScreen);
void Frog_RenderScreen(FrDrawContext * pDrac, FrScreen * pScr, FrVec2 posUL);
void Frog_MapScreen(FrScreen * pScr, FrTileMap * pTmap, const char * pCozScreen);

void Frog_SetTile(FrTileMap * pTmap, char ch, u32 wchOut, FrColor colFg,  FrColor colBg);



