//===== Copyright Bit Mage's Stuff, All rights probably reserved. =====
//
// Purpose: Dynamic Crosshair
//
//=====================================================================
#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "iclientmode.h"
#include "engine/IEngineSound.h"
#include "vgui_controls/AnimationController.h"
#include "vgui_controls/Controls.h"
#include "vgui_controls/Panel.h"
#include "vgui/ISurface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar cl_dynamiccrosshair("cl_dynamiccrosshair", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Enables dynamic crosshair; 0=off, 1=normal behavior (based on actual weapon accuracy)");
static ConVar cl_crosshairsize("cl_crosshairsize", "5", FCVAR_CLIENTDLL | FCVAR_ARCHIVE);
static ConVar cl_crosshairthickness("cl_crosshairthickness", ".5", FCVAR_CLIENTDLL | FCVAR_ARCHIVE);
static ConVar cl_crosshairspreadscale("cl_crosshairspreadscale", ".3", FCVAR_CLIENTDLL | FCVAR_ARCHIVE);
static ConVar cl_crosshairusealpha("cl_crosshairusealpha", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE);
static ConVar cl_crosshairalpha("cl_crosshairalpha", "200", FCVAR_CLIENTDLL | FCVAR_ARCHIVE);
static ConVar cl_crosshaircolor("cl_crosshaircolor", "0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Set crosshair color: 0=green, 1=red, 2=blue, 3=yellow, 4=cyan, 5=custom");
static ConVar cl_crosshaircolor_r("cl_crosshaircolor_r", "0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE);
static ConVar cl_crosshaircolor_g("cl_crosshaircolor_g", "0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE);
static ConVar cl_crosshaircolor_b("cl_crosshaircolor_b", "0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE);
static ConVar cl_crosshairdot("cl_crosshairdot", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE);

using namespace vgui;

class CHUDDynamicCrosshair : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CHUDDynamicCrosshair, vgui::Panel);
public:
	CHUDDynamicCrosshair(const char *pElementName);
	void VidInit(void);
	bool ShouldDraw(void);
	virtual void Paint();
	virtual void ApplySchemeSettings(IScheme *scheme);
	//bool debugged; // I use this bool so I can display debug messages once in the functions that run every frame here

private:
	float m_flAccSpread;
	float m_flScreenScale;
	int	m_ixCenter, m_iyCenter;
	float m_flScaledChSize, m_flScaledChThickness;
	int m_iTextureID_Reticle;
};
DECLARE_HUDELEMENT(CHUDDynamicCrosshair);

CHUDDynamicCrosshair::CHUDDynamicCrosshair(const char *pElementName) :
	CHudElement(pElementName), BaseClass(NULL, "HudNewCrosshair")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);
	SetHiddenBits(HIDEHUD_CROSSHAIR);

	m_flAccSpread = 0.0f;
	m_iTextureID_Reticle = surface()->CreateNewTextureID();
}

void CHUDDynamicCrosshair::ApplySchemeSettings(IScheme *scheme)
{
	BaseClass::ApplySchemeSettings(scheme);
	SetPaintBackgroundEnabled(false);
}

void CHUDDynamicCrosshair::VidInit(void)
{
	// HUD is a 640*480 "screen"
	// Scale is created from one dimension only to avoid stretching between different aspect ratios.
	m_flScreenScale = ScreenHeight() / 480;
	m_ixCenter = ScreenWidth() / 2;
	m_iyCenter = ScreenHeight() / 2;
}

//-----------------------------------------------------------------------------
// Purpose: Save CPU cycles by letting the HUD system early cull
// costly traversal.  Called per frame, return true if thinking and 
// painting need to occur.
//-----------------------------------------------------------------------------
bool CHUDDynamicCrosshair::ShouldDraw(void)
{
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	if (player == NULL)
		return false;

	if (!cl_dynamiccrosshair.GetBool())
		return false;

	return (CHudElement::ShouldDraw() && !engine->IsDrawingLoadingImage());
}

