#pragma once

#include "Common.h"
#include "FrogString.h"

typedef s32 FrDrawInt;		// tag = dri
typedef u32 FrDrawEnum;		// tag = dru
typedef s16 FrShaderHandle; // tag = shhand

enum { SHHAND_Nil = -1 };

#ifdef __cplusplus
#define FROG_CALL extern "C"
#else
#define FROG_CALL
#endif

typedef struct FrVec2_tag
{
	f32 m_x;
	f32 m_y;
} FrVec2;

typedef struct FrVec3_tag
{
	f32 m_x;
	f32 m_y;
	f32 m_z;
} FrVec3;

typedef struct FrVec4_tag
{
	f32 m_x;
	f32 m_y;
	f32 m_z;
	f32 m_w;
} FrVec4;

typedef struct FrRect_tag
{
	FrVec2		m_posMin;
	FrVec2		m_posMax;
} FrRect;

typedef FrVec4 FrColorVec;

inline FrVec2 Frog_Vec2Create(f32 x, f32 y)					{ FrVec2 vec2 = {x, y}; return vec2; }
inline FrVec3 Frog_Vec3Create(f32 x, f32 y, f32 z)			{ FrVec3 vec3 = {x, y, z}; return vec3; }
inline FrVec4 Frog_Vec4Create(f32 x, f32 y, f32 z, f32 w)	{ FrVec4 vec4 = {x, y, z, 2}; return vec4; }

inline FrRect Frog_RectCreate(f32 xMin, f32 yMin, f32 xMax, f32 yMax)	
	{ 
		FrRect rect = {{xMin, yMin}, {xMax, yMax}};  
		return rect;
	}

inline FrRect Frog_RectFromCenter(FrVec2 posCenter, f32 dX, f32 dY)	
	{ 
		FrRect rect; 
		f32 dXHalf = dX * 0.5f;
		f32 dYHalf = dY * 0.5f;
		rect.m_posMin.m_x = posCenter.m_x - dXHalf;		rect.m_posMin.m_y = posCenter.m_y - dYHalf;
		rect.m_posMax.m_x = posCenter.m_x + dXHalf;		rect.m_posMax.m_y = posCenter.m_y + dYHalf;
		return rect;
	}

inline FrRect Frog_RectFromExtents(FrVec2 posMin, FrVec2 posMax)	
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

typedef struct FrShader_tag // tag = shad
{
	FrDrawInt	m_driProgram;
	FrDrawInt	m_driVertexShader;
	FrDrawInt	m_driFragmentShader;
} FrShader;

typedef enum CORESHK_tag // core shader kind
{
	CORESHK_Sprite,
	CORESHK_Environment,	// like sprite, but with "perspective" distortion
	CORESHK_Max,
} CORESHK;

typedef struct FrShaderManager_tag // tag = shman
{
	FrShader		m_aShad[16];
	FrShaderHandle	m_mpCoreshkShhand[CORESHK_Max];
} FrShaderManager;

void Frog_InitShman(FrShaderManager * pShman);

enum { kFrBlack 	= (s32)0xFF000000 }; // funky casting to deal with clang error "doesn't fit in signed int"
enum { kFrWhite 	= (s32)0xFFFFFFFF };
enum { kFrRed	 	= (s32)0xFF1010FF };
enum { kFrGreen		= (s32)0xFF10FF10 };
enum { kFrBlue		= (s32)0xFFFF1010 };
enum { kFrYellow	= (s32)0xFF10FFFF };
enum { kFrMagenta	= (s32)0xFFFF10FF };
enum { kFrCyan 		= (s32)0xFFFFFF10 };

typedef union FrColor_tag
{
	u32 m_rgba;
	struct 
	{
		u8 m_r;
		u8 m_g;
		u8 m_b;
		u8 m_a;
	};
} FrColor;

