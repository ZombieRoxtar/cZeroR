//===== Copyright Bit Mage's Stuff, All rights probably reserved. =====
//
// Purpose: Dynamic Crosshair
//
//=====================================================================
#include "cbase.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>
// memdbgon must be the last include file in a .cpp file!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Simple HUD element for displaying a sniper scope on screen
//-----------------------------------------------------------------------------
class CHudScope : public vgui::Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE(CHudScope, vgui::Panel);

public:
	CHudScope(const char *pElementName);

	void Init();
	void MsgFunc_ShowScope(bf_read &msg);

protected:
	virtual void ApplySchemeSettings(vgui::IScheme *scheme);
	virtual void Paint(void);

private:
	bool			m_bShow;
	CHudTexture*	m_pScope;
	//bool			m_bDebugged; // So I can show debug info just once in a function that runs every frame
};

DECLARE_HUDELEMENT(CHudScope);
DECLARE_HUD_MESSAGE(CHudScope, ShowScope);

using namespace vgui;

//-----------------------------------------------------------------------------
// Constructor - generic HUD element initialization stuff. Make sure our 2 member variables
// are instantiated.
//-----------------------------------------------------------------------------
CHudScope::CHudScope(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudScope")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	m_bShow = false;
	m_pScope = 0;

	// Scope will not show when the player is dead
	SetHiddenBits(HIDEHUD_PLAYERDEAD);

	// Fix for users with diffrent screen ratios
	int screenWide, screenTall;
	GetHudSize(screenWide, screenTall);
	SetBounds(0, 0, screenWide, screenTall);

}

//-----------------------------------------------------------------------------
// Hook up our HUD message, and make sure we are not showing the scope
//-----------------------------------------------------------------------------
void CHudScope::Init()
{
	HOOK_HUD_MESSAGE(CHudScope, ShowScope);

	m_bShow = false;
}

//-----------------------------------------------------------------------------
// Load in the scope material here
//-----------------------------------------------------------------------------
void CHudScope::ApplySchemeSettings(vgui::IScheme *scheme)
{
	BaseClass::ApplySchemeSettings(scheme);

	SetPaintBackgroundEnabled(false);
	SetPaintBorderEnabled(false);

	if (!m_pScope)
	{
		m_pScope = gHUD.GetIcon("Scope");
	}
}

//-----------------------------------------------------------------------------
// Simple - if we want to show the scope, draw it. Otherwise don't.
//-----------------------------------------------------------------------------
void CHudScope::Paint(void)
{
	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
	{
		return;
	}

	if (m_bShow)
	{
		int x1 = (ScreenWidth() / 2) - (ScreenHeight() / 2);
		int x2 = ScreenWidth() - (x1 * 2);
		int x3 = ScreenWidth() - x1;

		surface()->DrawSetColor(COLOR_BLACK);
		surface()->DrawFilledRect(0, 0, x1, ScreenHeight()); // Fill in the left side

		surface()->DrawSetColor(COLOR_BLACK);
		surface()->DrawFilledRect(x3, 0, ScreenWidth(), ScreenHeight()); // Fill in the right side

		m_pScope->DrawSelf(x1, 0, x2, ScreenHeight(), COLOR_WHITE); // Draw the scope as a perfect square

		pPlayer->m_Local.m_iHideHUD |= HIDEHUD_CROSSHAIR;
	}
	else if ((pPlayer->m_Local.m_iHideHUD & HIDEHUD_CROSSHAIR) != 0)
	{
		pPlayer->m_Local.m_iHideHUD &= ~HIDEHUD_CROSSHAIR;
	}
}

//-----------------------------------------------------------------------------
// Callback for our message - set the show variable to whatever
// boolean value is received in the message
//-----------------------------------------------------------------------------
void CHudScope::MsgFunc_ShowScope(bf_read &msg)
{
	m_bShow = msg.ReadByte();
}