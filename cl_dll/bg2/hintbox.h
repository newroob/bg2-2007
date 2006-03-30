/*
	The Battle Grounds 2 - A Source modification
	Copyright (C) 206, The Battle Grounds 2 Team and Contributors

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
		Jason "Draco" Houston		iamlorddraco@gmail.com

	You may also contact us via the Battle Grounds website and/or forum at:
		www.bgmod.com
*/
#define NUM_HINTS	12

enum //hint displaymodes
{
	DISPLAY_ONCE = 0,
	DISPLAY_ALWAYS,
	OVERRIDE_ALL,
};

enum 
{
	HINT_CAMPING,
	HINT_CROUCH,
	HINT_CROUCH2,
	HINT_CROUCHSTAB,
	HINT_HINT,
	HINT_SCORE,
	HINT_MUSKET,
	HINT_JUMP,
	HINT_STAMINA,
	HINT_MELEE,
	HINT_RELOAD,
	HINT_RELOAD2,
};

char pVHints[NUM_HINTS][512] =
{//                                       |                                   |
	"Trying to sit still and hide will \nmake you vulerable to determined \nbayonet charges!",
	"Crouching takes some stamina (heavy \ngear) but will affect your \nability to aim (watch your crosshairs size).",
	"Crouching does not increase stamina \nregeneration.",
	"Cannot use melee weapon while being \ncrouched!",
	"If these hints start to annoy you \nyou can turn them off in the \noptions dialog.",
	"Capturing all flags will end the \nround and give your team extra \nbonus points.",
	"Muskets are inaccurate! Try getting \ncloser to your enemy to get a \nbetter chance of scoring a hit.",
	"Jumping takes lots of stamina \nJump only when absolutely necessary",
	"Low Stamina Warning! Relax for a \nsecond and let your stamina fill \nup again.",
	"Melee attack is the most powerful \nmethod to kill a large amount of \nenemies in a short amount of time.",
	"While you are reloading you are an \neasy target. Be sure when to hit \nthe reload button and when not."
	"You are in the reload process and \ndefenseless until you are done!"
};

// not implemented yet
static ConVar cl_hintbox( "cl_hintbox", "1", 0, "0 - Off, 1 - game relevant hints, 2 -  with newbie notices" );

class CHint
{
public:
	CHint(char *input);
	~CHint();
	char* GetText() { return m_text; };
	bool Shown() { return m_shown; };
	void SetShown(bool shown) { m_shown = shown; };
private:
	bool m_shown;
	char m_text[512];
};

class CHintbox : public CHudElement , public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHintbox, vgui::Panel );

public:
	CHintbox( const char *pElementName );
	virtual void Init( void );	
	virtual void VidInit( void );	
	void Reset();
	virtual void OnThink(void);
	void MsgFunc_Hintbox( bf_read &msg );
	void SetHint( char* text, int displaytime, int displaymode ); // for custom messages
	void UseHint( int textpreset, int displaytime, int displaymode ); // for predefined messages

private:
	vgui::HFont m_hHintFont;
	CHint *m_hint[NUM_HINTS];
	Vector m_textpos;
	float m_hintshowtime;
	bool m_hidden;
	vgui::HScheme scheme;
	vgui::Label * m_label; 

protected:
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
};

DECLARE_HUDELEMENT( CHintbox );
DECLARE_HUD_MESSAGE( CHintbox, Hintbox );

static void hintbox_f( void )
{
	if( engine->Cmd_Argc() == 2 )
	{
		int hint = atoi( engine->Cmd_Argv( 1 ) );
		CHintbox *hintbox = (CHintbox *)GET_HUDELEMENT( CHintbox );
		int displaytime = 8;
		int displaymode = DISPLAY_ALWAYS;
		hintbox->UseHint(hint, displaytime, displaymode);
	}
}

static ConCommand hintbox( "hintbox", hintbox_f );
