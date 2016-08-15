//========= Copyright Bit Mage's Stuff, All rights probably reserved. =========
//
// Purpose: Let's try putting all the grenades in one file...
//
//=============================================================================
#include "cbase.h"
#include "basegrenade_shared.h"
#include "advanced_grenades.h"
#include "Sprite.h"
#include "SpriteTrail.h"
#include "soundent.h"
#include "gamestats.h"

#include "fire.h"
#include "gib.h"
#include "engine/IEngineSound.h"
#include "npc_bullseye.h"
#include "entitylist.h"
#include "antlion_maker.h"
#include "eventqueue.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define FRAG_GRENADE_WARN_TIME 1.5f
#define GRENADE_NPC_CLASS "npc_advanced_grenade"

#define SMOKE_CLOUD_MATERIAL "particle/particle_smokegrenade.vmt" // For smoke grenades
#define SMOKE_LIFE_TIME 20.0f

#define FIRE_GROW_TIME 0.25f
#define FIRE_LIFE_TIME 10.0f

#define FLASHBANG_BLEND_OUT_TIME 0.5f

#define GRENADE_COEFFICIENT_OF_RESTITUTION 0.2f

#define FIREBALL_RADIUS ( m_DmgRadius / 4 ) // The brief fireball before spreading the actual fire

extern ConVar sk_plr_dmg_decoygrenade;
extern ConVar sk_npc_dmg_decoygrenade;
extern ConVar  sk_decoygrenade_radius;

extern ConVar sk_plr_dmg_flashgrenade;
extern ConVar sk_npc_dmg_flashgrenade;
extern ConVar  sk_flashgrenade_radius;

extern ConVar sk_plr_dmg_grenade;
extern ConVar sk_npc_dmg_grenade;
extern ConVar  sk_hegrenade_radius;

extern ConVar sk_plr_dmg_incgrenade;
extern ConVar sk_npc_dmg_incgrenade;
extern ConVar  sk_incgrenade_radius;

extern ConVar sk_plr_dmg_molotov;
extern ConVar sk_npc_dmg_molotov;
extern ConVar  sk_molotov_radius;

extern ConVar sk_plr_dmg_smokegrenade;
extern ConVar sk_npc_dmg_smokegrenade;
extern ConVar  sk_smokegrenade_radius;

extern short g_sModelIndexFireball;	  // (in combatweapon.cpp) holds the index for the fireball 
extern short g_sModelIndexWExplosion; // (in combatweapon.cpp) holds the index for the underwater explosion

class CAdvancedGrenade : public CBaseGrenade
{
	DECLARE_CLASS(CAdvancedGrenade, CBaseGrenade);
	DECLARE_DATADESC();

public:
	void Spawn(void);
	bool CreateVPhysics(void);
	void SetTimer(float detonateDelay, float warnDelay);
	void SetVelocity(const Vector &velocity, const AngularImpulse &angVelocity);
	int	 OnTakeDamage(const CTakeDamageInfo &inputInfo);
	void DelayThink();
	void SmokeThink();
	void VPhysicsUpdate(IPhysicsObject *pPhysics);
	void InputSetTimer(inputdata_t &inputdata);

	void DisorientNPCs();
	void BlindPlayers();
	void SetType(int type);
	int GetType(void) { return m_nType; }
	void GlassGibs(int count = 2);

	// Overrides
	virtual void Detonate(void);
	virtual void Explode(trace_t *pTrace, int bitsDamageType);

	virtual CBaseEntity *Create(const char *szName, const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner = NULL, int nType = HI_EX_GRENADE);

	void OtherExplode(trace_t *pTrace, int bitsDamageType);
	void GoBoom(trace_t *pTrace, int bitsDamageType, int nType = HI_EX_GRENADE);

protected:
	bool m_inSolid;
	bool m_bInFlash;
	CBasePlayer m_pThrower;
	const char *szModelName;
	int m_nType;
	float m_flRunTime;
};

