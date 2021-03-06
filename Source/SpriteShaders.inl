
const char* g_spriteMainVertex = "																		\n"
"varying vec4 vecColor;																					\n"
"																										\n"
"void main()																							\n"
"{																										\n"
"	gl_Position 	= gl_ModelViewProjectionMatrix * gl_Vertex;											\n"
"   vecColor	    = gl_Color;																			\n"
"   gl_TexCoord[0]  = gl_MultiTexCoord0;																\n"
"																										\n"
"}																										\n";

const char* g_spriteMainFragment = "																	\n"
"uniform sampler2D s_sampTexture;																		\n"
"varying vec4 vecColor;																					\n"
"																										\n"
"void main()																							\n"
"{																										\n"
"	vec4 vecSample = texture2D(s_sampTexture, gl_TexCoord[0].xy);										\n"
"	gl_FragColor = vecSample * vecColor;																\n"
"}																										\n";


const char* g_environmentMainVertex = "																	\n"
"varying vec4 vecColor;																					\n"
"uniform vec4 s_posCenterWs;                                                                            \n"
"vec2 dPosLateralRange = vec2(200, 200);                                                                \n"
"vec2 dLateralZ = vec2(0.0, 0.4);																		\n"
"																										\n"
"void main()																							\n"
"{																										\n"
"	vec2 lateralPerc = (gl_Vertex.xy - s_posCenterWs.xy) / dPosLateralRange;                            \n"
"	lateralPerc  = clamp(lateralPerc, vec2(-1, -1), vec2(1, 1));                                        \n"
"   lateralPerc  = lateralPerc * 0.5 + vec2(1,1);                                                       \n"
"   vec2 lateral = lateralPerc * dLateralZ * gl_Vertex.z;                                               \n"
"	vec4 pos     = vec4(gl_Vertex.xy + lateral.xy, 0, gl_Vertex.w);										        \n"
"                                                                                                       \n"
"	gl_Position 	= gl_ModelViewProjectionMatrix * pos;												\n"
"	vec2 clampedPerc = clamp(abs(lateralPerc), vec2(0,0), vec2(1,1));                                   \n"
"   vecColor	    = gl_Color;	                                                                        \n"
"   gl_TexCoord[0]  = gl_MultiTexCoord0;																\n"
"																										\n"
"}																										\n";

const char* g_environmentMainFragment = "																\n"
"uniform sampler2D s_sampTexture;																		\n"
"varying vec4 vecColor;																					\n"
"																										\n"
"void main()																							\n"
"{																										\n"
"	vec4 vecSample = texture2D(s_sampTexture, gl_TexCoord[0].xy);										\n"
"	gl_FragColor = vecSample * vecColor;																\n"
"}																										\n";