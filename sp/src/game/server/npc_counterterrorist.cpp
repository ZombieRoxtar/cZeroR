//========== Copyright Bit Mage's Stuff, All rights probably reserved. ==========//
//
// Purpose: Your everyday, multi-purpose CounterTerrorist
//
//===============================================================================//
#include "cbase.h"
#include "npc_counterterrorist.h"
#include "hl2_player.h"
#include "basehlcombatweapon.h"
#include "eventqueue.h"
#include "npcevent.h"
#include "ai_squad.h"
#include "ai_pathfinder.h"
#include "ai_route.h"
#include "ai_interactions.h"
#include "sceneentity.h"
#include "tier0/icommandline.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
#define COMMAND_POINT_CLASSNAME "info_target_command_point"
#define ShouldAutosquad() (npc_counterterrorist_auto_player_squad.GetBool())
#define IsExcludedHead( type, iHead) false // see XBox codeline for an implementation
#define CITIZEN_SCORCH_RATE	6 // Color drain rate
#define CITIZEN_SCORCH_FLOOR 75 // Minimum color value

const int MAX_PLAYER_SQUAD = 4;

ConVar	sk_counterterrorist_health("sk_counterterrorist_health", "0");
ConVar	npc_counterterrorist_auto_player_squad("npc_counterterrorist_auto_player_squad", "0");
ConVar	npc_counterterrorist_auto_player_squad_allow_use("npc_counterterrorist_auto_player_squad_allow_use", "1");
ConVar	npc_counterterrorist_dont_precache_all("npc_counterterrorist_dont_precache_all", "1");

//-----------------------------------------------------------------------------
// Citizen expressions for the citizen expression types
//-----------------------------------------------------------------------------
#define STATES_WITH_EXPRESSIONS 3 // Idle, Alert, Combat
#define EXPRESSIONS_PER_STATE	1

char *szCounterTerroristExpressionTypes[CT_EXP_LAST_TYPE] =
{
	"Unassigned",
	"Scared",
	"Normal",
	"Angry"
};

struct counterterrorist_expression_list_t
{
	char *szExpressions[EXPRESSIONS_PER_STATE];
};

// Scared
counterterrorist_expression_list_t ScaredExpressions[STATES_WITH_EXPRESSIONS] =
{
	{ "scenes/Expressions/citizen_scared_idle_01.vcd" },
	{ "scenes/Expressions/citizen_scared_alert_01.vcd" },
	{ "scenes/Expressions/citizen_scared_combat_01.vcd" },
};
// Normal
counterterrorist_expression_list_t NormalExpressions[STATES_WITH_EXPRESSIONS] =
{
	{ "scenes/Expressions/citizen_normal_idle_01.vcd" },
	{ "scenes/Expressions/citizen_normal_alert_01.vcd" },
	{ "scenes/Expressions/citizen_normal_combat_01.vcd" },
};
// Angry
counterterrorist_expression_list_t AngryExpressions[STATES_WITH_EXPRESSIONS] =
{
	{ "scenes/Expressions/citizen_angry_idle_01.vcd" },
	{ "scenes/Expressions/citizen_angry_alert_01.vcd" },
	{ "scenes/Expressions/citizen_angry_combat_01.vcd" },
};

//---------------------------------------------------------
// Citizen models
//---------------------------------------------------------
static const char *g_ppszRandomHeads[] =
{
	"male_01.mdl",
	"male_02.mdl",
	"female_01.mdl",
	"male_03.mdl",
	"female_02.mdl",
	"male_04.mdl",
	"female_03.mdl",
	"male_05.mdl",
	"female_04.mdl",
	"male_06.mdl",
	"female_06.mdl",
	"male_07.mdl",
	"female_07.mdl",
	"male_08.mdl",
	"male_09.mdl",
};

static const char *g_ppszModelLocs[] =
{
	"Group01",
	"Group01",
	"Group02",
	"Group03",
};

//---------------------------------------------------------
LINK_ENTITY_TO_CLASS(npc_counterterrorist, CNPC_CounterTerrorist);

BEGIN_DATADESC(CNPC_CounterTerrorist)

DEFINE_FIELD(m_flNextFearSoundTime, FIELD_TIME),
DEFINE_FIELD(m_bShouldPatrol, FIELD_BOOLEAN),
DEFINE_FIELD(m_iszOriginalSquad, FIELD_STRING),
DEFINE_FIELD(m_flTimeJoinedPlayerSquad, FIELD_TIME),
DEFINE_FIELD(m_bWasInPlayerSquad, FIELD_BOOLEAN),
DEFINE_FIELD(m_flTimeLastCloseToPlayer, FIELD_TIME),
DEFINE_EMBEDDED(m_AutoSummonTimer),
DEFINE_FIELD(m_vAutoSummonAnchor, FIELD_POSITION_VECTOR),
DEFINE_KEYFIELD(m_Type, FIELD_INTEGER, "ct_type"),
DEFINE_KEYFIELD(m_ExpressionType, FIELD_INTEGER, "expressiontype"),
DEFINE_FIELD(m_iHead, FIELD_INTEGER),
DEFINE_FIELD(m_hSavedFollowGoalEnt, FIELD_EHANDLE),
DEFINE_KEYFIELD(m_bNotifyNavFailBlocked, FIELD_BOOLEAN, "notifynavfailblocked"),
DEFINE_KEYFIELD(m_bNeverLeavePlayerSquad, FIELD_BOOLEAN, "neverleaveplayersquad"),
DEFINE_KEYFIELD(m_iszDenyCommandConcept, FIELD_STRING, "denycommandconcept"),

DEFINE_OUTPUT(m_OnJoinedPlayerSquad, "OnJoinedPlayerSquad"),
DEFINE_OUTPUT(m_OnLeftPlayerSquad, "OnLeftPlayerSquad"),
DEFINE_OUTPUT(m_OnFollowOrder, "OnFollowOrder"),
DEFINE_OUTPUT(m_OnStationOrder, "OnStationOrder"),
DEFINE_OUTPUT(m_OnPlayerUse, "OnPlayerUse"),
DEFINE_OUTPUT(m_OnNavFailBlocked, "OnNavFailBlocked"),

DEFINE_INPUTFUNC(FIELD_VOID, "RemoveFromPlayerSquad", InputRemoveFromPlayerSquad),
DEFINE_INPUTFUNC(FIELD_VOID, "StartPatrolling", InputStartPatrolling),
DEFINE_INPUTFUNC(FIELD_VOID, "StopPatrolling", InputStopPatrolling),
DEFINE_INPUTFUNC(FIELD_VOID, "SetCommandable", InputSetCommandable),
DEFINE_INPUTFUNC(FIELD_VOID, "SpeakIdleResponse", InputSpeakIdleResponse),

DEFINE_USEFUNC(CommanderUse),
DEFINE_USEFUNC(SimpleUse),

END_DATADESC()

//-----------------------------------------------------------------------------
CSimpleSimTimer CNPC_CounterTerrorist::gm_PlayerSquadEvaluateTimer;

//-----------------------------------------------------------------------------
bool CNPC_CounterTerrorist::CreateBehaviors()
{
	BaseClass::CreateBehaviors();
	AddBehavior(&m_FuncTankBehavior);

	return true;
}

