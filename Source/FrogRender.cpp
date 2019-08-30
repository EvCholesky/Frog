#include "FrogRender.h"

#define GL_WRAPPER_IMPLEMENTATION
#include "Array.h"
#include "FontShaders.inl"
#include "GlWrapper.h"
#include "SpriteShaders.inl"
#include "stb_image.h"
#include <stdlib.h>

static const int kChMaxDrawText = 1024;

struct FrFontVertex // tag = fvert
{
	f32			m_x, m_y;
	f32			m_u, m_v;
	f32			m_r, m_g, m_b, m_a;
	f32			m_uMin, m_vMin, m_uMax, m_vMax; // glyph bounds for clamping texture filtering
};

struct FrFontVertexBuffer // tag = fvbuf
{
	FrFontVertex	m_aFvert[10240]; 
	int				m_cFvert;
};

FixAry<FrTexture, 100>	s_aryTex;
FrFontVertexBuffer g_Fvbuf;

inline FrShaderHandle ShhandFindUnused(FrShaderManager * pShman)
{
	FrShaderHandle iShad = 0;
	FrShader  * pShadEnd = &pShman->m_aShad[FR_DIM(pShman->m_aShad)];
	FrShader * pShadIt = pShman->m_aShad;
	while (pShadIt != pShadEnd)
	{
		if (pShadIt->m_driProgram == 0)
			return iShad;

		++pShadIt;
		++iShad;
	}

	FR_ASSERT(false, "couldn't find unused shader handle");
	return SHHAND_Nil;
}

/*typedef PFnShaderLog (shader: gluint, bufSize: glsizei, length: &glsizei, infoLog: &glchar);
void PrintShaderInfoLog (FrDrawInt object, PFnShaderLog pFnGetLog)
{
	FrDrawInt cCh;
	glGetShaderiv(object, GL_INFO_LOG_LENGTH, &cCh);

	aCh := cast(&u8) PVMalloc(cast (uSize)cCh);
	pFnGetLog(object, cCh, null, aCh);
	printf("log:\n%s", aCh);
	FreeMalloc(aCh);
}*/

s32 IParamFind(const FrShaderManager * pShman, FrShaderHandle shhand, const char * pChzName)
{
	FR_ASSERT(shhand != SHHAND_Nil, "Bad shader handle");
	return glGetUniformLocation(pShman->m_aShad[shhand].m_driProgram, pChzName);
}

// note: This bullshit seems to silently fail with vec2
void SetShaderParam(s32 iParam, f32 x, f32 y, f32 z, f32 w)
{
	FR_ASSERT(iParam >= 0, "invalid slot in SetShaderParam");
	glUniform4fARB(iParam, x, y, z, w);
}

/*
void SetShaderParam(s32 iParam, CMat44 & mat)
{
	FR_ASSERT(iParam >= 0, "invalid slot in SetShaderParam");
	glUniformMatrix4fvARB(iParam, 1, false, mat.AG());
}*/

void SetShaderParam(s32 iParam, const FrTexture * pTex, int iTextureUnit)
{
	FR_ASSERT(pTex, "Null texture in SetShaderParam");
	if(pTex == 0)
		return;

	glEnable(pTex->m_druTarget);

	FR_ASSERT(iParam >= 0, "invalid slot in ShaderGLSL::setParam");
	glActiveTextureARB(GL_TEXTURE0 + iTextureUnit);
	glBindTexture(GL_TEXTURE_2D, pTex->m_driId);

	glUniform1iARB(iParam, iTextureUnit);

}

static void PrintShaderInfoLog(FrDrawInt object, PFNGLGETSHADERINFOLOGPROC getInfoLogProc)
{
	FrDrawInt cCh;
	glGetShaderiv(object, GL_INFO_LOG_LENGTH, &cCh);

	char * aCh = (char*)malloc(cCh);
	getInfoLogProc(object, cCh, 0, aCh);
	printf("%s", aCh);
	free(aCh);
}

