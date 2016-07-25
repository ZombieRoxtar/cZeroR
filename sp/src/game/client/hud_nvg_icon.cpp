//===== Copyright Bit Mage's Stuff, All rights probably reserved. =====
//
// Purpose: NVGs Indicator HUD Element
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
// Purpose: Displays to show that you have Nightvision goggles
//-----------------------------------------------------------------------------
class CHudNVGIndicator : public CHudNumericDisplay, public CHudElement
{
	DECLARE_CLASS_SIMPLE(CHudNVGIndicator, CHudNumericDisplay);

public:
	CHudNVGIndicator(const char *pElementName);
	void Init(void);
	virtual void Paint(void);

protected:
	virtual void OnThink();

private:
	C_BasePlayer *pPlayer;
	const CHudTexture *m_NVGicon;
};

DECLARE_HUDELEMENT(CHudNVGIndicator);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------

CHudNVGIndicator::CHudNVGIndicator(const char *pElementName)
	: BaseClass(NULL, "HudNightVisionIcon"), CHudElement(pElementName)
{
	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_MISCSTATUS);
}

//-----------------------------------------------------------------------------
// Purpose: Init
//-----------------------------------------------------------------------------
void CHudNVGIndicator::Init(void)
{
}

//-----------------------------------------------------------------------------
// Purpose: Called every frame, gets player's nightvisionability
//-----------------------------------------------------------------------------
void CHudNVGIndicator::OnThink()
{
	SetPaintEnabled(false);

	/* Get the player */
	pPlayer = C_BasePlayer::GetLocalPlayer();

	/* Make sure we got the player. */
	if (!pPlayer)
	{
		return;
	}

	SetPaintBackgroundEnabled(pPlayer->PlayerHasNVGs() && pPlayer->IsSuitEquipped());
	SetPaintEnabled(pPlayer->PlayerHasNVGs());
}

//-----------------------------------------------------------------------------
// Purpose: Add icons into the HUD
//-----------------------------------------------------------------------------
void CHudNVGIndicator::Paint(void)
{
	/*
	BUGBUG: HACK:
	Why can't the icon be set until SetPaintEnabled( true ); is called?
	This function runs every frame an icon is visible!
	Why can't I just initialize it in the constructor?
	*/
	m_NVGicon = gHUD.GetIcon("NVG_Equipped");

	if (pPlayer->PlayerHasNVGs())
	{
		m_NVGicon->DrawSelf(0, 0, GetFgColor());
	}
}
