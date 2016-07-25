//===== Copyright Bit Mage's Stuff, All rights probably reserved. =====
//
// Purpose: Hostages
//
//=============================================================================

#include "cbase.h"
#include "npc_hostage.h"
#include "npcevent.h"
#include "eventqueue.h"
#include "ai_squad.h"
#include "ai_pathfinder.h"
#include "ai_route.h"
#include "sceneentity.h"
#include "tier0/ICommandLine.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
const int MAX_PLAYER_HOSTAGES = 4;

#define HOSTAGE_FADE_TIME 5.0f

ConVar	sk_hostage_health("sk_hostage_health", "1");
ConVar	npc_hostage_dont_precache_all("npc_hostage_dont_precache_all", "0");

//-----------------------------------------------------------------------------
// Citizen expressions for the citizen expression types
//-----------------------------------------------------------------------------
#define STATES_WITH_EXPRESSIONS		3 // Idle, Alert, Combat
#define EXPRESSIONS_PER_STATE		1

char *szHostageExpressionTypes[CIT_EXP_LAST_TYPE] =
{
	"Unassigned",
	"Scared",
	"Normal",
	"Angry"
};

struct hostage_expression_list_t
{
	char *szExpressions[EXPRESSIONS_PER_STATE];
};

// Scared
hostage_expression_list_t ScaredExpressions[STATES_WITH_EXPRESSIONS] =
{
	{ "scenes/Expressions/citizen_scared_idle_01.vcd" },
	{ "scenes/Expressions/citizen_scared_alert_01.vcd" },
	{ "scenes/Expressions/citizen_scared_combat_01.vcd" },
};
// Normal
hostage_expression_list_t NormalExpressions[STATES_WITH_EXPRESSIONS] =
{
	{ "scenes/Expressions/citizen_normal_idle_01.vcd" },
	{ "scenes/Expressions/citizen_normal_alert_01.vcd" },
	{ "scenes/Expressions/citizen_normal_combat_01.vcd" },
};
// Angry
hostage_expression_list_t AngryExpressions[STATES_WITH_EXPRESSIONS] =
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

#define IsExcludedHead( type, bMedic, iHead) false // see XBox codeline for an implementation


//---------------------------------------------------------
// Hostage activities
//---------------------------------------------------------
int ACT_HOSTAGE_HANDSUP;
int	ACT_HOSTAGE_STARTLED;

//---------------------------------------------------------

LINK_ENTITY_TO_CLASS(npc_hostage, CNPC_Hostage);

//---------------------------------------------------------

BEGIN_DATADESC(CNPC_Hostage)

DEFINE_FIELD(m_flNextFearSoundTime, FIELD_TIME),
DEFINE_FIELD(m_iszOriginalSquad, FIELD_STRING),
DEFINE_FIELD(m_flTimeJoinedPlayerSquad, FIELD_TIME),
DEFINE_FIELD(m_bWasInPlayerSquad, FIELD_BOOLEAN),
DEFINE_FIELD(m_flTimeLastCloseToPlayer, FIELD_TIME),
DEFINE_KEYFIELD(m_Type, FIELD_INTEGER, "hostagetype"),
DEFINE_KEYFIELD(m_ExpressionType, FIELD_INTEGER, "expressiontype"),
DEFINE_FIELD(m_iHead, FIELD_INTEGER),
DEFINE_FIELD(m_flTimePlayerStare, FIELD_TIME),
DEFINE_FIELD(m_hSavedFollowGoalEnt, FIELD_EHANDLE),
DEFINE_KEYFIELD(m_bNotifyNavFailBlocked, FIELD_BOOLEAN, "notifynavfailblocked"),

DEFINE_OUTPUT(m_OnPlayerUse, "OnPlayerUse"),
DEFINE_OUTPUT(m_OnNavFailBlocked, "OnNavFailBlocked"),
DEFINE_OUTPUT(m_OnRescued, "OnRescued"),

DEFINE_INPUTFUNC(FIELD_VOID, "SpeakIdleResponse", InputSpeakIdleResponse),

DEFINE_USEFUNC(CommanderUse),

END_DATADESC()

//-----------------------------------------------------------------------------
CSimpleSimTimer CNPC_Hostage::gm_PlayerSquadEvaluateTimer;

