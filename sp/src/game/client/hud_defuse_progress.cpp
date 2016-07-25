//===== Copyright Bit Mage's Stuff, All rights probably reserved. =====
//
// Purpose: Defuse Progress HUD Element
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

ConVar cl_defuseprogressbar("cl_defuseprogressbar", "1", 0, "Enables progress bar when defusing bombs");

//-----------------------------------------------------------------------------
// Purpose: Displays progress bar when defusing a bomb
//-----------------------------------------------------------------------------
class CHudDefuseProgress : public CHudNumericDisplay, public CHudElement
{
	DECLARE_CLASS_SIMPLE(CHudDefuseProgress, CHudNumericDisplay);

public:
	CHudDefuseProgress(const char *pElementName);
	virtual void Paint(void);

protected:
	virtual void OnThink();

private:
	C_BasePlayer *pPlayer;
	CHud m_chDefuseBar;
	float m_flDefuseProgress;
};

DECLARE_HUDELEMENT(CHudDefuseProgress);


//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------

CHudDefuseProgress::CHudDefuseProgress(const char *pElementName)
	: BaseClass(NULL, "HudDefuseProgress"), CHudElement(pElementName)
{
	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_MISCSTATUS);
	m_flDefuseProgress = 0;
}

//-----------------------------------------------------------------------------
// Purpose: Called every frame, gets defuse progress (if applicable)
//-----------------------------------------------------------------------------
void CHudDefuseProgress::OnThink()
{
	if (!cl_defuseprogressbar.GetBool())
	{
		SetPaintEnabled(false);
		SetPaintBackgroundEnabled(false);
		return;
	}

	/* Get the player */
	pPlayer = C_BasePlayer::GetLocalPlayer();

	/* Make sure we got the player. */
	if (!pPlayer)
	{
		return;
	}

	m_flDefuseProgress = pPlayer->GetDefuseProgress();

	if (m_flDefuseProgress)
	{
		SetPaintBackgroundEnabled(true);
		SetPaintEnabled(true);
	}
	else
	{
		// Show the crosshair
		pPlayer->m_Local.m_iHideHUD &= ~HIDEHUD_CROSSHAIR;

		SetPaintEnabled(false);
		SetPaintBackgroundEnabled(false);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Add into the HUD
//-----------------------------------------------------------------------------
void CHudDefuseProgress::Paint(void)
{
	// Hide the crosshair
	pPlayer->m_Local.m_iHideHUD |= HIDEHUD_CROSSHAIR;

	/* The panel size from HudLayout.res is defined in a 640 x 480 space. */
	float ScreenScale = ScreenHeight() / 480.0;
	int x = 8 * ScreenScale;
	int y = 8 * ScreenScale;
	int length = 364 * ScreenScale;
	int height = 8 * ScreenScale;

	m_chDefuseBar.DrawProgressBar(x, y, length, height,
		m_flDefuseProgress, Color(153, 255, 153, 192), 2);
}