FrShaderHandle ShhandLoad (FrShaderManager * pShman, const char * pChzMaterialName, const char * pChzVertexSource, const char * pChzFragmentSource)
{
	auto shhand = ShhandFindUnused(pShman);
	if (shhand == SHHAND_Nil)
		return SHHAND_Nil;

	FrShader * pShad = &pShman->m_aShad[shhand];
	
	FrDrawInt glSuccess;
	pShad->m_driProgram			= glCreateProgram();
	pShad->m_driVertexShader		= glCreateShader(GL_VERTEX_SHADER_ARB);
	pShad->m_driFragmentShader	= glCreateShader(GL_FRAGMENT_SHADER_ARB);

	glShaderSource(pShad->m_driVertexShader, 1, &pChzVertexSource, nullptr);
	glShaderSource(pShad->m_driFragmentShader, 1, &pChzFragmentSource, nullptr);

	glCompileShader(pShad->m_driVertexShader);
	// if(checkForErrors("glCompileShaderARB(sd.vertexShader)", sd.m_vertexShader))
	// 	return SHHAND_Nil

	glGetShaderiv(pShad->m_driVertexShader, GL_COMPILE_STATUS, &glSuccess);
	if (!glSuccess)
	{
		printf("Failed to compile vertex shader %s:\n", pChzMaterialName);
		PrintShaderInfoLog(pShad->m_driVertexShader, glGetShaderInfoLog);
		glDeleteShader(pShad->m_driVertexShader);
		return SHHAND_Nil;
	}

	glCompileShader(pShad->m_driFragmentShader);
	// if(checkForErrors("glCompileShaderARB(pShad->fragmentShader)", pShad->m_driFragmentShader))
	// 	return SHHAND_Nil

	glGetShaderiv(pShad->m_driFragmentShader, GL_COMPILE_STATUS, &glSuccess);
	if (!glSuccess)
	{
		printf("Failed to compile fragment shader %s:\n", pChzMaterialName);
		printf("Source:\n%s\n\n", pChzFragmentSource);

		PrintShaderInfoLog(pShad->m_driFragmentShader, glGetShaderInfoLog);
		glDeleteShader(pShad->m_driVertexShader);
		glDeleteShader(pShad->m_driFragmentShader);
		return SHHAND_Nil;
	}

	glAttachShader(pShad->m_driProgram, pShad->m_driVertexShader);
	glAttachShader(pShad->m_driProgram, pShad->m_driFragmentShader);

	glLinkProgram(pShad->m_driProgram);
	glGetProgramiv(pShad->m_driProgram, GL_LINK_STATUS, &glSuccess);
	if (!glSuccess)
	{
		printf("Failed to link shader program: %s\n", pChzMaterialName);
		PrintShaderInfoLog(pShad->m_driProgram, glGetProgramInfoLog);
		glDeleteProgram(pShad->m_driProgram);
		return SHHAND_Nil;
	}
	// if(checkForErrors("glLinkProgram(mat.program)", sd.m_program))
	// 	return SHHAND_Nil;

	return shhand;
}

enum VERTATTR : u32 // tag VERTex ATTRibute streams
{
	VERTATTR_Position,
	VERTATTR_Color,
	VERTATTR_ColorOvr,	
	VERTATTR_TexCoord,
};

bool FTryStaticInitShaderManager(FrShaderManager * pShman)
{
	if (!Frog_FTryStaticInitGLExtensions())
	{
		printf ("Error loading GL Extensions.");
		return false;
	}

	pShman->m_mpCoreshkShhand[CORESHK_Sprite] = ShhandLoad(
													pShman,
													"BasicSpriteShader",
													g_spriteMainVertex,
													g_spriteMainFragment);

	FrShader * pShad = &pShman->m_aShad[pShman->m_mpCoreshkShhand[CORESHK_Sprite]];
	glBindAttribLocation(pShad->m_driVertexShader, VERTATTR_Position, "s_AttribPosition");
	glBindAttribLocation(pShad->m_driVertexShader, VERTATTR_Color, "s_AttribColor");
	glBindAttribLocation(pShad->m_driVertexShader, VERTATTR_ColorOvr, "s_AttribColorOvr");
	glBindAttribLocation(pShad->m_driVertexShader, VERTATTR_TexCoord, "s_AttribTexCoord");

	/*
	pShman->m_mpCoreshkShhand[CORESHK_SpriteUntextured] = ShhandLoad(
													pShman,
													"BasicSpriteShader",
													g_spriteMainVertex,
													g_spriteUntexturedFragment);

	pShad = &pShman->m_aShad[pShman->m_mpCoreshkShhand[CORESHK_SpriteUntextured]];
	glBindAttribLocation(pShad->m_driVertexShader, VERTATTR_Position, "s_AttribPosition");
	glBindAttribLocation(pShad->m_driVertexShader, VERTATTR_Color, "s_AttribColor");
	glBindAttribLocation(pShad->m_driVertexShader, VERTATTR_ColorOvr, "s_AttribColorOvr");
	glBindAttribLocation(pShad->m_driVertexShader, VERTATTR_TexCoord, "s_AttribTexCoord");
	*/

	return true;
}

