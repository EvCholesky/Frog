#include "FrogRender.h"

#define GL_WRAPPER_IMPLEMENTATION
#include "Array.h"
#include "FontShaders.inl"
#include "GlWrapper.h"
#include "SpriteShaders.inl"
#include "stb_image.h"

#include <cstdarg>
#include <float.h>
#include <stdlib.h>

static const int kChMaxDrawText = 1024;
bool FTryInitDrawState(FrDrawState * pDras);
void BeginShader(FrShaderManager * pShman, FrShaderHandle shhand);
void EndShader();

FROG_CALL const char * PChzFromFTile(FTILE ftile)
{
	static const char * s_mpFTilePChz[] =
	{
		"Collide",
		"Test",
	};
	FR_CASSERT((0x1 << FR_DIM(s_mpFTilePChz)) -1 == FTILE_All, "missing FTILE string");
	if (ftile == FTILE_None)
		return "None";

	for (int iFTile = 0; iFTile < FR_DIM(s_mpFTilePChz); ++iFTile) 
	{
		if ((ftile & (0x1 << iFTile)) != 0)
		{
			FR_ASSERT(ftile == (0x1 << iFTile), "single flag expected in ftile parameter");
			return s_mpFTilePChz[iFTile];
		}
	}

	return "Unknown";
}

FROG_CALL const char * PChzFromFSprite(FSPRITE fsprite)
{
	static const char * s_mpFSpritePChz[] =
	{
		"InsideCorner",
	};
	FR_CASSERT((0x1 << FR_DIM(s_mpFSpritePChz)) -1 == FSPRITE_All, "missing FSPRITE string");
	if (fsprite == FSPRITE_None)
		return "None";

	for (int iFsprite = 0; iFsprite < FR_DIM(s_mpFSpritePChz); ++iFsprite) 
	{
		if ((fsprite & (0x1 << iFsprite)) != 0)
		{
			FR_ASSERT(fsprite == (0x1 << iFsprite), "single flag expected in FSPRITE parameter");
			return s_mpFSpritePChz[iFsprite];
		}
	}

	return "Unknown";
}


struct  FrDrawCall // tag = drcall
{
	int			m_iVertBegin;
	int			m_iVertEnd;
	int			m_iTex;
	DRBUFK		m_drbufk;
	bool		m_fUseScissor;
	s16			m_xScissor;		// left side of scissor rect
	s16			m_yScissor;		// bottom of scissor rect
	s16			m_dXScissor;
	s16			m_dYScissor;
};

struct FrDrawCallBuffer // tag = drcallbuf
{
	FrDrawCall	m_aDrcall[256];
	int			m_cDrcall;
};

void InitDrawCallBuffer(FrDrawCallBuffer * pDrcallbuf);
void PushDrawCall(FrDrawContext * pDrac, FrDrawCallBuffer * pDrcallbuf);
void SubmitDrawCalls(FrDrawContext * pDrac, FrDrawCallBuffer * pDrcallbuf);

struct FrVertex // tag = vert
{
	f32			m_x, m_y;
	f32			m_u, m_v;
	f32			m_r, m_g, m_b, m_a;
};

struct FrVertexBuffer // tag = vbuf
{
	FrVertex		m_aVert[10240]; 
	int				m_cVert;
};


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
FrFontVertexBuffer g_fvbuf;
FrVertexBuffer g_vbuf;
FrDrawCallBuffer g_drcallbuf;


inline s32	CSolveQuadratic(f32 gA, f32 gB, f32 gC, f32 * aGOutput)
{
	f32 gDiscriminant = gB*gB - 4.0f * gA * gC;
	if (gDiscriminant < 0.0f) 
		return 0;

	f32 gRecip = 1.0f / (2.0f * gA);
	f32 gSqrtDescriminant = Frog_GSqrt(gDiscriminant);
	aGOutput[0] = (-gB + gSqrtDescriminant) * gRecip;
	aGOutput[1] = (-gB - gSqrtDescriminant) * gRecip;
	return gDiscriminant > 0.0000001f ? 2 : 1;
}

FROG_CALL f32 Frog_GSmooth(f32 gCurrent, f32 gTarget, const FrSmp * pSmp, f32 dT)
{
	// quadratic style smoother, incoming and outgoing tangents and dT define a curve. If the target is too large to hit with this curve
	// we linearly step towards it with dGMax. If the target is closer we'll hit it sooner than dTMax

	f32 dGMin = pSmp->m_dGMin;
	f32 dGMax = pSmp->m_dGMax;
	f32 dTMax = pSmp->m_dTMax;
	FR_ASSERT((dGMax > 0) & (dGMin > 0) & (dTMax > 0), "expected positive smoothing params");

	f32 gNegate = 1.0f;
	f32 g = gTarget - gCurrent;
	if (g < 0.0f)
	{
		g *= -1.0f;
		gNegate = -1.0f;
	}

	if (g < dGMin * dT)
	{
		return gTarget;
	}

	f32 dG = 0.0f; 

	// find the value of the curve after dTMax
	f32 gCurve = (dGMin + dGMax) * 0.5f * dTMax;
	if (g > gCurve)
	{
		// can't hit the target in dTMax, head towards it at dGMax
		f32 dTMaxSlope = (g - gCurve) / dGMax;
		dTMaxSlope = Frog_GMin(dTMaxSlope, dT);

		if (dTMaxSlope < dT)
			dT += 0.0f;
		dG = dTMaxSlope * dGMax;
		dT -= dTMaxSlope;
	}

	if (dT > 0.0f)
	{
		// take the indefinite integral of the line between our two slopes and set the constant term so 
		//  it's zeros are at g, then solve for x so we can step forward dT

		f32 gA = (dGMin - dGMax) / (2.0f * dTMax);
		f32 gB = dGMax;
		f32 gC = g - dG - gCurve;

		f32 aX[2];
		s32 cRoot = CSolveQuadratic(gA, gB, gC, aX);
		FR_ASSERT((cRoot > 0) & (aX[0] >= 0.0f), "bad solve in gSmooth");

		// our current t relative to the curve [0..dTMax] is aX[0], return aX[0] + dT;
		f32 tNew = aX[0] + dT;
		dG += gA * tNew * tNew + gB * tNew + gC;
	}

	return gCurrent + gNegate * dG;
}

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

	/*
	GLuint sampler = 0;
	glGenSamplers(1, &sampler);

	glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	GLuint texture_unit = //The texture unit to which your texture is bound
	glBindSampler(texture_unit, sampler);
	*/
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

	return true;
}

