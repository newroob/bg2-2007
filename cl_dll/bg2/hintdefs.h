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

#define NUM_HINTS	3

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

char *pVHints[NUM_HINTS] =
{
	"Trying to sit still and hide will make you vulerable to determined bayonet charges!",
	"Crouching takes some stamina (heavy gear) but will affect your ability to aim (watch your crosshairs size).",
	"Crouching does not increase stamina regeneration.",
	"Cannot use melee weapon while being crouched!",
	"If these hints start to annoy you you can turn them off in the options dialog.",
	"Capturing all flags will end the round and give your team bonus points.",
	"Muskets are inaccurate! Try getting closer to your enemy to get a better chance of scoring a hit.",
	"Jumping takes lots of stamina - Jump only when absolutely necessary",
	"Low Stamina Warning! Relax for a second and let your stamina fill up again.",
	"Melee attack is the most powerful method to kill a large amount of enemies in a short amount of time.",
	"While you are reloading you are an easy target. Be sure when to hit the reload button and when not."
	"You are in the reload process and defenseless until you are done!"
};