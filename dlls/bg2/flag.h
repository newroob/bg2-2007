/*
	The Battle Grounds 2 - A Source modification
	Copyright (C) 2005, The Battle Grounds 2 Team and Contributors

	The Battle Grounds 2 free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	The Battle Grounds 2 is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

	Contact information:
		Tomas "Tjoppen" Härdin		tjoppen@gamedev.se

	You may also contact the (future) team via the Battle Grounds website and/or forum at:
		www.bgmod.com

	 Note that because of the sheer volume of files in the Source SDK this
	notice cannot be put in all of them, but merely the ones that have any
	changes from the original SDK.
	 In order to facilitate easy searching, all changes are and must be
	commented on the following form:

	//BG2 - <name of contributer>[ - <small description>]
*/

/*#ifdef CLIENT_DLL
#define CFlag C_Flag
#endif*/

class CFlag : public CBaseAnimating
{
	DECLARE_CLASS( CFlag, CBaseAnimating );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();
	DECLARE_DATADESC();

	CNetworkVar( int, m_iLastTeam );		//which team is near this flag?
	CNetworkVar( int, m_iRequestingCappers); //yeah, extra int, im too drunk to think of a better way :o
	CNetworkVar( float, m_flNextCapture );

	CNetworkVar( int, m_iCapturePlayers );	//how many player must be nearby to capture this flag?
	CNetworkVar( int, m_iForTeam );

	int		m_iUncap,
			m_iTeamBonus,
			m_iTeamBonusInterval,
			m_iPlayerBonus;

	float	m_flNextTeamBonus;

	CNetworkVar( float,	m_flCaptureTime );	//.. and for how long?
	float	m_flCaptureRadius;				//.. and how close?

	CNetworkVar( string_t, m_sFlagName );

public:

	void Spawn( void );
	void Precache( void );
	void Think( void );
	void ChangeTeam( int iTeamNum );
	virtual int UpdateTransmitState();
};

class CFlagHandler
{
public:
	static void RespawnAll( char *pSound );
	static void RespawnWave();
	//static void PlayCaptureSound( void );
	static void ResetFlags( void );
	static void Update( void );
};