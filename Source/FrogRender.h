#pragma once

#include "Common.h"
#include "FrogString.h"
#include <math.h>

typedef s32 FrDrawInt;		// tag = dri
typedef u32 FrDrawEnum;		// tag = dru
typedef s16 FrShaderHandle; // tag = shhand

enum { SHHAND_Nil = -1 };

inline f32 Frog_GSqrt(f32 x)			{ return sqrtf(x); }
inline f32 Frog_GAbs(f32 x)				{ return fabsf(x); }

inline f32 Frog_GCurveS(f32 t)			{ return t * t * (3.0f - t - t); }
inline f32 Frog_GCurveZ(f32 t)			{ return 1.0f - Frog_GCurveS(t); }
inline f32 Frog_GCurveBell(f32 t)		{ return Frog_GCurveZ( Frog_GAbs(t + t - 1.0f)); }



typedef struct FrVec2_t
{
	f32 m_x;
	f32 m_y;
} FrVec2;

typedef struct FrVec3_t
{
	f32 m_x;
	f32 m_y;
	f32 m_z;
} FrVec3;

typedef struct FrVec4_t
{
	f32 m_x;
	f32 m_y;
	f32 m_z;
	f32 m_w;
} FrVec4;

typedef struct FrRect_t
{
	FrVec2		m_posMin;
	FrVec2		m_posMax;
} FrRect;

typedef FrVec4 FrColorVec;

inline FrVec2 Frog_Vec2Create(f32 x, f32 y)					{ FrVec2 vec2 = {x, y}; return vec2; }
inline FrVec3 Frog_Vec3Create(f32 x, f32 y, f32 z)			{ FrVec3 vec3 = {x, y, z}; return vec3; }
inline FrVec4 Frog_Vec4Create(f32 x, f32 y, f32 z, f32 w)	{ FrVec4 vec4 = {x, y, z, 2}; return vec4; }

inline FrVec2 Frog_Vec2Add(FrVec2 * pVecA, FrVec2 * pVecB)	
{
	FrVec2 vec2 = {pVecA->m_x + pVecB->m_x, pVecA->m_y + pVecB->m_y}; 
	return vec2; 
}
inline FrVec3 Frog_Vec3Add(FrVec3 * pVecA, FrVec3 * pVecB)
{
	FrVec3 vec3 = {pVecA->m_x + pVecB->m_x, pVecA->m_y + pVecB->m_y, pVecA->m_z + pVecB->m_z}; 
	return vec3; 
}

inline FrVec2 Frog_Vec2Sub(FrVec2 * pVecA, FrVec2 * pVecB)	
{
	FrVec2 vec2 = {pVecA->m_x - pVecB->m_x, pVecA->m_y - pVecB->m_y}; 
	return vec2; 
}
inline FrVec3 Frog_Vec3Sub(FrVec3 * pVecA, FrVec3 * pVecB)
{
	FrVec3 vec3 = {pVecA->m_x - pVecB->m_x, pVecA->m_y - pVecB->m_y, pVecA->m_z - pVecB->m_z}; 
	return vec3; 
}

inline FrVec2 Frog_Vec2MulAdd(FrVec2 * pVecA, FrVec2 * pVecB, f32 r)	
{
	FrVec2 vec2 = {pVecA->m_x + pVecB->m_x*r, pVecA->m_y + pVecB->m_y*r}; 
	return vec2; 
}
inline FrVec3 Frog_Vec3MulAdd(FrVec3 * pVecA, FrVec3 * pVecB, f32 r)
{
	FrVec3 vec3 = {pVecA->m_x + pVecB->m_x*r, pVecA->m_y + pVecB->m_y*r, pVecA->m_z + pVecB->m_z*r}; 
	return vec3; 
}

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

// smoothing parameters
typedef struct FrSmp_t // tag = smp
{
	float m_dGMin;	// ending slope 
	float m_dGMax;	// starting slope
	float m_dTMax;	// max time used to determine the smoothing curve, may take longer than this to smooth if dGMax limited
} FrSmp;

FROG_CALL f32 Frog_GSmooth(f32 gCurrent, f32 gTarget, const FrSmp * pSmp, f32 dT);
//f32 GSmoothR(f32 gCurrent, f32 gTarget, f32 rPerSecond, f32 gMinStep, f32 dT);

inline FrVec2		Frog_PosSmooth(FrVec2 gCurrent, FrVec2 gTarget, const FrSmp * pSmp, f32 dT)
						{
							// BB - should do 2D smooth so it doesn't smooth faster on diagonals

							f32 x = Frog_GSmooth(gCurrent.m_x, gTarget.m_y, pSmp, dT);
							f32 y = Frog_GSmooth(gCurrent.m_x, gTarget.m_y, pSmp, dT);
							return Frog_Vec2Create(x,y);
						}

