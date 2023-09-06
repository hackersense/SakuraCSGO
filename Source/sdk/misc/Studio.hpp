#pragma once

#include "../math/Vector.hpp"
#include "../math/VMatrix.hpp"
#include "../sdk/Memory.h"

typedef float RadianEuler[3];

#define MAX_QPATH						260

#define BONE_CALCULATE_MASK             0x1F
#define BONE_PHYSICALLY_SIMULATED       0x01    // bone is physically simulated when physics are active
#define BONE_PHYSICS_PROCEDURAL         0x02    // procedural when physics is active
#define BONE_ALWAYS_PROCEDURAL          0x04    // bone is always procedurally animated
#define BONE_SCREEN_ALIGN_SPHERE        0x08    // bone aligns to the screen, not constrained in motion.
#define BONE_SCREEN_ALIGN_CYLINDER      0x10    // bone aligns to the screen, constrained by it's own axis.

#define BONE_USED_BY_ANYTHING           0x0007FF00
#define BONE_USED_BY_HITBOX             0x00000100    // bone (or child) is used by a hit box
#define BONE_USED_BY_ATTACHMENT         0x00000200    // bone (or child) is used by an attachment point
#define BONE_USED_BY_VERTEX_MASK        0x0003FC00
#define BONE_USED_BY_VERTEX_LOD0        0x00000400    // bone (or child) is used by the toplevel model via skinned vertex
#define BONE_USED_BY_VERTEX_LOD1        0x00000800    
#define BONE_USED_BY_VERTEX_LOD2        0x00001000  
#define BONE_USED_BY_VERTEX_LOD3        0x00002000
#define BONE_USED_BY_VERTEX_LOD4        0x00004000
#define BONE_USED_BY_VERTEX_LOD5        0x00008000
#define BONE_USED_BY_VERTEX_LOD6        0x00010000
#define BONE_USED_BY_VERTEX_LOD7        0x00020000
#define BONE_USED_BY_BONE_MERGE         0x00040000    // bone is available for bone merge to occur against it

#define BONE_USED_BY_VERTEX_AT_LOD(lod) (BONE_USED_BY_VERTEX_LOD0 << (lod))
#define BONE_USED_BY_ANYTHING_AT_LOD(lod) ( BONE_USED_BY_ANYTHING & ~BONE_USED_BY_VERTEX_MASK) | BONE_USED_BY_VERTEX_AT_LOD(lod))

#define MAX_NUM_LODS					8
#define MAXSTUDIOBONES					256		// total bones actually used

#define BONE_TYPE_MASK                  0x00F00000
#define BONE_FIXED_ALIGNMENT            0x00100000    // bone can't spin 360 degrees, all interpolation is normalized around a fixed orientation

#define BONE_HAS_SAVEFRAME_POS          0x00200000    // Vector48
#define BONE_HAS_SAVEFRAME_ROT64        0x00400000    // Quaternion64
#define BONE_HAS_SAVEFRAME_ROT32        0x00800000    // Quaternion32

#define Assert(_exp) ((void)0)

#define HITGROUP_GENERIC 0
#define HITGROUP_HEAD 1
#define HITGROUP_CHEST 2
#define HITGROUP_STOMACH 3
#define HITGROUP_LEFTARM 4    
#define HITGROUP_RIGHTARM 5
#define HITGROUP_LEFTLEG 6
#define HITGROUP_RIGHTLEG 7
#define HITGROUP_NECK 8
#define HITGROUP_GEAR 10