inline FrColor Frog_ColCreate(u32 rgba)					{ FrColor col; col.m_rgba = rgba;	return col;}
inline FrColor Frog_ColFromRGB(u8 r, u8 g, u8 b)		{ FrColor col; col.m_r = r; col.m_g = g; col.m_b = b; col.m_a = 255; return col; }
inline FrColor Frog_ColFromRGBA(u8 r, u8 g, u8 b, u8 a)	{ FrColor col; col.m_r = r; col.m_g = g; col.m_b = b; col.m_a = a; return col; }
inline FrColorVec Frog_ColvecCreate(FrColor col)
{
	static const f32 s_rCol = 1.0f / 255.0f;
	FrColorVec colvec = {(f32)col.m_r * s_rCol, (f32)col.m_g * s_rCol, (f32)col.m_b * s_rCol, (f32)col.m_a * s_rCol};
	return colvec;
}

typedef enum ALIGNK_tag
{
	ALIGNK_Nil,
	ALIGNK_Center,
	ALIGNK_SideMin,
	ALIGNK_SideMax,
	ALIGNK_Left		= ALIGNK_SideMin,
	ALIGNK_Bottom	= ALIGNK_SideMin,
	ALIGNK_Right	= ALIGNK_SideMax,
	ALIGNK_Top		= ALIGNK_SideMax,
} ALIGNK;

typedef enum FONTK_tag
{
	FONTK_Regular,
	FONTK_Bold,

	FONTK_Max,
} FONTK;

typedef enum FONTSHK_tag
{
	FONTSHK_Basic,

	FONTSHK_Max,
} FONTSHK;



typedef struct FrFontData_tag // tag = fontd
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
} FrFontData;

typedef struct FrDrawState_tag // tag = dras
{
	FrFontData		m_fontd;
} FrDrawState;



typedef struct FrTextureMip_tag // tag=texmip
{
	s32			m_dX;
	s32			m_dY;
	size_t		m_cB; // data size
	u8*			m_aB; // actual texel data
} FrTextureMip;

typedef struct FrTexture_tag // tag=tex
{
	
	FrStringHash	m_shashFilename;	// filename hash, for texture reuse
	u8				m_cTexmip;
	s16				m_iTex;				// index into our global texture array
	FrDrawInt		m_driId;			// GL id for this texture
	FrDrawInt		m_driComponents;	// number of components or GL-specific format	(GL_RGBA)
	FrDrawEnum		m_druFormat;		// format of color such as indexed or RGBA		(GL_RGBA)
	FrDrawEnum		m_druType;			// data type for each component					(GL_UNSIGNED_BYTE)
	FrDrawEnum		m_druTarget;		// gl target type (ie. GL_TEXTURE_2D)

	FrTextureMip	m_aTexmip[1];
} FrTexture;

inline s32 DXFromTex(FrTexture * pTex)	{ return pTex->m_aTexmip[0].m_dX; }
inline s32 DYFromTex(FrTexture * pTex)	{ return pTex->m_aTexmip[0].m_dY; }



typedef struct FrDrawStateStack_tag	// tag = drasstk
{
	FrDrawState		m_aDras[10];
	s16				m_iDrasTop;

} FrDrawStateStack;

inline void				Frog_InitDrawStateStack(FrDrawStateStack * pDrasstk)	{ pDrasstk->m_iDrasTop = 0; }
inline FrDrawState *	Frog_PDrasTop(FrDrawStateStack * pDrasstk)				{ return &pDrasstk->m_aDras[pDrasstk->m_iDrasTop]; }

inline void				Frog_PushDrawStateStack(FrDrawStateStack * pDrasstk)
						{
							if(!FR_FVERIFY(pDrasstk->m_iDrasTop + 1 < FR_DIM(pDrasstk->m_aDras), "DrawStateStack Overflow"))
								return;
							pDrasstk->m_aDras[pDrasstk->m_iDrasTop + 1] = pDrasstk->m_aDras[pDrasstk->m_iDrasTop];
							++pDrasstk->m_iDrasTop;
						}

inline void				Frog_PopDrawStateStack(FrDrawStateStack * pDrasstk)
						{
							if (!FR_FVERIFY(pDrasstk->m_iDrasTop - 1 >= 0, "DrawStateStack Underflow"))
								return;
							--pDrasstk->m_iDrasTop;
						}