typedef struct FrShader_t // tag = shad
{
	FrDrawInt	m_driProgram;
	FrDrawInt	m_driVertexShader;
	FrDrawInt	m_driFragmentShader;
} FrShader;

typedef enum CORESHK_t // core shader kind
{
	CORESHK_Sprite,
	CORESHK_Environment,	// like sprite, but with "perspective" distortion
	CORESHK_Max,
} CORESHK;

typedef struct FrShaderManager_t // tag = shman
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

typedef union FrColor_t
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

typedef enum ALIGNK_t
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

typedef enum FONTK_t
{
	FONTK_Regular,
	FONTK_Bold,

	FONTK_Max,
} FONTK;

typedef enum FONTSHK_t
{
	FONTSHK_Basic,

	FONTSHK_Max,
} FONTSHK;



typedef struct FrFontData_t // tag = fontd
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

typedef struct FrDrawState_t // tag = dras
{
	FrFontData		m_fontd;
} FrDrawState;



typedef struct FrTextureMip_t // tag=texmip
{
	s32			m_dX;
	s32			m_dY;
	size_t		m_cB; // data size
	u8*			m_aB; // actual texel data
} FrTextureMip;

typedef struct FrTexture_t // tag=tex
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



typedef struct FrDrawStateStack_t	// tag = drasstk
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



typedef  struct FrFontGlyph_t // tag=glyph
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


typedef struct FrFontGlyphFile_t //tag=glyphf
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

typedef struct FrFont_t // tag=font
{
	FrTexture * 		m_pTex;
	FrFontGlyphFile *	m_pGlyphf;
	u16 * 				m_aWchKerning;
	f32 *				m_aDxKerning;
} FrFont;

typedef struct FrFontManager_t // tag=fontman
{
	FrFont 				m_aFont[FONTK_Max];
	FrShaderHandle		m_aShhand[FONTSHK_Max];
	s32					m_mpFontshkIParamTex[FONTSHK_Max];
} FrFontManager;

typedef struct FrDrawContext_t // tag=drac
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

FROG_CALL void Frog_DrawChar(FrDrawContext * pDrac, u32 wCh, const FrRect * pRect, FrColor colFg, FrColor colBg, float rRgb);
FROG_CALL void Frog_DrawTextRaw(FrDrawContext * pDrac, FrVec2 pos, const char * pCoz);


typedef enum FTILE
{
	FTILE_None		= 0,
	FTILE_Collide	= 1,
} FTILE;

typedef struct FrScreenTile_t // tag = tile
{
	u32				m_wch;
	FrColor			m_colFg;
	FrColor			m_colBg;
	u8				m_ftile;

} FrScreenTile;

// Map from ascii to tile (w/ colors)
typedef struct FrTileMap_t // tag = tmap
{
	FrScreenTile	m_mpChTile[255];
} FrTileMap;

typedef enum ENTID_t
{
	ENTID_Nil = -1,
} ENTID;

typedef enum ROOMID_t
{
	ROOMID_Nil = -1,
} ROOMID;

enum { kCEntCellMax = 4 };			// number of tile entities that can be on the same cell
enum { kCEntWorldMax = 1024 };		// maximum number of tile entities active in the world
enum { kCEntRoomMax = 120 };		// maximum number of entities per room
enum { kCRoomWorldMax = 64 };		// maximum number of rooms in a world

enum // Entity UPdate Order
{
	EUPO_Max = 32,
	EUPO_Min = 0,
	EUPO_Nil = -1
};

typedef struct  FrEntity_t // tag = ent
{
	int				m_iCellPrev;		// where is this tile cached in the room?
	int				m_iTile;			// character used to map into tile registry
	int				m_nGenRoom;
	int				m_ipEntUpdate;		// index of this entity in the room update array
	float			m_rTransition;

	int				m_eupo;				// entity update priority, lower updates earlier
	ROOMID			m_roomidUpdate;
	ENTID			m_entidNextCell;	// entry in list of entities in this cell
} FrEntity;