LINK_ENTITY_TO_CLASS(npc_advanced_grenade, CAdvancedGrenade);

BEGIN_DATADESC(CAdvancedGrenade)

DEFINE_KEYFIELD(m_nType, FIELD_INTEGER, "Type"),
DEFINE_FIELD(m_inSolid, FIELD_BOOLEAN),

// Function Pointers
DEFINE_THINKFUNC(DelayThink),
DEFINE_THINKFUNC(SmokeThink),

// Inputs
DEFINE_INPUTFUNC(FIELD_FLOAT, "SetTimer", InputSetTimer),

END_DATADESC()

CBaseEntity *CAdvancedGrenade::Create(const char *szName, const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner, int nType)
{
	SetType(nType);
	return BaseClass::Create(szName, vecOrigin, vecAngles, pOwner);
}

void CAdvancedGrenade::Spawn(void)
{
	if (GetOwnerEntity() && GetOwnerEntity()->IsPlayer())
	{
		m_flDamage = sk_plr_dmg_grenade.GetFloat();
	}
	else
	{
		m_flDamage = sk_npc_dmg_grenade.GetFloat();
	}
	m_DmgRadius = sk_hegrenade_radius.GetFloat();

	szModelName = "models/weapons/w_eq_fraggrenade_thrown.mdl";
	switch (this->GetType())
	{
	case DECOY_GRENADE:
		szModelName = "models/weapons/w_eq_decoy_thrown.mdl";
		if (GetOwnerEntity() && GetOwnerEntity()->IsPlayer())
		{
			m_flDamage = sk_plr_dmg_decoygrenade.GetFloat();
		}
		else
		{
			m_flDamage = sk_npc_dmg_decoygrenade.GetFloat();
		}
		m_DmgRadius = sk_decoygrenade_radius.GetFloat();
		break;

	case FLASH_GRENADE:
		szModelName = "models/weapons/w_eq_flashbang_thrown.mdl";
		if (GetOwnerEntity() && GetOwnerEntity()->IsPlayer())
		{
			m_flDamage = sk_plr_dmg_flashgrenade.GetFloat();
		}
		else
		{
			m_flDamage = sk_npc_dmg_flashgrenade.GetFloat();
		}
		m_DmgRadius = sk_flashgrenade_radius.GetFloat();
		break;

	case INCEN_GRENADE:
		szModelName = "models/weapons/w_eq_incendiarygrenade_thrown.mdl";
		if (GetOwnerEntity() && GetOwnerEntity()->IsPlayer())
		{
			m_flDamage = sk_plr_dmg_incgrenade.GetFloat();
		}
		else
		{
			m_flDamage = sk_plr_dmg_incgrenade.GetFloat();
		}
		m_DmgRadius = sk_incgrenade_radius.GetFloat();
		break;

	case MOLOT_GRENADE:
		szModelName = "models/weapons/w_eq_molotov_thrown.mdl";
		if (GetOwnerEntity() && GetOwnerEntity()->IsPlayer())
		{
			m_flDamage = sk_plr_dmg_molotov.GetFloat();
		}
		else
		{
			m_flDamage = sk_npc_dmg_molotov.GetFloat();
		}
		m_DmgRadius = sk_molotov_radius.GetFloat();
		break;

	case SMOKE_GRENADE:
		szModelName = "models/weapons/w_eq_smokegrenade_thrown.mdl";
		if (GetOwnerEntity() && GetOwnerEntity()->IsPlayer())
		{
			m_flDamage = sk_plr_dmg_smokegrenade.GetFloat();
		}
		else
		{
			m_flDamage = sk_npc_dmg_smokegrenade.GetFloat();
		}
		m_DmgRadius = sk_smokegrenade_radius.GetFloat();

		/* There is no case for default or HE because the vars are initialized to the HE Grenade's to avoid gremlins */
	}

	if (!szModelName || !*szModelName)
	{
		Warning("%s at %.0f, %.0f, %0.f missing modelname!\n",
			GetClassname(),
			GetAbsOrigin().x,
			GetAbsOrigin().y,
			GetAbsOrigin().z);
		UTIL_Remove(this);
		return;
	}
	PrecacheModel(szModelName);
	Precache();
	SetModel(szModelName);

	m_takedamage = DAMAGE_YES;
	m_iHealth = 1;

	SetSize(-Vector(4, 4, 4), Vector(4, 4, 4));
	SetCollisionGroup(COLLISION_GROUP_WEAPON);
	CreateVPhysics();

	AddSolidFlags(FSOLID_NOT_STANDABLE);

	BaseClass::Spawn();
}

