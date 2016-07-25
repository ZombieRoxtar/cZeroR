//===== Copyright Bit Mage's Stuff, All rights probably reserved. =====
//
// Purpose: Zone Indicator HUD Element
//
//=============================================================================
#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "hud_numericdisplay.h"
#include "iclientmode.h"
#include <vgui_controls/AnimationController.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Displays current zone icon
//-----------------------------------------------------------------------------
class CHudZoneIndicator : public CHudNumericDisplay, public CHudElement
{
	DECLARE_CLASS_SIMPLE(CHudZoneIndicator, CHudNumericDisplay);

public:
	CHudZoneIndicator(const char *pElementName);
	void Init(void);
	virtual void Paint(void);

protected:
	virtual void OnThink();
	Color SmartColor(int index);

private:
	C_BasePlayer *pPlayer;
	CHandle < C_BaseCombatWeapon > m_hCurrentActiveWeapon;
	const CHudTexture *m_stealthZoneIcon;
	const CHudTexture *m_rcBombZoneIcon;
	const CHudTexture *m_blowtorchZoneIcon;
	const CHudTexture *m_fiberopticCameraZoneIcon;
	const CHudTexture *m_radioZoneIcon;
	const CHudTexture *m_defuseZoneIcon;
	const CHudTexture *m_digitalCameraZoneIcon;
	const CHudTexture *m_briefcaseZoneIcon;
	const CHudTexture *m_rescueZoneIcon;
	bool m_bRightTool[9]; // Turns the icon green because we have right weapon out
};

DECLARE_HUDELEMENT(CHudZoneIndicator);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------

CHudZoneIndicator::CHudZoneIndicator(const char *pElementName)
	: BaseClass(NULL, "HudZoneIndicator"), CHudElement(pElementName)
{
	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_MISCSTATUS);
	m_hCurrentActiveWeapon = NULL;
	m_bRightTool[5] = true; // Defuse
	m_bRightTool[8] = true; // Rescue
}

//-----------------------------------------------------------------------------
// Purpose: Init
//-----------------------------------------------------------------------------
void CHudZoneIndicator::Init(void)
{
	SetPaintBackgroundEnabled(false);
}

