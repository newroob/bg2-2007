class C_Koth : public C_BaseEntity
{
	DECLARE_CLASS( C_Koth, C_BaseEntity );
public:
	DECLARE_CLIENTCLASS();

					C_Koth();
	virtual			~C_Koth();


public:

	int		m_iLastTeam;
	float	m_flNextCapture;

	int		m_iCapturePlayers;
	int		m_iNearbyPlayers;
	int		m_iForTeam;
	float	m_flCaptureTime;
	
	char	m_sTriggerName[256];

	int		m_iHUDSlot;		//in which slot is the icon for this flag?
	
};

extern CUtlVector< C_Koth * > g_Flags;