bool CAdvancedGrenade::CreateVPhysics()
{
	// Create the object in the physics system
	VPhysicsInitNormal(SOLID_BBOX, 0, false);
	return true;
}

// This will hit only things that are in newCollisionGroup, but NOT in collisionGroupAlreadyChecked
class CTraceFilterCollisionGroupDelta : public CTraceFilterEntitiesOnly
{
public:
	// It does have a base, but we'll never network anything below here..
	DECLARE_CLASS_NOBASE(CTraceFilterCollisionGroupDelta);

	CTraceFilterCollisionGroupDelta(const IHandleEntity *passentity, int collisionGroupAlreadyChecked, int newCollisionGroup)
		: m_pPassEnt(passentity), m_collisionGroupAlreadyChecked(collisionGroupAlreadyChecked), m_newCollisionGroup(newCollisionGroup)
	{
	}

	virtual bool ShouldHitEntity(IHandleEntity *pHandleEntity, int contentsMask)
	{
		if (!PassServerEntityFilter(pHandleEntity, m_pPassEnt))
			return false;
		CBaseEntity *pEntity = EntityFromEntityHandle(pHandleEntity);

		if (pEntity)
		{
			if (g_pGameRules->ShouldCollide(m_collisionGroupAlreadyChecked, pEntity->GetCollisionGroup()))
				return false;
			if (g_pGameRules->ShouldCollide(m_newCollisionGroup, pEntity->GetCollisionGroup()))
				return true;
		}

		return false;
	}

protected:
	const IHandleEntity *m_pPassEnt;
	int		m_collisionGroupAlreadyChecked;
	int		m_newCollisionGroup;
};

void CAdvancedGrenade::VPhysicsUpdate(IPhysicsObject *pPhysics)
{
	BaseClass::VPhysicsUpdate(pPhysics);
	Vector vel;
	AngularImpulse angVel;
	pPhysics->GetVelocity(&vel, &angVel);

	Vector start = GetAbsOrigin();
	// find all entities that my collision group wouldn't hit, but COLLISION_GROUP_NONE would and bounce off of them as a ray cast
	CTraceFilterCollisionGroupDelta filter(this, GetCollisionGroup(), COLLISION_GROUP_NONE);
	trace_t tr;

	UTIL_TraceLine(start, start + vel * gpGlobals->frametime, CONTENTS_HITBOX | CONTENTS_MONSTER | CONTENTS_SOLID, &filter, &tr);

	if (tr.startsolid)
	{
		if (!m_inSolid)
		{
			vel *= -GRENADE_COEFFICIENT_OF_RESTITUTION; // bounce backwards
			pPhysics->SetVelocity(&vel, NULL);
		}
		m_inSolid = true;
		return;
	}
	m_inSolid = false;
	if (tr.DidHit())
	{
		Vector dir = vel;
		VectorNormalize(dir);
		// Send a tiny amount of damage so the character will react to getting bonked
		CTakeDamageInfo info(this, GetThrower(), pPhysics->GetMass() * vel, GetAbsOrigin(), 0.1f, DMG_CRUSH);
		tr.m_pEnt->TakeDamage(info);

		// Reflect velocity around normal
		vel = -2.0f * tr.plane.normal * DotProduct(vel, tr.plane.normal) + vel;

		// Absorb 80% in impact
		vel *= GRENADE_COEFFICIENT_OF_RESTITUTION;
		angVel *= -0.5f;
		pPhysics->SetVelocity(&vel, &angVel);
	}
}