enum Hitboxes
{
	HITBOX_HEAD = 0,
	HITBOX_NECK,
	HITBOX_PELVIS,
	HITBOX_STOMACH,
	HITBOX_LOWER_CHEST,
	HITBOX_CHEST,
	HITBOX_UPPER_CHEST,
	HITBOX_RIGHT_THIGH,
	HITBOX_LEFT_THIGH,
	HITBOX_RIGHT_CALF,
	HITBOX_LEFT_CALF,
	HITBOX_RIGHT_FOOT,
	HITBOX_LEFT_FOOT,
	HITBOX_RIGHT_HAND,
	HITBOX_LEFT_HAND,
	HITBOX_RIGHT_UPPER_ARM,
	HITBOX_RIGHT_LOWER_ARM,
	HITBOX_LEFT_UPPER_ARM,
	HITBOX_LEFT_LOWER_ARM,
	HITBOX_MAX
};

typedef unsigned short MDLHandle_t;

struct mstudiobone_t
{
	int sznameindex;

	int parent;
	int bonecontroller[6];

	Vector pos;
	Quaternion quat;
	RadianEuler rot;

	Vector posscale;
	Vector rotscale;

	matrix3x4_t poseToBone;
	Quaternion qAlignment;
	int flags;
	int proctype;
	int procindex;
	mutable int physicsbone;

	int surfacepropidx;

	int contents;
	int surfacepropLookup;
	int unused[7];

	const char* getName()
	{
		return ((char*)this) + sznameindex;
	}

	void* getProcedure()
	{
		if (procindex == 0)
			return nullptr;
		return (void*)(((unsigned char*)this) + procindex);
	}

	const char* pszSurfaceProp()
	{
		return ((char*)this) + surfacepropidx;
	}

	int GetSurfaceProp()
	{
		return surfacepropLookup;
	}
};

struct mstudiobbox_t
{
	int		bone;
	int		group;
	Vector	bbmin;
	Vector	bbmax;
	int		hitboxnameindex;
	Vector	rotation;
	float	radius;
	int		unused[4];
};

struct mstudiohitboxset_t
{
	int    sznameindex;
	int    numhitboxes;
	int    hitboxindex;

	const char* getName()
	{
		return ((char*)this) + sznameindex;
	}

	mstudiobbox_t* getHitbox(int i)
	{
		return (mstudiobbox_t*)(((unsigned char*)this) + hitboxindex) + i;
	}
};

struct model_t
{
	char name[255];
};

struct StudioSeqdesc
{
	std::byte		pad[104];		// 0
	float			fadeInTime;		// 104
	float			fadeOutTime;	// 108
	std::byte		pad1[84];		// 112
	int				numAnimTags;	// 196
};

class StudioHdr
{
public:
	int id;
	int version;
	int checksum;
	char name[64];
	int length;
	Vector eyePosition;
	Vector illumPosition;
	Vector hullMin;
	Vector hullMax;
	Vector bbMin;
	Vector bbMax;
	int flags;
	int numBones;
	int boneIndex;
	int numBoneControllers;
	int boneControllerIndex;
	int numHitboxSets;
	int hitboxSetIndex;
	int numLocalAnim;
	int localAnimIndex;
	int numLocalSeq;

	const char* getName()
	{
		return name;
	}

	mstudiohitboxset_t* getHitboxSet(int i)
	{
		if (i > numHitboxSets)
			return nullptr;
		return reinterpret_cast<mstudiohitboxset_t*>(std::uintptr_t(this) + hitboxSetIndex) + i;
	}

	mstudiobone_t* getBone(int i)
	{
		if (i > numBones)
			return nullptr;
		return reinterpret_cast<mstudiobone_t*>(std::uintptr_t(this) + boneIndex) + i;
	}
};

class CStudioHdr
{
public:
	StudioHdr*						studioHdr;
	void*							virtualModel;
	void*							softBody;
	CUtlVector<const StudioHdr*>	hdrCache;
	int								numFrameUnlockCounter;
	int*							frameUnlockCounter;
	std::byte						pad[8];
	CUtlVector<int>					boneFlags;
	CUtlVector<int>					boneParent;
	void*							activityToSequence;

	StudioSeqdesc getSeqdesc(int sequence)
	{
		return *reinterpret_cast<StudioSeqdesc*>(memory->seqdesc(this, sequence));
	}
};