#pragma once

#include <cstddef>
#include "VirtualMethod.h"

template <typename Key, typename Value>
struct Node {
    int previousId;
    int nextId;
    void* _unknownPtr;
    int _unknown;
    Key key;
    Value value;
};

template <typename Key, typename Value>
struct Head {
    Node<Key, Value>* memory;
    int allocationCount;
    int growSize;
    int startElement;
    int nextAvailable;
    int _unknown;
    int lastElement;
};

struct String {
    char* buffer;
    int capacity;
    int growSize;
    int length;
};

struct PaintKit {
    int id;
    String name;
    String description;
    String itemName;
    String sameNameFamilyAggregate;
    String pattern;
    String normal;
    String logoMaterial;
    bool baseDiffuseOverride;
    int rarity;
    std::byte pad[40];
    float wearRemapMin;
    float wearRemapMax;
};

struct StickerKit {
    int id;
    int rarity;
    String name;
    String description;
    String itemName;
};

union AttributeDataUnion {
    float asFloat;
    std::uint32_t asUint32;
    char* asBlobPointer;
};

struct StaticAttrib {
    std::uint16_t defIndex;
    AttributeDataUnion value;
    bool forceGCToGenerate;
};
static_assert(sizeof(StaticAttrib) == 12);

struct EconTool {
    std::byte pad[sizeof(std::uintptr_t)];
    const char* typeName;
};

class EconItemDefinition {
public:
    VIRTUAL_METHOD(int, getWeaponId, 0, (), (this));
    VIRTUAL_METHOD(const char*, getItemBaseName, 2, (), (this));
    VIRTUAL_METHOD(const char*, getItemTypeName, 3, (), (this));
    VIRTUAL_METHOD(const char*, getInventoryImage, 5, (), (this));
    VIRTUAL_METHOD(const char*, getPlayerDisplayModel, 6, (), (this));
    VIRTUAL_METHOD(const char*, getWorldDisplayModel, 7, (), (this));
    VIRTUAL_METHOD(std::uint8_t, getRarity, 12, (), (this));
    VIRTUAL_METHOD_V(int, getNumberOfSupportedStickerSlots, 44, (), (this));

    std::uint8_t getQuality() noexcept
    {
        return *reinterpret_cast<std::uint8_t*>(std::uintptr_t(this) + 0x2B);
    }

    int getCapabilities() noexcept
    {
        return *reinterpret_cast<int*>(this + 0x148);
    }

    const char* getDefinitionName() noexcept
    {
        return *reinterpret_cast<const char**>(this + 0x1DC);
    }

    int getItemType() noexcept
    {
        return *reinterpret_cast<int*>(std::uintptr_t(this) + 0x130);
    }

    bool isServiceMedal() noexcept
    {
        return getItemType() == 5; /* prestige_coin */
    }

    bool isTournamentCoin() noexcept
    {
        return getItemType() == 8; /* fan_shield */
    }
};

struct ItemListEntry {
    int itemDef;
    int paintKit;
    int paintKitSeed;
    float paintKitWear;
    std::uint32_t stickerKit;
    std::uint32_t musicKit;
    bool isNestedList;
    bool isUnusualList;
    std::byte pad[2];

    auto weaponId() const noexcept
    {
        return itemDef;
    }
};

class EconLootListDefinition {
public:
    VIRTUAL_METHOD(const char*, getName, 0, (), (this));

    bool willProduceStatTrak() noexcept
    {
        return *reinterpret_cast<bool*>(std::uintptr_t(this) + 0x38);
    }
};

class EconItemSetDefinition {
public:
    VIRTUAL_METHOD(const char*, getLocKey, 1, (), (this));
    VIRTUAL_METHOD(int, getItemCount, 4, (), (this));
    VIRTUAL_METHOD(int, getItemDef, 5, (int index), (this, index));
    VIRTUAL_METHOD(int, getItemPaintKit, 6, (int index), (this, index));
};

struct EconItemQualityDefinition {
    int value;
    const char* name;
    unsigned weight;
    bool explicitMatchesOnly;
    bool canSupportSet;
    const char* hexColor;
};

struct AlternateIconData {
    String simpleName;
    String largeSimpleName;
    String iconURLSmall;
    String iconURLLarge;
    std::byte pad[28];
};

struct EconMusicDefinition {
    int id;
    const char* name;
    const char* nameLocalized;
    std::byte pad[2 * sizeof(const char*)];
    const char* inventoryImage;
};

class ItemSchema {
public:
    std::byte pad[0x88];
    Head<int, EconItemQualityDefinition> qualities;
    std::byte pad1[0x48];
    Head<int, EconItemDefinition*> itemsSorted;
    std::byte pad2[0x60];
    Head<int, const char*> revolvingLootLists;
    std::byte pad3[0x80];
    Head<std::uint64_t, AlternateIconData> alternateIcons;
    std::byte pad4[0x48];
    Head<int, PaintKit*> paintKits;
    Head<int, StickerKit*> stickerKits;
    std::byte pad5[0x11C];
    Head<int, EconMusicDefinition*> musicKits;

    VIRTUAL_METHOD(EconItemDefinition*, getItemDefinitionInterface, 4, (int id), (this, id));
};

class ItemSystem {
public:
    void* vmt;
    std::byte pad[0x28C];
    Head<int, PaintKit*> paintKits;
    std::byte pad1[0x8];
    Head<int, StickerKit*> stickerKits;

    VIRTUAL_METHOD(ItemSchema*, getItemSchema, 0, (), (this));
};