#pragma once

#include <tuple>

#include "../VirtualMethod.h"

class NEWMaterialVar {
public:
    VIRTUAL_METHOD(void, SetValue, 4, (float value), (this, value))
    VIRTUAL_METHOD(void, SetVectorValue, 11, (float x, float y, float z), (this, x, y, z))
    VIRTUAL_METHOD(void, SetVecComponentValue, 26, (float value, int component), (this, value, component))
};

class NEWMaterial {
public:
    VIRTUAL_METHOD(const char*, GetName, 0, (), (this))
    VIRTUAL_METHOD(const char*, GetTextureGroupName, 1, (), (this))
    VIRTUAL_METHOD(NEWMaterialVar*, FindVar, 11, (const char* name, bool* found = nullptr, bool complain = true), (this, name, found, complain))
    VIRTUAL_METHOD(void, IncrementReferenceCount, 12, (), (this))
    VIRTUAL_METHOD(void, DecrementReferenceCount, 13, (), (this))
    VIRTUAL_METHOD(void, AlphaModulate, 27, (float alpha), (this, alpha))
    VIRTUAL_METHOD(void, ColorModulate, 28, (const std::array<float, 3>& color), (this, color[0], color[1], color[2]))
    VIRTUAL_METHOD(void, ColorModulate, 28, (float r, float g, float b), (this, r, g, b))
    VIRTUAL_METHOD(void, ColorModulate, 28, (const std::tuple<float, float, float>& color), (this, std::get<0>(color), std::get<1>(color), std::get<2>(color)))
    VIRTUAL_METHOD(void, SetMaterialVarFlag, 29, (MaterialVarFlags_t flag, bool on), (this, flag, on))
    VIRTUAL_METHOD(bool, GetMaterialVarFlag, 30, (MaterialVarFlags_t flag), (this, flag))
    VIRTUAL_METHOD(void, Refresh, 37, (), (this))
    VIRTUAL_METHOD(bool, IsErrorMaterial, 42, (), (this))
    VIRTUAL_METHOD(float, GetAlphaModulation, 44, (), (this))
    VIRTUAL_METHOD(void, GetColorModulation, 45, (float* r, float* g, float* b), (this, r, g, b))
    VIRTUAL_METHOD(int, GetReferenceCount, 56, (), (this))
    VIRTUAL_METHOD(bool, IsPrecached, 70, (), (this))
};