#pragma once

#include "UtlMemory.hpp"

template <typename ElementType, typename IndexType>
struct CUtlRbTreeNode {
    IndexType left;
    IndexType right;
    IndexType parent;
    IndexType type;
    ElementType element;
};

template <typename ElementType, typename IndexType>
struct CUtlRbTree {
    bool isIndexUsed(IndexType index) const
    {
        return memory[index].left != index;
    }

    void* lessFunc;
    CUtlMemory<CUtlRbTreeNode<ElementType, IndexType>> memory;
    IndexType root;
    IndexType numElements;
    IndexType firstFree;
    IndexType lastAlloc;
    CUtlRbTreeNode<ElementType, IndexType>* elements;
};