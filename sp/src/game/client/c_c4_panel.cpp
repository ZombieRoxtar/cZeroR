//===== Copyright Bit Mage's Stuff, All rights probably reserved. =====
//
// Purpose: C4 VGUI Panel
//
//=============================================================================
#include "cbase.h"
#include "c_vguiscreen.h"
#include <vgui/IVGUI.h>
#include "ienginevgui.h"
#include "vgui_controls/label.h"
#include "clientmode_hlnormal.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// 7 didgits, 10 values, 14.29% per digit, 1.429% per value
#define C4_PERCENT_PER_DIGIT 14.2857142857142857143f
#define C4_PERCENT_PER_GUESS ( C4_PERCENT_PER_DIGIT / 10.0f )
#define C4_ARMING_CODE		 "7355608"
#define C4_CODE_MASK		 "*******"

//=============================================================================
// Purpose: In-game vgui panel which shows the C4's *s
//=============================================================================
class C4Panel : public CVGuiScreenPanel
{
	DECLARE_CLASS(C4Panel, CVGuiScreenPanel);

public:
	C4Panel(vgui::Panel *parent, const char *panelName) : BaseClass(parent, panelName, g_hVGuiC4Scheme) {}
	virtual bool Init(KeyValues* pKeyValues, VGuiScreenInitData_t* pInitData);
	virtual void OnTick();

private:
	char m_iszCode[8];
	vgui::Label *m_pLabel;
};

DECLARE_VGUI_SCREEN_FACTORY(C4Panel, "c4_panel");

//-----------------------------------------------------------------------------
// Purpose: Initialization
//-----------------------------------------------------------------------------
bool C4Panel::Init(KeyValues* pKeyValues, VGuiScreenInitData_t* pInitData)
{
	// Load all of the controls
	if (!BaseClass::Init(pKeyValues, pInitData))
		return false;

	// Make sure we get ticked...
	vgui::ivgui()->AddTickSignal(GetVPanel());

	m_pLabel = dynamic_cast <vgui::Label*> (FindChildByName("TimerLabel"));

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Update the label
//-----------------------------------------------------------------------------
void C4Panel::OnTick()
{
	BaseClass::OnTick();

	CBasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	// 7 didgits, 10 values, 14.29% per digit, 1.429% per value
	if (pPlayer->GetDefuseProgress())
	{
		float flDefused = pPlayer->GetDefuseProgress() * 100.0f; // GetDefuseProgress() returns percentage as 0.0 to 1.0

		Q_strcpy(m_iszCode, C4_CODE_MASK);
		if (m_pLabel)
		{
			int nDigit = flDefused / C4_PERCENT_PER_DIGIT; // How many digits already guessed, 14.29% per digit
			for (int i = 0; i < nDigit; i++)
			{
				m_iszCode[i] = C4_ARMING_CODE[i];
			}

			int nGuesses = (flDefused - (C4_PERCENT_PER_DIGIT * nDigit)) / C4_PERCENT_PER_GUESS; // How guesses already guessed, 1.429% per value
			int nGuess = ((C4_ARMING_CODE[nDigit] - '0') + nGuesses + 1) % 10; // Displayed guess (code digit+guesses). Stay under 10!

			m_iszCode[nDigit] = '0' + nGuess; // Char values of 0-9 are contiguous
			m_pLabel->SetText(m_iszCode);
		}
	}
	else
	{
		Q_strcpy(m_iszCode, C4_CODE_MASK);
		m_pLabel->SetText(m_iszCode);
	}
}
