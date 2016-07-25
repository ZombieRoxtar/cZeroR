//===== Copyright Bit Mage's Stuff, All rights probably reserved. =====
//
// Purpose: Stores and displays mission info
//
//=============================================================================
#include "cbase.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CMapObjectives : public CLogicalEntity
{
public:
	DECLARE_CLASS(CMapObjectives, CLogicalEntity);
	DECLARE_DATADESC();

	void DisplayObjectives(CBasePlayer *pPlayer);
	void DisplayBreifing(CBasePlayer *pPlayer);

	// Input function
	void InputDisplayObjectives(inputdata_t &inputData);
	void InputDisplayBreifing(inputdata_t &inputData);
	void SetObjectives(inputdata_t &inputData);
	void SetBreifing(inputdata_t &inputData);

private:
	string_t m_iszObjectiveText;
	string_t m_iszBreifingText;
};


LINK_ENTITY_TO_CLASS(info_map_objectives, CMapObjectives);

BEGIN_DATADESC(CMapObjectives)

DEFINE_KEYFIELD(m_iszObjectiveText, FIELD_STRING, "Objectives"),
DEFINE_KEYFIELD(m_iszBreifingText, FIELD_STRING, "MissionBreifing"),

DEFINE_INPUTFUNC(FIELD_VOID, "DisplayObjectives", InputDisplayObjectives),
DEFINE_INPUTFUNC(FIELD_VOID, "DisplayBreifing", InputDisplayBreifing),
DEFINE_INPUTFUNC(FIELD_STRING, "SetObjectives", SetObjectives),
DEFINE_INPUTFUNC(FIELD_STRING, "SetBreifing", SetBreifing),

END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Display objectives text
//-----------------------------------------------------------------------------
void CMapObjectives::DisplayObjectives(CBasePlayer *pPlayer)
{
	if (pPlayer)
	{
		UTIL_ShowMessage(STRING(m_iszObjectiveText), pPlayer);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Display breifing text
//-----------------------------------------------------------------------------
void CMapObjectives::DisplayBreifing(CBasePlayer *pPlayer)
{
	if (pPlayer)
	{
		UTIL_ShowMessage(STRING(m_iszBreifingText), pPlayer);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Input to Display objectives text
//-----------------------------------------------------------------------------
void CMapObjectives::InputDisplayObjectives(inputdata_t &inputData)
{
	UTIL_ShowMessageAll(STRING(m_iszObjectiveText));
}

//-----------------------------------------------------------------------------
// Purpose: Input to Display objectives text
//-----------------------------------------------------------------------------
void CMapObjectives::InputDisplayBreifing(inputdata_t &inputData)
{
	UTIL_ShowMessageAll(STRING(m_iszBreifingText));
}

//-----------------------------------------------------------------------------
// Purpose: Sets objectives text
//-----------------------------------------------------------------------------
void CMapObjectives::SetObjectives(inputdata_t &inputData)
{
	m_iszObjectiveText = castable_string_t(inputData.value.String());
}
//-----------------------------------------------------------------------------
// Purpose: Sets objectives text
//-----------------------------------------------------------------------------
void CMapObjectives::SetBreifing(inputdata_t &inputData)
{
	m_iszObjectiveText = castable_string_t(inputData.value.String());
}


//-----------------------------------------------------------------------------
// Purpose: ConCommand to Display objectives text
//-----------------------------------------------------------------------------
void Objectives_Display(const CCommand &args)
{
	CBaseEntity* pResult = gEntList.FindEntityByClassname(NULL, "info_map_objectives");
	CMapObjectives* pObjectivesEnt = dynamic_cast<CMapObjectives*>(pResult);
	if (pObjectivesEnt)
	{
		pObjectivesEnt->DisplayObjectives(UTIL_GetCommandClient());
	}
}
ConCommand list_objectives("list_objectives", Objectives_Display, "List Mission Objectives");

//-----------------------------------------------------------------------------
// Purpose: ConCommand to Display briefing text
//-----------------------------------------------------------------------------
void Briefing_Display(const CCommand &args)
{
	CBaseEntity* pResult = gEntList.FindEntityByClassname(NULL, "info_map_objectives");
	CMapObjectives* pObjectivesEnt = dynamic_cast<CMapObjectives*>(pResult);
	if (pObjectivesEnt)
	{
		pObjectivesEnt->DisplayBreifing(UTIL_GetCommandClient());
	}
}
ConCommand showbriefing("showbriefing", Briefing_Display, "Recap Mission Briefing");