typedef struct FrRoom_t // tag = room
{
	int				m_dX;				// number of columns
	int				m_dY;				// number of rows	
	float			m_dXCharPixel;		// pixel width of characters
	float			m_dYCharPixel;		// pixel width of characters
	int				m_nGenRoom;			// generation id used to safely check room equality
	int				m_roomid;			// user id
	int				m_cpEnt;			// number of entities parented to this room
	bool			m_fEntityListDirty;	// need to update dirty array

	FrScreenTile *	m_mpICellTileEnv;	// static environment tiles
	FrScreenTile *	m_mpICellTileCache;	// cached tiles to render this frame
	ENTID *			m_mpICellEntid;	// tile entites added to this room

	FrEntity *		m_apEnt[kCEntRoomMax];
	/*
	u32 *			m_aWchBase;			// base wide-character grid
	FrColor *		m_aColForeBase;		// base color grid
	FrColor *		m_aColBackBase;		// base color grid
	*/
} FrRoom;

typedef enum ROOMTK_t // ROOM Transition Kind
{
	ROOMTK_None,
	ROOMTK_Translate,
	ROOMTK_Fade,
} ROOMTK;

typedef struct FrRoomTransition_t // tag = roomt
{
	FrRoom *			m_pRoomPrev;
	FrRoom *			m_pRoom;
	float				m_r;			// percent complete
	FrVec2				m_dPos;			// ending offset for previous screen
	ROOMTK				m_roomtk;
} FrRoomTransition;

typedef struct FrTileWorld_t	// tag = tworld
{
	FrTileMap			m_tmap;
	FrEntity			m_aEnt[kCEntWorldMax];
	ENTID				m_aEntidFree[kCEntWorldMax];

	FrRoom				m_aRoom[kCRoomWorldMax];
	ROOMID				m_aRoomidFree[kCRoomWorldMax];

	int					m_cEntAllocated;
	int					m_cRoomAllocated;
	int					m_nGenRoom;			// generation id for next screen created
} FrTileWorld;

typedef struct FrCellContents_t // tag = cellc
{
	FrScreenTile		m_tile;
	ENTID *				m_aEntid;
	int					m_cEntid;
	int					m_cEntidMax;

} FrCellContents;

FROG_CALL void Frog_InitTileWorld(FrTileWorld * pTworld);

FROG_CALL void Frog_InitTransition(FrRoomTransition * pRoomt);
FROG_CALL void Frog_SetTransition(FrRoomTransition * pRoomt, ROOMTK roomtk, FrRoom * pFroomPrev, FrRoom * pFroomNext);
FROG_CALL void Frog_UpdateTransition(FrRoomTransition * pRoomt, f32 dT);
FROG_CALL void Frog_RenderTransition(FrDrawContext * pDrac, FrTileWorld * pTworld, FrRoomTransition * pRoomt, FrVec2 pos);

FROG_CALL FrRoom * Frog_PRoomAllocate(FrTileWorld * pTworld, int dXCell, int dYCell);
FROG_CALL FrRoom * Frog_PRoom(FrTileWorld * pTworld, ROOMID roomid);
FROG_CALL void Frog_FreeRoom(FrTileWorld * pTworld, FrRoom * pRoom);
FROG_CALL void Frog_RenderRoom(FrDrawContext * pDrac, FrTileWorld * pTworld, FrRoom * pFroom, FrVec2 posUL, float rRGB);
FROG_CALL void Frog_SetRoomTiles(FrRoom * pFroom, FrTileMap * pTmap, const char * pCozScreen);
FROG_CALL void Frog_SortEntityUpdateList(FrRoom * pRoom);

FROG_CALL void Frog_SetTile(FrTileMap * pTmap, char iTile, u32 wchOut, FrColor colFg,  FrColor colBg, u8 ftile);
FROG_CALL u8 Frog_FtileFromCell(FrRoom * pFroom, int xCell, int yCell);

FROG_CALL ENTID Frog_EntidAllocate(FrTileWorld * pTworld);
FROG_CALL FrEntity * Frog_PEnt(FrTileWorld * pTworld, ENTID entid);
FROG_CALL ENTID Frog_EntidFromEnt(FrTileWorld * pTworld, FrEntity * pEnt);
FROG_CALL void Frog_FreeEntity(FrTileWorld * pTworld, ENTID entid);
FROG_CALL void Frog_UpdateEntity(FrTileWorld * pTworld, FrRoom * pFroom, ENTID entid, int xCell, int yCell, u8 iTile);
FROG_CALL void Frog_AddToRoom(FrRoom * pRoom, FrEntity * pEnt, int eupo);
FROG_CALL void Frog_RemoveFromRoom(FrRoom * pRoom, FrEntity * pEnt);

FROG_CALL void Frog_InitCellContents(ENTID * aEntid, int cEntidMax, FrCellContents * pCellc);
FROG_CALL void Frog_FindCellContents(FrTileWorld * pTworld, FrRoom * pFroom, int xCell, int yCell, FrCellContents * pCellc);

inline bool FIsNull(ENTID entid)				{ return entid == ENTID_Nil; }
