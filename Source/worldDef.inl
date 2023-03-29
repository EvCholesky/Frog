
static char s_aiTileRmstart[] =
{
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1
};

static RoomDefinition s_rmdefStart = 
{
	"Start",			// m_pChzName;
	10,					// m_dX;
	10,					// m_dY;
	s_aiTileRmstart,	// m_aiTile

	{
		{ TRANSK_DirR,	"End" },
		{ TRANSK_Nil }
	}					//m_rmtrans
};

static char s_aiTileRmend[] = 
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
	1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, -1
};

static RoomDefinition s_rmdefEnd = 
{
	"End",				// m_pChzName;
	12,					// m_dX;
	10,					// m_dY;
	s_aiTileRmend,		// m_aiTile

	{
		{ TRANSK_DirL,	"Start" },
		{ TRANSK_DirR,	"Field", 0, 4},
		{ TRANSK_DirD,	"Path", 0, 0},
		{ TRANSK_Nil }
	}					//m_rmtrans
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
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1
};

static RoomDefinition s_rmdefField = 
{
	"Field",			// m_pChzName;
	12,					// m_dX;
	14,					// m_dY;
	s_aiTileRmField,	// m_aiTile

	{
		{ TRANSK_DirL,	"End", 0, -4 },
		{ TRANSK_DirL,	"Path", 0, 2 },
		{ TRANSK_Nil }
	}					//m_rmtrans
};


static char s_aiTileRmPath[] = 
{
	1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1,
	1, 0, 0, 0, 0, 0, 0, 0, 4, 4, 0, 0,
	1, 0, 4, 4, 4, 4, 4, 4, 4, 0, 0, 1,
	1, 0, 0, 4, 4, 0, 0, 0, 0, 0, 0, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, -1
};

static RoomDefinition s_rmdefPath = 
{
	"Path",				// m_pChzName;
	12,					// m_dX;
	5,					// m_dY;
	s_aiTileRmPath,		// m_aiTile

	{
		{ TRANSK_DirU,	"End", 0, 0 },
		{ TRANSK_DirR,	"Field", 0, -2 },
		{ TRANSK_Nil }
	}					//m_rmtrans
};

static RoomDefinition * s_apRmdef[] = 
{
	&s_rmdefStart,
	&s_rmdefEnd,
	&s_rmdefField,
	&s_rmdefPath,
	NULL
};

static TileDefinition s_aTiledef[] = 
{
	{ ' ',	(s32)0xFF0000FF, (s32)0xFF006000, FTILE_None },
	{ 'W',	(s32)0xFF0080FF, (s32)0xFF005060, FTILE_Collide },
	{ '?',	(s32)0xFF0000FF, (s32)0xFF000000, FTILE_Collide|FTILE_Test },
	{ '@',	(s32)0xFFBFBFBF, (s32)0xFF606060, FTILE_None },
	{ ' ',	(s32)0xFF000000, (s32)0xFF006060, FTILE_None },
	{ '\0' }
};

RoomLibrary s_rmlib =
{
	s_apRmdef,			// m_apRmdef
	s_aTiledef,			// m_aTiledef
};
