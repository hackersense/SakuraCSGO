#pragma once

#include <cstddef>
#include "..\math\Vector4D.hpp"
#include "..\math\Vector.hpp"
#include "IMaterialSystem.hpp"

class IVStudioRender
{
private:
	template <typename T, typename ... args_t>
	constexpr T CallVFunc(void* thisptr, std::size_t nIndex, args_t... argList)
	{
		using Fn = T(__thiscall*)(void*, decltype(argList)...);
		return (*static_cast<Fn**>(thisptr))[nIndex](thisptr, argList...);
	}
public:
	void SetAmbientLightColors(const Vector4D* pAmbientOnlyColors)
	{
		CallVFunc<void>(this, 20, pAmbientOnlyColors);
	}

	void SetAmbientLightColors(const Vector* pAmbientOnlyColors)
	{
		CallVFunc<void>(this, 21, pAmbientOnlyColors);
	}

	void SetLocalLights(int nLights, const void* pLights)
	{
		CallVFunc<void>(this, 22, nLights, pLights);
	}

	void SetColorModulation(float const* arrColor)
	{
		CallVFunc<void>(this, 27, arrColor);
	}

	void SetAlphaModulation(float flAlpha)
	{
		CallVFunc<void>(this, 28, flAlpha);
	}

	void ForcedMaterialOverride(IMaterial* pMaterial, OverrideType_t nOverrideType = 0, int nOverrides = 0)
	{
		CallVFunc<void>(this, 33, pMaterial, nOverrideType, nOverrides);
	}
};
