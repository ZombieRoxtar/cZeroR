//===== Copyright Bit Mage's Stuff, All rights probably reserved. =====
//
// Purpose: Clientside nightvision invokes the overlayed noise texture
//			Also kinda hacky (Adding a FIXME here to straightup replace this effect)
//
//=====================================================================
#include "cbase.h"
#include "iviewrender.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define NIGHTVISION_OVERLAY "overlays/NightVision"

//-----------------------------------------------------------------------------
// NightVision
//-----------------------------------------------------------------------------
class C_NightVision : public C_BaseAnimating
{
public:
	DECLARE_CLASS(C_NightVision, C_BaseAnimating);
	DECLARE_CLIENTCLASS();

	~C_NightVision();

	virtual void Spawn(void);
	virtual void Precache(void);

	virtual void PostDataUpdate(DataUpdateType_t updateType); // This is called after we receive and process a network data packet

	void Start(void);
	void Stop(void);

private:
	bool m_bOverlayOn;

	CNetworkVar(bool, m_bEquipped);
	CNetworkVar(bool, m_bActive);
};

LINK_ENTITY_TO_CLASS(item_nightvision, C_NightVision);
IMPLEMENT_CLIENTCLASS_DT(C_NightVision, DT_NightVision, CNightVision)
RecvPropBool(RECVINFO(m_bEquipped)),
RecvPropBool(RECVINFO(m_bActive)),
END_RECV_TABLE()

C_NightVision::~C_NightVision()
{
	Stop();
}

void C_NightVision::Spawn(void)
{
	Precache();
}

void C_NightVision::Precache(void)
{
	PrecacheMaterial(NIGHTVISION_OVERLAY);
	BaseClass::Precache();
}

void C_NightVision::Start(void)
{
	m_bOverlayOn = true;
	IMaterial *pMaterial = materials->FindMaterial(NIGHTVISION_OVERLAY, TEXTURE_GROUP_CLIENT_EFFECTS, true);
	view->SetScreenOverlayMaterial(pMaterial); // Overlay the screen
	cvar->FindVar("mat_fullbright")->SetValue(1); // Activate the "light"
}
void C_NightVision::Stop(void)
{
	m_bOverlayOn = false;
	cvar->FindVar("mat_fullbright")->SetValue(0);
	view->SetScreenOverlayMaterial(null);
}

void C_NightVision::PostDataUpdate(DataUpdateType_t updateType)
{
	BaseClass::PostDataUpdate(updateType);

	if (m_bActive.Get() != m_bOverlayOn)
		if (m_bActive.Get())
			Start();
		else
			Stop();
}