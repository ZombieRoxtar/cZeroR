//===== Copyright Bit Mage's Stuff, All rights probably reserved. =====
//
// Purpose: Nightvision, kinda hacky
//
//=====================================================================
#include "cbase.h"
#include "items.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar	cl_nightvisioncolor_r("cl_nightvisioncolor_r", "0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE);
static ConVar	cl_nightvisioncolor_g("cl_nightvisioncolor_g", "128", FCVAR_CLIENTDLL | FCVAR_ARCHIVE);
static ConVar	cl_nightvisioncolor_b("cl_nightvisioncolor_b", "0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE);
static ConVar	cl_nightvisioncolor_a("cl_nightvisioncolor_a", "128", FCVAR_CLIENTDLL | FCVAR_ARCHIVE);

class CNightVision : public CItem
{
public:
	DECLARE_CLASS(CNightVision, CItem);
	DECLARE_SERVERCLASS()

	int UpdateTransmitState() // Always send to all clients
	{
		return SetTransmitState(FL_EDICT_ALWAYS);
	}

	virtual void Spawn(void);

	bool MyTouch(CBasePlayer *pPlayer);

	void StartUp(void);
	void Shutdown(void);
	void Toggle(void);

private:
	int m_nTint_r;
	int m_nTint_g;
	int m_nTint_b;
	int m_nTint_a;

	CNetworkVar(bool, m_bEquipped);
	CNetworkVar(bool, m_bActive);

};

LINK_ENTITY_TO_CLASS(item_nightvision, CNightVision);

IMPLEMENT_SERVERCLASS_ST(CNightVision, DT_NightVision)
SendPropBool(SENDINFO(m_bEquipped)),
SendPropBool(SENDINFO(m_bActive)),
END_SEND_TABLE()

CNightVision *pEquippedNVGs;

//-----------------------------------------------------------------------------
// Purpose: Get color values and spawn
//-----------------------------------------------------------------------------
void CNightVision::Spawn(void)
{
	m_nTint_r = cl_nightvisioncolor_r.GetInt();
	m_nTint_g = cl_nightvisioncolor_g.GetInt();
	m_nTint_b = cl_nightvisioncolor_b.GetInt();
	m_nTint_a = cl_nightvisioncolor_a.GetInt();

	BaseClass::Spawn();
}

bool CNightVision::MyTouch(CBasePlayer *pPlayer)
{
	if (pPlayer)
	{
		if (!pPlayer->PlayerHasNVGs())
		{
			pEquippedNVGs = this;
			m_bEquipped.Set(true);
			pPlayer->SetPlayerHasNVGs(true);

			CPASAttenuationFilter filter(pPlayer, "Player.NightVisionEquip");
			EmitSound(filter, pPlayer->entindex(), "Player.NightVisionEquip");
		}
		else
		{
			return true; // This destroys the goggles... I think...
		}
	}
	return false; // Do not destroy the goggles. Nigntvision lives in them...
}

//-----------------------------------------------------------------------------
// Purpose: Console command to force the goggles to turn on.
//-----------------------------------------------------------------------------
static void CC_FadeOut(const CCommand &args)
{
	if (!pEquippedNVGs)
	{
		return;
	}
	pEquippedNVGs->StartUp();
}
static ConCommand nvg_on("nvg_on", CC_FadeOut, "Turns on the NVG test effect.", FCVAR_CHEAT);
//-----------------------------------------------------------------------------
// Purpose: Console command to force the goggles to turn off.
//-----------------------------------------------------------------------------
static void CC_FadeIn(const CCommand &args)
{
	if (!pEquippedNVGs)
	{
		return;
	}
	pEquippedNVGs->Shutdown();
}
static ConCommand nvg_off("nvg_off", CC_FadeIn, "Turns off the NVG test effect.", FCVAR_CHEAT);
//-----------------------------------------------------------------------------
// Purpose: Toggle the goggles!
//-----------------------------------------------------------------------------
static void ToggleNVGs(const CCommand &args)
{
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	if (pPlayer)
	{
		if (pPlayer->PlayerHasNVGs())
		{
			pEquippedNVGs->Toggle();
		}
	}
	return;
}
static ConCommand nightvision("nightvision", ToggleNVGs, "Toggles the nightvision goggles (if equipped).");

void CNightVision::Toggle(void)
{
	if (pEquippedNVGs != this) // These goggles are not equipped
	{
		return;
	}
	if (!m_bActive)
	{
		StartUp();
	}
	else
	{
		Shutdown();
	}
}

void CNightVision::StartUp(void)
{
	color32 clrFade;
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();

	clrFade.r = cl_nightvisioncolor_r.GetInt();
	clrFade.g = cl_nightvisioncolor_g.GetInt();
	clrFade.b = cl_nightvisioncolor_b.GetInt();
	clrFade.a = cl_nightvisioncolor_a.GetInt();

	UTIL_ScreenFade(pPlayer, clrFade, 0, -1, FFADE_OUT | FFADE_PURGE | FFADE_STAYOUT);

	m_bActive.Set(true);
}
void CNightVision::Shutdown(void)
{
	color32 clrFade;
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();

	clrFade.r = cl_nightvisioncolor_r.GetInt();
	clrFade.g = cl_nightvisioncolor_g.GetInt();
	clrFade.b = cl_nightvisioncolor_b.GetInt();
	clrFade.a = cl_nightvisioncolor_a.GetInt();

	UTIL_ScreenFade(pPlayer, clrFade, 0, 0, FFADE_IN | FFADE_PURGE);
	m_bActive.Set(false);
}