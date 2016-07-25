//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "fx.h"
#include "c_te_effect_dispatch.h"
#include "c_te_legacytempents.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ShellEjectCallback( const CEffectData &data )
{
	// Use the gun angles to orient the shell
	IClientRenderable *pRenderable = data.GetRenderable();
	if ( pRenderable )
	{
		tempents->EjectBrass( data.m_vOrigin, data.m_vAngles, pRenderable->GetRenderAngles(), 0 );
	}
}

DECLARE_CLIENT_EFFECT( "ShellEject", ShellEjectCallback );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void RifleShellEjectCallback( const CEffectData &data )
{
	// Use the gun angles to orient the shell
	IClientRenderable *pRenderable = data.GetRenderable();
	if ( pRenderable )
	{
		tempents->EjectBrass( data.m_vOrigin, data.m_vAngles, pRenderable->GetRenderAngles(), 1 );
	}
}

DECLARE_CLIENT_EFFECT( "RifleShellEject", RifleShellEjectCallback );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ShotgunShellEjectCallback( const CEffectData &data )
{
	// Use the gun angles to orient the shell
	IClientRenderable *pRenderable = data.GetRenderable();
	if ( pRenderable )
	{
		tempents->EjectBrass( data.m_vOrigin, data.m_vAngles, pRenderable->GetRenderAngles(), 2 );
	}
}

DECLARE_CLIENT_EFFECT( "ShotgunShellEject", ShotgunShellEjectCallback );

//-----------------------------------------------------------------------------
// Purpose: Main func for CZ Shells
//-----------------------------------------------------------------------------
void CZShellEject(const CEffectData &data, const int &type)
{
	IClientRenderable *pRenderable = data.GetRenderable();
	if (pRenderable)
	{
		C_BasePlayer* pPlayer =
			ToBasePlayer(dynamic_cast < C_BaseCombatWeapon* >
				(dynamic_cast < C_BaseViewModel* >
					(data.m_hEntity.Get())->GetWeapon())->GetOwner());

		if (pPlayer)
		{
			int EjectForce = 32;
			switch (type)
			{
			case CS_SHELL_762NATO:
				EjectForce = 64;
				break;
			case CS_SHELL_12GAUGE:
				EjectForce = 16;
				break;
			//All these can use the default of 32
			//case CS_SHELL_338MAG:
			//case CS_SHELL_57:
			//case CS_SHELL_9MM:
			//case CS_SHELL_556:
			//default:
			}
			tempents->CSEjectBrass(data.m_vOrigin, data.m_vAngles, EjectForce, type, pPlayer);
		}
	}
}
//-----------------------------------------------------------------------------
// Purpose: 12G Brass
//-----------------------------------------------------------------------------
void SG12GShellEjectCallback(const CEffectData &data)
{
	CZShellEject(data, CS_SHELL_12GAUGE);
}
//-----------------------------------------------------------------------------
// Purpose: 338 Brass
//-----------------------------------------------------------------------------
void Rif338ShellEjectCallback(const CEffectData &data)
{
	CZShellEject(data, CS_SHELL_338MAG);
}
//-----------------------------------------------------------------------------
// Purpose: 556 Brass
//-----------------------------------------------------------------------------
void Rif556ShellEjectCallback(const CEffectData &data)
{
	CZShellEject(data, CS_SHELL_556);
}
//-----------------------------------------------------------------------------
// Purpose: 57 Brass
//-----------------------------------------------------------------------------
void Pist57ShellEjectCallback(const CEffectData &data)
{
	CZShellEject(data, CS_SHELL_57);
}
//-----------------------------------------------------------------------------
// Purpose: 762 Brass
//-----------------------------------------------------------------------------
void Rif762ShellEjectCallback(const CEffectData &data)
{
	CZShellEject(data, CS_SHELL_762NATO);
}
//-----------------------------------------------------------------------------
// Purpose: 9MM Brass
//-----------------------------------------------------------------------------
void Pist9MMShellEjectCallback(const CEffectData &data)
{
	CZShellEject(data, CS_SHELL_9MM);
}

DECLARE_CLIENT_EFFECT("EjectBrass_12Gauge", SG12GShellEjectCallback);
DECLARE_CLIENT_EFFECT("EjectBrass_338Mag", Rif338ShellEjectCallback);
DECLARE_CLIENT_EFFECT("EjectBrass_556", Rif556ShellEjectCallback);
DECLARE_CLIENT_EFFECT("EjectBrass_57", Pist57ShellEjectCallback);
DECLARE_CLIENT_EFFECT("EjectBrass_762Nato", Rif762ShellEjectCallback);
DECLARE_CLIENT_EFFECT("EjectBrass_9mm", Pist9MMShellEjectCallback);