//-----------------------------------------------------------------------------
void CNPC_CounterTerrorist::Precache()
{
	SelectModel();
	SelectExpressionType();

	if (!npc_counterterrorist_dont_precache_all.GetBool())
		PrecacheAllOfType(m_Type);
	else
		PrecacheModel(STRING(GetModelName()));

	PrecacheScriptSound("NPC_Citizen.FootstepLeft");
	PrecacheScriptSound("NPC_Citizen.FootstepRight");
	PrecacheScriptSound("NPC_Citizen.Die");

	PrecacheInstancedScene("scenes/Expressions/CitizenIdle.vcd");
	PrecacheInstancedScene("scenes/Expressions/CitizenAlert_loop.vcd");
	PrecacheInstancedScene("scenes/Expressions/CitizenCombat_loop.vcd");

	for (int i = 0; i < STATES_WITH_EXPRESSIONS; i++)
	{
		for (int j = 0; j < ARRAYSIZE(ScaredExpressions[i].szExpressions); j++)
		{
			PrecacheInstancedScene(ScaredExpressions[i].szExpressions[j]);
		}
		for (int j = 0; j < ARRAYSIZE(NormalExpressions[i].szExpressions); j++)
		{
			PrecacheInstancedScene(NormalExpressions[i].szExpressions[j]);
		}
		for (int j = 0; j < ARRAYSIZE(AngryExpressions[i].szExpressions); j++)
		{
			PrecacheInstancedScene(AngryExpressions[i].szExpressions[j]);
		}
	}

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
void CNPC_CounterTerrorist::PrecacheAllOfType(CitizenType_t type)
{
	if (m_Type == CT_UNIQUE)
		return;

	int nHeads = ARRAYSIZE(g_ppszRandomHeads);
	int i;
	for (i = 0; i < nHeads; ++i)
	{
		PrecacheModel(CFmtStr("models/Humans/%s/%s", (const char *)(CFmtStr(g_ppszModelLocs[m_Type])), g_ppszRandomHeads[i]));
	}
}

//-----------------------------------------------------------------------------
void CNPC_CounterTerrorist::Spawn()
{
	BaseClass::Spawn();

#ifdef _XBOX
	// Always fade the corpse
	AddSpawnFlags(SF_NPC_FADE_CORPSE);
#endif // _XBOX

	if (ShouldAutosquad())
	{
		if (m_SquadName == GetPlayerSquadName())
		{
			CAI_Squad *pPlayerSquad = g_AI_SquadManager.FindSquad(GetPlayerSquadName());
			if (pPlayerSquad && pPlayerSquad->NumMembers() >= MAX_PLAYER_SQUAD)
				m_SquadName = NULL_STRING;
		}
		gm_PlayerSquadEvaluateTimer.Force();
	}

	m_bShouldPatrol = false;
	m_iHealth = sk_counterterrorist_health.GetFloat();

	// Are we on a train? Used in trainstation to have NPCs on trains.
	if (GetMoveParent() && FClassnameIs(GetMoveParent(), "func_tracktrain"))
	{
		CapabilitiesRemove(bits_CAP_MOVE_GROUND);
		SetMoveType(MOVETYPE_NONE);
		if (NameMatches("citizen_train_2"))
		{
			SetSequenceByName("d1_t01_TrainRide_Sit_Idle");
			SetIdealActivity(ACT_DO_NOT_DISTURB);
		}
		else
		{
			SetSequenceByName("d1_t01_TrainRide_Stand");
			SetIdealActivity(ACT_DO_NOT_DISTURB);
		}
	}

	m_iszIdleExpression = MAKE_STRING("scenes/expressions/citizenidle.vcd");
	m_iszAlertExpression = MAKE_STRING("scenes/expressions/citizenalert_loop.vcd");
	m_iszCombatExpression = MAKE_STRING("scenes/expressions/citizencombat_loop.vcd");

	m_iszOriginalSquad = m_SquadName;

	AddEFlags(EFL_NO_DISSOLVE | EFL_NO_MEGAPHYSCANNON_RAGDOLL | EFL_NO_PHYSCANNON_INTERACTION);

	NPCInit();

	SetUse(&CNPC_CounterTerrorist::CommanderUse);
	Assert(!ShouldAutosquad() || !IsInPlayerSquad());

	m_bWasInPlayerSquad = IsInPlayerSquad();

	// Use render bounds instead of human hull for guys sitting in chairs, etc.
	m_ActBusyBehavior.SetUseRenderBounds(HasSpawnFlags(SF_CITIZEN_USE_RENDER_BOUNDS));
}

//-----------------------------------------------------------------------------
void CNPC_CounterTerrorist::PostNPCInit()
{
	if (!gEntList.FindEntityByClassname(NULL, COMMAND_POINT_CLASSNAME))
	{
		CreateEntityByName(COMMAND_POINT_CLASSNAME);
	}

	if (IsInPlayerSquad())
	{
		if (m_pSquad->NumMembers() > MAX_PLAYER_SQUAD)
			DevMsg("Error: Spawning ct in player squad but exceeds squad limit of %d members\n", MAX_PLAYER_SQUAD);

		FixupPlayerSquad();
	}
	else
	{
		if ((m_spawnflags & SF_CITIZEN_FOLLOW) && AI_IsSinglePlayer())
		{
			m_FollowBehavior.SetFollowTarget(UTIL_GetLocalPlayer());
			m_FollowBehavior.SetParameters(AIF_SIMPLE);
		}
	}

	BaseClass::PostNPCInit();
}

//-----------------------------------------------------------------------------
struct HeadCandidate_t
{
	int iHead;
	int nHeads;

	static int __cdecl Sort(const HeadCandidate_t *pLeft, const HeadCandidate_t *pRight)
	{
		return (pLeft->nHeads - pRight->nHeads);
	}
};

void CNPC_CounterTerrorist::SelectModel()
{
	// If making reslists, precache everything!!!
	static bool madereslists = false;

	if (CommandLine()->CheckParm("-makereslists") && !madereslists)
	{
		madereslists = true;

		PrecacheAllOfType(CT_DOWNTRODDEN);
		PrecacheAllOfType(CT_REFUGEE);
		PrecacheAllOfType(CT_REBEL);
	}

	const char *pszModelName = NULL;

	if (m_Type == CT_DEFAULT)
	{
		struct CitizenTypeMapping
		{
			const char *pszMapTag;
			CitizenType_t type;
		};

		static CitizenTypeMapping CitizenTypeMappings[] =
		{
			{ "trainstation",	CT_DOWNTRODDEN },
			{ "canals",			CT_REFUGEE },
			{ "town",			CT_REFUGEE },
			{ "coast",			CT_REFUGEE },
			{ "prison",			CT_DOWNTRODDEN },
			{ "c17",			CT_REBEL },
			{ "citadel",		CT_DOWNTRODDEN },
		};

		char szMapName[256];
		Q_strncpy(szMapName, STRING(gpGlobals->mapname), sizeof(szMapName));
		Q_strlower(szMapName);

		for (int i = 0; i < ARRAYSIZE(CitizenTypeMappings); i++)
		{
			if (Q_stristr(szMapName, CitizenTypeMappings[i].pszMapTag))
			{
				m_Type = CitizenTypeMappings[i].type;
				break;
			}
		}

		if (m_Type == CT_DEFAULT)
			m_Type = CT_DOWNTRODDEN;
	}

	if (HasSpawnFlags(SF_CITIZEN_RANDOM_HEAD | SF_CITIZEN_RANDOM_HEAD_MALE | SF_CITIZEN_RANDOM_HEAD_FEMALE) || GetModelName() == NULL_STRING)
	{
		Assert(m_iHead == -1);
		char gender = (HasSpawnFlags(SF_CITIZEN_RANDOM_HEAD_MALE)) ? 'm' :
			(HasSpawnFlags(SF_CITIZEN_RANDOM_HEAD_FEMALE)) ? 'f' : 0;

		RemoveSpawnFlags(SF_CITIZEN_RANDOM_HEAD | SF_CITIZEN_RANDOM_HEAD_MALE | SF_CITIZEN_RANDOM_HEAD_FEMALE);
		if (HasSpawnFlags(SF_NPC_START_EFFICIENT))
		{
			SetModelName(AllocPooledString("models/humans/male_cheaple.mdl"));
			return;
		}
		else
		{
			// Count the heads
			int headCounts[ARRAYSIZE(g_ppszRandomHeads)] = { 0 };
			int i;

			for (i = 0; i < g_AI_Manager.NumAIs(); i++)
			{
				CNPC_CounterTerrorist *pCitizen = dynamic_cast<CNPC_CounterTerrorist *>(g_AI_Manager.AccessAIs()[i]);
				if (pCitizen && pCitizen != this && pCitizen->m_iHead >= 0 && pCitizen->m_iHead < ARRAYSIZE(g_ppszRandomHeads))
				{
					headCounts[pCitizen->m_iHead]++;
				}
			}

			// Find all candidates
			CUtlVectorFixed<HeadCandidate_t, ARRAYSIZE(g_ppszRandomHeads)> candidates;

			for (i = 0; i < ARRAYSIZE(g_ppszRandomHeads); i++)
			{
				if (!gender || g_ppszRandomHeads[i][0] == gender)
				{
					if (!IsExcludedHead(m_Type, i))
					{
						HeadCandidate_t candidate = { i, headCounts[i] };
						candidates.AddToTail(candidate);
					}
				}
			}

			Assert(candidates.Count());
			candidates.Sort(&HeadCandidate_t::Sort);

			int iSmallestCount = candidates[0].nHeads;
			int iLimit;

			for (iLimit = 0; iLimit < candidates.Count(); iLimit++)
			{
				if (candidates[iLimit].nHeads > iSmallestCount)
					break;
			}

			m_iHead = candidates[random->RandomInt(0, iLimit - 1)].iHead;
			pszModelName = g_ppszRandomHeads[m_iHead];
			SetModelName(NULL_STRING);
		}
	}

	Assert(pszModelName || GetModelName() != NULL_STRING);

	if (!pszModelName)
	{
		if (GetModelName() == NULL_STRING)
			return;
		pszModelName = strrchr(STRING(GetModelName()), '/');
		if (!pszModelName)
			pszModelName = STRING(GetModelName());
		else
		{
			pszModelName++;
			if (m_iHead == -1)
			{
				for (int i = 0; i < ARRAYSIZE(g_ppszRandomHeads); i++)
				{
					if (Q_stricmp(g_ppszRandomHeads[i], pszModelName) == 0)
					{
						m_iHead = i;
						break;
					}
				}
			}
		}
		if (!*pszModelName)
			return;
	}

	// Unique citizen models are left alone
	if (m_Type != CT_UNIQUE)
	{
		SetModelName(AllocPooledString(CFmtStr("models/Humans/%s/%s", (const char *)(CFmtStr(g_ppszModelLocs[m_Type])), pszModelName)));
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_CounterTerrorist::SelectExpressionType()
{
	// If we've got a mapmaker assigned type, leave it alone
	if (m_ExpressionType != CT_EXP_UNASSIGNED)
		return;

	switch (m_Type)
	{
	case CT_DOWNTRODDEN:
	case CT_REFUGEE:
	case CT_REBEL:
		m_ExpressionType = (CitizenExpressionTypes_t)RandomInt(CT_EXP_NORMAL, CT_EXP_ANGRY);
		break;

	case CT_DEFAULT:
	case CT_UNIQUE:
	default:
		break;
	}
}

//-----------------------------------------------------------------------------
void CNPC_CounterTerrorist::OnRestore()
{
	gm_PlayerSquadEvaluateTimer.Force();

	BaseClass::OnRestore();

	if (!gEntList.FindEntityByClassname(NULL, COMMAND_POINT_CLASSNAME))
	{
		CreateEntityByName(COMMAND_POINT_CLASSNAME);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Overridden to switch our behavior between passive and rebel. We
//			become combative after Gordon becomes a criminal.
//-----------------------------------------------------------------------------
Class_T	CNPC_CounterTerrorist::Classify()
{
	return CLASS_COUNTERTERRORIST;
}

//-----------------------------------------------------------------------------
bool CNPC_CounterTerrorist::ShouldAlwaysThink()
{
	return (BaseClass::ShouldAlwaysThink() || IsInPlayerSquad());
}

//-----------------------------------------------------------------------------
#define CITIZEN_FOLLOWER_DESERT_FUNCTANK_DIST 540 // 45.0f*12.0f
bool CNPC_CounterTerrorist::ShouldBehaviorSelectSchedule(CAI_BehaviorBase *pBehavior)
{
	if (pBehavior == &m_FollowBehavior)
	{
		// Suppress follow behavior if I have a func_tank and the func tank is near
		// what I'm supposed to be following.
		if (m_FuncTankBehavior.CanSelectSchedule())
		{
			// Is the tank close to the follow target?
			Vector vecTank = m_FuncTankBehavior.GetFuncTank()->WorldSpaceCenter();
			Vector vecFollowGoal = m_FollowBehavior.GetFollowGoalInfo().position;

			float flTankDistSqr = (vecTank - vecFollowGoal).LengthSqr();
			float flAllowDist = m_FollowBehavior.GetFollowGoalInfo().followPointTolerance * 2.0f;
			float flAllowDistSqr = flAllowDist * flAllowDist;
			if (flTankDistSqr < flAllowDistSqr)
			{
				// Deny follow behavior so the tank can go.
				return false;
			}
		}
	}
	else if (IsInPlayerSquad() && pBehavior == &m_FuncTankBehavior && m_FuncTankBehavior.IsMounted())
	{
		if (m_FollowBehavior.GetFollowTarget())
		{
			Vector vecFollowGoal = m_FollowBehavior.GetFollowTarget()->GetAbsOrigin();
			if (vecFollowGoal.DistToSqr(GetAbsOrigin()) > Square(CITIZEN_FOLLOWER_DESERT_FUNCTANK_DIST))
			{
				return false;
			}
		}
	}

	return BaseClass::ShouldBehaviorSelectSchedule(pBehavior);
}

void CNPC_CounterTerrorist::OnChangeRunningBehavior(CAI_BehaviorBase *pOldBehavior, CAI_BehaviorBase *pNewBehavior)
{
	if (pNewBehavior == &m_FuncTankBehavior)
	{
		m_bReadinessCapable = false;
	}
	else if (pOldBehavior == &m_FuncTankBehavior)
	{
		m_bReadinessCapable = IsReadinessCapable();
	}

	BaseClass::OnChangeRunningBehavior(pOldBehavior, pNewBehavior);
}

//-----------------------------------------------------------------------------
void CNPC_CounterTerrorist::GatherConditions()
{
	BaseClass::GatherConditions();

	if (IsInPlayerSquad())
	{
		// Leave the player squad if someone has made me neutral to player.
		if (IRelationType(UTIL_GetLocalPlayer()) == D_NU)
		{
			RemoveFromPlayerSquad();
		}
	}

	if (!SpokeConcept(TLK_JOINPLAYER) && IsRunningScriptedSceneWithSpeech(this, true))
	{
		SetSpokeConcept(TLK_JOINPLAYER, NULL);
		for (int i = 0; i < g_AI_Manager.NumAIs(); i++)
		{
			CAI_BaseNPC *pNpc = g_AI_Manager.AccessAIs()[i];
			if (pNpc != this && pNpc->GetClassname() == GetClassname() && pNpc->GetAbsOrigin().DistToSqr(GetAbsOrigin()) < Square(15 * 12) && FVisible(pNpc))
			{
				(assert_cast<CNPC_CounterTerrorist *>(pNpc))->SetSpokeConcept(TLK_JOINPLAYER, NULL);
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CNPC_CounterTerrorist::PredictPlayerPush()
{
	if (!AI_IsSinglePlayer())
		return;

	BaseClass::PredictPlayerPush();
}

//-----------------------------------------------------------------------------
void CNPC_CounterTerrorist::PrescheduleThink()
{
	BaseClass::PrescheduleThink();
	UpdatePlayerSquad();
	UpdateFollowCommandPoint();
}

//-----------------------------------------------------------------------------
// Purpose: Allows for modification of the interrupt mask for the current schedule.
//			In the most cases the base implementation should be called first.
//-----------------------------------------------------------------------------
void CNPC_CounterTerrorist::BuildScheduleTestBits()
{
	BaseClass::BuildScheduleTestBits();

	if (!IsCurSchedule(SCHED_NEW_WEAPON))
	{
		SetCustomInterruptCondition(COND_RECEIVED_ORDERS);
	}

	if (GetCurSchedule()->HasInterrupt(COND_IDLE_INTERRUPT))
	{
		SetCustomInterruptCondition(COND_BETTER_WEAPON_AVAILABLE);
	}
}

//-----------------------------------------------------------------------------
int CNPC_CounterTerrorist::SelectFailSchedule(int failedSchedule, int failedTask, AI_TaskFailureCode_t taskFailCode)
{
	switch (failedSchedule)
	{
	case SCHED_NEW_WEAPON:
		// If failed trying to pick up a weapon, try again in one second. This is because other AI code
		// has put this off for 10 seconds under the assumption that the citizen would be able to 
		// pick up the weapon that they found. 
		m_flNextWeaponSearchTime = gpGlobals->curtime + 1.0f;
		break;

	case SCHED_ESTABLISH_LINE_OF_FIRE_FALLBACK:
	case SCHED_MOVE_TO_WEAPON_RANGE:
		if (!IsMortar(GetEnemy()))
		{
			if (GetActiveWeapon() && (GetActiveWeapon()->CapabilitiesGet() & bits_CAP_WEAPON_RANGE_ATTACK1) && random->RandomInt(0, 1) && HasCondition(COND_SEE_ENEMY) && !HasCondition(COND_NO_PRIMARY_AMMO))
				return TranslateSchedule(SCHED_RANGE_ATTACK1);

			return SCHED_STANDOFF;
		}
		break;
	}

	return BaseClass::SelectFailSchedule(failedSchedule, failedTask, taskFailCode);
}

//-----------------------------------------------------------------------------
int CNPC_CounterTerrorist::SelectSchedulePriorityAction()
{
	int schedule = BaseClass::SelectSchedulePriorityAction();
	if (schedule != SCHED_NONE)
		return schedule;

	schedule = SelectScheduleRetrieveItem();
	if (schedule != SCHED_NONE)
		return schedule;

	return SCHED_NONE;
}

//-----------------------------------------------------------------------------
int CNPC_CounterTerrorist::SelectScheduleRetrieveItem()
{
	if (HasCondition(COND_BETTER_WEAPON_AVAILABLE))
	{
		CBaseHLCombatWeapon *pWeapon = dynamic_cast<CBaseHLCombatWeapon *>(Weapon_FindUsable(WEAPON_SEARCH_DELTA));
		if (pWeapon)
		{
			m_flNextWeaponSearchTime = gpGlobals->curtime + 10.0;
			// Now lock the weapon for several seconds while we go to pick it up.
			pWeapon->Lock(10.0, this);
			SetTarget(pWeapon);
			return SCHED_NEW_WEAPON;
		}
	}
	return SCHED_NONE;
}

//-----------------------------------------------------------------------------
int CNPC_CounterTerrorist::SelectScheduleNonCombat()
{
	if (m_bShouldPatrol)
		return SCHED_CITIZEN_PATROL;

	return SCHED_NONE;
}

//-----------------------------------------------------------------------------
int CNPC_CounterTerrorist::TranslateSchedule(int scheduleType)
{
	CBasePlayer *pLocalPlayer = AI_GetSinglePlayer();

	switch (scheduleType)
	{
	case SCHED_IDLE_STAND:
	case SCHED_ALERT_STAND:
		if (m_NPCState != NPC_STATE_COMBAT && pLocalPlayer && !pLocalPlayer->IsAlive() && CanJoinPlayerSquad())
		{
			// Player is dead! 
			float flDist;
			flDist = (pLocalPlayer->GetAbsOrigin() - GetAbsOrigin()).Length();

			if (flDist < 50 * 12)
			{
				AddSpawnFlags(SF_CITIZEN_NOT_COMMANDABLE);
				return SCHED_CITIZEN_MOURN_PLAYER;
			}
		}
		break;

	case SCHED_ESTABLISH_LINE_OF_FIRE:
	case SCHED_MOVE_TO_WEAPON_RANGE:
		if (!IsMortar(GetEnemy()) && HaveCommandGoal())
		{
			if (GetActiveWeapon() && (GetActiveWeapon()->CapabilitiesGet() & bits_CAP_WEAPON_RANGE_ATTACK1) && random->RandomInt(0, 1) && HasCondition(COND_SEE_ENEMY) && !HasCondition(COND_NO_PRIMARY_AMMO))
				return TranslateSchedule(SCHED_RANGE_ATTACK1);

			return SCHED_STANDOFF;
		}
		break;

	case SCHED_CHASE_ENEMY:
		if (!IsMortar(GetEnemy()) && HaveCommandGoal())
		{
			return SCHED_STANDOFF;
		}
		break;
	}

	return BaseClass::TranslateSchedule(scheduleType);
}

//-----------------------------------------------------------------------------
bool CNPC_CounterTerrorist::ShouldAcceptGoal(CAI_BehaviorBase *pBehavior, CAI_GoalEntity *pGoal)
{
	if (BaseClass::ShouldAcceptGoal(pBehavior, pGoal))
	{
		CAI_FollowBehavior *pFollowBehavior = dynamic_cast<CAI_FollowBehavior *>(pBehavior);
		if (pFollowBehavior)
		{
			if (IsInPlayerSquad())
			{
				m_hSavedFollowGoalEnt = (CAI_FollowGoal *)pGoal;
				return false;
			}
		}
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
void CNPC_CounterTerrorist::OnClearGoal(CAI_BehaviorBase *pBehavior, CAI_GoalEntity *pGoal)
{
	if (m_hSavedFollowGoalEnt == pGoal)
		m_hSavedFollowGoalEnt = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : code - 
//-----------------------------------------------------------------------------
void CNPC_CounterTerrorist::TaskFail(AI_TaskFailureCode_t code)
{
	if (code == FAIL_NO_ROUTE_BLOCKED && m_bNotifyNavFailBlocked)
	{
		m_OnNavFailBlocked.FireOutput(this, this);
	}
	BaseClass::TaskFail(code);
}

//-----------------------------------------------------------------------------
// Purpose: Override base class activiites
//-----------------------------------------------------------------------------
Activity CNPC_CounterTerrorist::NPC_TranslateActivity(Activity activity)
{
	if (activity == ACT_MELEE_ATTACK1)
	{
		return ACT_MELEE_ATTACK_SWING;
	}

	// !!!HACKHACK - Citizens don't have the required animations for shotguns, 
	// so trick them into using the rifle counterparts for now (sjb)
	if (activity == ACT_RUN_AIM_SHOTGUN)
		return ACT_RUN_AIM_RIFLE;
	if (activity == ACT_WALK_AIM_SHOTGUN)
		return ACT_WALK_AIM_RIFLE;
	if (activity == ACT_IDLE_ANGRY_SHOTGUN)
		return ACT_IDLE_ANGRY_SMG1;
	if (activity == ACT_RANGE_ATTACK_SHOTGUN_LOW)
		return ACT_RANGE_ATTACK_SMG1_LOW;

	return BaseClass::NPC_TranslateActivity(activity);
}

//------------------------------------------------------------------------------
void CNPC_CounterTerrorist::HandleAnimEvent(animevent_t *pEvent)
{
	switch (pEvent->event)
	{
	case NPC_EVENT_LEFTFOOT:
	{
		EmitSound("NPC_Citizen.FootstepLeft", pEvent->eventtime);
	}
	break;

	case NPC_EVENT_RIGHTFOOT:
	{
		EmitSound("NPC_Citizen.FootstepRight", pEvent->eventtime);
	}
	break;

	default:
		BaseClass::HandleAnimEvent(pEvent);
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CNPC_CounterTerrorist::IgnorePlayerPushing(void)
{
	// If the NPC's on a func_tank that the player cannot man, ignore player pushing
	if (m_FuncTankBehavior.IsMounted())
	{
		CFuncTank *pTank = m_FuncTankBehavior.GetFuncTank();
		if (pTank && !pTank->IsControllable())
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Return a random expression for the specified state to play over 
//			the state's expression loop.
//-----------------------------------------------------------------------------
const char *CNPC_CounterTerrorist::SelectRandomExpressionForState(NPC_STATE state)
{
	// Hacky remap of NPC states to expression states that we care about
	int iExpressionState = 0;
	switch (state)
	{
	case NPC_STATE_IDLE:
		iExpressionState = 0;
		break;

	case NPC_STATE_ALERT:
		iExpressionState = 1;
		break;

	case NPC_STATE_COMBAT:
		iExpressionState = 2;
		break;

	default:
		// An NPC state we don't have expressions for
		return NULL;
	}

	// Now pick the right one for our expression type
	switch (m_ExpressionType)
	{
	case CT_EXP_SCARED:
	{
		int iRandom = RandomInt(0, ARRAYSIZE(ScaredExpressions[iExpressionState].szExpressions) - 1);
		return ScaredExpressions[iExpressionState].szExpressions[iRandom];
	}

	case CT_EXP_NORMAL:
	{
		int iRandom = RandomInt(0, ARRAYSIZE(NormalExpressions[iExpressionState].szExpressions) - 1);
		return NormalExpressions[iExpressionState].szExpressions[iRandom];
	}

	case CT_EXP_ANGRY:
	{
		int iRandom = RandomInt(0, ARRAYSIZE(AngryExpressions[iExpressionState].szExpressions) - 1);
		return AngryExpressions[iExpressionState].szExpressions[iRandom];
	}

	default:
		break;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
void CNPC_CounterTerrorist::SimpleUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	// Under these conditions, citizens will refuse to go with the player.
	// Robin: NPCs should always respond to +USE even if someone else has the semaphore.
	m_bDontUseSemaphore = true;

	// First, try to speak the +USE concept
	if (!SelectPlayerUseSpeech())
	{
		if (HasSpawnFlags(SF_CITIZEN_NOT_COMMANDABLE) || IRelationType(pActivator) == D_NU)
		{
			// If I'm denying commander mode because a level designer has made that decision,
			// then fire this output in case they've hooked it to an event.
			m_OnDenyCommanderUse.FireOutput(this, this);
		}
	}

	m_bDontUseSemaphore = false;
}

//-----------------------------------------------------------------------------
bool CNPC_CounterTerrorist::OnBeginMoveAndShoot()
{
	if (BaseClass::OnBeginMoveAndShoot())
	{
		if (m_iMySquadSlot == SQUAD_SLOT_ATTACK1 || m_iMySquadSlot == SQUAD_SLOT_ATTACK2)
			return true; // already have the slot I need

		if (m_iMySquadSlot == SQUAD_SLOT_NONE && OccupyStrategySlotRange(SQUAD_SLOT_ATTACK1, SQUAD_SLOT_ATTACK2))
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
void CNPC_CounterTerrorist::OnEndMoveAndShoot()
{
	VacateStrategySlot();
}

//-----------------------------------------------------------------------------
void CNPC_CounterTerrorist::OnChangeActiveWeapon(CBaseCombatWeapon *pOldWeapon, CBaseCombatWeapon *pNewWeapon)
{
	if (pNewWeapon)
	{
		GetShotRegulator()->SetParameters(pNewWeapon->GetMinBurst(), pNewWeapon->GetMaxBurst(), pNewWeapon->GetMinRestTime(), pNewWeapon->GetMaxRestTime());
	}
	BaseClass::OnChangeActiveWeapon(pOldWeapon, pNewWeapon);
}

//-----------------------------------------------------------------------------
bool CNPC_CounterTerrorist::ShouldLookForBetterWeapon()
{
	return BaseClass::ShouldLookForBetterWeapon() || !GetActiveWeapon();
}

//-----------------------------------------------------------------------------
int CNPC_CounterTerrorist::OnTakeDamage_Alive(const CTakeDamageInfo &info)
{
	if ((info.GetDamageType() & DMG_BURN) && (info.GetDamageType() & DMG_DIRECT))
	{
		Scorch(CITIZEN_SCORCH_RATE, CITIZEN_SCORCH_FLOOR);
	}
	return BaseClass::OnTakeDamage_Alive(info);
}

//-----------------------------------------------------------------------------
bool CNPC_CounterTerrorist::IsCommandable()
{
	return (!HasSpawnFlags(SF_CITIZEN_NOT_COMMANDABLE) && IsInPlayerSquad());
}

//-----------------------------------------------------------------------------
bool CNPC_CounterTerrorist::CanJoinPlayerSquad()
{
	if (!AI_IsSinglePlayer())
		return false;

	if (m_NPCState == NPC_STATE_SCRIPT || m_NPCState == NPC_STATE_PRONE)
		return false;

	if (HasSpawnFlags(SF_CITIZEN_NOT_COMMANDABLE))
		return false;

	if (IsInAScript())
		return false;

	// Don't bother people who don't want to be bothered
	if (!CanBeUsedAsAFriend())
		return false;

	if (IRelationType(UTIL_GetLocalPlayer()) != D_LI)
		return false;

	return true;
}

//-----------------------------------------------------------------------------
bool CNPC_CounterTerrorist::WasInPlayerSquad()
{
	return m_bWasInPlayerSquad;
}

//-----------------------------------------------------------------------------
bool CNPC_CounterTerrorist::HaveCommandGoal() const
{
	if (GetCommandGoal() != vec3_invalid)
		return true;
	return false;
}

//-----------------------------------------------------------------------------
bool CNPC_CounterTerrorist::IsCommandMoving()
{
	if (AI_IsSinglePlayer() && IsInPlayerSquad())
	{
		if (m_FollowBehavior.GetFollowTarget() == UTIL_GetLocalPlayer() ||
			IsFollowingCommandPoint())
		{
			return (m_FollowBehavior.IsMovingToFollowTarget());
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
// Is this entity something that the citizen should interact with (return true)
// or something that he should try to get close to (return false)
//-----------------------------------------------------------------------------
bool CNPC_CounterTerrorist::IsValidCommandTarget(CBaseEntity *pTarget)
{
	return false;
}

//-----------------------------------------------------------------------------
bool CNPC_CounterTerrorist::SpeakCommandResponse(AIConcept_t concept, const char *modifiers)
{
	return SpeakIfAllowed(concept,
		CFmtStr("numselected:%d,"
			"useradio:%d%s",
			(GetSquad()) ? GetSquad()->NumMembers() : 1,
			ShouldSpeakRadio(AI_GetSinglePlayer()),
			(modifiers) ? CFmtStr(",%s", modifiers).operator const char *() : ""));
}

//-----------------------------------------------------------------------------
// Purpose: return TRUE if the commander mode should try to give this order
//			to more people. return FALSE otherwise. For instance, we don't
//			try to send all 3 selectedcitizens to pick up the same gun.
//-----------------------------------------------------------------------------
bool CNPC_CounterTerrorist::TargetOrder(CBaseEntity *pTarget, CAI_BaseNPC **Allies, int numAllies)
{
	if (pTarget->IsPlayer())
	{
		// I'm the target! Toggle follow!
		if (m_FollowBehavior.GetFollowTarget() != pTarget)
		{
			ClearFollowTarget();
			SetCommandGoal(vec3_invalid);

			// Turn follow on!
			m_AssaultBehavior.Disable();
			m_FollowBehavior.SetFollowTarget(pTarget);
			m_FollowBehavior.SetParameters(AIF_SIMPLE);
			SpeakCommandResponse(TLK_STARTFOLLOW);

			m_OnFollowOrder.FireOutput(this, this);
		}
		else if (m_FollowBehavior.GetFollowTarget() == pTarget)
		{
			// Stop following.
			m_FollowBehavior.SetFollowTarget(NULL);
			SpeakCommandResponse(TLK_STOPFOLLOW);
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Turn off following before processing a move order.
//-----------------------------------------------------------------------------
void CNPC_CounterTerrorist::MoveOrder(const Vector &vecDest, CAI_BaseNPC **Allies, int numAllies)
{
	if (!AI_IsSinglePlayer())
		return;

	if (hl2_episodic.GetBool() && m_iszDenyCommandConcept != NULL_STRING)
	{
		SpeakCommandResponse(STRING(m_iszDenyCommandConcept));
		return;
	}

	CHL2_Player *pPlayer = (CHL2_Player *)UTIL_GetLocalPlayer();

	if (m_StandoffBehavior.IsRunning())
	{
		m_StandoffBehavior.SetStandoffGoalPosition(vecDest);
	}

	// If in assault, cancel and move.
	if (m_AssaultBehavior.HasHitRallyPoint() && !m_AssaultBehavior.HasHitAssaultPoint())
	{
		m_AssaultBehavior.Disable();
		ClearSchedule("Moving from rally point to assault point");
	}

	bool spoke = false;

	CAI_BaseNPC *pClosest = NULL;
	float closestDistSq = FLT_MAX;

	for (int i = 0; i < numAllies; i++)
	{
		if (Allies[i]->IsInPlayerSquad())
		{
			Assert(Allies[i]->IsCommandable());
			float distSq = (pPlayer->GetAbsOrigin() - Allies[i]->GetAbsOrigin()).LengthSqr();
			if (distSq < closestDistSq)
			{
				pClosest = Allies[i];
				closestDistSq = distSq;
			}
		}
	}

	if (m_FollowBehavior.GetFollowTarget() && !IsFollowingCommandPoint())
	{
		ClearFollowTarget();
	}

	if (!spoke && pClosest == this)
	{
		float destDistToPlayer = (vecDest - pPlayer->GetAbsOrigin()).Length();
		float destDistToClosest = (vecDest - GetAbsOrigin()).Length();
		CFmtStr modifiers("commandpoint_dist_to_player:%.0f,"
			"commandpoint_dist_to_npc:%.0f",
			destDistToPlayer,
			destDistToClosest);
		SpeakCommandResponse(TLK_COMMANDED, modifiers);
	}

	m_OnStationOrder.FireOutput(this, this);

	BaseClass::MoveOrder(vecDest, Allies, numAllies);
}

//-----------------------------------------------------------------------------
void CNPC_CounterTerrorist::OnMoveOrder()
{
	SetReadinessLevel(AIRL_STIMULATED, false, false);
	BaseClass::OnMoveOrder();
}

//-----------------------------------------------------------------------------
void CNPC_CounterTerrorist::CommanderUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	m_OnPlayerUse.FireOutput(pActivator, pCaller);

	// Under these conditions, citizens will refuse to go with the player.
	// Robin: NPCs should always respond to +USE even if someone else has the semaphore.
	if (!AI_IsSinglePlayer() || !CanJoinPlayerSquad())
	{
		SimpleUse(pActivator, pCaller, useType, value);
		return;
	}

	if (pActivator == UTIL_GetLocalPlayer())
	{
		// Don't say hi after you've been addressed by the player
		SetSpokeConcept(TLK_HELLO, NULL);

		if (GetCurSchedule() && ConditionInterruptsCurSchedule(COND_IDLE_INTERRUPT))
		{
			if (SpeakIfAllowed(TLK_QUESTION, NULL, true))
			{
				if (random->RandomInt(1, 4) < 4)
				{
					CBaseEntity *pRespondant = FindSpeechTarget(AIST_NPCS);
					if (pRespondant)
					{
						g_EventQueue.AddEvent(pRespondant, "SpeakIdleResponse", (GetTimeSpeechComplete() - gpGlobals->curtime) + .2, this, this);
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
bool CNPC_CounterTerrorist::ShouldSpeakRadio(CBaseEntity *pListener)
{
	if (!pListener)
		return false;

	const float	radioRange = 147456; // 384^2
	Vector		vecDiff;

	vecDiff = WorldSpaceCenter() - pListener->WorldSpaceCenter();

	if (vecDiff.LengthSqr() > radioRange)
	{
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
void CNPC_CounterTerrorist::OnMoveToCommandGoalFailed()
{
	// Clear the goal.
	SetCommandGoal(vec3_invalid);

	// Announce failure.
	SpeakCommandResponse(TLK_COMMAND_FAILED);
}

//-----------------------------------------------------------------------------
void CNPC_CounterTerrorist::AddToPlayerSquad()
{
	Assert(!IsInPlayerSquad());

	AddToSquad(AllocPooledString(PLAYER_SQUADNAME));
	m_hSavedFollowGoalEnt = m_FollowBehavior.GetFollowGoal();
	m_FollowBehavior.SetFollowGoalDirect(NULL);

	FixupPlayerSquad();

	SetCondition(COND_PLAYER_ADDED_TO_SQUAD);
}

//-----------------------------------------------------------------------------
void CNPC_CounterTerrorist::RemoveFromPlayerSquad()
{
	Assert(IsInPlayerSquad());

	ClearFollowTarget();
	ClearCommandGoal();
	if (m_iszOriginalSquad != NULL_STRING && strcmp(STRING(m_iszOriginalSquad), PLAYER_SQUADNAME) != 0)
		AddToSquad(m_iszOriginalSquad);
	else
		RemoveFromSquad();

	if (m_hSavedFollowGoalEnt)
		m_FollowBehavior.SetFollowGoal(m_hSavedFollowGoalEnt);

	SetCondition(COND_PLAYER_REMOVED_FROM_SQUAD);

	// Don't evaluate the player squad for 2 seconds. 
	gm_PlayerSquadEvaluateTimer.Set(2.0);
}

//-----------------------------------------------------------------------------
void CNPC_CounterTerrorist::TogglePlayerSquadState()
{
	if (!AI_IsSinglePlayer())
		return;

	if (!IsInPlayerSquad())
	{
		AddToPlayerSquad();

		if (HaveCommandGoal())
		{
			SpeakCommandResponse(TLK_COMMANDED);
		}
		else if (m_FollowBehavior.GetFollowTarget() == UTIL_GetLocalPlayer())
		{
			SpeakCommandResponse(TLK_STARTFOLLOW);
		}
	}
	else
	{
		SpeakCommandResponse(TLK_STOPFOLLOW);
		RemoveFromPlayerSquad();
	}
}

//-----------------------------------------------------------------------------
struct SquadCandidate_t
{
	CNPC_CounterTerrorist *pCitizen;
	bool		  bIsInSquad;
	float		  distSq;
	int			  iSquadIndex;
};

void CNPC_CounterTerrorist::UpdatePlayerSquad()
{
	if (!AI_IsSinglePlayer())
		return;

	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	if ((pPlayer->GetAbsOrigin().AsVector2D() - GetAbsOrigin().AsVector2D()).LengthSqr() < Square(20 * 12))
		m_flTimeLastCloseToPlayer = gpGlobals->curtime;

	if (!gm_PlayerSquadEvaluateTimer.Expired())
		return;

	gm_PlayerSquadEvaluateTimer.Set(2.0);

	// Remove stragglers
	CAI_Squad *pPlayerSquad = g_AI_SquadManager.FindSquad(MAKE_STRING(PLAYER_SQUADNAME));
	if (pPlayerSquad)
	{
		CUtlVectorFixed<CNPC_CounterTerrorist *, MAX_PLAYER_SQUAD> squadMembersToRemove;
		AISquadIter_t iter;

		for (CAI_BaseNPC *pPlayerSquadMember = pPlayerSquad->GetFirstMember(&iter); pPlayerSquadMember; pPlayerSquadMember = pPlayerSquad->GetNextMember(&iter))
		{
			if (pPlayerSquadMember->GetClassname() != GetClassname())
				continue;

			CNPC_CounterTerrorist *pCitizen = assert_cast<CNPC_CounterTerrorist *>(pPlayerSquadMember);

			if (!pCitizen->m_bNeverLeavePlayerSquad &&
				pCitizen->m_FollowBehavior.GetFollowTarget() &&
				!pCitizen->m_FollowBehavior.FollowTargetVisible() &&
				pCitizen->m_FollowBehavior.GetNumFailedFollowAttempts() > 0 &&
				gpGlobals->curtime - pCitizen->m_FollowBehavior.GetTimeFailFollowStarted() > 20 &&
				(fabsf((pCitizen->m_FollowBehavior.GetFollowTarget()->GetAbsOrigin().z - pCitizen->GetAbsOrigin().z)) > 196 ||
					(pCitizen->m_FollowBehavior.GetFollowTarget()->GetAbsOrigin().AsVector2D() - pCitizen->GetAbsOrigin().AsVector2D()).LengthSqr() > Square(50 * 12)))
			{
				squadMembersToRemove.AddToTail(pCitizen);
			}
		}

		for (int i = 0; i < squadMembersToRemove.Count(); i++)
		{
			squadMembersToRemove[i]->RemoveFromPlayerSquad();
		}
	}

	// Autosquadding
	const float JOIN_PLAYER_XY_TOLERANCE_SQ = Square(36 * 12);
	const float UNCONDITIONAL_JOIN_PLAYER_XY_TOLERANCE_SQ = Square(12 * 12);
	const float UNCONDITIONAL_JOIN_PLAYER_Z_TOLERANCE = 5 * 12;
	const float SECOND_TIER_JOIN_DIST_SQ = Square(48 * 12);
	if (pPlayer && ShouldAutosquad() && !(pPlayer->GetFlags() & FL_NOTARGET) && pPlayer->IsAlive())
	{
		CAI_BaseNPC **ppAIs = g_AI_Manager.AccessAIs();
		CUtlVector<SquadCandidate_t> candidates;
		const Vector &vPlayerPos = pPlayer->GetAbsOrigin();
		bool bFoundNewGuy = false;
		int i;

		for (i = 0; i < g_AI_Manager.NumAIs(); i++)
		{
			if (ppAIs[i]->GetState() == NPC_STATE_DEAD)
				continue;

			if (ppAIs[i]->GetClassname() != GetClassname())
				continue;

			CNPC_CounterTerrorist *pCitizen = assert_cast<CNPC_CounterTerrorist *>(ppAIs[i]);
			int iNew;

			if (pCitizen->IsInPlayerSquad())
			{
				iNew = candidates.AddToTail();
				candidates[iNew].pCitizen = pCitizen;
				candidates[iNew].bIsInSquad = true;
				candidates[iNew].distSq = 0;
				candidates[iNew].iSquadIndex = pCitizen->GetSquad()->GetSquadIndex(pCitizen);
			}
			else
			{
				float distSq = (vPlayerPos.AsVector2D() - pCitizen->GetAbsOrigin().AsVector2D()).LengthSqr();
				if (distSq > JOIN_PLAYER_XY_TOLERANCE_SQ &&
					(pCitizen->m_flTimeJoinedPlayerSquad == 0 || gpGlobals->curtime - pCitizen->m_flTimeJoinedPlayerSquad > 60.0) &&
					(pCitizen->m_flTimeLastCloseToPlayer == 0 || gpGlobals->curtime - pCitizen->m_flTimeLastCloseToPlayer > 15.0))
					continue;

				if (!pCitizen->CanJoinPlayerSquad())
					continue;

				bool bShouldAdd = false;

				if (pCitizen->HasCondition(COND_SEE_PLAYER))
					bShouldAdd = true;
				else
				{
					bool bPlayerVisible = pCitizen->FVisible(pPlayer);
					if (bPlayerVisible)
					{
						if (pCitizen->HasCondition(COND_HEAR_PLAYER))
							bShouldAdd = true;
						else if (distSq < UNCONDITIONAL_JOIN_PLAYER_XY_TOLERANCE_SQ && fabsf(vPlayerPos.z - pCitizen->GetAbsOrigin().z) < UNCONDITIONAL_JOIN_PLAYER_Z_TOLERANCE)
							bShouldAdd = true;
					}
				}

				if (bShouldAdd)
				{
					// @TODO (toml 05-25-04): probably everyone in a squad should be a candidate if one of them sees the player
					AI_Waypoint_t *pPathToPlayer = pCitizen->GetPathfinder()->BuildRoute(pCitizen->GetAbsOrigin(), vPlayerPos, pPlayer, 5 * 12, NAV_NONE, true);
					GetPathfinder()->UnlockRouteNodes(pPathToPlayer);

					if (!pPathToPlayer)
						continue;

					CAI_Path tempPath;
					tempPath.SetWaypoints(pPathToPlayer); // path object will delete waypoints

					iNew = candidates.AddToTail();
					candidates[iNew].pCitizen = pCitizen;
					candidates[iNew].bIsInSquad = false;
					candidates[iNew].distSq = distSq;
					candidates[iNew].iSquadIndex = -1;

					bFoundNewGuy = true;
				}
			}
		}

		if (bFoundNewGuy)
		{
			// Look for second order guys
			int initialCount = candidates.Count();
			for (i = 0; i < initialCount; i++)
				candidates[i].pCitizen->AddSpawnFlags(SF_CITIZEN_NOT_COMMANDABLE); // Prevents double-add
			for (i = 0; i < initialCount; i++)
			{
				if (candidates[i].iSquadIndex == -1)
				{
					for (int j = 0; j < g_AI_Manager.NumAIs(); j++)
					{
						if (ppAIs[j]->GetState() == NPC_STATE_DEAD)
							continue;

						if (ppAIs[j]->GetClassname() != GetClassname())
							continue;

						if (ppAIs[j]->HasSpawnFlags(SF_CITIZEN_NOT_COMMANDABLE))
							continue;

						CNPC_CounterTerrorist *pCitizen = assert_cast<CNPC_CounterTerrorist *>(ppAIs[j]);

						float distSq = (vPlayerPos - pCitizen->GetAbsOrigin()).Length2DSqr();
						if (distSq > JOIN_PLAYER_XY_TOLERANCE_SQ)
							continue;

						distSq = (candidates[i].pCitizen->GetAbsOrigin() - pCitizen->GetAbsOrigin()).Length2DSqr();
						if (distSq > SECOND_TIER_JOIN_DIST_SQ)
							continue;

						if (!pCitizen->CanJoinPlayerSquad())
							continue;

						if (!pCitizen->FVisible(pPlayer))
							continue;

						int iNew = candidates.AddToTail();
						candidates[iNew].pCitizen = pCitizen;
						candidates[iNew].bIsInSquad = false;
						candidates[iNew].distSq = distSq;
						candidates[iNew].iSquadIndex = -1;
						pCitizen->AddSpawnFlags(SF_CITIZEN_NOT_COMMANDABLE); // Prevents double-add
					}
				}
			}
			for (i = 0; i < candidates.Count(); i++)
				candidates[i].pCitizen->RemoveSpawnFlags(SF_CITIZEN_NOT_COMMANDABLE);

			if (candidates.Count() > MAX_PLAYER_SQUAD)
			{
				candidates.Sort(PlayerSquadCandidateSortFunc);

				for (i = MAX_PLAYER_SQUAD; i < candidates.Count(); i++)
				{
					if (candidates[i].pCitizen->IsInPlayerSquad())
					{
						candidates[i].pCitizen->RemoveFromPlayerSquad();
					}
				}
			}

			if (candidates.Count())
			{
				CNPC_CounterTerrorist *pClosest = NULL;
				float closestDistSq = FLT_MAX;
				int nJoined = 0;

				for (i = 0; i < candidates.Count() && i < MAX_PLAYER_SQUAD; i++)
				{
					if (!candidates[i].pCitizen->IsInPlayerSquad())
					{
						candidates[i].pCitizen->AddToPlayerSquad();
						nJoined++;

						if (candidates[i].distSq < closestDistSq)
						{
							pClosest = candidates[i].pCitizen;
							closestDistSq = candidates[i].distSq;
						}
					}
				}

				if (pClosest)
				{
					if (!pClosest->SpokeConcept(TLK_JOINPLAYER))
					{
						pClosest->SpeakCommandResponse(TLK_JOINPLAYER, CFmtStr("numjoining:%d", nJoined));
					}
					else
					{
						pClosest->SpeakCommandResponse(TLK_STARTFOLLOW);
					}

					for (i = 0; i < candidates.Count() && i < MAX_PLAYER_SQUAD; i++)
					{
						candidates[i].pCitizen->SetSpokeConcept(TLK_JOINPLAYER, NULL);
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
int CNPC_CounterTerrorist::PlayerSquadCandidateSortFunc(const SquadCandidate_t *pLeft, const SquadCandidate_t *pRight)
{
	// "Bigger" means less approprate 
	CNPC_CounterTerrorist *pLeftCitizen = pLeft->pCitizen;
	CNPC_CounterTerrorist *pRightCitizen = pRight->pCitizen;

	// Medics are better than anyone
	if (pLeftCitizen->IsMedic() && !pRightCitizen->IsMedic())
		return -1;

	if (!pLeftCitizen->IsMedic() && pRightCitizen->IsMedic())
		return 1;

	CBaseCombatWeapon *pLeftWeapon = pLeftCitizen->GetActiveWeapon();
	CBaseCombatWeapon *pRightWeapon = pRightCitizen->GetActiveWeapon();

	// People with weapons are better than those without
	if (pLeftWeapon && !pRightWeapon)
		return -1;

	if (!pLeftWeapon && pRightWeapon)
		return 1;

	// Existing squad members are better than non-members
	if (pLeft->bIsInSquad && !pRight->bIsInSquad)
		return -1;

	if (!pLeft->bIsInSquad && pRight->bIsInSquad)
		return 1;

	// New squad members are better than older ones
	if (pLeft->bIsInSquad && pRight->bIsInSquad)
		return pRight->iSquadIndex - pLeft->iSquadIndex;

	// Finally, just take the closer
	return (int)(pRight->distSq - pLeft->distSq);
}

//-----------------------------------------------------------------------------
void CNPC_CounterTerrorist::FixupPlayerSquad()
{
	if (!AI_IsSinglePlayer())
		return;

	m_flTimeJoinedPlayerSquad = gpGlobals->curtime;
	m_bWasInPlayerSquad = true;
	if (m_pSquad->NumMembers() > MAX_PLAYER_SQUAD)
	{
		CAI_BaseNPC *pFirstMember = m_pSquad->GetFirstMember(NULL);
		m_pSquad->RemoveFromSquad(pFirstMember);
		pFirstMember->ClearCommandGoal();

		CNPC_CounterTerrorist *pFirstMemberCitizen = dynamic_cast< CNPC_CounterTerrorist * >(pFirstMember);
		if (pFirstMemberCitizen)
		{
			pFirstMemberCitizen->ClearFollowTarget();
		}
		else
		{
			CAI_FollowBehavior *pOldMemberFollowBehavior;
			if (pFirstMember->GetBehavior(&pOldMemberFollowBehavior))
			{
				pOldMemberFollowBehavior->SetFollowTarget(NULL);
			}
		}
	}

	ClearFollowTarget();

	CAI_BaseNPC *pLeader = NULL;
	AISquadIter_t iter;
	for (CAI_BaseNPC *pAllyNpc = m_pSquad->GetFirstMember(&iter); pAllyNpc; pAllyNpc = m_pSquad->GetNextMember(&iter))
	{
		if (pAllyNpc->IsCommandable())
		{
			pLeader = pAllyNpc;
			break;
		}
	}

	if (pLeader && pLeader != this)
	{
		const Vector &commandGoal = pLeader->GetCommandGoal();
		if (commandGoal != vec3_invalid)
		{
			SetCommandGoal(commandGoal);
			SetCondition(COND_RECEIVED_ORDERS);
			OnMoveOrder();
		}
		else
		{
			CAI_FollowBehavior *pLeaderFollowBehavior;
			if (pLeader->GetBehavior(&pLeaderFollowBehavior))
			{
				m_FollowBehavior.SetFollowTarget(pLeaderFollowBehavior->GetFollowTarget());
				m_FollowBehavior.SetParameters(m_FollowBehavior.GetFormation());
			}

		}
	}
	else
	{
		m_FollowBehavior.SetFollowTarget(UTIL_GetLocalPlayer());
		m_FollowBehavior.SetParameters(AIF_SIMPLE);
	}
}

//-----------------------------------------------------------------------------
void CNPC_CounterTerrorist::ClearFollowTarget()
{
	m_FollowBehavior.SetFollowTarget(NULL);
	m_FollowBehavior.SetParameters(AIF_SIMPLE);
}

//-----------------------------------------------------------------------------
void CNPC_CounterTerrorist::UpdateFollowCommandPoint()
{
	if (!AI_IsSinglePlayer())
		return;

	if (IsInPlayerSquad())
	{
		if (HaveCommandGoal())
		{
			CBaseEntity *pFollowTarget = m_FollowBehavior.GetFollowTarget();
			CBaseEntity *pCommandPoint = gEntList.FindEntityByClassname(NULL, COMMAND_POINT_CLASSNAME);

			if (!pCommandPoint)
			{
				DevMsg("**\nVERY BAD THING\nCommand point vanished! Creating a new one\n**\n");
				pCommandPoint = CreateEntityByName(COMMAND_POINT_CLASSNAME);
			}

			if (pFollowTarget != pCommandPoint)
			{
				pFollowTarget = pCommandPoint;
				m_FollowBehavior.SetFollowTarget(pFollowTarget);
				m_FollowBehavior.SetParameters(AIF_COMMANDER);
			}

			if ((pCommandPoint->GetAbsOrigin() - GetCommandGoal()).LengthSqr() > 0.01)
			{
				UTIL_SetOrigin(pCommandPoint, GetCommandGoal(), false);
			}
		}
		else
		{
			if (IsFollowingCommandPoint())
				ClearFollowTarget();
			if (m_FollowBehavior.GetFollowTarget() != UTIL_GetLocalPlayer())
			{
				DevMsg("Expected to be following player, but not\n");
				m_FollowBehavior.SetFollowTarget(UTIL_GetLocalPlayer());
				m_FollowBehavior.SetParameters(AIF_SIMPLE);
			}
		}
	}
	else if (IsFollowingCommandPoint())
		ClearFollowTarget();
}

//-----------------------------------------------------------------------------
bool CNPC_CounterTerrorist::IsFollowingCommandPoint()
{
	CBaseEntity *pFollowTarget = m_FollowBehavior.GetFollowTarget();
	if (pFollowTarget)
		return FClassnameIs(pFollowTarget, COMMAND_POINT_CLASSNAME);
	return false;
}

//-----------------------------------------------------------------------------
struct SquadMemberInfo_t
{
	CNPC_CounterTerrorist *	pMember;
	bool			bSeesPlayer;
	float			distSq;
};

CAI_BaseNPC *CNPC_CounterTerrorist::GetSquadCommandRepresentative()
{
	if (!AI_IsSinglePlayer())
		return NULL;

	if (IsInPlayerSquad())
	{
		static float lastTime;
		static AIHANDLE hCurrent;

		if (gpGlobals->curtime - lastTime > 2.0 || !hCurrent || !hCurrent->IsInPlayerSquad()) // hCurrent will be NULL after level change
		{
			lastTime = gpGlobals->curtime;
			hCurrent = NULL;

			CUtlVectorFixed<SquadMemberInfo_t, MAX_SQUAD_MEMBERS> candidates;
			CBasePlayer *pPlayer = UTIL_GetLocalPlayer();

			if (pPlayer)
			{
				AISquadIter_t iter;
				for (CAI_BaseNPC *pAllyNpc = m_pSquad->GetFirstMember(&iter); pAllyNpc; pAllyNpc = m_pSquad->GetNextMember(&iter))
				{
					if (pAllyNpc->IsCommandable() && dynamic_cast<CNPC_CounterTerrorist *>(pAllyNpc))
					{
						int i = candidates.AddToTail();
						candidates[i].pMember = (CNPC_CounterTerrorist *)(pAllyNpc);
						candidates[i].bSeesPlayer = pAllyNpc->HasCondition(COND_SEE_PLAYER);
						candidates[i].distSq = (pAllyNpc->GetAbsOrigin() - pPlayer->GetAbsOrigin()).LengthSqr();
					}
				}

				if (candidates.Count() > 0)
				{
					hCurrent = candidates[0].pMember;
				}
			}
		}

		if (hCurrent != NULL)
		{
			Assert(dynamic_cast<CNPC_CounterTerrorist *>(hCurrent.Get()) && hCurrent->IsInPlayerSquad());
			return hCurrent;
		}
	}
	return NULL;
}

//-----------------------------------------------------------------------------
void CNPC_CounterTerrorist::SetSquad(CAI_Squad *pSquad)
{
	bool bWasInPlayerSquad = IsInPlayerSquad();

	BaseClass::SetSquad(pSquad);

	if (IsInPlayerSquad() && !bWasInPlayerSquad)
	{
		m_OnJoinedPlayerSquad.FireOutput(this, this);
	}
	else if (!IsInPlayerSquad() && bWasInPlayerSquad)
	{
		m_OnLeftPlayerSquad.FireOutput(this, this);
	}
}

//-----------------------------------------------------------------------------
// Purpose:  This is a generic function (to be implemented by sub-classes) to
//			 handle specific interactions between different types of characters
//			 (For example the barnacle grabbing an NPC)
// Input  :  Constant for the type of interaction
// Output :	 true  - if sub-class has a response for the interaction
//			 false - if sub-class has no response
//-----------------------------------------------------------------------------
bool CNPC_CounterTerrorist::HandleInteraction(int interactionType, void *data, CBaseCombatCharacter* sourceEnt)
{
	if (interactionType == g_interactionHitByPlayerThrownPhysObj)
	{
		if (IsOkToSpeakInResponseToPlayer())
		{
			Speak(TLK_PLYR_PHYSATK);
		}
		return true;
	}
	return BaseClass::HandleInteraction(interactionType, data, sourceEnt);
}

//-----------------------------------------------------------------------------
bool CNPC_CounterTerrorist::FValidateHintType(CAI_Hint *pHint)
{
	switch (pHint->HintType())
	{
	case HINT_WORLD_VISUALLY_INTERESTING:
		return true;
	default:
		break;
	}

	return BaseClass::FValidateHintType(pHint);
}

//-----------------------------------------------------------------------------
void CNPC_CounterTerrorist::InputStartPatrolling(inputdata_t &inputdata)
{
	m_bShouldPatrol = true;
}

//-----------------------------------------------------------------------------
void CNPC_CounterTerrorist::InputStopPatrolling(inputdata_t &inputdata)
{
	m_bShouldPatrol = false;
}

//------------------------------------------------------------------------------
void CNPC_CounterTerrorist::InputSetCommandable(inputdata_t &inputdata)
{
	RemoveSpawnFlags(SF_CITIZEN_NOT_COMMANDABLE);
	gm_PlayerSquadEvaluateTimer.Force();
}

//------------------------------------------------------------------------------
void CNPC_CounterTerrorist::InputSpeakIdleResponse(inputdata_t &inputdata)
{
	SpeakIfAllowed(TLK_ANSWER, NULL, true);
}

//-----------------------------------------------------------------------------
void CNPC_CounterTerrorist::DeathSound(const CTakeDamageInfo &info)
{
	// Sentences don't play on dead NPCs
	SentenceStop();
	EmitSound("NPC_Citizen.Die");
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_CounterTerrorist::UseSemaphore(void)
{
	// Ignore semaphore if we're told to work outside it
	if (HasSpawnFlags(SF_CITIZEN_IGNORE_SEMAPHORE))
		return false;

	return BaseClass::UseSemaphore();
}

//-----------------------------------------------------------------------------
//
// Schedules
//
//-----------------------------------------------------------------------------
AI_BEGIN_CUSTOM_NPC(npc_counterterrorist, CNPC_CounterTerrorist)

DECLARE_TASK(TASK_CIT_SPEAK_MOURNING)

//=========================================================
// > SCHED_CITIZEN_PATROL
//=========================================================
DEFINE_SCHEDULE
(
	SCHED_CITIZEN_PATROL,

	"	Tasks"
	"		TASK_STOP_MOVING				0"
	"		TASK_WANDER						901024"		// 90 to 1024 units
	"		TASK_WALK_PATH					0"
	"		TASK_WAIT_FOR_MOVEMENT			0"
	"		TASK_STOP_MOVING				0"
	"		TASK_WAIT						3"
	"		TASK_WAIT_RANDOM				3"
	"		TASK_SET_SCHEDULE				SCHEDULE:SCHED_CITIZEN_PATROL" // keep doing it
	""
	"	Interrupts"
	"		COND_ENEMY_DEAD"
	"		COND_LIGHT_DAMAGE"
	"		COND_HEAVY_DAMAGE"
	"		COND_HEAR_DANGER"
	"		COND_NEW_ENEMY"
	)

	DEFINE_SCHEDULE
	(
		SCHED_CITIZEN_MOURN_PLAYER,

		"	Tasks"
		"		TASK_GET_PATH_TO_PLAYER		0"
		"		TASK_RUN_PATH_WITHIN_DIST	180"
		"		TASK_WAIT_FOR_MOVEMENT		0"
		"		TASK_STOP_MOVING			0"
		"		TASK_TARGET_PLAYER			0"
		"		TASK_FACE_TARGET			0"
		"		TASK_CIT_SPEAK_MOURNING		0"
		"		TASK_SUGGEST_STATE			STATE:IDLE"
		""
		"	Interrupts"
		"		COND_LIGHT_DAMAGE"
		"		COND_HEAVY_DAMAGE"
		"		COND_HEAR_DANGER"
		"		COND_NEW_ENEMY"
		)
	AI_END_CUSTOM_NPC()
