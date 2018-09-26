////////////////////////////////////////////////////////////////////////////
//	Module 		: ai_stalker_alife.cpp
//	Created 	: 15.10.2004
//  Modified 	: 15.10.2004
//	Author		: Dmitriy Iassenev
//	Description : Stalker ALife functions
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ai/stalker/ai_stalker.h"
#include "ai_space.h"
#include "alife_simulator.h"
#include "../xrAICore/alife_space.h"
#include "inventory.h"
#include "pda.h"
#include "eatable_item.h"
#include "weapon.h"
#include "Grenade.h"
#include "customdetector.h"
#include "ef_storage.h"
#include "ef_primary.h"
#include "ef_pattern.h"
#include "trade_parameters.h"
#include "clsid_game.h"

static const int MAX_AMMO_ATTACH_COUNT = 1;
static const int enough_ammo_box_count = 1;

IC	bool CAI_Stalker::CTradeItem::operator<		(const CTradeItem &trade_item) const
{
	return			(m_item->object().ID() < trade_item.m_item->object().ID());
}

IC	bool CAI_Stalker::CTradeItem::operator==	(u16 id) const
{
	return			(m_item->object().ID() == id);
}

bool CAI_Stalker::tradable_item					(CInventoryItem *inventory_item, const u16 &current_owner_id)
{
	if (!inventory_item->useful_for_NPC())
		return			(false);

	if (CLSID_DEVICE_PDA == inventory_item->object().CLS_ID) {
		CPda			*pda = smart_cast<CPda*>(inventory_item);
		VERIFY			(pda);
		if (pda->GetOriginalOwnerID() == current_owner_id)
			return		(false);
	}

	return				(
		trade_parameters().enabled(
			CTradeParameters::action_sell(nullptr),
			inventory_item->object().cNameSect()
		)
	);
}

u32 CAI_Stalker::fill_items						(CInventory &inventory, CGameObject *old_owner, ALife::_OBJECT_ID new_owner_id)
{
	u32							result = 0;
	TIItemContainer::iterator	I = inventory.m_all.begin();
	TIItemContainer::iterator	E = inventory.m_all.end();
	for ( ; I != E; ++I) {
		if (!tradable_item(*I,old_owner->ID()))
			continue;
		
		m_temp_items.emplace_back	(*I,old_owner->ID(),new_owner_id);
		result					+= (*I)->Cost();
	}

	return						(result);
}

void CAI_Stalker::transfer_item					(CInventoryItem *item, CGameObject *old_owner, CGameObject *new_owner)
{
	NET_Packet			P;
	CGameObject			*O = old_owner;
	O->u_EventGen		(P,GE_TRADE_SELL,O->ID());
	P.w_u16				(u16(item->object().ID()));
	O->u_EventSend		(P);

	O					= new_owner;
	O->u_EventGen		(P,GE_TRADE_BUY,O->ID());
	P.w_u16				(u16(item->object().ID()));
	O->u_EventSend		(P);
}

IC	void CAI_Stalker::buy_item_virtual			(CTradeItem &item)
{
	item.m_new_owner_id			= ID();
	m_total_money				-= item.m_item->Cost();
	if (m_current_trader)
		m_current_trader->set_money(m_current_trader->get_money() + item.m_item->Cost(), true);
}

void CAI_Stalker::choose_food					()
{
	// stalker cannot change food due to the game design :-(((
}

void CAI_Stalker::attach_available_ammo			(CWeapon *weapon)
{
	if (!weapon || weapon->m_ammoTypes.empty())
		return;

	u32							count = 0;
	xr_vector<CTradeItem>::iterator	I = m_temp_items.begin();
	xr_vector<CTradeItem>::iterator	E = m_temp_items.end();
	for ( ; I != E; ++I) {
		if (m_total_money < (*I).m_item->Cost())
			continue;

		if (
			std::find(
				weapon->m_ammoTypes.begin(),
				weapon->m_ammoTypes.end(),
				(*I).m_item->object().cNameSect().c_str()
			) == 
			weapon->m_ammoTypes.end()
		)
			continue;

		buy_item_virtual			(*I);

		++count;
		if (count >= MAX_AMMO_ATTACH_COUNT)
			break;
	}
}