FrTexture * PTexLoad(const char* pChzFilename, bool fFlipVertically)
{
	// TODO - should replace this with a hash
	FrStringHash shashFilename = ShashCreate(pChzFilename);
	for (int iTex = 0, cTex = s_aryTex.C(); iTex < cTex; ++iTex)
	{
		FrTexture * pTex = &s_aryTex[iTex];
		if (pTex->m_shashFilename.m_hv == shashFilename.m_hv)
			return pTex;
	}

	s32 iTex = s_aryTex.C();
	if ((!FR_FVERIFY(iTex == (s32)iTex, "iTex overflow")) |
		(!FR_FVERIFY(s_aryTex.C() < s_aryTex.CMax(), "s_aryTex overflow")))
		return nullptr;

	FrTexture * pTex = (FrTexture*)s_aryTex.AppendNew();
	pTex->m_cTexmip = 1;
	pTex->m_shashFilename = shashFilename;

	s32 dX, dY;
	s32 cComponents;
	u8* pB = stbi_load(pChzFilename, &dX, &dY, &cComponents, 4);
	if(pB	== 0)
		return 0;
	
	// we'll flip most of our images so the UV origin is in the lower left

	if (fFlipVertically)
	{
		u32 bTemp;
		for (int iLine = 0, cLine = dY>>1; iLine < cLine; ++iLine)
		{
			u32 * pN1 = &((u32*)pB)[iLine * dX];
			u32 * pN2 = &((u32*)pB)[(dY - 1 - iLine) * dX];
			for (u32 * pN1End = &pN1[dX]; pN1 != pN1End; ++pN1, ++pN2)
			{
				bTemp = *pN1;
				*pN1 = *pN2;
				*pN2 = bTemp;
			}
		}
	}

	pTex->m_iTex = iTex;
	pTex->m_aTexmip[0].m_aB = pB;
	pTex->m_aTexmip[0].m_dX = dX;
	pTex->m_aTexmip[0].m_dY = dY;
	pTex->m_driComponents	= GL_RGBA; //numComponents;
	pTex->m_druFormat		= GL_RGBA; //GL_TEXTURE_2D;
	pTex->m_druType			= GL_UNSIGNED_BYTE;
	pTex->m_druTarget		= GL_TEXTURE_2D;

	glGenTextures(1, (GLuint*)&pTex->m_driId);
	glBindTexture(GL_TEXTURE_2D, pTex->m_driId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, pTex->m_cTexmip - 1);

	u32 iTexmip = 0;
	FrTextureMip * pTexmipEnd = &pTex->m_aTexmip[pTex->m_cTexmip];
	for(FrTextureMip * pTexmipIt = pTex->m_aTexmip; pTexmipIt != pTexmipEnd; ++pTexmipIt, ++iTexmip)
	{
		glTexImage2D(
			GL_TEXTURE_2D,
			iTexmip,
			pTex->m_driComponents,
			pTexmipIt->m_dX,
			pTexmipIt->m_dY,
			0,
			pTex->m_druFormat,
			pTex->m_druType,
			pTexmipIt->m_aB);
	}

	return pTex;
}