void CAdvancedGrenade::SetTimer(float detonateDelay, float warnDelay)
{
	m_flDetonateTime = gpGlobals->curtime + detonateDelay;
	m_flWarnAITime = gpGlobals->curtime + warnDelay;
	SetThink(&CAdvancedGrenade::DelayThink);
	SetNextThink(gpGlobals->curtime);
}

void CAdvancedGrenade::DelayThink()
{
	if (gpGlobals->curtime > m_flDetonateTime)
	{
		Detonate();
		return;
	}

	if (!m_bHasWarnedAI && gpGlobals->curtime >= m_flWarnAITime)
	{
		CSoundEnt::InsertSound(SOUND_DANGER, GetAbsOrigin(), 400, 1.5, this);
		m_bHasWarnedAI = true;
	}

	SetNextThink(gpGlobals->curtime + 0.1);
}

void CAdvancedGrenade::SmokeThink()
{
	float flSmokeBirthRate = 0.1f;
	int nFrameRate = 5;
	Vector vOrigin = GetAbsOrigin();
	CPASFilter filter(vOrigin);
	float flOriginVariance = 8.0f;
	// Apply some variance
	vOrigin += Vector(random->RandomFloat(flOriginVariance * -1, flOriginVariance), random->RandomFloat(flOriginVariance * -1, flOriginVariance), random->RandomFloat(flOriginVariance * -1, flOriginVariance));

	//BUGBUG: There's some kinda sprite-culling here so the smoke stops rendering if <1/2 of the giant puff is visible
	short g_sModelIndexSmokeScreen = CBaseEntity::PrecacheModel(SMOKE_CLOUD_MATERIAL);
	te->Smoke(filter, 0, &vOrigin, g_sModelIndexSmokeScreen, m_DmgRadius, nFrameRate);

	// Reset the origin and apply some positional variance again
	vOrigin = GetAbsOrigin();
	vOrigin += Vector(random->RandomFloat(flOriginVariance * -1, flOriginVariance), random->RandomFloat(flOriginVariance * -1, flOriginVariance), random->RandomFloat(flOriginVariance * -1, flOriginVariance));
	UTIL_Smoke(vOrigin, m_DmgRadius, nFrameRate);

	SetThink(&CAdvancedGrenade::SmokeThink);
	if (m_flRunTime > SMOKE_LIFE_TIME)
	{
		SetThink(&CBaseGrenade::SUB_Remove);
	}
	SetNextThink(gpGlobals->curtime + flSmokeBirthRate);
	m_flRunTime += flSmokeBirthRate;
}

void CAdvancedGrenade::SetVelocity(const Vector &velocity, const AngularImpulse &angVelocity)
{
	IPhysicsObject *pPhysicsObject = VPhysicsGetObject();
	if (pPhysicsObject)
	{
		pPhysicsObject->AddVelocity(&velocity, &angVelocity);
	}
}

int CAdvancedGrenade::OnTakeDamage(const CTakeDamageInfo &inputInfo)
{
	// Manually apply vphysics because BaseCombatCharacter takedamage doesn't call back to CBaseEntity OnTakeDamage
	VPhysicsTakeDamage(inputInfo);

	// Grenades only suffer blast damage and burn damage.
	if (!(inputInfo.GetDamageType() & (DMG_BLAST | DMG_BURN)))
		return 0;

	return BaseClass::OnTakeDamage(inputInfo);
}

void CAdvancedGrenade::InputSetTimer(inputdata_t &inputdata)
{
	SetTimer(inputdata.value.Float(), inputdata.value.Float() - FRAG_GRENADE_WARN_TIME);
}

