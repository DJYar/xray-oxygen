////////////////////////////////////////////////////////////////////////////
//	Module 		: level_changer.h
//	Created 	: 10.07.2003
//  Modified 	: 10.07.2003
//	Author		: Dmitriy Iassenev
//	Description : Level change object
////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GameObject.h"
#include "../xrEngine/feel_touch.h"
#include "../xrAICore/game_graph_space.h"
#include "../xrUICore/UIDialogWnd.h"
#include "../xrUICore/UIMessageBox.h"

class CUIDialogWnd;

class CLevelChanger : public CGameObject, public Feel::Touch
{
private:
	using inherited =	CGameObject;

	GameGraph::_GRAPH_ID	m_game_vertex_id;
	u32						m_level_vertex_id;
	Fvector					m_position;
	Fvector					m_angles;
	float					m_entrance_time;
	shared_str				m_invite_str;
	bool					m_b_enabled;
	bool					m_bSilentMode;

	void					update_actor_invitation			();
	bool					get_reject_pos					(Fvector& p, Fvector& r);

public:
	virtual					~CLevelChanger					();
	virtual BOOL			net_Spawn						(CSE_Abstract* DC);
	virtual void			net_Destroy						();
	virtual void			Center							(Fvector& C) const;
	virtual float			Radius							() const;
	virtual void			shedule_Update					(u32 dt);
	virtual void			feel_touch_new					(CObject* O);
	virtual BOOL			feel_touch_contact				(CObject* O);

	virtual bool			IsVisibleForZones				()			 { return false; }
		    void			EnableLevelChanger				(bool b)	 { m_b_enabled = b; }
			bool			IsLevelChangerEnabled			() const	 { return m_b_enabled; }
			void			SetLEvelChangerInvitationStr	(LPCSTR str) { m_invite_str = str; }

	//serialization
	virtual BOOL			net_SaveRelevant				();
	virtual void			save							(NET_Packet &output_packet);
	virtual void			load							(IReader &input_packet);
};

class CChangeLevelWnd : public CUIDialogWnd
{
	CUIMessageBox* m_messageBox;
	using inherited = CUIDialogWnd;
	void OnCancel();
	void OnOk();

public:
	GameGraph::_GRAPH_ID m_game_vertex_id;
	u32			m_level_vertex_id;
	Fvector		m_position;
	Fvector		m_angles;
	Fvector		m_position_cancel;
	Fvector		m_angles_cancel;
	bool		m_b_position_cancel;
	bool		m_b_allow_change_level;
	shared_str  m_message_str;

	CChangeLevelWnd();
	virtual ~CChangeLevelWnd() {};
	virtual void SendMessage(CUIWindow *pWnd, s16 msg, void *pData);
	virtual bool WorkInPause()const { return true; }
	virtual void ShowDialog(bool bDoHideIndicators);
	virtual void HideDialog();
	virtual bool OnKeyboardAction(int dik, EUIMessages keyboard_action);
};