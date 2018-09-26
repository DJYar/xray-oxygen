// hit_immunity.h: класс для тех объектов, которые поддерживают
//				   коэффициенты иммунитета для разных типов хитов
//////////////////////////////////////////////////////////////////////
#pragma once
#include "../xrServerEntities/../xrAICore/alife_space.h"

namespace HitImmunity
{
	using HitTypeSVec = svector<float, ALife::eHitTypeMax>;
};

class CHitImmunity
{
	//коэффициенты на которые домножается хит
	//при соответствующем типе воздействия
	//(для защитных костюмов и специфичных животных)
private:
	HitImmunity::HitTypeSVec m_HitImmunityKoefs;
public:
						CHitImmunity	();
	virtual				~CHitImmunity	();

			void		LoadImmunities	(LPCSTR section, CInifile* ini);
			void		AddImmunities	(LPCSTR section, CInifile* ini);
			float		GetHitImmunity	(ALife::EHitType hit_type) const				{return m_HitImmunityKoefs[hit_type];}
			float		AffectHit		(float power, ALife::EHitType hit_type) const	{return power*GetHitImmunity(hit_type);}
};