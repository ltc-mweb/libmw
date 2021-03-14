#pragma once

#include <mw/mmr/Index.h>
#include <mw/mmr/LeafIndex.h>
#include <mw/util/BitUtil.h>
#include <boost/dynamic_bitset.hpp>
#include <cmath>

MMR_NAMESPACE

struct SiblingIter
{
public:
    SiblingIter(const uint64_t height, const mmr::Index& last_node)
        : m_height(height), m_lastNode(last_node), m_baseInc((uint64_t)std::pow(2, height + 1) - 1), m_siblingNum(0), m_next()
    {

    }

    bool Next()
    {
        if (m_siblingNum == 0) {
            m_next = mmr::Index(m_baseInc - 1, m_height);
        } else {
            uint64_t increment = m_baseInc + BitUtil::CountRightmostZeros(m_siblingNum);
            m_next = mmr::Index(m_next.GetPosition() + increment, m_height);
        }

        ++m_siblingNum;
        return m_next <= m_lastNode;
    }

    const mmr::Index& Get() const noexcept { return m_next; }
    const uint64_t GetPosition() const noexcept { return m_next.GetPosition(); }

private:
    uint64_t m_height;
    mmr::Index m_lastNode;
    uint64_t m_baseInc;

    uint64_t m_siblingNum;
    mmr::Index m_next;
};

class MMRUtil
{
public:
    static void BuildCompactBitSet(
        const uint64_t num_leaves,
        const boost::dynamic_bitset<>& unspent_leaf_indices,
        boost::dynamic_bitset<>& compactable_node_indices)
    {
        compactable_node_indices = boost::dynamic_bitset<>(num_leaves * 2);
        boost::dynamic_bitset<> prunable_nodes(num_leaves * 2);

        mmr::LeafIndex leaf_idx = mmr::LeafIndex::At(0);
        while (leaf_idx.GetLeafIndex() <= num_leaves) {
            if (unspent_leaf_indices.size() > leaf_idx.GetLeafIndex() && !unspent_leaf_indices.test(leaf_idx.GetLeafIndex())) {
                prunable_nodes.set(leaf_idx.GetPosition());
            }

            leaf_idx = leaf_idx.Next();
        }

        mmr::Index last_node = mmr::Index::At(mmr::LeafIndex::At(num_leaves).GetPosition());

        uint64_t height = 1;
        while ((std::pow(2, height + 1) - 2) <= last_node.GetPosition()) {
            SiblingIter iter(height, last_node);
            while (iter.Next()) {
                mmr::Index right_child = iter.Get().GetRightChild();
                if (prunable_nodes.test(right_child.GetPosition())) {
                    mmr::Index left_child = iter.Get().GetLeftChild();
                    if (prunable_nodes.test(left_child.GetPosition())) {
                        compactable_node_indices.set(right_child.GetPosition());
                        compactable_node_indices.set(left_child.GetPosition());
                        prunable_nodes.set(iter.Get().GetPosition());
                    }
                }
            }

            ++height;
        }
    }

    static boost::dynamic_bitset<> DiffCompactBitSet(
        const boost::dynamic_bitset<>& prev_compact,
        const boost::dynamic_bitset<>& new_compact)
    {
        boost::dynamic_bitset<> diff;

        for (size_t i = 0; i < new_compact.size(); i++) {
            if (prev_compact.size() > i && prev_compact.test(i)) {
                assert(new_compact.test(i));
                continue;
            }

            diff.push_back(new_compact.test(i));
        }

        return diff;
    }
};

END_NAMESPACE