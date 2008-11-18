//HairyPotter - FOR CTF MODE!!

const int CtfFlag_START_DISABLED = 1;		// spawnflag definition

class CtfFlag : public CBaseAnimating
{
	DECLARE_CLASS( CtfFlag, CBaseAnimating );
	DECLARE_DATADESC();

	COutputEvent m_OnPickedUp;
	COutputEvent m_OnDropped;
	COutputEvent m_OnReturned;
	COutputEvent m_OnEnable;
	COutputEvent m_OnDisable;

	void InputReset( inputdata_t &inputData );
	void InputEnable( inputdata_t &inputData );
	void InputDisable( inputdata_t &inputData );
	void InputToggle( inputdata_t &inputData );

	int		m_iForTeam,
			m_iTeamBonus,
			iTeam,
			m_iPlayerBonus,
			m_iDropSound,
			m_iReturnSound,
			m_iPickupSound;

	float	m_flPickupRadius, m_fReturnTime, fReturnTime;

	char CTFMsg[512];

public:

	bool m_bFlagIsDropped, m_bFlagIsCarried, m_bActive;   

	char *cFlagName;	

	Vector FlagOrigin;    
	QAngle FlagAngle;

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