//-----------------------------------------------------------------------------
// Purpose: Called every frame, gets player's zone states
//-----------------------------------------------------------------------------
void CHudZoneIndicator::OnThink()
{
	SetPaintEnabled(false);

	pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	C_BaseCombatWeapon *wpn = GetActiveWeapon();
	if (!wpn)
		return;

	if (wpn != m_hCurrentActiveWeapon)
	{
		m_bRightTool[1] = false; // RC Bombs
		m_bRightTool[2] = false; // Blowtorch
		m_bRightTool[3] = false; // Fiberoptic Camera
		m_bRightTool[4] = false; // Radio
		m_bRightTool[6] = false; // Camera
		m_bRightTool[7] = false; // Briefcase

			 // strcmp() returns 0 when neither string is greater? (based off a cpu instruction)
		if (!strcmp(wpn->GetName(), "weapon_rcbomb"))
		{
			m_bRightTool[1] = true;
		}
		if (!strcmp(wpn->GetName(), "weapon_blowtorch"))
		{
			m_bRightTool[2] = true;
		}
		if (!strcmp(wpn->GetName(), "weapon_fiberopticcamera"))
		{
			m_bRightTool[3] = true;
		}
		if (!strcmp(wpn->GetName(), "weapon_radio"))
		{
			m_bRightTool[4] = true;
		}
		if (!strcmp(wpn->GetName(), "weapon_camera"))
		{
			m_bRightTool[5] = true;
		}
		if (!strcmp(wpn->GetName(), "weapon_briefcase"))
		{
			m_bRightTool[7] = true;
		}
		m_hCurrentActiveWeapon = wpn;
	}

	if (pPlayer->IsPlayerInZoneStealth() ||
		pPlayer->IsPlayerInZoneRcBomb() ||
		pPlayer->IsPlayerInZoneBlowtorch() ||
		pPlayer->IsPlayerInZoneFiberopticCamera() ||
		pPlayer->IsPlayerInZoneRadio() ||
		pPlayer->IsPlayerInZoneDefuse() ||
		pPlayer->IsPlayerInZoneDigitalCamera() ||
		pPlayer->IsPlayerInZoneBriefcase() ||
		pPlayer->IsPlayerInZoneRescue())
	{
		SetPaintEnabled(true);
	}
	else
	{
		SetPaintEnabled(false);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Dynamic color decision
//-----------------------------------------------------------------------------
Color CHudZoneIndicator::SmartColor(int index)
{
	if (index == 0) // Gray Stealth Zone icon
	{
		return Color(192, 192, 192, 192);
	}
	else
	{
		// Turn the icon green if you've got the right tool out
		// TODO: Consider moving these RGBAs into ClientScheme
		if (m_bRightTool[index])
		{
			return Color(0, 255, 64, 80);
		}
		else
		{
			return GetFgColor(); // This is "HL2 HUD Orange"
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Add icons into the HUD
//-----------------------------------------------------------------------------
void CHudZoneIndicator::Paint(void)
{
	/*
		BUGBUG?: These weren't working when put with other initialization calls
		HACK?: Why can't these be set until SetPaintEnabled(true) is called?
		This function runs every frame that an icon is visible!
		Why can't I just initialize them at Init() or in the constructor?
	*/
	m_stealthZoneIcon = gHUD.GetIcon("ZoneIcon_Stealth");
	m_rcBombZoneIcon = gHUD.GetIcon("ZoneIcon_RcBomb");
	m_blowtorchZoneIcon = gHUD.GetIcon("ZoneIcon_Blowtorch");
	m_fiberopticCameraZoneIcon = gHUD.GetIcon("ZoneIcon_FiberopticCamera");
	m_radioZoneIcon = gHUD.GetIcon("ZoneIcon_Radio");
	m_defuseZoneIcon = gHUD.GetIcon("ZoneIcon_Defuse");
	m_digitalCameraZoneIcon = gHUD.GetIcon("ZoneIcon_DigitalCamera");
	m_briefcaseZoneIcon = gHUD.GetIcon("ZoneIcon_Briefcase");
	m_rescueZoneIcon = gHUD.GetIcon("ZoneIcon_Rescue");

	/*
		This is all because of some parts of CZ where you are in a
		Stealth Zone and a then enter tool Zone within the Stealth Zone.
		When this happens, the Stealth Zone icon is moved down below
		the tool icon. This is a behavior I want to duplicate. Sort of...
	*/
	int shownIcons = 0;
	// For each shown icon, our starting position is moved down
	const int spacing = 80; // 64px icons + 16px margin

	if (pPlayer->IsPlayerInZoneRcBomb())
	{
		m_rcBombZoneIcon->DrawSelf(0, shownIcons*spacing, SmartColor(1));
		shownIcons++;
	}
	if (pPlayer->IsPlayerInZoneBlowtorch())
	{
		m_blowtorchZoneIcon->DrawSelf(0, shownIcons*spacing, SmartColor(2));
		shownIcons++;
	}
	if (pPlayer->IsPlayerInZoneFiberopticCamera())
	{
		m_fiberopticCameraZoneIcon->DrawSelf(0, shownIcons*spacing, SmartColor(3));
		shownIcons++;
	}
	if (pPlayer->IsPlayerInZoneRadio())
	{
		m_radioZoneIcon->DrawSelf(0, shownIcons*spacing, SmartColor(4));
		shownIcons++;
	}
	if (pPlayer->IsPlayerInZoneDefuse())
	{
		m_defuseZoneIcon->DrawSelf(0, shownIcons*spacing, SmartColor(5));
		shownIcons++;
	}
	if (pPlayer->IsPlayerInZoneDigitalCamera())
	{
		m_digitalCameraZoneIcon->DrawSelf(0, shownIcons*spacing, SmartColor(6));
		shownIcons++;
	}
	if (pPlayer->IsPlayerInZoneBriefcase())
	{
		m_briefcaseZoneIcon->DrawSelf(0, shownIcons*spacing, SmartColor(7));
		shownIcons++;
	}
	if (pPlayer->IsPlayerInZoneRescue())
	{
		m_rescueZoneIcon->DrawSelf(0, shownIcons*spacing, SmartColor(8));
		shownIcons++;
	}
	if (pPlayer->IsPlayerInZoneStealth())
	{
		m_stealthZoneIcon->DrawSelf(0, shownIcons*spacing, SmartColor(0));
	}
}