void CAdvancedGrenade::SetType(int type)
{
	if ((type >= FIRST_GRENADE_TYPE) && (type <= LAST_GRENADE_TYPE))
	{
		m_nType = type;

		szModelName = "models/weapons/w_eq_fraggrenade_thrown.mdl";
		switch (this->GetType())
		{
		case DECOY_GRENADE:
			szModelName = "models/weapons/w_eq_decoy_thrown.mdl";
			break;

		case FLASH_GRENADE:
			szModelName = "models/weapons/w_eq_flashbang_thrown.mdl";
			break;

		case INCEN_GRENADE:
			szModelName = "models/weapons/w_eq_incendiarygrenade_thrown.mdl";
			break;

		case MOLOT_GRENADE:
			szModelName = "models/weapons/w_eq_molotov_thrown.mdl";
			break;

		case SMOKE_GRENADE:
			PrecacheModel(SMOKE_CLOUD_MATERIAL);
			szModelName = "models/weapons/w_eq_smokegrenade_thrown.mdl";
			/* There is no case for default or HE because the vars are initialized to the HE Grenade's to avoid gremlins */
		}

		if (!szModelName || !*szModelName)
		{
			Warning("%s at %.0f, %.0f, %0.f missing modelname!\n",
				GetClassname(),
				GetAbsOrigin().x,
				GetAbsOrigin().y,
				GetAbsOrigin().z);
			UTIL_Remove(this);
			return;
		}
		PrecacheModel(szModelName);
		SetModel(szModelName);
	}
	else
	{
		Warning("Failed to set grenade type!\n");
		m_nType = HI_EX_GRENADE;
	}
}

//HACKHACK: The type is set too late for the spawn function to set the thrown model -- need a clever solution, but will settle for a hacky one ;)
CAdvancedGrenade *Grenade_Create(const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer, int type)
{
	// Don't set the owner here, or the player can't interact with grenades he's thrown
	CAdvancedGrenade *pGrenade = (CAdvancedGrenade*)CBaseEntity::Create(GRENADE_NPC_CLASS, position, angles, pOwner);

	pGrenade->SetType(type);
	pGrenade->SetTimer(timer, timer - FRAG_GRENADE_WARN_TIME);
	pGrenade->SetVelocity(velocity, angVelocity);
	pGrenade->SetThrower(ToBaseCombatCharacter(pOwner));
	pGrenade->m_takedamage = DAMAGE_EVENTS_ONLY;

	return pGrenade;
}

void CAdvancedGrenade::Detonate(void)
{
	SetThink(NULL);

	trace_t	tr;
	Vector	vecSpot;

	vecSpot = GetAbsOrigin() + Vector(0, 0, 8);
	UTIL_TraceLine(vecSpot, vecSpot + Vector(0, 0, -32), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr);

	if (tr.startsolid)
	{
		UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + Vector(0, 0, -32), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr);
	}

	switch (this->GetType())
	{
	case DECOY_GRENADE:
	case FLASH_GRENADE:
	case SMOKE_GRENADE:
	case MOLOT_GRENADE:
	case INCEN_GRENADE:
		Explode(&tr, DMG_BURN);
		break;
	default:
		Explode(&tr, DMG_BLAST);
	}
}

