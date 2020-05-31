#include <mw/mmr/MMR.h>
#include <mw/mmr/backends/FileBackend.h>

using namespace mmr;

void MMR::Add(std::vector<uint8_t>&& data)
{
    const LeafIndex leafIdx = m_pBackend->GetNextLeaf();
    m_pBackend->AddLeaf(Leaf::Create(leafIdx, std::move(data)));
}

uint64_t MMR::GetNumNodes() const noexcept
{
    const uint64_t numLeaves = m_pBackend->GetNumLeaves();
    if (numLeaves == 0)
    {
        return 0;
    }

    return LeafIndex::At(numLeaves).GetPosition();
}

//
// Unlike a Merkle tree, a MMR generally has no single root so we need a method to compute one.
// The process we use is called "bagging the peaks." We first identify the peaks (nodes with no parents).
// We then "bag" them by hashing them iteratively from the right, using the total size of the MMR as prefix. 
//
Hash MMR::Root() const
{
    const uint64_t size = GetNumNodes();
    if (size == 0)
    {
        return ZERO_HASH;
    }

    // Find the "peaks"
    std::vector<uint64_t> peakIndices;

    uint64_t peakSize = BitUtil::FillOnesToRight(size);
    uint64_t numLeft = size;
    uint64_t sumPrevPeaks = 0;
    while (peakSize != 0)
    {
        if (numLeft >= peakSize)
        {
            peakIndices.push_back(sumPrevPeaks + peakSize - 1);
            sumPrevPeaks += peakSize;
            numLeft -= peakSize;
        }

        peakSize >>= 1;
    }

    assert(numLeft == 0);

    // Bag 'em
    Hash hash = ZERO_HASH;
    for (auto iter = peakIndices.crbegin(); iter != peakIndices.crend(); iter++)
    {
        Hash peakHash = m_pBackend->GetHash(Index::At(*iter));
        if (hash == ZERO_HASH)
        {
            hash = peakHash;
        }
        else
        {
            hash = Node::CreateParent(Index::At(size), peakHash, hash).GetHash();
        }
    }

    return hash;
}

void MMR::Rewind(const uint64_t numNodes)
{
    const Index nextIdx = Index::At(numNodes);
    assert(nextIndex.IsLeaf());

    m_pBackend->Rewind(LeafIndex(nextIdx.GetLeafIndex(), nextIdx.GetPosition()));
}