void CHUDDynamicCrosshair::Paint()
{
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	if (player == NULL)
		return;

	C_BaseCombatWeapon *pWeapon = GetActiveWeapon();
	if (pWeapon == NULL)
		return;

	// Interpret cl_crosshairusealpha and cl_crosshaircolor
	int realAlpha;
	if (cl_crosshairusealpha.GetBool())
	{
		realAlpha = cl_crosshairalpha.GetInt();
	}
	else
	{
		realAlpha = 255;
	}

	int crosshair_r;
	int crosshair_g;
	int crosshair_b;
	if ((cl_crosshaircolor.GetInt() >= 0) && (cl_crosshaircolor.GetInt() <= 4))
	{
		switch (cl_crosshaircolor.GetInt())
		{
		default:
		case 0: // green
			crosshair_r = 50;
			crosshair_g = 250;
			crosshair_b = 50;
			break;
		case 1: // red
			crosshair_r = 250;
			crosshair_g = 50;
			crosshair_b = 50;
			break;
		case 2: // blue
			crosshair_r = 50;
			crosshair_g = 50;
			crosshair_b = 250;
			break;
		case 3: // yellow
			crosshair_r = 250;
			crosshair_g = 250;
			crosshair_b = 50;
			break;
		case 4: // cyan
			crosshair_r = 50;
			crosshair_g = 250;
			crosshair_b = 250;
			break;
		}
	}
	else
	{
		crosshair_r = cl_crosshaircolor_r.GetInt();
		crosshair_g = cl_crosshaircolor_g.GetInt();
		crosshair_b = cl_crosshaircolor_b.GetInt();
	}

	// So special weapons can set their "accuracy" to 0 and have no crosshair? BUGBUG? What about VERY accurate weapons?
	if (pWeapon->m_vCone.m_Value.x)
	{
		// Scale our crosshair
		m_flScaledChSize = (cl_crosshairsize.GetFloat() * m_flScreenScale);
		m_flScaledChThickness = (cl_crosshairthickness.GetFloat() * m_flScreenScale);

		surface()->DrawSetColor(crosshair_r, crosshair_g, crosshair_b, realAlpha);

		// Center Dot
		if (cl_crosshairdot.GetBool())
		{
			//BUGBUG: DrawFilledRect() can't draw a 1px dot
			if (m_flScaledChThickness < 1)
			{
				// Currently drawing a 3x3 "dot" so it has a center pixel in the center of the screen.
				surface()->DrawFilledRect(m_ixCenter - 1, m_iyCenter - 1, m_ixCenter + 1, m_iyCenter + 1);
			}
			else
			{
				surface()->DrawFilledRect(m_ixCenter - (m_flScaledChThickness / 2), m_iyCenter - (m_flScaledChThickness / 2),
					m_ixCenter + (m_flScaledChThickness / 2), m_iyCenter + (m_flScaledChThickness / 2));
			}
		}

		// cl_dynamiccrosshair 0 shows the CS crosshair, just static
		if (cl_dynamiccrosshair.GetInt())
		{
			m_flAccSpread = (RAD2DEG(asin(pWeapon->m_vCone.m_Value.x)) * 2) *  cl_crosshairspreadscale.GetFloat() * m_flScreenScale;
		}

		// HACK?: If this doesn't happen, the top & left lines read these values slightly higher.
		m_flScaledChSize = RoundFloatToInt(m_flScaledChSize);
		m_flAccSpread = RoundFloatToInt(m_flAccSpread);

		//BUGBUG: DrawFilledRect() can't draw a 1px line
		if (m_flScaledChThickness <= 1)
		{
			//  top line
			surface()->DrawLine(m_ixCenter, m_iyCenter - 2 - m_flAccSpread, m_ixCenter, m_iyCenter - 2 - m_flAccSpread - m_flScaledChSize);
			//  bottom line
			surface()->DrawLine(m_ixCenter, m_iyCenter + 2 + m_flAccSpread, m_ixCenter, m_iyCenter + 2 + m_flAccSpread + m_flScaledChSize);
			//  left line
			surface()->DrawLine(m_ixCenter - 2 - m_flAccSpread, m_iyCenter, m_ixCenter - 2 - m_flAccSpread - m_flScaledChSize, m_iyCenter);
			//  right line
			surface()->DrawLine(m_ixCenter + 2 + m_flAccSpread, m_iyCenter, m_ixCenter + 2 + m_flAccSpread + m_flScaledChSize, m_iyCenter);
		}
		else
		{
			//  top line
			surface()->DrawFilledRect(m_ixCenter - (m_flScaledChThickness / 2),
				m_iyCenter - 2 - (m_flScaledChThickness / 2) - m_flAccSpread - m_flScaledChSize,
				m_ixCenter + (m_flScaledChThickness / 2), m_iyCenter - 2 - (m_flScaledChThickness / 2) - m_flAccSpread);
			//  bottom line
			surface()->DrawFilledRect(m_ixCenter - (m_flScaledChThickness / 2), m_iyCenter + 2 + m_flAccSpread + (m_flScaledChThickness / 2),
				m_ixCenter + (m_flScaledChThickness / 2), m_iyCenter + 2 + m_flAccSpread + m_flScaledChSize + (m_flScaledChThickness / 2));
			//  left line
			surface()->DrawFilledRect(m_ixCenter - 2 - m_flAccSpread - m_flScaledChSize - (m_flScaledChThickness / 2),
				m_iyCenter - (m_flScaledChThickness / 2),
				m_ixCenter - 2 - m_flAccSpread - (m_flScaledChThickness / 2), m_iyCenter + (m_flScaledChThickness / 2));
			//  right line
			surface()->DrawFilledRect(m_ixCenter + 2 + m_flAccSpread + (m_flScaledChThickness / 2), m_iyCenter - (m_flScaledChThickness / 2),
				m_ixCenter + 2 + m_flAccSpread + m_flScaledChSize + (m_flScaledChThickness / 2), m_iyCenter + (m_flScaledChThickness / 2));
		}
	}
}