//-----------------------------------------------------------------------------
void CNPC_Hostage::Precache()
{
	SelectModel();
	SelectExpressionType();

	if (!npc_hostage_dont_precache_all.GetBool())
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
void CNPC_Hostage::PrecacheAllOfType(HostageType_t type)
{
	if (m_Type == CT_UNIQUE)
		return;

	int nHeads = ARRAYSIZE(g_ppszRandomHeads);
	int i;
	for (i = 0; i < nHeads; ++i)
	{
		if (!IsExcludedHead(type, false, i))
		{
			PrecacheModel(CFmtStr("models/Humans/%s/%s",
				(const char *)(CFmtStr(g_ppszModelLocs[m_Type])), g_ppszRandomHeads[i]));
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Should this NPC follow the player?
//-----------------------------------------------------------------------------
bool CNPC_Hostage::ShouldAutosquad(void)
{
	return m_bShouldBeFollowing;
}

//-----------------------------------------------------------------------------
void CNPC_Hostage::Spawn()
{
	BaseClass::Spawn();

#ifdef _XBOX
	// Always fade the corpse
	AddSpawnFlags(SF_NPC_FADE_CORPSE);
#endif // _XBOX

	m_bShouldBeFollowing = HasSpawnFlags(SF_HOSTAGE_FOLLOW);

	if (ShouldAutosquad())
	{
		if (m_SquadName == GetPlayerSquadName())
		{
			CAI_Squad *pPlayerSquad = g_AI_SquadManager.FindSquad(GetPlayerSquadName());
			if (pPlayerSquad && pPlayerSquad->NumMembers() >= MAX_PLAYER_HOSTAGES)
				m_SquadName = NULL_STRING;
		}
		gm_PlayerSquadEvaluateTimer.Force();
	}

	m_iHealth = sk_hostage_health.GetFloat();

	m_iszIdleExpression = MAKE_STRING("scenes/expressions/citizenidle.vcd");
	m_iszAlertExpression = MAKE_STRING("scenes/expressions/citizenalert_loop.vcd");
	m_iszCombatExpression = MAKE_STRING("scenes/expressions/citizencombat_loop.vcd");

	m_iszOriginalSquad = m_SquadName;

	m_flTimePlayerStare = FLT_MAX;

	NPCInit();

	SetUse(&CNPC_Hostage::CommanderUse);
	Assert(!ShouldAutosquad() || !IsInPlayerSquad());

	m_bWasInPlayerSquad = IsInPlayerSquad();
}

//-----------------------------------------------------------------------------
void CNPC_Hostage::PostNPCInit()
{
	if (IsInPlayerSquad())
	{
		if (m_pSquad->NumMembers() > MAX_PLAYER_HOSTAGES)
			DevMsg(
				"Error: Spawning following hostage, but exceeded limit of %d!\n",
				MAX_PLAYER_HOSTAGES);

		FixupPlayerSquad();
	}
	else
	{
		if ((m_spawnflags & SF_HOSTAGE_FOLLOW) && AI_IsSinglePlayer())
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

void CNPC_Hostage::SelectModel()
{
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
			HostageType_t type;
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

	if (HasSpawnFlags(SF_HOSTAGE_RANDOM_HEAD | SF_HOSTAGE_RANDOM_HEAD_MALE |
		SF_HOSTAGE_RANDOM_HEAD_FEMALE) || GetModelName() == NULL_STRING)
	{
		Assert(m_iHead == -1);
		char gender = (HasSpawnFlags(SF_HOSTAGE_RANDOM_HEAD_MALE)) ? 'm' :
			(HasSpawnFlags(SF_HOSTAGE_RANDOM_HEAD_FEMALE)) ? 'f' : 0;

		RemoveSpawnFlags(SF_HOSTAGE_RANDOM_HEAD | SF_HOSTAGE_RANDOM_HEAD_MALE | SF_HOSTAGE_RANDOM_HEAD_FEMALE);
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
				CNPC_Hostage *pHostage = dynamic_cast<CNPC_Hostage *>(g_AI_Manager.AccessAIs()[i]);
				if (pHostage && pHostage != this && pHostage->m_iHead >= 0
					&& pHostage->m_iHead < ARRAYSIZE(g_ppszRandomHeads))
				{
					headCounts[pHostage->m_iHead]++;
				}
			}

			// Find all candidates
			CUtlVectorFixed<HeadCandidate_t, ARRAYSIZE(g_ppszRandomHeads)> candidates;

			for (i = 0; i < ARRAYSIZE(g_ppszRandomHeads); i++)
			{
				if (!gender || g_ppszRandomHeads[i][0] == gender)
				{
					if (!IsExcludedHead(m_Type, IsMedic(), i))
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

	// Unique models are left alone
	if (m_Type != CT_UNIQUE)
	{
		SetModelName(AllocPooledString(CFmtStr("models/Humans/%s/%s",
			(const char *)(CFmtStr(g_ppszModelLocs[m_Type])), pszModelName)));
	}
}

//-----------------------------------------------------------------------------
void CNPC_Hostage::SelectExpressionType()
{
	// If we've got a mapmaker assigned type, leave it alone
	if (m_ExpressionType != CIT_EXP_UNASSIGNED)
		return;

	switch (m_Type)
	{
	case CT_DOWNTRODDEN:
	case CT_REFUGEE:
		m_ExpressionType = (HostageExpressionTypes_t)RandomInt(CIT_EXP_SCARED, CIT_EXP_NORMAL);
		break;
	case CT_REBEL:
		m_ExpressionType = (HostageExpressionTypes_t)RandomInt(CIT_EXP_SCARED, CIT_EXP_ANGRY);
		break;

	case CT_DEFAULT:
	case CT_UNIQUE:
	default:
		break;
	}
}

//-----------------------------------------------------------------------------
void CNPC_Hostage::OnRestore()
{
	gm_PlayerSquadEvaluateTimer.Force();

	BaseClass::OnRestore();
}

//-----------------------------------------------------------------------------
Class_T	CNPC_Hostage::Classify()
{
	return CLASS_HOSTAGE;
}

//-----------------------------------------------------------------------------
bool CNPC_Hostage::ShouldAlwaysThink()
{
	return (BaseClass::ShouldAlwaysThink() || IsInPlayerSquad());
}

//-----------------------------------------------------------------------------
void CNPC_Hostage::GatherConditions()
{
	BaseClass::GatherConditions();

	if (!SpokeConcept(TLK_JOINPLAYER) && IsRunningScriptedSceneWithSpeech(this, true))
	{
		SetSpokeConcept(TLK_JOINPLAYER, NULL);
		for (int i = 0; i < g_AI_Manager.NumAIs(); i++)
		{
			CAI_BaseNPC *pNpc = g_AI_Manager.AccessAIs()[i];
			if (pNpc != this && pNpc->GetClassname() == GetClassname() &&
				pNpc->GetAbsOrigin().DistToSqr(GetAbsOrigin()) < Square(15 * 12) && FVisible(pNpc))
			{
				(assert_cast<CNPC_Hostage *>(pNpc))->SetSpokeConcept(TLK_JOINPLAYER, NULL);
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CNPC_Hostage::PrescheduleThink()
{
	BaseClass::PrescheduleThink();

	if (CanJoinPlayerSquad() || IsInPlayerSquad())
	{
		UpdatePlayerSquad();
	}
}

//-----------------------------------------------------------------------------
int CNPC_Hostage::TranslateSchedule(int scheduleType)
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
				return SCHED_HOSTAGE_MOURN_PLAYER;
			}
		}
		else if (m_bWaitingInRescue)
		{
			return SCHED_HOSTAGE_RESCUE_WAIT;
		}
		break;

	case SCHED_ESTABLISH_LINE_OF_FIRE:
	case SCHED_MOVE_TO_WEAPON_RANGE:
		if (!IsMortar(GetEnemy()) && HaveCommandGoal())
		{
			if (GetActiveWeapon() && (GetActiveWeapon()->CapabilitiesGet() & bits_CAP_WEAPON_RANGE_ATTACK1)
				&& random->RandomInt(0, 1) && HasCondition(COND_SEE_ENEMY)
				&& !HasCondition(COND_NO_PRIMARY_AMMO))
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
bool CNPC_Hostage::ShouldAcceptGoal(CAI_BehaviorBase *pBehavior, CAI_GoalEntity *pGoal)
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
void CNPC_Hostage::OnClearGoal(CAI_BehaviorBase *pBehavior, CAI_GoalEntity *pGoal)
{
	if (m_hSavedFollowGoalEnt == pGoal)
		m_hSavedFollowGoalEnt = NULL;
}

//-----------------------------------------------------------------------------
void CNPC_Hostage::StartTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{
	case TASK_HOSTAGE_SPEAK_MOURNING:
		if (!IsSpeaking() && CanSpeakAfterMyself())
		{
			Speak(TLK_PLDEAD);
		}
		TaskComplete();
		break;

	case TASK_HOSTAGE_RESCUE_SPEAK:
		m_bWaitingInRescue = false;
		Speak(TLK_ANSWER);
		TaskComplete();
		break;

	case TASK_HOSTAGE_RESCUE_WAIT:
		m_flFadeOutStartTime = gpGlobals->curtime;
		m_nRenderMode = kRenderTransTexture;
		break;

	default:
		BaseClass::StartTask(pTask);
		break;
	}
}

//-----------------------------------------------------------------------------
void CNPC_Hostage::RunTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{

	case TASK_HOSTAGE_RESCUE_WAIT:
		if (!IsSpeaking())
		{
			if (!HasSpawnFlags(SF_HOSTAGE_CUSTOM_RESCUE))
			{
				CapabilitiesRemove(bits_CAP_MOVE_GROUND);
				if (gpGlobals->curtime - m_flFadeOutStartTime < HOSTAGE_FADE_TIME)
				{
					SetRenderColorA(abs(int((1.0f - (gpGlobals->curtime - m_flFadeOutStartTime) / HOSTAGE_FADE_TIME) * 255.0f)));
				}
				else
				{
					TaskComplete();
					Remove();
				}
			}
			else
			{
				TaskComplete();
			}
		}
		break;

	default:
		BaseClass::RunTask(pTask);
		break;
	}
}

//-----------------------------------------------------------------------------
void CNPC_Hostage::TaskFail(AI_TaskFailureCode_t code)
{
	if (code == FAIL_NO_ROUTE_BLOCKED && m_bNotifyNavFailBlocked)
	{
		m_OnNavFailBlocked.FireOutput(this, this);
	}

	BaseClass::TaskFail(code);
}

//------------------------------------------------------------------------------
void CNPC_Hostage::HandleAnimEvent(animevent_t *pEvent)
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
// Purpose: Return a random expression for the specified state to play over 
//			the state's expression loop.
//-----------------------------------------------------------------------------
const char *CNPC_Hostage::SelectRandomExpressionForState(NPC_STATE state)
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
	case CIT_EXP_SCARED:
	{
		int iRandom = RandomInt(0, ARRAYSIZE(ScaredExpressions[iExpressionState].szExpressions) - 1);
		return ScaredExpressions[iExpressionState].szExpressions[iRandom];
	}

	case CIT_EXP_NORMAL:
	{
		int iRandom = RandomInt(0, ARRAYSIZE(NormalExpressions[iExpressionState].szExpressions) - 1);
		return NormalExpressions[iExpressionState].szExpressions[iRandom];
	}

	case CIT_EXP_ANGRY:
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
bool CNPC_Hostage::CanJoinPlayerSquad()
{
	if (!AI_IsSinglePlayer())
		return false;

	if (m_NPCState == NPC_STATE_SCRIPT)
		return false;

	if (IsInAScript())
		return false;

	// Don't bother people who don't want to be bothered
	if (!CanBeUsedAsAFriend())
		return false;

	if (IRelationType(UTIL_GetLocalPlayer()) != D_LI)
		return false;

	return m_bShouldBeFollowing;
}

//-----------------------------------------------------------------------------
bool CNPC_Hostage::WasInPlayerSquad()
{
	return m_bWasInPlayerSquad;
}

//-----------------------------------------------------------------------------
bool CNPC_Hostage::HaveCommandGoal() const
{
	return (GetCommandGoal() != vec3_invalid);
}

//-----------------------------------------------------------------------------
bool CNPC_Hostage::SpeakCommandResponse(AIConcept_t concept, const char *modifiers)
{
	return SpeakIfAllowed(concept,
		CFmtStr("numselected:%d,"
			"useradio:%d%s",
			(GetSquad()) ? GetSquad()->NumMembers() : 1,
			ShouldSpeakRadio(AI_GetSinglePlayer()),
			(modifiers) ? CFmtStr(",%s", modifiers).operator const char *() : ""));
}

//-----------------------------------------------------------------------------
void CNPC_Hostage::CommanderUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	m_OnPlayerUse.FireOutput(pActivator, pCaller);

	if (!IsInPlayerSquad())
	{
		m_bShouldBeFollowing = true;
		AddToPlayerSquad();
		SpeakIfAllowed(TLK_STARTFOLLOW, NULL, true);
	}
	else
	{
		m_bShouldBeFollowing = false;
		RemoveFromPlayerSquad();
		SpeakIfAllowed(TLK_STOPFOLLOW, NULL, true);
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
						g_EventQueue.AddEvent(pRespondant, "SpeakIdleResponse",
							(GetTimeSpeechComplete() - gpGlobals->curtime) + .2, this, this);
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
bool CNPC_Hostage::ShouldSpeakRadio(CBaseEntity *pListener)
{
	if (!pListener)
		return false;

	const float		radioRange = 384 * 384;
	Vector			vecDiff;

	vecDiff = WorldSpaceCenter() - pListener->WorldSpaceCenter();

	if (vecDiff.LengthSqr() > radioRange)
	{
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
void CNPC_Hostage::AddToPlayerSquad()
{
	Assert(!IsInPlayerSquad());

	AddToSquad(AllocPooledString(PLAYER_SQUADNAME));
	m_hSavedFollowGoalEnt = m_FollowBehavior.GetFollowGoal();
	m_FollowBehavior.SetFollowGoalDirect(NULL);

	FixupPlayerSquad();

	SetCondition(COND_PLAYER_ADDED_TO_SQUAD);
}

//-----------------------------------------------------------------------------
void CNPC_Hostage::RemoveFromPlayerSquad()
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
struct SquadCandidate_t
{
	CNPC_Hostage *pCitizen;
	bool		  bIsInSquad;
	float		  distSq;
	int			  iSquadIndex;
};

void CNPC_Hostage::UpdatePlayerSquad()
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
		CUtlVectorFixed<CNPC_Hostage *, MAX_PLAYER_HOSTAGES> squadMembersToRemove;
		AISquadIter_t iter;

		for (CAI_BaseNPC *pPlayerSquadMember = pPlayerSquad->GetFirstMember(&iter);
		pPlayerSquadMember; pPlayerSquadMember = pPlayerSquad->GetNextMember(&iter))
		{
			if (pPlayerSquadMember->GetClassname() != GetClassname())
				continue;

			CNPC_Hostage *pHostage = assert_cast<CNPC_Hostage *>(pPlayerSquadMember);

			if (!pHostage->m_bNeverLeavePlayerSquad &&
				pHostage->m_FollowBehavior.GetFollowTarget() &&
				!pHostage->m_FollowBehavior.FollowTargetVisible() &&
				pHostage->m_FollowBehavior.GetNumFailedFollowAttempts() > 0 &&
				gpGlobals->curtime - pHostage->m_FollowBehavior.GetTimeFailFollowStarted() > 20 &&
				(fabsf((pHostage->m_FollowBehavior.GetFollowTarget()->GetAbsOrigin().z -
					pHostage->GetAbsOrigin().z)) > 196 ||
					(pHostage->m_FollowBehavior.GetFollowTarget()->GetAbsOrigin().AsVector2D() -
						pHostage->GetAbsOrigin().AsVector2D()).LengthSqr() > Square(50 * 12)))
			{
				squadMembersToRemove.AddToTail(pHostage);
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

			CNPC_Hostage *pHostage = assert_cast<CNPC_Hostage *>(ppAIs[i]);
			int iNew;

			if (pHostage->IsInPlayerSquad())
			{
				iNew = candidates.AddToTail();
				candidates[iNew].pCitizen = pHostage;
				candidates[iNew].bIsInSquad = true;
				candidates[iNew].distSq = 0;
				candidates[iNew].iSquadIndex = pHostage->GetSquad()->GetSquadIndex(pHostage);
			}
			else
			{
				float distSq = (vPlayerPos.AsVector2D() - pHostage->GetAbsOrigin().AsVector2D()).LengthSqr();
				if (distSq > JOIN_PLAYER_XY_TOLERANCE_SQ &&
					(pHostage->m_flTimeJoinedPlayerSquad == 0 ||
						gpGlobals->curtime - pHostage->m_flTimeJoinedPlayerSquad > 60.0) &&
					(pHostage->m_flTimeLastCloseToPlayer == 0 ||
						gpGlobals->curtime - pHostage->m_flTimeLastCloseToPlayer > 15.0))
					continue;

				if (!pHostage->CanJoinPlayerSquad())
					continue;

				bool bShouldAdd = false;

				if (pHostage->HasCondition(COND_SEE_PLAYER))
					bShouldAdd = true;
				else
				{
					bool bPlayerVisible = pHostage->FVisible(pPlayer);
					if (bPlayerVisible)
					{
						if (pHostage->HasCondition(COND_HEAR_PLAYER))
							bShouldAdd = true;
						else if (distSq < UNCONDITIONAL_JOIN_PLAYER_XY_TOLERANCE_SQ &&
							fabsf(vPlayerPos.z - pHostage->GetAbsOrigin().z) <
							UNCONDITIONAL_JOIN_PLAYER_Z_TOLERANCE)
							bShouldAdd = true;
					}
				}

				if (bShouldAdd)
				{
					// @TODO (toml 05-25-04): probably everyone in a squad should be a candidate
					// if one of them sees the player
					AI_Waypoint_t *pPathToPlayer = pHostage->GetPathfinder()->
						BuildRoute(pHostage->GetAbsOrigin(), vPlayerPos, pPlayer, 5 * 12, NAV_NONE, true);
					GetPathfinder()->UnlockRouteNodes(pPathToPlayer);

					if (!pPathToPlayer)
						continue;

					CAI_Path tempPath;
					tempPath.SetWaypoints(pPathToPlayer); // path object will delete waypoints

					iNew = candidates.AddToTail();
					candidates[iNew].pCitizen = pHostage;
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
			{
				if (candidates[i].iSquadIndex == -1)
				{
					for (int j = 0; j < g_AI_Manager.NumAIs(); j++)
					{
						if (ppAIs[j]->GetState() == NPC_STATE_DEAD)
							continue;

						if (ppAIs[j]->GetClassname() != GetClassname())
							continue;

						CNPC_Hostage *pHostage = assert_cast<CNPC_Hostage *>(ppAIs[j]);

						float distSq = (vPlayerPos - pHostage->GetAbsOrigin()).Length2DSqr();
						if (distSq > JOIN_PLAYER_XY_TOLERANCE_SQ)
							continue;

						distSq = (candidates[i].pCitizen->GetAbsOrigin() -
							pHostage->GetAbsOrigin()).Length2DSqr();
						if (distSq > SECOND_TIER_JOIN_DIST_SQ)
							continue;

						if (!pHostage->CanJoinPlayerSquad())
							continue;

						if (!pHostage->FVisible(pPlayer))
							continue;

						int iNew = candidates.AddToTail();
						candidates[iNew].pCitizen = pHostage;
						candidates[iNew].bIsInSquad = false;
						candidates[iNew].distSq = distSq;
						candidates[iNew].iSquadIndex = -1;
					}
				}
			}

			if (candidates.Count())
			{
				CNPC_Hostage *pClosest = NULL;
				float closestDistSq = FLT_MAX;
				int nJoined = 0;

				for (i = 0; i < candidates.Count() && i < MAX_PLAYER_HOSTAGES; i++)
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

					for (i = 0; i < candidates.Count() && i < MAX_PLAYER_HOSTAGES; i++)
					{
						candidates[i].pCitizen->SetSpokeConcept(TLK_JOINPLAYER, NULL);
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
void CNPC_Hostage::FixupPlayerSquad()
{
	if (!AI_IsSinglePlayer())
		return;

	m_flTimeJoinedPlayerSquad = gpGlobals->curtime;
	m_bWasInPlayerSquad = true;
	if (m_pSquad->NumMembers() > MAX_PLAYER_HOSTAGES)
	{
		CAI_BaseNPC *pFirstMember = m_pSquad->GetFirstMember(NULL);
		m_pSquad->RemoveFromSquad(pFirstMember);
		pFirstMember->ClearCommandGoal();

		CNPC_Hostage *pFirstMemberCitizen = dynamic_cast< CNPC_Hostage * >(pFirstMember);
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

	AISquadIter_t iter;
	CAI_BaseNPC *pLeader = m_pSquad->GetFirstMember(&iter);

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
void CNPC_Hostage::ClearFollowTarget()
{
	m_FollowBehavior.SetFollowTarget(NULL);
	m_FollowBehavior.SetParameters(AIF_SIMPLE);
}

//-----------------------------------------------------------------------------
bool CNPC_Hostage::FValidateHintType(CAI_Hint *pHint)
{
	switch (pHint->HintType())
	{
	case HINT_WORLD_VISUALLY_INTERESTING:
		return true;
		break;

	default:
		break;
	}

	return BaseClass::FValidateHintType(pHint);
}

//------------------------------------------------------------------------------
void CNPC_Hostage::InputSpeakIdleResponse(inputdata_t &inputdata)
{
	SpeakIfAllowed(TLK_ANSWER, NULL, true);
}

//-----------------------------------------------------------------------------
void CNPC_Hostage::DeathSound(const CTakeDamageInfo &info)
{
	// Sentences don't play on dead NPCs
	SentenceStop();

	EmitSound("NPC_Citizen.Die");
}

//------------------------------------------------------------------------------
void CNPC_Hostage::FearSound(void)
{
	SpeakIfAllowed(TLK_ANSWER, NULL, true);
}

//-----------------------------------------------------------------------------
//
// Schedules
//
//-----------------------------------------------------------------------------

AI_BEGIN_CUSTOM_NPC(npc_hostage, CNPC_Hostage)

DECLARE_TASK(TASK_HOSTAGE_SPEAK_MOURNING)
DECLARE_TASK(TASK_HOSTAGE_RESCUE_SPEAK)
DECLARE_TASK(TASK_HOSTAGE_RESCUE_WAIT)

DECLARE_ACTIVITY(ACT_HOSTAGE_HANDSUP)
DECLARE_ACTIVITY(ACT_HOSTAGE_STARTLED)

DEFINE_SCHEDULE
(
	SCHED_HOSTAGE_MOURN_PLAYER,

	"	Tasks"
	"		TASK_GET_PATH_TO_PLAYER		0"
	"		TASK_RUN_PATH_WITHIN_DIST	180"
	"		TASK_WAIT_FOR_MOVEMENT		0"
	"		TASK_STOP_MOVING			0"
	"		TASK_TARGET_PLAYER			0"
	"		TASK_FACE_TARGET			0"
	"		TASK_HOSTAGE_SPEAK_MOURNING	0"
	"		TASK_SUGGEST_STATE			STATE:IDLE"
	""
	"	Interrupts"
	"		COND_LIGHT_DAMAGE"
	"		COND_HEAVY_DAMAGE"
	"		COND_HEAR_DANGER"
	"		COND_NEW_ENEMY"
	)
	DEFINE_SCHEDULE
	(
		SCHED_HOSTAGE_RESCUE_WAIT,

		"	Tasks"
		"		TASK_STOP_MOVING			0"
		"		TASK_SUGGEST_STATE			STATE:IDLE"
		"		TASK_HOSTAGE_RESCUE_SPEAK	0"
		"		TASK_HOSTAGE_RESCUE_WAIT	0"
		""
		"	Interrupts"
		)
	AI_END_CUSTOM_NPC()

	//------------------------------------------------------------------------------
	// Purpose: Rescues the hostage, removing them, after firing the output.
	//------------------------------------------------------------------------------
	void CNPC_Hostage::Rescue(CSpecialZone *zone)
{
	m_bShouldBeFollowing = false;
	RemoveFromPlayerSquad();
	m_bWaitingInRescue = true;
	m_OnRescued.FireOutput(this, zone);
}