typedef  struct FrFontGlyph_tag // tag=glyph
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
} FrFontGlyph;


typedef struct FrFontGlyphFile_tag //tag=glyphf
{
	s16					m_cGlyph;
	s32					m_iBaWchKerning; 		// byte offset from the start of this structure to n^2 list of kerning glyph pairs (UCS2)
	s32					m_iBaDxKerning;			// byte offset from the start of this structure to the kerning values (U8)

	float				m_dYAscent;				// y offset from the baseline to the height of tallest character
	float				m_dYDescent;			// y offset from the baseline to the bottom of the lowest descender
	float				m_dYLineGap;			// lineGap is the spacing between one row's descent and the next row's ascent...
												// you should advance the vertical position by "ascent - descent + lineGap"

	FrFontGlyph			m_aGlyph[1];			// sorted by UCS2 glyph (for binary search)
} FrFontGlyphFile;

typedef struct FrFont_tag // tag=font
{
	FrTexture * 		m_pTex;
	FrFontGlyphFile *	m_pGlyphf;
	u16 * 				m_aWchKerning;
	f32 *				m_aDxKerning;
} FrFont;

typedef struct FrFontManager_tag // tag=fontman
{
	FrFont 				m_aFont[FONTK_Max];
	FrShaderHandle		m_aShhand[FONTSHK_Max];
	s32					m_mpFontshkIParamTex[FONTSHK_Max];
} FrFontManager;

typedef struct FrDrawContext_tag // tag=drac
{

	FrShaderManager 	m_shman;
	FrFontManager 		m_fontman;
	FrDrawState *		m_pDras;
	FrDrawStateStack 	m_drasstk;
} FrDrawContext;

inline void	Frog_PushDras(FrDrawContext * pDrac)	
	{
		Frog_PushDrawStateStack(&pDrac->m_drasstk);
		pDrac->m_pDras = Frog_PDrasTop(&pDrac->m_drasstk);
	}

inline void	Frog_PopDras(FrDrawContext * pDrac)	
	{
		Frog_PopDrawStateStack(&pDrac->m_drasstk);
		pDrac->m_pDras = Frog_PDrasTop(&pDrac->m_drasstk);
	}

bool Frog_FTryStaticInitDrawContext(FrDrawContext * pDrac);
bool Frog_FTryInitDrawContext(FrDrawContext * pDrac);
void Frog_SetupOrthoViewport(f64 xMin, f64 yMin, f64 xMax, f64 yMax);
void Frog_FlushFontVerts(FrDrawContext * pDrac);

FROG_CALL void Frog_DrawChar(FrDrawContext * pDrac, u32 wCh, const FrRect * pRect, FrColor colFg, FrColor colBg);
FROG_CALL void Frog_DrawTextRaw(FrDrawContext * pDrac, FrVec2 pos, const char * pCoz);



typedef struct FrScreenTile_tag // tag = tile
{
	u32				m_wch;
	FrColor			m_colFg;
	FrColor			m_colBg;
} FrScreenTile;

// Map from ascii to tile (w/ colors)
typedef struct FrTileMap_tag // tag = tmap
{
	FrScreenTile	m_mpChTile[255];
} FrTileMap;

typedef struct FrScreen_tag // tag = scr
{
	int				m_dX;
	int				m_dY;

	FrScreenTile *	m_aTile;
	/*
	u32 *			m_aWchBase;			// base wide-character grid
	FrColor *		m_aColForeBase;		// base color grid
	FrColor *		m_aColBackBase;		// base color grid
	*/
} FrScreen;

FROG_CALL FrScreen * Frog_AllocateScreen(int dX, int dY);
FROG_CALL void Frog_FreeScreen(FrScreen * pScreen);
FROG_CALL void Frog_RenderScreen(FrDrawContext * pDrac, FrScreen * pScr, FrVec2 posUL);
FROG_CALL void Frog_MapScreen(FrScreen * pScr, FrTileMap * pTmap, const char * pCozScreen);

FROG_CALL void Frog_SetTile(FrTileMap * pTmap, char ch, u32 wchOut, FrColor colFg,  FrColor colBg);