bool FTryLoadFont(FrFontManager * pFontman, FONTK fontk, const char * pCozFilename)
{
	char aCh[1024];

	// BB - need to change to ConcatPCoz
	ConcatPChz(pCozFilename, ".png", aCh, sizeof(aCh));
	pFontman->m_aFont[fontk].m_pTex = PTexLoad(aCh, false);

	if (!pFontman->m_aFont[fontk].m_pTex)
	{
		printf("Failed to load font texture %s\n", aCh);
		return false;
	}

	// BB - need to change to ConcatPCoz
	ConcatPChz(pCozFilename, ".font", aCh, sizeof(aCh));

	FILE * pFile;
	fopen_s(&pFile, aCh, "rb");
	if (!pFile)
		return false;

	fseek(pFile, 0L, SEEK_END);
	size_t cBFile = ftell(pFile);
	fseek(pFile, 0L, SEEK_SET);

	FrFont * pFont = &pFontman->m_aFont[fontk];
	pFont->m_pGlyphf = (FrFontGlyphFile *)malloc(cBFile); //pAlloc->EWC_ALLOC(cBFile, 16);
	fread(pFont->m_pGlyphf, 1, cBFile, pFile);
	fclose(pFile);

	FrFontGlyphFile * pGlyphf = pFont->m_pGlyphf;
	pFont->m_aWchKerning = (u16 *)(reinterpret_cast<u8 *>(pGlyphf) + pGlyphf->m_iBaWchKerning);
	FR_ASSERT((void *)pFont->m_aWchKerning == (void *)&pGlyphf->m_aGlyph[pGlyphf->m_cGlyph], "bad offsets");

	pFont->m_aDxKerning = (f32 *)(reinterpret_cast<u8 *>(pGlyphf) + pGlyphf->m_iBaDxKerning);
	return true;
}


bool FTryStaticInitFontManager(FrFontManager * pFontman, FrShaderManager * pShman)
{
	if (!FTryLoadFont(pFontman, FONTK_Regular, "Assets/Fonts/Console"))
		return false;

	pFontman->m_aShhand[FONTSHK_Basic] = ShhandLoad(pShman, "BasicFontShader", g_fontMainVertex, g_fontMainFragment);
	pFontman->m_mpFontshkIParamTex[FONTSHK_Basic] = IParamFind(pShman, pFontman->m_aShhand[FONTSHK_Basic], "s_glyphTexture");
	return true;
}

void InitFontd(FrFontData * pFontd)
{
	pFontd->m_colMain = Frog_ColCreate(kFrWhite);
	pFontd->m_colShadow	= Frog_ColCreate(0, 0, 0, 128);

	pFontd->m_gCharSize = 48.0f;
	pFontd->m_rKerning = 1.0f;
	pFontd->m_uOpacity = 1.0f;
	pFontd->m_fontk = FONTK_Regular;
	pFontd->m_alignkX = ALIGNK_Left;
	pFontd->m_alignkY = ALIGNK_Bottom;

	pFontd->m_fUseFixedWidth = false;
	pFontd->m_fUseWordWrap = false;

	pFontd->m_posCursor = Frog_Vec2Create(0.0f, 0.0f);
}

bool FTryStaticInitDrawState(FrDrawState * pDras)
{
	InitFontd(&pDras->m_fontd);

	return true;
}

bool Frog_FTryStaticInitDrawContext(FrDrawContext * pDrac)
{
//	s_aQskey 	   = (SQuadSortKey *)pAlloc->EWC_ALLOC(sizeof(SQuadSortKey) * s_cQskeyMax, FR_ALIGN_OF(SQuadSortKey));
//	s_aQvert 	   = (SQuadVertex *)pAlloc->EWC_ALLOC(sizeof(SQuadVertex) * s_cQvertMax, FR_ALIGN_OF(SQuadVertex));
//	s_aQvertSorted = (SQuadVertex *)pAlloc->EWC_ALLOC(sizeof(SQuadVertex) * s_cQvertMax, FR_ALIGN_OF(SQuadVertex));
//	EWC_CASSERT(s_cQvertMax == s_cQskeyMax * 4, "Bad qvert max calculation");

	if (!FTryStaticInitShaderManager(&pDrac->m_shman))
		return false;

	if (!FTryStaticInitFontManager(&pDrac->m_fontman, &pDrac->m_shman))
		return false;
		//= PFontmanStaticInit(pAlloc, s_drac.m_pShman);
	//pDrac->m_pDrasstk = &s_drasstk;

	pDrac->m_pDras 	  = pDrac->m_drasstk.PDrasTop();
	if (!FTryStaticInitDrawState(pDrac->m_pDras))
		return false;

	return true;
}