void CAI_Stalker::choose_weapon					(ALife::EWeaponPriorityType weapon_priority_type)
{
	CTradeItem						*best_weapon	= nullptr;
	float							best_value		= -1.f;
	ai().ef_storage().non_alife().member()	= this;

	xr_vector<CTradeItem>::iterator	I = m_temp_items.begin();
	xr_vector<CTradeItem>::iterator	E = m_temp_items.end();
	for ( ; I != E; ++I) {
		if (m_total_money < (*I).m_item->Cost())
			continue;

		ai().ef_storage().non_alife().member_item() = &(*I).m_item->object();

		int						j = ai().ef_storage().m_pfPersonalWeaponType->dwfGetWeaponType();
		float					current_value = -1.f;
		switch (weapon_priority_type) {
			case ALife::eWeaponPriorityTypeKnife : {
				if (1 != j)
					continue;
				current_value		= ai().ef_storage().m_pfItemValue->ffGetValue();
				if(I->m_item->m_ItemCurrPlace.type==eItemPlaceSlot)
					current_value += 10.0f;
				
				break;
			}
			case ALife::eWeaponPriorityTypeSecondary : {
				if (5 != j)
					continue;
				current_value		= ai().ef_storage().m_pfSmallWeaponValue->ffGetValue();
				if(I->m_item->m_ItemCurrPlace.type==eItemPlaceSlot)
					current_value += 10.0f;
				break;
			}
			case ALife::eWeaponPriorityTypePrimary : {
				if ((6 != j) && (7 != j) && (8 != j) && (9 != j) && (11 != j) && (12 != j))
					continue;
				current_value		= ai().ef_storage().m_pfMainWeaponValue->ffGetValue();
				if(I->m_item->m_ItemCurrPlace.type==eItemPlaceSlot)
					current_value += 10.0f;
				break;
			}
			case ALife::eWeaponPriorityTypeGrenade : {
				if (4 != j)
					continue;
				current_value		= ai().ef_storage().m_pfItemValue->ffGetValue();
				if(I->m_item->m_ItemCurrPlace.type==eItemPlaceSlot)
					current_value += 10.0f;
				break;
			}
			default : NODEFAULT;
		}
		
		if ((current_value > best_value)) {
			best_value	= current_value;
			best_weapon = &*I;
		}
	}
	if (best_weapon) {
		buy_item_virtual			(*best_weapon);
		attach_available_ammo		(smart_cast<CWeapon*>(best_weapon->m_item));
	}
}

void CAI_Stalker::choose_medikit				()
{
	// stalker cannot change medikit due to the game design :-(((
}

void CAI_Stalker::choose_detector				()
{
	CTradeItem					*best_detector	= nullptr;
	float						best_value		= -1.f;
	ai().ef_storage().non_alife().member()	= this;
	xr_vector<CTradeItem>::iterator	I = m_temp_items.begin();
	xr_vector<CTradeItem>::iterator	E = m_temp_items.end();
	for ( ; I != E; ++I) {
		if (m_total_money < (*I).m_item->Cost())
			continue;

		CCustomDetector			*detector = smart_cast<CCustomDetector*>((*I).m_item);
		if (!detector)
			continue;

		// evaluating item
		ai().ef_storage().non_alife().member_item() = detector;
		float					current_value = ai().ef_storage().m_pfDetectorType->ffGetValue();
		// choosing the best item
		if ((current_value > best_value)) {
			best_detector		= &*I;
			best_value			= current_value;
		}
	}
	if (best_detector)
		buy_item_virtual		(*best_detector);
}

void CAI_Stalker::choose_equipment				()
{
	// stalker cannot change their equipment due to the game design :-(((
}

void CAI_Stalker::select_items						()
{
	if (!m_can_select_items)
		return;

	choose_food			();
	choose_weapon		(ALife::eWeaponPriorityTypeKnife);
	choose_weapon		(ALife::eWeaponPriorityTypeSecondary);
	choose_weapon		(ALife::eWeaponPriorityTypePrimary);
	choose_weapon		(ALife::eWeaponPriorityTypeGrenade);
	choose_medikit		();
	choose_detector		();
	choose_equipment	();
}

