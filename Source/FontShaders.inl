
const char* g_fontMainVertex = "												\n"
"varying vec4 color;															\n"
"varying vec4 minMaxUV;															\n"
"void main()																	\n"
"{																				\n"
"	gl_Position 	= gl_ModelViewProjectionMatrix * gl_Vertex;					\n"
"   color	    	= gl_Color;													\n"
"   gl_TexCoord[0]  = gl_MultiTexCoord0;										\n"
"	minMaxUV		= gl_MultiTexCoord1;										\n" 
"																				\n"
"}																				\n";


// ok, this is a mess... but good enough for now. It's doing both distance map AA and 4x supersampling to
//  deal with tiny fonts. It currently hard codes the font size and AA distance in a really hacky way
//  and doesn't deal with DuDy being different from DuDx, but it'll do for now.

// "Real-Time Texture-Mapped Vector Glyphs" is very unclear about how to compute the smoothstep scalar
//  (and wrong? s is in pixels?)

const char* g_fontMainFragment = "												\n"
"uniform sampler2D s_glyphTexture;												\n"
"varying vec4 color;															\n"
"varying vec4 minMaxUV;															\n"
"																				\n"
"vec2 clampMinMax(vec4 minMax, vec2 uv)											\n"
"{																				\n"
"	return clamp(uv.xy, minMax.xy, minMax.zw);									\n"
"}																				\n"
"																				\n"
"																				\n"
"																				\n"
"void main()																	\n"
"{																				\n"
"float dudx	= dFdx(gl_TexCoord[0].x);											\n"
"float dvdx	= dFdx(gl_TexCoord[0].y);											\n" 
"float dvdy	= dFdy(gl_TexCoord[0].y);											\n" 
"float width = 512.0 / 12.0;													\n"
"float s = sqrt(dudx*dudx + dvdx*dvdx);											\n"
"float rF = 1.0 / (2.0 * width * s);											\n"
"dudx *= 0.25;																	\n"
"dvdy *= 0.25;																	\n"
	" float alpha =	smoothstep(-0.5f, 0.5f, (texture2D(s_glyphTexture, clampMinMax(minMaxUV, gl_TexCoord[0].xy + vec2(dudx, dvdy) )).g - 0.5f) * rF) +		\n"
	" 				smoothstep(-0.5f, 0.5f, (texture2D(s_glyphTexture, clampMinMax(minMaxUV, gl_TexCoord[0].xy + vec2(dudx, -dvdy) )).g - 0.5f) * rF) +		\n"
	" 				smoothstep(-0.5f, 0.5f, (texture2D(s_glyphTexture, clampMinMax(minMaxUV, gl_TexCoord[0].xy + vec2(-dudx, -dvdy) )).g - 0.5f) * rF) +	\n"
	" 				smoothstep(-0.5f, 0.5f, (texture2D(s_glyphTexture, clampMinMax(minMaxUV, gl_TexCoord[0].xy + vec2(-dudx, dvdy) )).g - 0.5f) * rF);		\n"
	"alpha *= 0.25;																\n"
	"alpha += -clamp(color.a, -1, 0);											\n"
	"gl_FragColor = vec4(color.rgb, alpha * abs(color.a));						\n"
"}																				\n";