void BeginShader(FrShaderManager * pShman, FrShaderHandle shhand)
{
	auto pShad = &pShman->m_aShad[shhand];
	glUseProgram(pShad->m_driProgram);
}

void EndShader()
{
	glUseProgram(0);
}
/*
void DrawTextRaw(const FrDrawContext * pDrac, const CRect & rect, const s16 * aWch)
{
	FrDrawState * pDras = pDrac->m_pDras;
	int iFont = pDras->NGet(DRASK_IFont);
	SFontManager * pFontman = pDrac->m_pFontman;
	SFontGlyphFile * pGlyphf = pFontman->m_aFont[iFont].m_pGlyphf;

	// BB - could mix extents calculation with vertex creation

	// BB - don't need to calculate extents if we're x=ALIGNK_Left, y=ALIGNK_Center
	CVec2 vecDxDy = DXDYExtentsFromAWch(pDrac, aWch);
	float rXY = pDras->GGet(DRASK_GCharSize);

	float x;
	float y;
	ALIGNK alignkX = (ALIGNK)pDras->NGet(DRASK_NAlignkX);
	ALIGNK alignkY = (ALIGNK)pDras->NGet(DRASK_NAlignkY);
	switch (alignkX)
	{
		case ALIGNK_Left:	x = rect.XMin();							break;
		case ALIGNK_Center:	x = rect.XCenter() - vecDxDy.X() * 0.5f;	break;
		case ALIGNK_Right:	x = rect.XMax() - vecDxDy.X();				break;
		default: FR_ASSERT(false, "unknown ALIGNK");
	}

	float dYHalf = (pGlyphf->m_dYAscent + pGlyphf->m_dYDescent) * rXY;
	switch (alignkY)
	{
		case ALIGNK_Top:	y = rect.YMax() - pGlyphf->m_dYAscent * rXY;				break;
		//case ALIGNK_Center:	y = rect.YCenter() - dYHalf - pGlyphf->m_dYDescent * rXY;	break;
		case ALIGNK_Center:	y = rect.YCenter() - 0.5f * pGlyphf->m_dYAscent * rXY;		break;
		case ALIGNK_Bottom:	y = rect.YMin() - pGlyphf->m_dYDescent * rXY;				break;
		default: FR_ASSERT(false, "unknown ALIGNK");
	}

	CVec2 pos(x,y);
	DrawTextRaw(pDrac, pos, aWch);
}
*/
f32 DXFindKerning(FrFont * pFont, int iDxMin, int iDxMax, u32 wchOther)
{
	for(int iDx=iDxMin; iDx<iDxMax; ++iDx)
	{
		if (pFont->m_aWchKerning[iDx] == wchOther)
		{
			return pFont->m_aDxKerning[iDx];
		}
	}
	return 0;
}

inline void FillOutVert (
	f32 x, f32 y, 
	f32 u, f32 v, 
	FrColorVec * pColvec,
	FrFontGlyph * pGlyph, 
	FrFontVertexBuffer * pFvbuf)
{
	FrFontVertex * pFvert = &pFvbuf->m_aFvert[pFvbuf->m_cFvert];
	++pFvbuf->m_cFvert;

	pFvert->m_x = x;
	pFvert->m_y = y;

	pFvert->m_u = u;
	pFvert->m_v = v;

	pFvert->m_r = pColvec->m_x;
	pFvert->m_g = pColvec->m_y;
	pFvert->m_b = pColvec->m_z;
	pFvert->m_a = pColvec->m_w;

	pFvert->m_uMin = pGlyph->m_uMin;
	pFvert->m_vMin = pGlyph->m_vMin;
	pFvert->m_uMax = pGlyph->m_uMax;
	pFvert->m_vMax = pGlyph->m_vMax;
}