void CAdvancedGrenade::Explode(trace_t *pTrace, int bitsDamageType)
{
	CBaseEntity *pThrower = this->GetOriginalThrower();
	CBasePlayer *pPlayer = ToBasePlayer(pThrower);
	CTakeDamageInfo info;

	// Use the thrower's position as the reported position
	Vector vecReported = pThrower ? pThrower->GetAbsOrigin() : vec3_origin;

	AddSolidFlags(FSOLID_NOT_SOLID);

	m_takedamage = DAMAGE_NO;

	int nType = this->GetType();

	if (nType == HI_EX_GRENADE)
	{
		GoBoom(pTrace, bitsDamageType);
	}
	else
	{
		OtherExplode(pTrace, bitsDamageType);
	}

	info.Set(this, pThrower, GetBlastForce(), GetAbsOrigin(), m_flDamage, bitsDamageType, 0, &vecReported);

	if (nType != SMOKE_GRENADE) // Wait, I'm running an effect!
	{
		SetThink(&CBaseGrenade::SUB_Remove);
		SetNextThink(gpGlobals->curtime + 0.1);
	}
	SetTouch(NULL);
	SetSolid(SOLID_NONE);

	AddEffects(EF_NODRAW);
	SetAbsVelocity(vec3_origin);


	if (pPlayer)
	{
		char *szWeaponClass = "weapon_hegrenade";

		switch (nType)
		{
		case DECOY_GRENADE:
			szWeaponClass = "weapon_decoy";
			break;
		case FLASH_GRENADE:
			szWeaponClass = "weapon_flashbang";
			break;
		case INCEN_GRENADE:
			szWeaponClass = "weapon_incgrenade";
			break;
		case MOLOT_GRENADE:
			szWeaponClass = "weapon_molotov";
			break;
		case SMOKE_GRENADE:
			szWeaponClass = "weapon_smokegrenade";
			/* There is no case for default or HE because szWeaponClass is initialized to "weapon_hegrenade" */
		}
		gamestats->Event_WeaponHit(pPlayer, true, szWeaponClass, info);
	}
}

void CAdvancedGrenade::OtherExplode(trace_t *pTrace, int bitsDamageType)
{
	int nType = this->GetType();

	if ((nType == MOLOT_GRENADE) || (nType == INCEN_GRENADE))
	{
		float fFullRadius = m_DmgRadius;
		m_DmgRadius = FIREBALL_RADIUS;
		GoBoom(pTrace, bitsDamageType);
		m_DmgRadius = fFullRadius;

		if (nType == MOLOT_GRENADE)
		{
			GlassGibs();
		}

		QAngle angles;
		VectorAngles(pTrace->plane.normal, angles);
		FireSystem_StartFire(GetAbsOrigin(), m_DmgRadius, FIRE_GROW_TIME, FIRE_LIFE_TIME, 0, this->GetOriginalThrower()->GetBaseEntity());
		//BUGBUG: FIXME: The visual fire has the height of m_DmgRadius, but appears to have a small radius.
		//TODO: Make many, small fires?
	}

	if ((nType == SMOKE_GRENADE) || (nType == FLASH_GRENADE))
	{
		UTIL_DecalTrace(pTrace, "Scorch");
		CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), BASEGRENADE_EXPLOSION_VOLUME, 3.0);
		if (nType == FLASH_GRENADE)
		{
			BlindPlayers();
		}
		if (nType == SMOKE_GRENADE)
		{
			m_flRunTime = 0.0f;
			SetThink(&CAdvancedGrenade::SmokeThink);
			SetNextThink(gpGlobals->curtime);
		}
		DisorientNPCs();
	}

	if (nType == DECOY_GRENADE)
	{
		CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), BASEGRENADE_EXPLOSION_VOLUME, 3.0);
		EmitSound("DecoyGrenade.Decoy");
	}

	// Only do these effects if we're not submerged
	if (!(UTIL_PointContents(GetAbsOrigin()) & CONTENTS_WATER))
	{
		// draw sparks
		int sparkCount = random->RandomInt(1, 4);

		for (int i = 0; i < sparkCount; i++)
		{
			QAngle angles;
			VectorAngles(pTrace->plane.normal, angles);
			Create("spark_shower", GetAbsOrigin(), angles, NULL);
		}
	}

	GoBoom(pTrace, bitsDamageType, nType);
}
void CAdvancedGrenade::DisorientNPCs(void)
{
	CBaseEntity* pThrower = this->GetOriginalThrower();
	Vector		 delta(m_DmgRadius, m_DmgRadius, m_DmgRadius);
	CBaseEntity* pList[128];

	int count = UTIL_EntitiesInBox(pList, SIZE_OF_ARRAY(pList), GetAbsOrigin() - delta, GetAbsOrigin() + delta, 0);

	for (int i = 0; i < count; i++)
	{
		// If close enough, make people freak out
		if (UTIL_DistApprox(pList[i]->WorldSpaceCenter(), GetAbsOrigin()) < m_DmgRadius)
		{
			// Must be a CZ combat character
			if (FClassnameIs(pList[i], "npc_*errorist_*")) /* "*errorist" will match Ts and CTs */
			{
				CAI_BaseNPC *pPerson = pList[i]->MyNPCPointer();

				if (pPerson != NULL)
				{
					trace_t tr;
					UTIL_TraceLine(GetAbsOrigin(), pPerson->EyePosition(), MASK_ALL, pThrower, COLLISION_GROUP_NONE, &tr);

					if (tr.fraction == 1.0 || tr.m_pEnt == pPerson)
					{
						// Randomize the start time a little so they don't all dance in synch.
						g_EventQueue.AddEvent(pPerson, "HitBySmoke", RandomFloat(0, 0.5), pThrower, pThrower);
					}
				}
			}
		}
	}
}

