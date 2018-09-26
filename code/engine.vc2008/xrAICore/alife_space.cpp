#include "stdafx.h"
#include "alife_space.h"

namespace ALife 
{
AI_API xr_token			hit_types_token		[ ]={
	{ "burn",			eHitTypeBurn			},
	{ "shock",			eHitTypeShock			},
	{ "strike",			eHitTypeStrike			},
	{ "wound",			eHitTypeWound			},
	{ "radiation",		eHitTypeRadiation		},
	{ "telepatic",		eHitTypeTelepatic		},
	{ "fire_wound",		eHitTypeFireWound		},
	{ "chemical_burn",	eHitTypeChemicalBurn	},
	{ "explosion",		eHitTypeExplosion		},
	{ "wound_2",		eHitTypeWound_2			},
	{ "light_burn",		eHitTypeLightBurn		},
	{ 0,				0						}
};

AI_API EHitType g_tfString2HitType(LPCSTR caHitType)
{
	if (!_stricmp(caHitType,"burn"))
		return(eHitTypeBurn);
	else if (!_stricmp(caHitType,"light_burn"))
		return(eHitTypeLightBurn);
	else if (!_stricmp(caHitType,"shock"))
			return(eHitTypeShock);
	else if (!_stricmp(caHitType,"strike"))
           	return(eHitTypeStrike);
	else if (!_stricmp(caHitType,"wound"))
			return(eHitTypeWound);
	else if (!_stricmp(caHitType,"radiation"))
			return(eHitTypeRadiation);
	else if (!_stricmp(caHitType,"telepatic"))
			return(eHitTypeTelepatic);
	else if (!_stricmp(caHitType,"fire_wound"))
			return(eHitTypeFireWound);
	else if (!_stricmp(caHitType,"chemical_burn"))
			return(eHitTypeChemicalBurn);
	else if (!_stricmp(caHitType,"explosion"))
			return(eHitTypeExplosion);
	else if (!_stricmp(caHitType,"wound_2"))
			return(eHitTypeWound_2);
	else
			FATAL	("Unsupported hit type!");
	NODEFAULT;
#ifdef DEBUG
		return(eHitTypeMax);
#endif
	}
};