FrFontGlyph * PGlyphFind(FrFontGlyphFile * pGlyphf , u32 wch)
{
	// binary search through the glyphs to find our data

	int iLb = 0;
	int iUb = pGlyphf->m_cGlyph;

	FrFontGlyph * pGlyph = pGlyphf->m_aGlyph;
	while (iLb <= iUb)
	{
		int iMid = (iLb + iUb) >> 1;
		u32 chMid = pGlyph[iMid].m_wch;
		if (chMid == wch)
		{
			return &pGlyph[iMid];
		}

		if (wch < chMid)
		{
			iUb = iMid - 1;
		}
		else
		{
			iLb = iMid + 1;
		}
	}

	return nullptr;
}

u32 NCodepoint(const char * pCoz, size_t cBCodepoint)
{
	u32 wch = 0;
	u8* pCh = (u8 *)&wch;

	const char * pCozGlyphEnd = pCoz + cBCodepoint;
	for (const char * pCozGlyph = pCoz; pCozGlyph != pCozGlyphEnd; ++pCozGlyph)
	{
		*pCh++ = *pCozGlyph;
	}
	return wch;
}

void Frog_SetupOrthoViewport(f64 xMin, f64 yMin, f64 xMax, f64 yMax)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(xMin, xMax, yMin, yMax, 1, -1);
	glMatrixMode(GL_MODELVIEW);
}

void CreateVerts(FrFontManager * pFontman , FrDrawState * pDras, FrFontVertexBuffer * pFvbuf, FrVec2 posCursorArg, const char * pCoz)
{
	FrVec2 posCursor = posCursorArg;
	FrColorVec colvec = Frog_ColvecCreate(pDras->m_fontd.m_colMain);

	auto fontk = pDras->m_fontd.m_fontk;
	FrFontGlyphFile * pGlyphf = pFontman->m_aFont[fontk].m_pGlyphf;

	f32 gCharSize = pDras->m_fontd.m_gCharSize;
	f32 rKerning = pDras->m_fontd.m_rKerning;
	//FrFontVertex * pFvert;

	const char * pCozIt = pCoz;
	while (*pCozIt != 0)
	{
		size_t cBCodepoint = CBCodepoint(pCozIt);
		u32 wch = NCodepoint(pCozIt, cBCodepoint);

		auto pGlyph = PGlyphFind(pGlyphf, wch);
		if (pGlyph == nullptr)
		{
			// missing character...
			pCozIt += cBCodepoint;
			continue;
		}

		f32 left		= posCursor.m_x + pGlyph->m_xOffset * gCharSize;
		f32 top			= posCursor.m_y + pGlyph->m_yOffset * gCharSize;
		f32 right		= left + pGlyph->m_dXPixels * gCharSize;
		f32 bottom		= top + pGlyph->m_dYPixels * gCharSize;

		if (pFvbuf->m_cFvert + 4 >= FR_DIM(pFvbuf->m_aFvert))
			break;

		FillOutVert(left, top,
					pGlyph->m_uMin, pGlyph->m_vMax,
					&colvec,
					pGlyph, pFvbuf);

		FillOutVert(right, top,
					pGlyph->m_uMax, pGlyph->m_vMax,
					&colvec,
					pGlyph, pFvbuf);

		FillOutVert(right, bottom,
					pGlyph->m_uMax, pGlyph->m_vMin,
					&colvec,
					pGlyph, pFvbuf);

		FillOutVert(left, bottom,
					pGlyph->m_uMin, pGlyph->m_vMin,
					&colvec,
					pGlyph, pFvbuf);


		pCozIt += cBCodepoint;
		f32 kerningAdvance = 0.0f;

		if (*pCozIt != 0)
		{
			u32 wchNext = NCodepoint(pCozIt, CBCodepoint(pCozIt));

			kerningAdvance = DXFindKerning(
								&pFontman->m_aFont[fontk],
								pGlyph->m_iDxKerningMin,
								pGlyph->m_iDxKerningMax,
								wchNext);
		}

		posCursor.m_x += (pGlyph->m_dXKerningDefault + kerningAdvance) * gCharSize * rKerning;
	}

	pDras->m_fontd.m_posCursor = posCursor;
}