void CAdvancedGrenade::BlindPlayers(void)
{
	// An array for all the player pointers
	CBaseEntity *list[128];

	int count = UTIL_EntitiesInSphere(list, SIZE_OF_ARRAY(list), GetAbsOrigin() + Vector(0, 10, 0), 4096, MASK_PLAYERSOLID);

	for (int i = 0; i < count; i++)
	{
		// make sure its a player
		if (list[i]->IsPlayer())
		{
			// trace line to make
			trace_t tr;
			UTIL_TraceLine(list[i]->GetAbsOrigin(), GetAbsOrigin() + Vector(0, 10, 0), MASK_PLAYERSOLID, NULL, COLLISION_GROUP_NONE, &tr);
			CBasePlayer *pPlayer = ToBasePlayer(list[i]);

			float distancesq = (pPlayer->GetAbsOrigin() - GetAbsOrigin() + Vector(0, 10, 0)).LengthSqr();

			float ratio = 1.0;

			if (distancesq >= 40000)
			{
				ratio = (200 - (vec_t)FastSqrt(distancesq)) / 200.0;
			}

			color32 white = { 255, 255, 255, 255 };

			int fadehold = random->RandomInt(1, 3);

			if (tr.fraction < 1.0f)
			{
				if (pPlayer->FInViewCone(this))
				{
					float directivity = 1.0f - tr.fraction; // (I think) This measures how hard the player is staring at the grenade (0 = Can't see it)

					UTIL_ScreenFade(pPlayer, white, FLASHBANG_BLEND_OUT_TIME + ((.5f * fadehold) + directivity), fadehold + directivity, FFADE_IN);
					CSingleUserRecipientFilter user(pPlayer);
					enginesound->SetPlayerDSP(user, random->RandomInt(35, 37), false);
				}
			}
		}
	}
}

