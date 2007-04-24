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

//vcomm.cpp - arrays holding data for voice command menus and stuff

#include "vcomm.h"

#ifdef CLIENT_DLL
/*char *pVChats[2][NUM_VOICECOMMS] =
{
	//american voiecomm chats
	{
		"#BG2_VoiceComm_A1",
		"#BG2_VoiceComm_A2",
		"#BG2_VoiceComm_A3",
		"#BG2_VoiceComm_A4",
		"#BG2_VoiceComm_A5",
		"#BG2_VoiceComm_A6",
		"#BG2_VoiceComm_A7",
		"#BG2_VoiceComm_A8",
		"#BG2_VoiceComm_A9",
		
		"#BG2_VoiceComm_A10",
		"#BG2_VoiceComm_A11",
		"#BG2_VoiceComm_A12",
		"#BG2_VoiceComm_A13",
		"#BG2_VoiceComm_A14",
		"#BG2_VoiceComm_A15",
		"#BG2_VoiceComm_A16",
		"#BG2_VoiceComm_A17",
		"#BG2_VoiceComm_A18",
		"#BG2_VoiceComm_A19"
	},

	//british voiecomm chats
	{
		"#BG2_VoiceComm_B1",
		"#BG2_VoiceComm_B2",
		"#BG2_VoiceComm_B3",
		"#BG2_VoiceComm_B4",
		"#BG2_VoiceComm_B5",
		"#BG2_VoiceComm_B6",
		"#BG2_VoiceComm_B7",
		"#BG2_VoiceComm_B8",
		"#BG2_VoiceComm_B9",
		
		"#BG2_VoiceComm_B10",
		"#BG2_VoiceComm_B11",
		"#BG2_VoiceComm_B12",
		"#BG2_VoiceComm_B13",
		"#BG2_VoiceComm_B14",
		"#BG2_VoiceComm_B15",
		"#BG2_VoiceComm_B16",
		"#BG2_VoiceComm_B17",
		"#BG2_VoiceComm_B18",
		"#BG2_VoiceComm_B19"
	}
};*/
#else
char *pVCommScripts[NUM_VOICECOMMS] =
{
	".Yes",
	".No",
	".FollowMe",
	".LeaveItToMe",
	".UnderFire",
	".SpreadOut",
	".BattleCry",//"Voicecomms.BattleCry", //different for american and british( ABattleCry/BBattleCry )
	".Medic",
	"",

	".Advance",
	".Retreat",
	".RallyTheMen",
	".Halt",
	".LineUp",
	".MakeReady",
	".PresentArms",
	".Fire",
	".CeaseFire",
	"",
};
#endif