void Frog_FlushFontVerts(FrDrawContext * pDrac)
{
	FrFontVertexBuffer * pFvbuf = &g_Fvbuf;

	// push orthographic projection
	glPushMatrix();
	glLoadIdentity();

	// set translate/scale
	// Point2 translation  = s_activePrimState->getPoint(PrimState::kTranslation);.
	// Point2 scale		= s_activePrimState->getPoint(PrimState::kScale);
	// glTranslatef(translation.x, translation.y, 0.0f);
	// glScalef(scale.x, scale.y, 1.0f);

	// set scissor rect

	// set up shader
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	FR_ASSERT(pDrac != nullptr, "NULL GContext in flushVerts");

	auto pFontman = &pDrac->m_fontman;
	FONTK fontk = pDrac->m_pDras->m_fontd.m_fontk;
	FONTSHK iShhand	= FONTSHK_Basic;
	BeginShader(&pDrac->m_shman, pFontman->m_aShhand[iShhand]);

	//g_fontman.m_mpFontshkIParamTex[FONTSHK.Basic] = IParamFind(pShman, g_fofntman.m_aShhand[FONTSHK.Basic], "s_glyphTexture")

	FrFontVertex * aFvert = pFvbuf->m_aFvert;
	FrTexture * pTex = pFontman->m_aFont[fontk].m_pTex;
	SetShaderParam(pFontman->m_mpFontshkIParamTex[iShhand], pTex, 0);

	glVertexPointer(3, GL_FLOAT, sizeof(FrFontVertex), &aFvert[0].m_x);
	glEnableClientState(GL_VERTEX_ARRAY);
			
	glColorPointer(4, GL_FLOAT, (s32)sizeof(FrFontVertex), &aFvert[0].m_r);
	glEnableClientState(GL_COLOR_ARRAY);

	glClientActiveTexture(GL_TEXTURE0);
	glTexCoordPointer(2, GL_FLOAT, (s32)sizeof(FrFontVertex), &aFvert[0].m_u);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glClientActiveTexture(GL_TEXTURE1);
	glTexCoordPointer(4, GL_FLOAT, (s32)sizeof(FrFontVertex), &aFvert[0].m_uMin);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glDrawArrays(GL_QUADS, 0, pFvbuf->m_cFvert);

	if (pTex)
	{
		glDisable(pTex->m_druTarget);
	}

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	EndShader();

	glPopMatrix();
	pFvbuf->m_cFvert = 0;
}	

void Frog_DrawTextRaw(FrDrawContext * pDrac, FrVec2 pos, const char * pCoz)
{
	// draw characters to the right. pos is the left side baseline

	CreateVerts(&pDrac->m_fontman, pDrac->m_pDras, &g_Fvbuf, pos, pCoz);
}

void Frog_DrawChar(FrDrawContext * pDrac, u32 wch, const FrRect * pRect, FrColor colFg, FrColor colBg)
{
	FrFontManager * pFontman = &pDrac->m_fontman;
	FrDrawState * pDras = pDrac->m_pDras;

	auto fontk = pDras->m_fontd.m_fontk;
	FrFontGlyphFile * aGlyphf = pFontman->m_aFont[fontk].m_pGlyphf;

	f32 gCharSize = 30.0f; //pDras->m_fontd.m_gCharSize;
	f32 rKerning = pDras->m_fontd.m_rKerning;

	auto pGlyph = PGlyphFind(aGlyphf, wch);
	if (pGlyph == nullptr)
		return;	

	FrFontVertexBuffer * pFvbuf = &g_Fvbuf;
	if (pFvbuf->m_cFvert + 8 >= FR_DIM(pFvbuf->m_aFvert))
		return;

	FrColorVec colvecBg = Frog_ColvecCreate(colBg);
	f32 minX = pRect->m_posMin.m_x;
	f32 minY = pRect->m_posMin.m_y;
	f32 maxX = pRect->m_posMax.m_x;
	f32 maxY = pRect->m_posMax.m_y;
	colvecBg.m_w *= -1;
	FillOutVert(minX, maxY,
				pGlyph->m_uMin, pGlyph->m_vMax,
				&colvecBg,
				pGlyph, pFvbuf);

	FillOutVert(maxX, maxY,
				pGlyph->m_uMax, pGlyph->m_vMax,
				&colvecBg,
				pGlyph, pFvbuf);

	FillOutVert(maxX, minY,
				pGlyph->m_uMax, pGlyph->m_vMin,
				&colvecBg,
				pGlyph, pFvbuf);

	FillOutVert(minX, minY,
				pGlyph->m_uMin, pGlyph->m_vMin,
				&colvecBg,
				pGlyph, pFvbuf);

	FrVec2 posCenter = Frog_PosCenter(pRect);
	f32 dXHalf = pGlyph->m_dXPixels * gCharSize * 0.5f;
	f32 dYHalf = pGlyph->m_dYPixels * gCharSize * 0.5f;
	f32 left		= posCenter.m_x - dXHalf;
	f32 top			= posCenter.m_y - dYHalf;
	f32 right		= posCenter.m_x + dXHalf;
	f32 bottom		= posCenter.m_y + dYHalf;

	/*f32 left		= posCenter.m_x + pGlyph->m_xOffset * gCharSize;
	f32 top			= posCenter.m_y + pGlyph->m_yOffset * gCharSize;
	f32 right		= left + pGlyph->m_dXPixels * gCharSize;
	f32 bottom		= top + pGlyph->m_dYPixels * gCharSize;
	*/

	FrColorVec colvecFg = Frog_ColvecCreate(colFg);
	FillOutVert(left, top,
				pGlyph->m_uMin, pGlyph->m_vMax,
				&colvecFg,
				pGlyph, pFvbuf);

	FillOutVert(right, top,
				pGlyph->m_uMax, pGlyph->m_vMax,
				&colvecFg,
				pGlyph, pFvbuf);

	FillOutVert(right, bottom,
				pGlyph->m_uMax, pGlyph->m_vMin,
				&colvecFg,
				pGlyph, pFvbuf);

	FillOutVert(left, bottom,
				pGlyph->m_uMin, pGlyph->m_vMin,
				&colvecFg,
				pGlyph, pFvbuf);

}

