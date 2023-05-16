#pragma once

#include <cstddef>

template<class T> struct UtlReference {
    UtlReference* next;
    UtlReference* prev;
    T* object;
};

enum AttributeDataType {
    ATTRDATATYPE_NONE = -1,
    ATTRDATATYPE_FLOAT = 0,
    ATTRDATATYPE_4V,
    ATTRDATATYPE_INT,
    ATTRDATATYPE_POINTER,

    ATTRDATATYPE_COUNT,
};

#define MAX_PARTICLE_ATTRIBUTES 24

#define DEFPARTICLE_ATTRIBUTE( name, bit, datatype )			\
	const int PARTICLE_ATTRIBUTE_##name##_MASK = (1 << bit);	\
	const int PARTICLE_ATTRIBUTE_##name = bit;					\
	const AttributeDataType PARTICLE_ATTRIBUTE_##name##_DATATYPE = datatype;

DEFPARTICLE_ATTRIBUTE(TINT_RGB, 6, ATTRDATATYPE_4V);

DEFPARTICLE_ATTRIBUTE(ALPHA, 7, ATTRDATATYPE_FLOAT);

struct ParticleAttributeAddressTable {
    float* attributes[MAX_PARTICLE_ATTRIBUTES];
    size_t floatStrides[MAX_PARTICLE_ATTRIBUTES];

    __forceinline float* FloatAttributePtr(int attribute, int particleNumber) const {
        int block_ofs = particleNumber / 4;
        return attributes[attribute] +
               floatStrides[attribute] * block_ofs +
               (particleNumber & 3);
    }

};

struct UtlStringSimple {
    char* buffer;
    int capacity;
    int growSize;
    int length;
};

class ParticleSystemDefinition {
    std::byte pad[308];
public:
    UtlStringSimple name;
};

class ParticleCollection {
    std::byte pad[48];//0
public:
    int activeParticles;//48
private:
    std::byte pad1[12];//52
public:
    UtlReference<ParticleSystemDefinition> def;//64
private:
    std::byte pad2[60];//80
public:
    ParticleCollection* parent;//136
private:
    std::byte pad3[84];//140
public:
    ParticleAttributeAddressTable particleAttributes;//224
};