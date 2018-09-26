#pragma once
#include "..\..\xrUICore\UIWindow.h"
#include "..\..\xrServerEntities\../xrAICore/alife_space.h"
#include "..\..\xrServerEntities\inventory_space.h"
#include "..\actor_defs.h"

class CUIStatic;
class CUITextWnd;
class CUIProgressBar;
class CUIProgressShape;
class CUIXml;
class UI_Arrow;
class CActor;

int const it_max = ALife::infl_max_count - 1;

class CUIHudStatesWnd : public CUIWindow
{
private:
	typedef CUIWindow inherited;

	CUIStatic*			m_back;

	CUIStatic*			m_indik[it_max];

	CUITextWnd*			m_ui_weapon_cur_ammo;
	CUITextWnd*			m_ui_weapon_fmj_ammo;
	CUITextWnd*			m_ui_weapon_ap_ammo;
	CUITextWnd*			m_fire_mode;
	CUITextWnd*			m_ui_grenade;
	II_BriefInfo		m_item_info;
	
	CUIStatic*			m_ui_weapon_icon;
	Frect				m_ui_weapon_icon_rect;

	CUIProgressBar*		m_ui_health_bar;
	CUIProgressBar*		m_ui_stamina_bar;
	CUIProgressShape*	m_health_shape;
	CUIProgressShape*	m_stamina_shape;

	CUIProgressShape*	m_rad_shape;
	UI_Arrow*			m_arrow_rad;
	UI_Arrow*			m_arrow_rad_shadow;	
	CUIStatic*			m_radia_damage;

	float				m_health_self;
	float				m_stamina_self;
	float				m_last_health;
	float				m_health_blink;

	float				m_radia_self;
	float				m_radia_hit;
	shared_str			m_lanim_name;
	
	float				m_zone_cur_power[ALife::infl_max_count];
	float				m_zone_feel_radius[ALife::infl_max_count ];
	ALife::EHitType		m_zone_hit_type[ALife::infl_max_count ];
	float				m_zone_threshold[ALife::infl_max_count ];

	float				m_zone_feel_radius_max;
	u32					m_timer_1sec;
	
	bool				m_fake_indicators_update;
	bool				m_b_force_update;
public:
					CUIHudStatesWnd		();
	virtual			~CUIHudStatesWnd	();

			void	InitFromXml			( CUIXml& xml, LPCSTR path );
			void	Load_section		();
	virtual void	Update				();

			void	on_connected		();
			void	reset_ui			();
			void	UpdateHealth		( CActor* actor );
			void	SetAmmoIcon			( const shared_str& sect_name );
			void	UpdateActiveItemInfo( CActor* actor );

			void 	UpdateZones			();
			void	UpdateIndicators	( CActor* actor );

			float	get_zone_cur_power	( ALife::EHitType hit_type );
			float	get_main_sensor_value()	{ return m_radia_hit; }

			void	DrawZoneIndicators	();
			void	FakeUpdateIndicatorType(u8 t, float power);
			void	EnableFakeIndicators(bool enable);
protected:
	static	ALife::EInfluenceType	get_indik_type( ALife::EHitType hit_type );

			void	Load_section_type	( ALife::EInfluenceType type, LPCSTR section );
			void	UpdateIndicatorType	( CActor* actor, ALife::EInfluenceType type );

}; // class CUIHudStatesWnd