FrScreen * Frog_AllocateScreen(int dX, int dY)
{
	size_t cBTile = sizeof(FrScreenTile) * dX * dY;
	size_t cBTotal = sizeof(FrScreen) + cBTile; 
	FrScreen * pScr = (FrScreen*)malloc(cBTotal);
	pScr->m_dX = dX;
	pScr->m_dY = dY;

	FrScreenTile * aTile = (FrScreenTile *)(pScr + 1);
	ZeroAB(aTile, cBTile);
	pScr->m_aTile = aTile;

	return pScr;
}

void Frog_FreeScreen(FrScreen * pScreen)
{
	free(pScreen);
}

void Frog_RenderScreen(FrDrawContext * pDrac, FrScreen * pScr, FrVec2 posUL)
{
	static const float s_dXChar = 20.0f;
	static const float s_dYChar = 30.0f;
	FrRect rect;
	int dY = pScr->m_dY;

	for (int y = 0; y < pScr->m_dY; ++y)
	{
		f32 yMax = posUL.m_y + (y * s_dYChar);
		rect.m_posMax.m_y = yMax;
		rect.m_posMin.m_y = yMax - s_dYChar;

		for (int x = 0; x < pScr->m_dX; ++x)
		{
			f32 xMin = posUL.m_x + (x * s_dXChar);
			rect.m_posMin.m_x = xMin;
			rect.m_posMax.m_x = xMin + s_dXChar;

			FrScreenTile * pTile = &pScr->m_aTile[x + (y * dY)];
			Frog_DrawChar(pDrac, pTile->m_wch, &rect, pTile->m_colFg, pTile->m_colBg);
		}
	}
}

void Frog_MapScreen(FrScreen * pScr, FrTileMap * pTmap, const char * pChzScreen)
{
	for (int y = 0; y < pScr->m_dY; ++y)
	{
		for (int x = 0; x < pScr->m_dX; ++x)
		{
			int iTile = x + (y * pScr->m_dX);
			u8 ch = pChzScreen[iTile];

			FrScreenTile * pTile = &pTmap->m_mpChTile[ch];
			pScr->m_aTile[iTile] = *pTile;
		}
	}
}

void Frog_SetTile(FrTileMap * pTmap, char ch, u32 wchOut, FrColor colFg,  FrColor colBg)
{
	FrScreenTile * pTile = &pTmap->m_mpChTile[ch];
	pTile->m_wch = wchOut;
	pTile->m_colFg = colFg;
	pTile->m_colBg = colBg;
}