//HACHACK: (or is it?) The type is passed since we can't query a grenade you already threw.
void CAdvancedGrenade::GoBoom(trace_t *pTrace, int bitsDamageType, int nType)
{
	CBaseEntity* pThrower = this->GetOriginalThrower();
	// Use the thrower's position as the reported position
	Vector vecReported = pThrower ? pThrower->GetAbsOrigin() : vec3_origin;

	SetModelName(NULL_STRING);

	Vector vecAbsOrigin = GetAbsOrigin();
	int contents = UTIL_PointContents(vecAbsOrigin);
	
	if (pTrace->fraction != 1.0)
	{
		// Pull out of the wall a bit
		SetAbsOrigin(pTrace->endpos + (pTrace->plane.normal * 0.6));

		Vector vecNormal = pTrace->plane.normal;
		surfacedata_t *pdata = physprops->GetSurfaceData(pTrace->surface.surfaceProps);
		CPASFilter filter(vecAbsOrigin);

		te->Explosion(filter, -1.0,
			&vecAbsOrigin,
			!(contents & MASK_WATER) ? g_sModelIndexFireball : g_sModelIndexWExplosion,
			m_DmgRadius * .03,
			25,
			TE_EXPLFLAG_NONE,
			m_DmgRadius,
			m_flDamage,
			&vecNormal,
			(char)pdata->game.material);
	}
	else
	{
		CPASFilter filter(vecAbsOrigin);
		te->Explosion(filter, -1.0,
			&vecAbsOrigin,
			!(contents & MASK_WATER) ? g_sModelIndexFireball : g_sModelIndexWExplosion,
			m_DmgRadius * .03,
			25,
			TE_EXPLFLAG_NONE,
			m_DmgRadius,
			m_flDamage);
	}

	CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), BASEGRENADE_EXPLOSION_VOLUME, 3.0);

	if (nType == HI_EX_GRENADE)
	{
		CTakeDamageInfo tmpInfo(this, pThrower, GetBlastForce(), GetAbsOrigin(), m_flDamage, bitsDamageType, 0, &vecReported);
		RadiusDamage(tmpInfo, GetAbsOrigin(), m_DmgRadius, CLASS_NONE, NULL);
		UTIL_ScreenShake(GetAbsOrigin(), GetShakeAmplitude(), m_flDamage, 1.0, GetShakeRadius(), SHAKE_START);
		EmitSound("BaseGrenade.Explode");
	}
	else
	{
		EmitSound("AdvancedGrenade.SelfDestruct");
	}
	UTIL_DecalTrace(pTrace, "Scorch");
}

void CAdvancedGrenade::GlassGibs(int count)
{
	for (int i = 0; i < count; i++)
	{
		CGib *pGib = (CGib *)CreateEntityByName("gib");

		char* szGibModel = "models/props_junk/garbage_glassbottle001a_chunk04.mdl";
		switch (RandomInt(1, 4))
		{
		case 1:
			szGibModel = "models/props_junk/garbage_glassbottle001a_chunk01.mdl";
			break;

		case 2:
			szGibModel = "models/props_junk/garbage_glassbottle001a_chunk02.mdl";
			break;

		case 3:
			szGibModel = "models/props_junk/garbage_glassbottle001a_chunk03.mdl";
		}

		if (!szGibModel || !*szGibModel)
		{
			Warning("%s glass gib at %.0f, %.0f, %0.f missing modelname!\n",
				GetClassname(),
				GetAbsOrigin().x,
				GetAbsOrigin().y,
				GetAbsOrigin().z);
			return;
		}
		PrecacheModel(szGibModel);
		pGib->Spawn(szGibModel);

		pGib->SetLocalOrigin(
			Vector(GetAbsOrigin().x + random->RandomFloat(-3, 3),
				GetAbsOrigin().y + random->RandomFloat(-3, 3),
				GetAbsOrigin().z + random->RandomFloat(-3, 3)));

		// make the gib fly away from the attack vector
		Vector vecNewVelocity = g_vecAttackDir * -1;

		// mix in some noise
		vecNewVelocity.x += random->RandomFloat(-0.15, 0.15);
		vecNewVelocity.y += random->RandomFloat(-0.15, 0.15);
		vecNewVelocity.z += random->RandomFloat(-0.15, 0.15);
		vecNewVelocity *= 900;

		QAngle vecAngVelocity(random->RandomFloat(250, 400), random->RandomFloat(250, 400), 0);
		pGib->SetLocalAngularVelocity(vecAngVelocity);
		pGib->SetAbsVelocity(vecNewVelocity);

		pGib->SetMoveType(MOVETYPE_FLYGRAVITY);
		pGib->RemoveSolidFlags(FSOLID_NOT_SOLID);
		pGib->SetCollisionBounds(vec3_origin, vec3_origin);
		pGib->SetThink(NULL);

		pGib->LimitVelocity();
	}
}