bool FTryInitShaderManager(FrShaderManager * pShman)
{
	for (int iShad = 0; iShad < FR_DIM(pShman->m_aShad); ++iShad)
	{
		FrShader * pShad = &pShman->m_aShad[iShad];
		pShad->m_driProgram = 0;
		pShad->m_driVertexShader = 0;
		pShad->m_driFragmentShader = 0;
	}

	for (int iShhand = 0; iShhand < FR_DIM(pShman->m_mpCoreshkShhand); ++iShhand)
	{
		pShman->m_mpCoreshkShhand[iShhand] = SHHAND_Nil;
	}

	FrShaderHandle shhandSprite = ShhandLoad(
											pShman,
											"BasicSpriteShader",
											g_spriteMainVertex,
											g_spriteMainFragment);

	pShman->m_mpCoreshkShhand[CORESHK_Sprite] = shhandSprite;
	pShman->m_mpCoreshkIParamTex[CORESHK_Sprite] = IParamFind(pShman, shhandSprite, "s_sampTexture");

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

FROG_CALL FrTexture * Frog_PTexLoad(char* pChzFilename, bool fFlipVertically)
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

FROG_CALL void Frog_SetTextureFilteringNearest(FrTexture * pTex)
{
	glBindTexture(GL_TEXTURE_2D, pTex->m_driId);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

bool FTryLoadFont(FrFontManager * pFontman, FONTK fontk, const char * pCozFilename)
{
	char aCh[1024];

	// BB - need to change to ConcatPCoz
	ConcatPChz(pCozFilename, ".png", aCh, sizeof(aCh));
	pFontman->m_aFont[fontk].m_pTex = Frog_PTexLoad(aCh, false);

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

bool FTryInitFontManager(FrFontManager * pFontman, FrShaderManager * pShman)
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
	pFontd->m_colShadow	= Frog_ColFromRGBA(0, 0, 0, 128);

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

void SetDrawCallState(FrDrawState * pDras, FrDrawCall * pDrcall, int iVertBegin, int iTex)
{
	pDrcall->m_iVertBegin = iVertBegin;
	pDrcall->m_iVertEnd = iVertBegin;
	pDrcall->m_iTex = iTex;
	pDrcall->m_drbufk = pDras->m_drbufk;
	pDrcall->m_fUseScissor = pDras->m_fUseScissor;
	pDrcall->m_xScissor = pDras->m_xScissor;
	pDrcall->m_yScissor = pDras->m_yScissor;
	pDrcall->m_dXScissor = pDras->m_dXScissor;
	pDrcall->m_dYScissor = pDras->m_dYScissor;
}

void AppendDrawCall(FrDrawContext * pDrac, FrDrawCallBuffer * pDrcallbuf, int iVertBegin, int iTex)
{
	// cache off the current draw state. When it changes we'll save off the verts with these settings and make a new draw call

	FR_ASSERT(pDrcallbuf->m_cDrcall < FR_DIM(pDrcallbuf->m_aDrcall), "Draw call overflow");
	FrDrawCall * pDrcall = &pDrcallbuf->m_aDrcall[pDrcallbuf->m_cDrcall];
	++pDrcallbuf->m_cDrcall;

	FrDrawState * pDras = pDrac->m_pDras;
	SetDrawCallState(pDras, pDrcall, iVertBegin, iTex);
}


void Frog_PushDrawCallIfChanged(FrDrawContext * pDrac, DRBUFK drbufkCur)
{
	FrDrawState * pDras = pDrac->m_pDras;
	if (pDras->m_drbufk != drbufkCur)
	{
		pDras->m_drbufk = drbufkCur;
		pDras->m_fNeedsNewCall = true;
	}

	if (pDras->m_fNeedsNewCall)
	{
		PushDrawCall(pDrac, &g_drcallbuf);
	}
}

// save initial state in draw call
// try to draw, if state is dirty - finalize call and save new state

int CVertFromDrbufk(DRBUFK drbufk)
{
	switch (drbufk)
	{
	case DRBUFK_Font:
		return g_fvbuf.m_cFvert;

	case DRBUFK_Sprite:
		return g_vbuf.m_cVert;

	default:
		FR_ASSERT(false, "Unknown DRBUFK (%d)", drbufk);
		return 0;
	}
}

void PushDrawCall(FrDrawContext * pDrac, FrDrawCallBuffer * pDrcallbuf)
{
	FrDrawState * pDras = pDrac->m_pDras;
	pDras->m_fNeedsNewCall = false;
	int iVert  = 0;
	int iTex = 0;
	switch (pDras->m_drbufk)
	{
	case DRBUFK_Font:
		{
			iVert = g_fvbuf.m_cFvert;

			FrTexture * pTex = pDrac->m_fontman.m_aFont[pDras->m_fontd.m_fontk].m_pTex;
			if (pTex)
			{
				iTex = pTex->m_iTex;
			}

			break;
		}

	case DRBUFK_Sprite:
		iVert = g_vbuf.m_cVert;
		iTex = pDras->m_iTexSprite;
		break;
	}

	if (pDrcallbuf->m_cDrcall)
	{
		FrDrawCall * pDrcallTail = &pDrcallbuf->m_aDrcall[pDrcallbuf->m_cDrcall-1];
		pDrcallTail->m_iVertEnd = CVertFromDrbufk(pDrcallTail->m_drbufk);

		if (pDrcallTail->m_iVertEnd == pDrcallTail->m_iVertBegin)
		{
			// no verts with this draw state - just push the new settings into the active draw call

			SetDrawCallState(pDras, pDrcallTail, iVert, iTex);
			return;
		}
	}

	AppendDrawCall(pDrac, pDrcallbuf, iVert, iTex);
}

inline void FillOutSpriteVert (
	f32 x, f32 y, 
	f32 u, f32 v, 
	FrColorVec * pColvec,
	FrVertexBuffer * pVbuf)
{
	FrVertex * pVert = &pVbuf->m_aVert[pVbuf->m_cVert];
	++pVbuf->m_cVert;

	pVert->m_x = x;
	pVert->m_y = y;

	pVert->m_u = u;
	pVert->m_v = v;

	pVert->m_r = pColvec->m_x;
	pVert->m_g = pColvec->m_y;
	pVert->m_b = pColvec->m_z;
	pVert->m_a = pColvec->m_w;
}

FROG_CALL void Frog_DrawSprite(FrDrawContext * pDrac, FrTexture * pTex, FrRect * pRectPos,  FrRect * pRectUv, FrColorVec * pColvec)
{
	Frog_SetSpriteTexture(pDrac, pTex);
	Frog_PushDrawCallIfChanged(pDrac, DRBUFK_Sprite);

	FrVertexBuffer * pVbuf = &g_vbuf;
	if (pVbuf->m_cVert + 4 >= FR_DIM(pVbuf->m_aVert))
		return;

	f32 xMin = pRectPos->m_posMin.m_x;
	f32 yMin = pRectPos->m_posMin.m_y;
	f32 xMax = pRectPos->m_posMax.m_x;
	f32 yMax = pRectPos->m_posMax.m_y;

	f32 uMin = pRectUv->m_posMin.m_x;
	f32 vMin = pRectUv->m_posMin.m_y;
	f32 uMax = pRectUv->m_posMax.m_x;
	f32 vMax = pRectUv->m_posMax.m_y;
	FillOutSpriteVert(xMin, yMax,
				uMin, vMax,	
				pColvec,
				pVbuf);
	FillOutSpriteVert(xMax, yMax,
				uMax, vMax,	
				pColvec,
				pVbuf);
	FillOutSpriteVert(xMax, yMin,
				uMax, vMin,	
				pColvec,
				pVbuf);
	FillOutSpriteVert(xMin, yMin,
				uMin, vMin,	
				pColvec,
				pVbuf);

}

void Frog_FlushSpriteVerts(FrDrawContext * pDrac, int iTex, int iVertBegin, int iVertEnd)
{
	FrVertexBuffer * pVbuf = &g_vbuf;

	// push orthographic projection
	glPushMatrix();
	glLoadIdentity();

	// set up shader
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	FR_ASSERT(pDrac != nullptr, "NULL GContext in flushVerts");

	FrShaderManager * pShman = &pDrac->m_shman;
	BeginShader(pShman, pShman->m_mpCoreshkShhand[CORESHK_Sprite]);

	FrVertex * aVert = pVbuf->m_aVert;
	FrTexture * pTex = NULL;
	
	if (iTex > 0)
	{
		pTex = &s_aryTex[iTex];
		SetShaderParam(pShman->m_mpCoreshkIParamTex[CORESHK_Sprite], pTex, 0);
	}

	glVertexPointer(3, GL_FLOAT, sizeof(FrVertex), &aVert[iVertBegin].m_x);
	glEnableClientState(GL_VERTEX_ARRAY);
			
	glColorPointer(4, GL_FLOAT, (s32)sizeof(FrVertex), &aVert[iVertBegin].m_r);
	glEnableClientState(GL_COLOR_ARRAY);

	glClientActiveTexture(GL_TEXTURE0);
	glTexCoordPointer(2, GL_FLOAT, (s32)sizeof(FrVertex), &aVert[iVertBegin].m_u);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glDrawArrays(GL_QUADS, 0, iVertEnd - iVertBegin);

	if (pTex)
	{
		glDisable(pTex->m_druTarget);
	}

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisable(GL_BLEND);

	EndShader();

	glPopMatrix();
}	


void SubmitDrawCalls(FrDrawContext * pDrac, FrDrawCallBuffer * pDrcallbuf)
{
	PushDrawCall(pDrac, pDrcallbuf);
	for (int iDrcall = 0; iDrcall < pDrcallbuf->m_cDrcall; ++iDrcall)
	{
		FrDrawCall * pDrcall = &pDrcallbuf->m_aDrcall[iDrcall];
		if (pDrcall->m_iVertEnd == pDrcall->m_iVertBegin)
			continue;

		if (pDrcall->m_fUseScissor)
		{
			glEnable(GL_SCISSOR_TEST);
			glScissor(pDrcall->m_xScissor, pDrcall->m_yScissor, pDrcall->m_dXScissor, pDrcall->m_dYScissor);
		}
		else
		{
			glDisable(GL_SCISSOR_TEST);
		}

		switch (pDrcall->m_drbufk)
		{
		case DRBUFK_Font:
			Frog_FlushFontVerts(pDrac, pDrcall->m_iVertBegin, pDrcall->m_iVertEnd);
			break;

		case DRBUFK_Sprite:
			Frog_FlushSpriteVerts(pDrac, pDrcall->m_iTex, pDrcall->m_iVertBegin, pDrcall->m_iVertEnd);
			break;
		}
	}

	g_fvbuf.m_cFvert = 0;
	g_vbuf.m_cVert = 0;
	pDrcallbuf->m_cDrcall = 0;

	FTryInitDrawState(pDrac->m_pDras);
	PushDrawCall(pDrac, pDrcallbuf);
	glDisable(GL_SCISSOR_TEST);
}

void Frog_FlushDrawCalls(FrDrawContext * pDrac)
{
	SubmitDrawCalls(pDrac, &g_drcallbuf);
}

FROG_CALL void Frog_SetSpriteTexture(FrDrawContext * pDrac, FrTexture * pTex)
{
	int iTex = -1;
	if (pTex)
	{
		iTex = pTex->m_iTex;
	}
		
	FrDrawState * pDras = pDrac->m_pDras;
	if (pDras->m_iTexSprite == iTex)
		return;

	pDras->m_fNeedsNewCall = true;
	pDras->m_iTexSprite = iTex;
}

FROG_CALL void Frog_SetScissor(FrDrawContext * pDrac, s16 xScissor, s16 yScissor, s16 dXScissor, s16 dYScissor)
{
	FrDrawState * pDras = pDrac->m_pDras;
	if (pDras->m_fUseScissor && 
		pDras->m_xScissor == xScissor &&
		pDras->m_yScissor == yScissor &&
		pDras->m_dXScissor == dXScissor &&
		pDras->m_dYScissor == dYScissor)
	{
		return;
	}

	pDras->m_fNeedsNewCall = true;
	pDras->m_fUseScissor = true;
	pDras->m_xScissor = xScissor;
	pDras->m_yScissor = yScissor;
	pDras->m_dXScissor = dXScissor;
	pDras->m_dYScissor = dYScissor;
}

FROG_CALL void Frog_DisableScissor(FrDrawContext * pDrac)
{
	FrDrawState * pDras = pDrac->m_pDras;
	if (pDras->m_fUseScissor)
	{
		pDras->m_fUseScissor = false;
		pDras->m_fNeedsNewCall = true;
	}
}

bool FTryInitDrawState(FrDrawState * pDras)
{
	InitFontd(&pDras->m_fontd);
	pDras->m_drbufk = DRBUFK_Font;
	pDras->m_fNeedsNewCall = true;
	pDras->m_iTexSprite = -1;

	pDras->m_fUseScissor = false;
	pDras->m_xScissor = SHRT_MIN;
	pDras->m_yScissor = SHRT_MIN;
	pDras->m_dXScissor = SHRT_MAX;
	pDras->m_dYScissor = SHRT_MAX;
	return true;
}

bool Frog_FTryStaticInitDrawContext(FrDrawContext * pDrac)
{
	if (!FTryStaticInitShaderManager(&pDrac->m_shman))
		return false;

	g_fvbuf.m_cFvert = 0;
	g_vbuf.m_cVert = 0;
	g_drcallbuf.m_cDrcall = 0;

	return true;
}

bool Frog_FTryInitDrawContext(FrDrawContext * pDrac)
{
//	s_aQskey 	   = (SQuadSortKey *)pAlloc->EWC_ALLOC(sizeof(SQuadSortKey) * s_cQskeyMax, FR_ALIGN_OF(SQuadSortKey));
//	s_aQvert 	   = (SQuadVertex *)pAlloc->EWC_ALLOC(sizeof(SQuadVertex) * s_cQvertMax, FR_ALIGN_OF(SQuadVertex));
//	s_aQvertSorted = (SQuadVertex *)pAlloc->EWC_ALLOC(sizeof(SQuadVertex) * s_cQvertMax, FR_ALIGN_OF(SQuadVertex));
//	EWC_CASSERT(s_cQvertMax == s_cQskeyMax * 4, "Bad qvert max calculation");

	if (!FTryInitShaderManager(&pDrac->m_shman))
		return false;

	if (!FTryInitFontManager(&pDrac->m_fontman, &pDrac->m_shman))
		return false;

	pDrac->m_pDras = Frog_PDrasTop(&pDrac->m_drasstk);
	if (!FTryInitDrawState(pDrac->m_pDras))
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

void Frog_FlushFontVerts(FrDrawContext * pDrac, int iVertBegin, int iVertEnd)
{
	FrFontVertexBuffer * pFvbuf = &g_fvbuf;

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

	glVertexPointer(3, GL_FLOAT, sizeof(FrFontVertex), &aFvert[iVertBegin].m_x);
	glEnableClientState(GL_VERTEX_ARRAY);
			
	glColorPointer(4, GL_FLOAT, (s32)sizeof(FrFontVertex), &aFvert[iVertBegin].m_r);
	glEnableClientState(GL_COLOR_ARRAY);

	glClientActiveTexture(GL_TEXTURE1);
	glTexCoordPointer(4, GL_FLOAT, (s32)sizeof(FrFontVertex), &aFvert[iVertBegin].m_uMin);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glClientActiveTexture(GL_TEXTURE0);
	glTexCoordPointer(2, GL_FLOAT, (s32)sizeof(FrFontVertex), &aFvert[iVertBegin].m_u);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glDrawArrays(GL_QUADS, 0, iVertEnd - iVertBegin);

	if (pTex)
	{
		glDisable(pTex->m_druTarget);
	}

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisable(GL_BLEND);

	EndShader();

	glPopMatrix();
}	


FROG_CALL void Frog_DrawTextRaw(FrDrawContext * pDrac, FrVec2 pos, const char * pCoz)
{
	Frog_PushDrawCallIfChanged(pDrac, DRBUFK_Font);

	// draw characters to the right. pos is the left side baseline

	CreateVerts(&pDrac->m_fontman, pDrac->m_pDras, &g_fvbuf, pos, pCoz);
}

FrVec2 DXDYExtentsFromAWch(FrDrawContext * pDrac, const char * pCoz)
{
	FrFontManager * pFontman = &pDrac->m_fontman;
	auto fontk = pDrac->m_pDras->m_fontd.m_fontk;
	FrFontGlyphFile * pGlyphf = pFontman->m_aFont[fontk].m_pGlyphf;

	FrVec2 posCursor = Frog_Vec2Create(0.0f, 0.0f);
	float yMin = FLT_MAX;
	float yMax = 0; 

	FrDrawState * pDras = pDrac->m_pDras;
	float rXY = pDras->m_fontd.m_gCharSize;

	const char * pCozIt = pCoz;
	while (*pCozIt != 0)
	{
		size_t cBCodepoint = CBCodepoint(pCozIt);
		u32 wch = NCodepoint(pCozIt, cBCodepoint);
		pCozIt += cBCodepoint;

		FrFontGlyph * pGlyph = PGlyphFind(pGlyphf, wch);

		float xRight = posCursor.m_x;
		if (pGlyph)
		{
			float yBottom	= posCursor.m_y + pGlyph->m_yOffset * rXY;
			float yTop		= yBottom + pGlyph->m_dYPixels * rXY;

			yMin = frMin(yMin, yBottom);
			yMax = frMax(yMax, yTop);

			float xLeft		= posCursor.m_x + pGlyph->m_xOffset * rXY;
			xRight	= xLeft + pGlyph->m_dXPixels * rXY;
		}

		if (*pCozIt == 0)
		{
			return Frog_Vec2Create(xRight, yMax - yMin);
		}

		u32 wchNext = NCodepoint(pCozIt, CBCodepoint(pCozIt));

		f32 kerningAdvance = DXFindKerning(
							&pFontman->m_aFont[fontk],
							pGlyph->m_iDxKerningMin,
							pGlyph->m_iDxKerningMax,
							wchNext);
		
		posCursor.m_x += (pGlyph->m_dXKerningDefault + kerningAdvance) * rXY;
	}

	return Frog_Vec2Create(0.0f, 0.0f);
}


FROG_CALL void Frog_DrawTextAligned(FrDrawContext * pDrac, FrVec2 pos, const char * pCoz)
{

	FrDrawState * pDras = pDrac->m_pDras;
	// BB - could mix extents calculation with vertex creation

	FrVec2 vecDxDy = Frog_Vec2Create(0.0f, 0.0f);
	ALIGNK alignkX = pDras->m_fontd.m_alignkX;
	ALIGNK alignkY = pDras->m_fontd.m_alignkY;
	if (alignkX != ALIGNK_Left || alignkY != ALIGNK_Bottom)
	{
		// Don't need to calculate extents if we're x=ALIGNK_Left, y=ALIGNK_Bottom
		vecDxDy = DXDYExtentsFromAWch(pDrac, pCoz);
	}

	float x;
	switch (alignkX)
	{
		case ALIGNK_Left:	x = pos.m_x;						break;
		case ALIGNK_Center:	x = pos.m_x - vecDxDy.m_x * 0.5f;	break;
		case ALIGNK_Right:	x = pos.m_x - vecDxDy.m_x;			break;
		default: FR_ASSERT(false, "unknown ALIGNK");
	}

	float y;
	switch (alignkY)
	{
		case ALIGNK_Top:	y = pos.m_y - vecDxDy.m_y;			break;
		case ALIGNK_Center:	y = pos.m_y - vecDxDy.m_y * 0.5f;	break;
		case ALIGNK_Bottom:	y = pos.m_y;						break;
		default: FR_ASSERT(false, "unknown ALIGNK");
	}

	// draw characters to the right. pos is the left side baseline

	FrVec2 posCursor = Frog_Vec2Create(x, y);
	Frog_DrawTextRaw(pDrac, posCursor, pCoz);
}

FROG_CALL void Frog_DrawChar(FrDrawContext * pDrac, u32 wch, const FrRect * pRect, FrColor colFg, FrColor colBg, float rRGB)
{
	FrFontManager * pFontman = &pDrac->m_fontman;
	FrDrawState * pDras = pDrac->m_pDras;

	auto fontk = pDras->m_fontd.m_fontk;
	FrFontGlyphFile * aGlyphf = pFontman->m_aFont[fontk].m_pGlyphf;

	f32 gCharSize = pDras->m_fontd.m_gCharSize;
	f32 rKerning = pDras->m_fontd.m_rKerning;

	auto pGlyph = PGlyphFind(aGlyphf, wch);
	if (pGlyph == nullptr)
		return;	

	FrFontVertexBuffer * pFvbuf = &g_fvbuf;
	if (pFvbuf->m_cFvert + 8 >= FR_DIM(pFvbuf->m_aFvert))
		return;

	FrColorVec colvecBg = Frog_ColvecCreate(colBg);
	f32 minX = pRect->m_posMin.m_x;
	f32 minY = pRect->m_posMin.m_y;
	f32 maxX = pRect->m_posMax.m_x;
	f32 maxY = pRect->m_posMax.m_y;

	colvecBg.m_x *= rRGB;
	colvecBg.m_y *= rRGB;
	colvecBg.m_z *= rRGB;
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
	colvecFg.m_x *= rRGB;
	colvecFg.m_y *= rRGB;
	colvecFg.m_z *= rRGB;
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

FROG_CALL void Frog_InitTransition(FrRoomTransition * pRoomt)
{
	pRoomt->m_pRoomPrev = nullptr;
	pRoomt->m_pRoom = nullptr;
	pRoomt->m_r = 0.0f;
	pRoomt->m_roomtk = ROOMTK_None;
}

FROG_CALL void Frog_SetTransition(FrRoomTransition * pRoomt, ROOMTK roomtk, FrRoom * pRoomPrev, FrRoom * pRoomNext)
{
	FR_ASSERT(roomtk != ROOMTK_None || pRoomPrev == nullptr, "expected single screen for SCRTRK_None");
	pRoomt->m_pRoomPrev = pRoomPrev;
	pRoomt->m_pRoom = pRoomNext;
	pRoomt->m_r = 0.0f;
	pRoomt->m_roomtk = roomtk;
}

FROG_CALL void Frog_UpdateTransition(FrRoomTransition * pRoomt, f32 dT)
{
	static FrSmp s_smp = { 0.2f, 30.0f, 3.0f };
	if (pRoomt->m_roomtk != ROOMTK_None)
	{
		pRoomt->m_r = Frog_GSmooth(pRoomt->m_r, 1.0f, &s_smp, dT);
	}
}

FROG_CALL void Frog_RenderTransition(FrDrawContext * pDrac, FrTileWorld * pTworld, FrRoomTransition * pRoomt, FrVec2 posUL)
{
	float rRgbNew = 1.0f;
	FrVec2 posNew = posUL;
	if (pRoomt->m_roomtk == ROOMTK_Translate)
	{
		f32 rSlide = Frog_GCurveS(pRoomt->m_r);
		FrVec2 posPrev = Frog_Vec2MulAdd(&posUL, &pRoomt->m_dPos, rSlide);

		rRgbNew = Frog_GMin(pRoomt->m_r * 4.0f, 1.0f);
		float rRgbPrev = 1.0f - (Frog_GMax(0.0f, pRoomt->m_r - 0.75f) * 4.0f);

		Frog_RenderRoom(pDrac, pTworld, pRoomt->m_pRoomPrev, posPrev, rRgbPrev);
		posNew = Frog_Vec2Sub(&posPrev, &pRoomt->m_dPos);
	}

	Frog_RenderRoom(pDrac, pTworld, pRoomt->m_pRoom, posNew, rRgbNew);
}

FROG_CALL FrVec2 PosWinToRm(FrCamera * pCam)
{
	FrVec2 dPosWinToCam = Frog_Vec2Add(&pCam->m_posScissor, &pCam->m_dPosScToCam);
	return Frog_Vec2Sub(&dPosWinToCam, &pCam->m_posRm);
}

FROG_CALL void Frog_FindCellFromPosSc(FrCamera * pCam, FrRoom * pRoom, FrVec2 * pPosSc, int * pXCell, int * pYCell)
{
	FrVec2 dPosWinToRoom = PosWinToRm(pCam);

	FrVec2 posRoom = Frog_Vec2MulAdd(pPosSc, &dPosWinToRoom, -1.0f);
	*pXCell = (int)(posRoom.m_x / pCam->m_dXyCell.m_x);
	*pYCell = (int)(posRoom.m_y / pCam->m_dXyCell.m_y);
}

FROG_CALL void Frog_RenderTransitionWithCamera(FrDrawContext * pDrac, FrTileWorld * pTworld, FrRoomTransition * pRoomt)
{
	FrCamera * pCam = &pTworld->m_cam;
	FrVec2 dPosWinToRoom = PosWinToRm(pCam);

	dPosWinToRoom.m_x = roundf(dPosWinToRoom.m_x);
	dPosWinToRoom.m_y = roundf(dPosWinToRoom.m_y);

	float rRgbNew = 1.0f;
	if (pRoomt->m_roomtk == ROOMTK_Translate)
	{
		f32 rSlide = Frog_GCurveS(pRoomt->m_r);
		FrVec2 posPrev = Frog_Vec2MulAdd(&dPosWinToRoom, &pRoomt->m_dPos, rSlide);

		rRgbNew = Frog_GMin(pRoomt->m_r * 4.0f, 1.0f);
		float rRgbPrev = 1.0f - (Frog_GMax(0.0f, pRoomt->m_r - 0.75f) * 4.0f);

		FrVec2 posPrevWithOffset = Frog_Vec2Add(&posPrev, &pRoomt->m_dPosPrevRoom);
		Frog_RenderRoom(pDrac, pTworld, pRoomt->m_pRoomPrev, posPrevWithOffset, rRgbPrev);
		dPosWinToRoom = Frog_Vec2Sub(&posPrev, &pRoomt->m_dPos);
	}

	Frog_RenderRoom(pDrac, pTworld, pRoomt->m_pRoom, dPosWinToRoom, rRgbNew);
}

FROG_CALL void Frog_RenderRoom(FrDrawContext * pDrac, FrTileWorld * pTworld, FrRoom * pRoom, FrVec2 posLL, float rRGB)
{
	FrRect rect;
	int dX = pRoom->m_dX;
	int dY = pRoom->m_dY;
	float dXCharPixel = pRoom->m_dXCharPixel;
	float dYCharPixel = pRoom->m_dYCharPixel;

	FrDrawState * pDras = pDrac->m_pDras;
	float gCharSizePrev = pDras->m_fontd.m_gCharSize;
	pDras->m_fontd.m_gCharSize = ceilf(dYCharPixel * 0.85f);

	// draw sprite room first

	if (pTworld->m_pTexSprite)
	{
		FrColorVec colvecWhite = Frog_ColvecCreate(Frog_ColCreate(0xFFFFFFFF));
		FrTexture * pTex = pTworld->m_pTexSprite;

		Frog_PushDrawCallIfChanged(pDrac, DRBUFK_Sprite);

		for (int y = 0; y < dY; ++y)
		{
			f32 yMin = posLL.m_y + (y * dYCharPixel);
			rect.m_posMin.m_y = yMin;
			rect.m_posMax.m_y = yMin + dYCharPixel;

			for (int x = 0; x < dX; ++x)
			{
				f32 xMin = posLL.m_x + (x * dXCharPixel);
				rect.m_posMin.m_x = xMin;
				rect.m_posMax.m_x = xMin + dXCharPixel;

				int iCell = x + (y * dX);
				FrScreenTile * pTile = &pRoom->m_mpICellTileEnv[iCell];
				ENTID entid = pRoom->m_mpICellEntid[iCell];

				if (pTile->m_iSptile >= 0)
				{
					int iSptile = pTile->m_iSptile;
					FrSpriteTile * pSptile = &pTworld->m_tmap.m_aSptile[pTile->m_iSptile];
					FrRect rectUv = Frog_RectCreate(pSptile->m_uMin, pSptile->m_vMin, pSptile->m_uMax, pSptile->m_vMax);

					int xLeft = Frog_NMax(0, x - 1);
					int xRight = Frog_NMin(dX, x + 1);
					int yUp = Frog_NMin(dY, y + 1);
					int yDown = Frog_NMax(0, y - 1);
					int iSptileUp = pRoom->m_mpICellTileEnv[x + (yUp * dX)].m_iSptile;
					int iSptileDown = pRoom->m_mpICellTileEnv[x + (yDown * dX)].m_iSptile;
					int iSptileLeft = pRoom->m_mpICellTileEnv[xLeft + (y * dX)].m_iSptile;
					int iSptileRight = pRoom->m_mpICellTileEnv[xRight + (y * dX)].m_iSptile;
					int iSptileUL = pRoom->m_mpICellTileEnv[xLeft + (yUp * dX)].m_iSptile;
					int iSptileUR = pRoom->m_mpICellTileEnv[xRight + (yUp * dX)].m_iSptile;
					int iSptileDL = pRoom->m_mpICellTileEnv[xLeft + (yDown * dX)].m_iSptile;
					int iSptileDR = pRoom->m_mpICellTileEnv[xRight + (yDown * dX)].m_iSptile;

					int diSpriteUp = (int)(iSptileUp != iSptile) * 2;
					int diSpriteDown = (int)(iSptileDown != iSptile) * 2;
					int diSpriteLeft = (int)(iSptileLeft != iSptile) * 1;
					int diSpriteRight = (int)(iSptileRight != iSptile) * 1;
					int diSpriteUL = (int)((diSpriteUp + diSpriteLeft == 0) & (iSptileUL != iSptile)) * 4;
					int diSpriteUR = (int)((diSpriteUp + diSpriteRight == 0) & (iSptileUR != iSptile)) * 4;
					int diSpriteDL = (int)((diSpriteDown + diSpriteLeft == 0) & (iSptileDL != iSptile)) * 4;
					int diSpriteDR = (int)((diSpriteDown + diSpriteRight == 0) & (iSptileDR != iSptile)) * 4;

					if ((pSptile->m_grfsprite & FSPRITE_InsideCorner) == 0 ||
						(diSpriteUp + diSpriteDown + diSpriteLeft + diSpriteRight == 0))
					{
						Frog_DrawSprite(pDrac, pTex, &rect, &rectUv, &colvecWhite);
					}
					else
					{
						int iSptileUL = diSpriteUp + diSpriteLeft + diSpriteUL;
						int iSptileUR = diSpriteUp + diSpriteRight + diSpriteUR;
						int iSptileDL = diSpriteDown + diSpriteLeft + diSpriteDL;
						int iSptileDR = diSpriteDown + diSpriteRight + diSpriteDR;

						float dUTile = 32.0f / 512.0f;
						float dUHalf = 16.0f / 512.0f;
						float dVHalf = 16.0f / 512.0f;

						float xMin = rect.m_posMin.m_x;
						float yMin = rect.m_posMin.m_y;
						float xMax = rect.m_posMax.m_x;
						float yMax = rect.m_posMax.m_y;
						float xMid = xMin + 0.5f * (xMax - xMin);
						float yMid = yMin + 0.5f * (yMax - yMin);

						float uMin = rectUv.m_posMin.m_x;
						float vMin = rectUv.m_posMin.m_y;
						float uMax = rectUv.m_posMax.m_x;
						float vMax = rectUv.m_posMax.m_y;
						float uMid = uMin + 0.5f * (uMax - uMin);
						float vMid = vMin + 0.5f * (vMax - vMin);

						FrRect rectPosUL = Frog_RectCreate(xMin, yMid, xMid, yMax);
						FrRect rectUvUL = Frog_RectCreate(
													uMin + iSptileUL * dUTile, 
													vMid,
													uMid + iSptileUL * dUTile,
													vMax );
						Frog_DrawSprite(pDrac, pTex, &rectPosUL, &rectUvUL, &colvecWhite);

						FrRect rectPosUR = Frog_RectCreate(xMid, yMid, xMax, yMax);
						FrRect rectUvUR = Frog_RectCreate(
													uMid + iSptileUR * dUTile, 
													vMid,
													uMax + iSptileUR * dUTile,
													vMax );
						Frog_DrawSprite(pDrac, pTex, &rectPosUR, &rectUvUR, &colvecWhite);

						FrRect rectPosDL = Frog_RectCreate(xMin, yMin, xMid, yMid);
						FrRect rectUvDL = Frog_RectCreate(
													uMin + iSptileDL * dUTile, 
													vMin,
													uMid + iSptileDL * dUTile,
													vMid );
						Frog_DrawSprite(pDrac, pTex, &rectPosDL, &rectUvDL, &colvecWhite);

						FrRect rectPosDR = Frog_RectCreate(xMid, yMin, xMax, yMid);
						FrRect rectUvDR = Frog_RectCreate(
													uMid + iSptileDR * dUTile, 
													vMin,
													uMax + iSptileDR * dUTile,
													vMid );
						Frog_DrawSprite(pDrac, pTex, &rectPosDR, &rectUvDR, &colvecWhite);

					}
				}

				if (entid != ENTID_Nil)
				{
					FrEntity * pEnt = &pTworld->m_aEnt[entid];
					FrScreenTile * pTileEnt = &pTworld->m_tmap.m_mpChTile[pEnt->m_iTile];

					int iSptileEnt = pTileEnt->m_iSptile;
					if (iSptileEnt >= 0)
					{
						FrSpriteTile * pSptile = &pTworld->m_tmap.m_aSptile[iSptileEnt];
						FrRect rectUv = Frog_RectCreate(pSptile->m_uMin, pSptile->m_vMin, pSptile->m_uMax, pSptile->m_vMax);
						Frog_DrawSprite(pDrac, pTex, &rect, &rectUv, &colvecWhite);
					}
				}
			}
		}
	}

	Frog_PushDrawCallIfChanged(pDrac, DRBUFK_Font);

	for (int y = 0; y < pRoom->m_dY; ++y)
	{
		f32 yMin = posLL.m_y + (y * dYCharPixel);
		rect.m_posMin.m_y = yMin;
		rect.m_posMax.m_y = yMin + dYCharPixel;

		for (int x = 0; x < pRoom->m_dX; ++x)
		{
			f32 xMin = posLL.m_x + (x * dXCharPixel);
			rect.m_posMin.m_x = xMin;
			rect.m_posMax.m_x = xMin + dXCharPixel;

			int iCell = x + (y * dX);
			FrScreenTile * pTile = &pRoom->m_mpICellTileEnv[iCell];
			ENTID entid = pRoom->m_mpICellEntid[iCell];
			if (entid != ENTID_Nil)
			{
				FrEntity * pEnt = &pTworld->m_aEnt[entid];
				FrScreenTile * pTile = &pTworld->m_tmap.m_mpChTile[pEnt->m_iTile];
				Frog_DrawChar(pDrac, pTile->m_wch, &rect, pTile->m_colFg, pTile->m_colBg, rRGB);
			}
			else
			{
				Frog_DrawChar(pDrac, pTile->m_wch, &rect, pTile->m_colFg, pTile->m_colBg, rRGB);
			}
		}
	}

	pDras->m_fontd.m_gCharSize = gCharSizePrev;
}

FROG_CALL void Frog_SetRoomTiles(FrRoom * pRoom, FrTileMap * pTmap, const char * pChzScreen)
{
	for (int y = 0; y < pRoom->m_dY; ++y)
	{
		for (int x = 0; x < pRoom->m_dX; ++x)
		{
			int iTileFlip = x + ((pRoom->m_dY - 1 - y) * pRoom->m_dX);
			int iTile = x + (y * pRoom->m_dX);
			u8 ch = pChzScreen[iTile];

			FrScreenTile * pTile = &pTmap->m_mpChTile[ch];
			pRoom->m_mpICellTileEnv[iTileFlip] = *pTile;
		}
	}
}

FROG_CALL void Frog_SetRoomTile(FrRoom * pRoom, FrTileMap * pTmap, int xTile, int yTile, int ch)
{
	int iTile = xTile + (yTile * pRoom->m_dX);

	FrScreenTile * pTile = &pTmap->m_mpChTile[ch];
	pRoom->m_mpICellTileEnv[iTile] = *pTile;
}

FROG_CALL void Frog_SetTile(FrTileMap * pTmap, char ch, u32 wchOut, FrColor colFg,  FrColor colBg, s16 iSptile, u8 ftile)
{
	FrScreenTile * pTile = &pTmap->m_mpChTile[ch];
	pTile->m_wch = wchOut;
	pTile->m_colFg = colFg;
	pTile->m_colBg = colBg;
	pTile->m_iSptile = iSptile;
	pTile->m_ftile = ftile;
}

FROG_CALL void Frog_SetSpriteTile(FrTileMap * pTmap, s16 iSptile, f32 uMin, f32 vMin, f32 uMax, f32 vMax, u8 grfsprite)
{
	FrSpriteTile * pSptile = &pTmap->m_aSptile[iSptile];
	pSptile->m_uMin = uMin;
	pSptile->m_vMin = vMin;
	pSptile->m_uMax = uMax;
	pSptile->m_vMax = vMax;
	pSptile->m_grfsprite = grfsprite;
}

FROG_CALL void Frog_InitCellContents(ENTID * aEntid, int cEntidMax, FrCellContents * pCellc)
{
	pCellc->m_tile.m_wch = 0;

	pCellc->m_aEntid = aEntid;
	pCellc->m_cEntidMax = cEntidMax;
	pCellc->m_cEntid = 0;
}

FROG_CALL void Frog_FindCellContents(FrTileWorld * pTworld, FrRoom * pRoom, int xCell, int yCell, FrCellContents * pCellc)
{
	if (xCell < 0 || xCell >= pRoom->m_dX || yCell < 0 || yCell >= pRoom->m_dY) 
		return;

	int iCell = xCell + yCell * pRoom->m_dX;
	pCellc->m_tile = pRoom->m_mpICellTileEnv[iCell];

	ENTID entid = pRoom->m_mpICellEntid[iCell];
	while (entid != ENTID_Nil)
	{
		if (pCellc->m_cEntid >= pCellc->m_cEntidMax)
		{
			FR_ASSERT(false, "Cell content overflow");
			break;
		}

		pCellc->m_aEntid[pCellc->m_cEntid++] = entid;

		FrEntity * pEnt = &pTworld->m_aEnt[entid];
		entid = pEnt->m_entidNextCell;
	}
}

FROG_CALL u8 Frog_FtileFromCell(FrRoom * pRoom, int xCell, int yCell)
{
	int iCell = xCell + yCell * pRoom->m_dX;
	return pRoom->m_mpICellTileEnv[iCell].m_ftile;
}

static void InitializeFreeRoster(int * pCAllocated, s32 * aNId, int cNIdMax)
{
	s32 n = 0;
	s32 * pNMax = &aNId[cNIdMax];
	for (s32 * pN = aNId; pN != pNMax; ++pN)
	{
		*pN = n++;
	}

	*pCAllocated = 0;
}

#define DEBUG_ROSTER 1
static s32 IdAllocateFromRoster(s32 * aNId, int * pCAllocate, int cNIdMax)
{
	if (*pCAllocate >= cNIdMax)
		return -1;

	(*pCAllocate)++;
	s32 nReturn = aNId[cNIdMax - *pCAllocate];

#if DEBUG_ROSTER
	// make sure this entity is not listed twice in the roster
	s32 * pNMax = FR_PMAX(aNId);
	for (s32 * pN = aNId; pN != pNMax; ++pN)
	{
		FR_ASSERT(*pN != nReturn, "entity id listed twice in the free entity list");
	}
#endif

	return nReturn;
}

static void FreeToRoster(s32 nId, s32 * aNId, int * pCAllocate, int cNIdMax)
{
	if (FR_FVERIFY(*pCAllocate > 0, "cannot free entity, no entities are allocated"))
	{

#if DEBUG_ROSTER
		// make sure this entity is not already listed as freed
		s32 * pNMax = FR_PMAX(aNId);
		for (s32 * pN = aNId; pN != pNMax; ++pN)
		{
			FR_ASSERT(*pN != nId, "trying to list freed entity id");
		}
#endif

		aNId[cNIdMax - *pCAllocate] = nId;
		*pCAllocate--;
	}
}

FROG_CALL void Frog_InitTileWorld(FrTileWorld * pTworld)
{
	ZeroAB(pTworld->m_tmap.m_mpChTile, sizeof(pTworld->m_tmap.m_mpChTile));
	ZeroAB(pTworld->m_aEnt, sizeof(pTworld->m_aEnt));

	InitializeFreeRoster(&pTworld->m_cEntAllocated, (s32*)pTworld->m_aEntidFree, FR_DIM(pTworld->m_aEntidFree));
	InitializeFreeRoster(&pTworld->m_cRoomAllocated, (s32*)pTworld->m_aRoomidFree, FR_DIM(pTworld->m_aRoomidFree));
	pTworld->m_nGenRoom = 1;
	pTworld->m_pTexSprite = NULL;

	FrVec2 pPosScissor = Frog_Vec2Create(1.0f, 1.0f);
	FrVec2 dXyScissor = Frog_Vec2Create(1.0f, 1.0f);
	FrVec2 dXyCell = Frog_Vec2Create(32.0f, 32.0f);
	Frog_SetViewParams(&pTworld->m_cam, &pPosScissor, &dXyScissor, &dXyCell);
}

FROG_CALL void Frog_SetViewParams(FrCamera * pCam, FrVec2 * pPosScissor, FrVec2 * pDXyScissor,  FrVec2 * pDXyCell)
{
	pCam->m_posRm = Frog_Vec2Create(0.0f, 0.0f);
	pCam->m_posRmDesired = pCam->m_posRm;

	pCam->m_posScissor = *pPosScissor;

	pCam->m_dXyScissor = *pDXyScissor; 
	pCam->m_dPosScToCam = Frog_Vec2Mul(pDXyScissor, 0.5f);

	pCam->m_dXyCell = *pDXyCell;
}

FROG_CALL void Frog_CameraLookAt(FrCamera * pCam, f32 xFocusRm, f32 yFocusRm)
{
	pCam->m_posRm = Frog_Vec2Create(xFocusRm, yFocusRm);
	pCam->m_posRmDesired = pCam->m_posRm;
}

FROG_CALL void Frog_CameraSetLookAtClamped(FrRoom * pRoom, FrCamera * pCam, f32 xFocusRm, f32 yFocusRm)
{
	FrVec2 dXyScissor = pCam->m_dXyScissor;
	FrVec2 posPrev = pCam->m_posRm;
	FrVec2 posDesired = Frog_Vec2Create(xFocusRm, yFocusRm);
	FrVec2 dPos = Frog_Vec2Sub(&posDesired, &posPrev);

	float dXRoom = pRoom->m_dX * pRoom->m_dXCharPixel;
	float dYRoom = pRoom->m_dY * pRoom->m_dYCharPixel;
	posDesired.m_x = Frog_GMin(Frog_GMax(posDesired.m_x, dXyScissor.m_x * 0.5f), dXRoom - (dXyScissor.m_x * 0.5f));
	posDesired.m_y = Frog_GMin(Frog_GMax(posDesired.m_y, dXyScissor.m_y * 0.5f), dYRoom - (dXyScissor.m_y * 0.5f));

	pCam->m_posRmDesired = posDesired; 
	pCam->m_posRm = pCam->m_posRmDesired;
}

FROG_CALL void Frog_CameraPanToLookAt(FrRoom * pRoom, FrCamera * pCam, f32 xFocusRm, f32 yFocusRm, float dT)
{
	FrVec2 dXyView = pCam->m_dXyScissor;
	FrVec2 posPrev = pCam->m_posRmDesired;
	FrVec2 posDesired = Frog_Vec2Create(xFocusRm, yFocusRm);
	FrVec2 dPos = Frog_Vec2Sub(&posDesired, &posPrev);

	if (dPos.m_x < 0.0f)
	{
		// make sure the left side of the screen doesn't pan off the left edge

		posDesired.m_x = Frog_GMin(Frog_GMax(posDesired.m_x, dXyView.m_x * 0.5f), posPrev.m_x);
	}
	else if (dPos.m_x > 0.0f)
	{
		// make sure the right side of the screen doesn't pan off the right edge

		float dXRoom = pRoom->m_dX * pRoom->m_dXCharPixel;
		posDesired.m_x = Frog_GMax(Frog_GMin(posDesired.m_x, dXRoom - (dXyView.m_x * 0.5f)), posPrev.m_x);
	}

	if (dPos.m_y < 0.0f)
	{
		// make sure the bottom side of the screen doesn't pan off the bottom edge

		posDesired.m_y = Frog_GMin(Frog_GMax(posDesired.m_y, dXyView.m_y * 0.5f), posPrev.m_y);
	}
	else if (dPos.m_y > 0.0f)
	{
		// make sure the top side of the screen doesn't pan off the top edge

		float dYRoom = pRoom->m_dY * pRoom->m_dYCharPixel;
		posDesired.m_y = Frog_GMax(Frog_GMin(posDesired.m_y, dYRoom - (dXyView.m_y * 0.5f)), posPrev.m_y);
	}

	pCam->m_posRmDesired = posDesired;

	static FrSmp s_smp = { 0.6f, 200.0f, 0.05f };
	pCam->m_posRm = Frog_PosSmooth(pCam->m_posRm, pCam->m_posRmDesired, &s_smp, dT);
}

FROG_CALL ENTID Frog_EntidAllocate(FrTileWorld * pTworld)
{
	ENTID entid = (ENTID)IdAllocateFromRoster((s32*)pTworld->m_aEntidFree, &pTworld->m_cEntAllocated, kCEntWorldMax);
	if (entid == ENTID_Nil)
		return entid;

	FrEntity * pEnt = &pTworld->m_aEnt[entid];
	pEnt->m_iCellPrev = -1;
	pEnt->m_nGenRoom = -1;
	pEnt->m_ipEntUpdate = -1;
	pEnt->m_eupo = EUPO_Nil;
	pEnt->m_roomidUpdate = ROOMID_Nil;
	pEnt->m_entidNextCell = ENTID_Nil;

	return entid;
}

FROG_CALL FrEntity * Frog_PEnt(FrTileWorld * pTworld, ENTID entid)
{
	if (!FR_FVERIFY(entid >= 0 && entid < FR_DIM(pTworld->m_aEnt), "bad entity id"))
		return nullptr;
	return &pTworld->m_aEnt[entid];
}

FROG_CALL ENTID Frog_EntidFromEnt(FrTileWorld * pTworld, FrEntity * pEnt)
{
	return (ENTID)(pEnt - pTworld->m_aEnt);
}

FROG_CALL void Frog_FreeEntity(FrTileWorld * pTworld, ENTID entid)
{
	FrEntity * pEnt = &pTworld->m_aEnt[entid];

	if (pEnt->m_roomidUpdate != -1)
	{
		FrRoom * pRoom = &pTworld->m_aRoom[pEnt->m_roomidUpdate];
		Frog_RemoveFromRoom(pRoom, pEnt);
	}

	FreeToRoster(entid, (s32*)pTworld->m_aEntidFree, &pTworld->m_cEntAllocated, kCEntWorldMax);
}

FROG_CALL FrRoom * Frog_PRoomAllocate(FrTileWorld * pTworld, int dXCell, int dYCell, int dXCharPixel, int dYCharPixel)
{
	ROOMID roomid = (ROOMID)IdAllocateFromRoster((s32*)pTworld->m_aRoomidFree, &pTworld->m_cRoomAllocated, kCRoomWorldMax);
	if (roomid == ROOMID_Nil)
		return nullptr;

	FrRoom * pRoom = &pTworld->m_aRoom[roomid];
	pRoom->m_dX = dXCell;
	pRoom->m_dY = dYCell;
	pRoom->m_dXCharPixel = (float)dXCharPixel;
	pRoom->m_dYCharPixel = (float)dYCharPixel;
	pRoom->m_nGenRoom = pTworld->m_nGenRoom++;
	pRoom->m_roomid = roomid;
	pRoom->m_cpEnt = 0;
	pRoom->m_fEntityListDirty = false;

	size_t cBTile = sizeof(FrScreenTile) * dXCell * dYCell;
	size_t cBEntidCell = sizeof(ENTID) * dXCell * dYCell;
	size_t cBTotal = cBTile * 2 + cBEntidCell; 

	u8 * pB = (u8 *)malloc(cBTotal);
	FrScreenTile * aTileEnv = (FrScreenTile *)pB;
	pB += cBTile;
	FrScreenTile * aTileCache = (FrScreenTile *)pB;
	pB += cBTile;

	ZeroAB(aTileEnv, cBTile);
	ZeroAB(aTileCache, cBTile);
	pRoom->m_mpICellTileEnv = aTileEnv;
	pRoom->m_mpICellTileCache = aTileCache;

	ENTID * aEntidCell = (ENTID *)pB;
	ENTID * pEntidMax = &aEntidCell[dXCell * dYCell];
	for (ENTID * pEntidIt = aEntidCell; pEntidIt != pEntidMax; ++pEntidIt)
	{
		*pEntidIt = ENTID_Nil;
	}

	pRoom->m_mpICellEntid = aEntidCell;
	return pRoom;
}

FROG_CALL FrRoom * Frog_PRoom(FrTileWorld * pTworld, ROOMID roomid)
{
	return &pTworld->m_aRoom[roomid];
}

FROG_CALL void Frog_FreeRoom(FrTileWorld * pTworld, FrRoom * pRoom)
{
	FR_ASSERT(pRoom->m_cpEnt == 0, "expected entities to be removed from room");

	FreeToRoster(pRoom->m_roomid, (s32*)pTworld->m_aRoomidFree, &pTworld->m_cRoomAllocated, kCRoomWorldMax);
}

static inline void Frog_RemoveEntityFromCell(FrRoom * pRoom, FrEntity * pEnt)
{
	if (!FR_FVERIFY(pEnt != nullptr, "bad tile entity"))
		return;

	if (pEnt->m_iCellPrev >= 0 && pEnt->m_nGenRoom == pRoom->m_nGenRoom)
	{
		pRoom->m_mpICellEntid[pEnt->m_iCellPrev] = ENTID_Nil;
	}
}

FROG_CALL void Frog_AddToRoom(FrRoom * pRoom, FrEntity * pEnt, int eupo)
{
	if (!FR_FVERIFY(pRoom->m_cpEnt < FR_DIM(pRoom->m_apEnt), "too many active entities"))	
		return;

	if (!FR_FVERIFY(pEnt->m_eupo == EUPO_Nil && pEnt->m_ipEntUpdate < 0, "Entity is already registered for update"))
		return;

	pEnt->m_eupo = eupo;
	pEnt->m_ipEntUpdate = pRoom->m_cpEnt++;
	pEnt->m_roomidUpdate = (ROOMID)pRoom->m_roomid;

	pRoom->m_apEnt[pEnt->m_ipEntUpdate] = pEnt;
	pRoom->m_fEntityListDirty = true;
}

FROG_CALL void Frog_RemoveFromRoom(FrRoom * pRoom, FrEntity * pEnt)
{
	if (!FR_FVERIFY(pEnt->m_eupo != EUPO_Nil && pEnt->m_ipEntUpdate >= 0, "Entity is not registered for update"))
		return;

	Frog_RemoveEntityFromCell(pRoom, pEnt);

	pRoom->m_apEnt[pEnt->m_ipEntUpdate] = NULL;
	pEnt->m_ipEntUpdate = -1;
	pEnt->m_eupo = EUPO_Nil;
	pEnt->m_roomidUpdate = ROOMID_Nil;

	pRoom->m_fEntityListDirty = true;
}

FROG_CALL void Frog_SortEntityUpdateList(FrRoom * pRoom)
{
	if (pRoom->m_fEntityListDirty)
		return;

	pRoom->m_fEntityListDirty = false;

	// radix sort our entity updates
	int mpEupoCEnt[EUPO_Max] = {0};
	FrEntity * apEntWork[kCEntRoomMax];

	int cEntRemoved = 0;
	FrEntity ** ppEntMax = &pRoom->m_apEnt[pRoom->m_cpEnt];
	FrEntity ** ppEntWork = apEntWork;
	for (FrEntity** ppEnt = pRoom->m_apEnt; ppEnt != ppEntMax; ++ppEnt)
	{
		if (*ppEnt == NULL)
		{
			++cEntRemoved;
			continue;
		}

		*ppEntWork++ = *ppEnt;
		FrEntity * pEnt = *ppEnt;
		++mpEupoCEnt[pEnt->m_eupo];
	}

	int mpEupoIEntidMin[EUPO_Max];
	int iEntid = 0;
	for (int eupo = 0; eupo < EUPO_Max; ++eupo)
	{
		mpEupoIEntidMin[eupo] = iEntid;
		iEntid += mpEupoCEnt[eupo];
	}
	
	pRoom->m_cpEnt -= cEntRemoved;
	FR_ASSERT(pRoom->m_cpEnt == iEntid, "bad entity array calculations");

	FrEntity ** ppEntWorkMax = &apEntWork[pRoom->m_cpEnt];
	for (FrEntity ** ppEntWork = apEntWork; ppEntWork != ppEntWorkMax; ++ppEntWork)
	{
		FrEntity * pEnt = *ppEntWork;
		int iEntid = mpEupoIEntidMin[pEnt->m_eupo]++;
		
		pEnt->m_ipEntUpdate = iEntid;
		pRoom->m_apEnt[iEntid] = pEnt;
	}
}

FROG_CALL void Frog_UpdateEntity(FrTileWorld * pTworld, FrRoom * pRoom, ENTID entid, int xCell, int yCell, u8 iTile)
{
	if (!FR_FVERIFY(entid != ENTID_Nil, "bad tile entity id"))
		return;

	FrEntity * pEnt = &pTworld->m_aEnt[entid];

	Frog_RemoveEntityFromCell(pRoom, pEnt);

	int iCell = xCell + yCell * pRoom->m_dX;
	pEnt->m_iCellPrev = iCell;
	pEnt->m_nGenRoom = pRoom->m_nGenRoom;
	pEnt->m_iTile = iTile;

	pEnt->m_entidNextCell = pRoom->m_mpICellEntid[iCell];
	pRoom->m_mpICellEntid[iCell] = entid;
}

void RenderEntities(FrTileWorld * pTworld, FrRoom * pRoom)
{
}

FROG_CALL void Frog_InitNoteQueue(FrNoteQueue * pNoteq, int cNoteMax)
{
	pNoteq->m_cNoteMax = cNoteMax;
	pNoteq->m_aNote = (FrNote *)malloc(sizeof(FrNote) * cNoteMax);
	pNoteq->m_iNoteLatest = -1;

	FrNote * pNoteMax = &pNoteq->m_aNote[cNoteMax];
	for (FrNote * pNote = pNoteq->m_aNote; pNote != pNoteMax; ++pNote)
	{
		pNote->m_aCh[0] = '\0';
		pNote->m_notek = NOTEK_Nil;
	}
}

FROG_CALL void Frog_FreeNoteQueue(FrNoteQueue * pNoteq, int cNoteMax)
{
	if (pNoteq->m_aNote)
	{
		pNoteq->m_aNote = nullptr;
	}

	pNoteq->m_cNoteMax = 0;
}

FROG_CALL void Frog_RenderNoteQueue(FrDrawContext * pDrac, FrNoteQueue * pNoteq, FrVec2 posTopLeft, float dT)
{
	struct NoteConfig
	{
		float		m_dY;
		float		m_gCharSize;
		float		m_tBeforeFade;
		FrColor		m_col;
	} s_mpNotekCfg[] = 
	{
		{ 17.0f, 17.0f, 5.0f, Frog_ColFromRGB(128, 128, 128)},		//NOTEK_LowPriority,
		{ 22.0f, 22.0f, 7.0f, Frog_ColFromRGB(200, 200, 200)},		//NOTEK_Normal,
		{ 20.0f, 20.0f, 5.0f, Frog_ColFromRGB(200, 200, 200)}, 		//NOTEK_Bold,
	};

	static_assert(FR_DIM(s_mpNotekCfg) == NOTEK_Max, "missing notek config");

	int iNote = pNoteq->m_iNoteLatest;
	if (iNote < 0)
		return;

	FrFontData * pFontd = &pDrac->m_pDras->m_fontd;
	float gCharSizePrev = pFontd->m_gCharSize;
	FrColor colPrev = pFontd->m_colMain;
	ALIGNK alignkYPrev = pFontd->m_alignkY;
	pFontd->m_alignkY = ALIGNK_Center;

	FrVec2 pos = posTopLeft;
	int cNoteDrawn = 0;

	static const float s_dYSpacer = 2.0f;
	while (cNoteDrawn++ < pNoteq->m_cNoteMax)
	{
		FrNote * pNote = &pNoteq->m_aNote[iNote];
		if (pNote->m_notek == NOTEK_Nil)
			break;

		struct NoteConfig * pCfg = &s_mpNotekCfg[pNote->m_notek];
		pFontd->m_gCharSize = pCfg->m_gCharSize;

		pNote->m_t += dT;
		static const float s_tFade = 0.5f;
		float rFade = frClamp((pNote->m_t - pCfg->m_tBeforeFade) / s_tFade, 0.0f, 1.0f);

		FrColor col = pCfg->m_col;
		col.m_a = (u8)((float)col.m_a * (1.0f - rFade));
		pFontd->m_colMain = col;

		pos.m_y -= pCfg->m_dY * 0.5f;
		Frog_DrawTextAligned(pDrac, pos, pNote->m_aCh);
		pos.m_y -= pCfg->m_dY * 0.5f + s_dYSpacer;

		iNote = (iNote + pNoteq->m_cNoteMax - 1) % pNoteq->m_cNoteMax;
	}

	pFontd->m_colMain = colPrev;
	pFontd->m_gCharSize = gCharSizePrev;
	pFontd->m_alignkY = alignkYPrev;
}

FROG_CALL void Frog_PostNote(FrNoteQueue * pNoteq, NOTEK notek, const char * pChzFormat, ...)
{
	if (pNoteq->m_cNoteMax <= 0)
		return;

	pNoteq->m_iNoteLatest = (++pNoteq->m_iNoteLatest) % pNoteq->m_cNoteMax;
	FrNote * pNote = &pNoteq->m_aNote[pNoteq->m_iNoteLatest];
	pNote->m_notek = notek;
	pNote->m_t = 0.0f;

	va_list ap;
	va_start(ap, pChzFormat);
	vsprintf_s(pNote->m_aCh, sizeof(pNote->m_aCh), pChzFormat, ap);
	va_end(ap);
}


