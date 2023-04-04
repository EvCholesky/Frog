static char s_aiTileRmStart[] = 
{
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 4, 4, 4, 0, 0, 0, 0, 0, 1, 
	1, 4, 4, 4, 0, 0, 5, 0, 0, 1, 
	1, 4, 4, 4, 0, 0, 0, 5, 0, 0, 
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	1, 0, 5, 0, 5, 0, 5, 0, 0, 0, 
	1, 0, 0, 0, 5, 5, 0, 0, 0, 0, 
	1, 0, 5, 0, 0, 5, 0, 5, 0, 1, 
	1, 0, 0, 5, 0, 0, 0, 0, 0, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
};

static RoomDefinition s_rmdefStart = 
{
	"Start",												// m_pChzName
	10,														// m_dX
	10,														// m_dY
	s_aiTileRmStart,										// m_aiTile

	{
		{ TRANSK_DirR, "End", 0, 0 },
		{ TRANSK_Nil },
	}
};

static char s_aiTileRmEnd[] = 
{
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	1, 0, 0, 0, 0, 0, 4, 4, 4, 4, 4, 4, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 
	1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 4, 
	1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 
};

static RoomDefinition s_rmdefEnd = 
{
	"End",													// m_pChzName
	12,														// m_dX
	10,														// m_dY
	s_aiTileRmEnd,											// m_aiTile

	{
		{ TRANSK_DirL, "Start", 0, 0 },
		{ TRANSK_DirR, "Field", 0, 4 },
		{ TRANSK_DirD, "Path", 0, 0 },
		{ TRANSK_Nil },
	}
};

static char s_aiTileRmField[] = 
{
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 
	4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 1, 
	0, 0, 0, 4, 4, 0, 0, 0, 0, 0, 0, 1, 
	1, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 1, 
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
};

static RoomDefinition s_rmdefField = 
{
	"Field",												// m_pChzName
	12,														// m_dX
	14,														// m_dY
	s_aiTileRmField,										// m_aiTile

	{
		{ TRANSK_DirL, "End", 0, -4 },
		{ TRANSK_DirL, "Path", 0, 2 },
		{ TRANSK_Nil },
	}
};

static char s_aiTileRmPath[] = 
{
	1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 
	1, 0, 0, 0, 0, 0, 0, 0, 4, 4, 0, 0, 
	1, 0, 4, 4, 4, 4, 4, 4, 4, 0, 0, 1, 
	1, 0, 0, 4, 4, 0, 0, 0, 0, 0, 0, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
};

static RoomDefinition s_rmdefPath = 
{
	"Path",													// m_pChzName
	12,														// m_dX
	5,														// m_dY
	s_aiTileRmPath,											// m_aiTile

	{
		{ TRANSK_DirU, "End", 0, 0 },
		{ TRANSK_DirR, "Field", 0, -2 },
		{ TRANSK_Nil },
	}
};

static RoomDefinition * s_apRmdef[] = 
{
	&s_rmdefStart,
	&s_rmdefEnd,
	&s_rmdefField,
	&s_rmdefPath,
	NULL,
};

static SpriteDefinition s_aSpdef[] = 
{
	{ UFROMPX(0), VFROMPX(480), UFROMPX(32), VFROMPX(512), "grass1"},
	{ UFROMPX(32), VFROMPX(480), UFROMPX(64), VFROMPX(512), "grass2"},
	{ UFROMPX(64), VFROMPX(480), UFROMPX(96), VFROMPX(512), "path"},
	{ UFROMPX(96), VFROMPX(480), UFROMPX(128), VFROMPX(512), "rockwall"},
	{ 0, 0, 0, 0, NULL},
};

static TileDefinition s_aTiledef[] = 
{
	{ ' ', (s32)0xFF0000FF, (s32)0x00006000, 0, FTILE_None},
	{ 'W', (s32)0xFF0080FF, (s32)0x00005060, 3, FTILE_Collide},
	{ '?', (s32)0xFF0000FF, (s32)0xFF000000, -1, FTILE_Collide|FTILE_Test},
	{ '@', (s32)0xFFBFBFBF, (s32)0xFF606060, -1, FTILE_None},
	{ ' ', (s32)0xFF000000, (s32)0x00006060, 2, FTILE_None},
	{ ' ', (s32)0xFF0000FF, (s32)0x00006000, 1, FTILE_None},
	{'\0'},
};

RoomLibrary s_rmlib = 
{
	s_apRmdef,												// m_apRmdef
	s_aTiledef,												// m_aTiledef
	s_aSpdef,												// m_aSpdef
};

