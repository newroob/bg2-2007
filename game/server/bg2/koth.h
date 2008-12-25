//HairyPotter - FOR KOTH ENT!!


const int CKoth_START_DISABLED = 1;		// spawnflag definition

//BG2 - Tjoppen - TODO: replace the use of ClientPrintAll with custom usermessages
void ClientPrintAll( char *str, bool printfordeadplayers = false, bool forcenextclientprintall = false );

class CKoth : public CTriggerMultiple
{
	DECLARE_CLASS( CKoth, CTriggerMultiple );
	//DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_DATADESC();

	//BG2 - Used with the flag triggers. -HairyPotter
	bool m_bIsParent;										//Helps the flag remember it's place... as a parent.
	CUtlVector<CBasePlayer*>	m_vTriggerBritishPlayers,	//British players who have stepped into the trigger
								m_vTriggerAmericanPlayers;	//American players who have stepped into the trigger
	int americans,
	british,
	friendlies,
	enemies;
	//

	/*CNetworkVar( int, m_iLastTeam );		//which team is near this flag?
	CNetworkVar( int, m_iRequestingCappers); //yeah, extra int, im too drunk to think of a better way :o
	CNetworkVar( float, m_flNextCapture );

	CNetworkVar( int, m_iCapturePlayers );	//how many player must be nearby to capture this flag?
	CNetworkVar( int, m_iNearbyPlayers );	//how many players ARE nearby to capture this flag?
											// alternatively: how overloaded is this flag
	CNetworkVar( int, m_iForTeam );

	CNetworkVar( int, m_iHUDSlot );		//in which slot is the icon for this flag?

	CNetworkVar( bool, m_bActive );		//BG2 - Tjoppen - adding SaintGreg's flag stuff from way back as a placeholder
										//				  until the new flag code is done.*/

	bool m_bActive;

	int m_iForTeam, m_iNearbyPlayers, m_iRequestingCappers, m_iLastTeam, m_iCapturePlayers;

	float m_flNextCapture;

	//BG2 - SaintGreg - Output functions similar to BG's
	COutputEvent m_OnAmericanStartCapture;
	COutputEvent m_OnBritishStartCapture;
	COutputEvent m_OnStartCapture;
	COutputEvent m_OnAmericanCapture;
	COutputEvent m_OnBritishCapture;
	COutputEvent m_OnCapture;
	COutputEvent m_OnAmericanStopCapture;
	COutputEvent m_OnBritishStopCapture;
	COutputEvent m_OnStopCapture;
	COutputEvent m_OnAmericanLosePoint;
	COutputEvent m_OnBritishLosePoint;
	COutputEvent m_OnLosePoint;
	COutputEvent m_OnEnable;
	COutputEvent m_OnDisable;

	bool IsActive( void );
	void InputEnable( inputdata_t &inputData );
	void InputDisable( inputdata_t &inputData );
	void InputToggle( inputdata_t &inputData );


	int		m_iTeamBonus,
			m_iTeamBonusInterval,
			m_iPlayerBonus;

	float	m_flNextTeamBonus;

	CNetworkVar( float,	m_flCaptureTime );	//.. and for how long?

	CNetworkVar( string_t, m_sTriggerName );

public:
	CUtlVector<CBasePlayer*>	m_vOverloadingPlayers;	//nearby or overloading players
	/*
	some notes on the list:
	- when a player dies, remove from list
	- when a player changes team, remove from list
	- when a player disconnects, remove from list
	- I couldn't be arsed to make it private with accessors and shit. Java has made me quite sick of that
	*/

	void Spawn( void );
	void Think( void );
	void ThinkUncapped( void );
	void ThinkCapped( void );
	void Capture( int iTeam );	//let iTeam have this flag now..
	void ChangeTeam( int iTeamNum );
	virtual int UpdateTransmitState();
};