void CAI_Stalker::update_sell_info()
{
	m_sell_info_actuality = true;
	m_temp_items.clear();
	m_current_trader = nullptr;
	m_total_money = get_money();
	m_total_money += fill_items(inventory(), this, ALife::_OBJECT_ID(-1));;
	std::sort(m_temp_items.begin(), m_temp_items.end());
	select_items();

	for (PIItem it : inventory().m_all)
	{
		if (!tradable_item(it, ID()))
			m_temp_items.emplace_back(it, ID(), ID());
	}
}

bool CAI_Stalker::can_sell							(CInventoryItem* item)
{
	if (READ_IF_EXISTS(pSettings, r_bool, cNameSect(), "is_trader", false))
		return				(tradable_item(item, ID()));

	update_sell_info		();
	xr_vector<CTradeItem>::const_iterator	I = std::find(m_temp_items.begin(),m_temp_items.end(),item->object().ID());
	VERIFY					(I != m_temp_items.end());
	return					((*I).m_new_owner_id != ID());
}

bool CAI_Stalker::AllowItemToTrade 					(CInventoryItem const * item, const SInvItemPlace& place) const
{
	if (!g_Alive())
		return				(trade_parameters().enabled(CTradeParameters::action_show(nullptr),item->object().cNameSect()));

	return					(const_cast<CAI_Stalker*>(this)->can_sell(const_cast<CInventoryItem*>(item)));
}

bool CAI_Stalker::non_conflicted					(const CInventoryItem *item, const CWeapon *new_weapon) const
{
	if (item == new_weapon)
		return				(true);

	const CWeapon			*weapon = smart_cast<const CWeapon*>(item);
	if (!weapon)
		return				(true);

	return					(weapon->ef_weapon_type() != new_weapon->ef_weapon_type());
}

bool CAI_Stalker::enough_ammo						(const CWeapon *new_weapon) const
{
	int						ammo_box_count = 0;

	TIItemContainer::const_iterator	I = inventory().m_all.begin();
	TIItemContainer::const_iterator	E = inventory().m_all.end();
	for ( ; I != E; ++I) {
		if (std::find(new_weapon->m_ammoTypes.begin(),new_weapon->m_ammoTypes.end(),(*I)->object().cNameSect().c_str()) == new_weapon->m_ammoTypes.end())
			continue;

		++ammo_box_count;
		if (ammo_box_count >= enough_ammo_box_count)
			return			(true);
	}

	return					(false);
}

void CAI_Stalker::remove_personal_only_ammo(const CInventoryItem *item)
{
	const CWeapon			*weapon = smart_cast<const CWeapon*>(item);
	VERIFY					(weapon);

	for (const shared_str& it: weapon->m_ammoTypes)
	{
		bool found = false;

		for (PIItem inv: inventory().m_all)
		{
			if (inv->object().ID() == weapon->ID())
				continue;

			const CWeapon	*temp = smart_cast<const CWeapon*>(inv);
			if (!temp)
				continue;

			if (std::find(temp->m_ammoTypes.begin(),temp->m_ammoTypes.end(), it) == temp->m_ammoTypes.end())
				continue;

			found = true;
			break;
		}

		if (found)
			continue;

		for (PIItem i: inventory().m_all)
		{
			if (strcmp(it.c_str(), i->object().cNameSect().c_str()))
				continue;

			NET_Packet		packet;
			u_EventGen		(packet,GE_DESTROY, i->object().ID());
			u_EventSend		(packet);
		}
	}
}

void CAI_Stalker::update_conflicted					(CInventoryItem *item, const CWeapon *new_weapon)
{
	if (non_conflicted(item,new_weapon))
		return;

	remove_personal_only_ammo	(item);
	item->SetDropManual			(TRUE);
}

void CAI_Stalker::on_after_take						(const CGameObject *object)
{
	if (!g_Alive())
		return;

	if (!READ_IF_EXISTS(pSettings,r_bool,cNameSect(),"use_single_item_rule",true))
		return;

	const CWeapon				*new_weapon = smart_cast<const CWeapon*>(object);
	if (!new_weapon)
		return;

	for (PIItem it: inventory().m_all)
		update_conflicted		(it, new_weapon);
}
