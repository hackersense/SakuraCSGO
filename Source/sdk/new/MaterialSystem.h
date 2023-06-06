#pragma once

#include "../VirtualMethod.h"
#include "Material.h"

class KeyValues;
// class NEWMaterial;
class IMatRenderContext;

class MaterialSystem {
public:
    VIRTUAL_METHOD(NEWMaterial*, CreateMaterial, 83, (const char* materialName, KeyValues* keyValues), (this, materialName, keyValues))
    VIRTUAL_METHOD(NEWMaterial*, FindMaterial, 84, (const char* materialName, const char* textureGroupName = nullptr, bool complain = true, const char* complainPrefix = nullptr), (this, materialName, textureGroupName, complain, complainPrefix))
    VIRTUAL_METHOD(short, FirstMaterial, 86, (), (this))
    VIRTUAL_METHOD(short, NextMaterial, 87, (short handle), (this, handle))
    VIRTUAL_METHOD(short, InvalidMaterial, 88, (), (this))
    VIRTUAL_METHOD(NEWMaterial*, GetMaterial, 89, (short handle), (this, handle))
    VIRTUAL_METHOD(void, BeginRenderTargetAllocation, 94, (), (this))
    VIRTUAL_METHOD(void, EndRenderTargetAllocation, 95, (), (this))
    VIRTUAL_METHOD(IMatRenderContext*, GetRenderContext, 115, (), (this))
    VIRTUAL_METHOD(void, FinishRenderTargetAllocation, 136, (), (this))
};