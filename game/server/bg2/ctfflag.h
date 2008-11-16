//HairyPotter - FOR CTF MODE!!

class CtfFlag : public CBaseAnimating
{
	DECLARE_CLASS( CtfFlag, CBaseAnimating );
	DECLARE_DATADESC();

	COutputEvent m_OnPickedUp;
	COutputEvent m_OnDropped;
	COutputEvent m_OnReturned;

	void InputReset( inputdata_t &inputData );

public:
	int		m_iForTeam,
			m_iTeamBonus,
			iTeam,
			m_iPlayerBonus,
			m_iDropSound,
			m_iReturnSound,
			m_iPickupSound;

	char CTFMsg[512];

	Vector FlagOrigin;    
	QAngle FlagAngle;	  
	CBasePlayer *pCapturer;

	bool m_bFlagIsDropped, m_bFlagIsCarried;   

	float	m_flPickupRadius, m_fReturnTime, fReturnTime;


	char *cFlagName;	

	void PlaySound( Vector origin, int sound );
	void ResetFlag();
	void Spawn( void );
	void Precache( void );
	static void PrintAlert( char *Msg );
	void ReturnFlag( void );
	void PlaySound( int iSound );
	void Think( void );
};
//END CTF FLAG
