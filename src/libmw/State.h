#pragma once

#include <libmw/libmw.h>
#include <mw/common/BitSet.h>
#include <mw/models/block/Header.h>
#include <mw/models/tx/Kernel.h>
#include <mw/models/tx/UTXO.h>

MW_NAMESPACE

struct State
{
    static State Deserialize(Deserializer& deserializer)
    {
        const uint64_t num_utxos = deserializer.Read<uint64_t>();
        std::vector<UTXO::CPtr> utxos(num_utxos);
        for (uint64_t i = 0; i < num_utxos; i++) {
            utxos[i] = std::make_shared<UTXO>(UTXO::Deserialize(deserializer));
        }

        std::vector<Kernel> kernels = deserializer.ReadVec<Kernel>();
        std::vector<mw::Hash> output_parent_hashes = deserializer.ReadVec<mw::Hash>();
        std::vector<mw::Hash> rangeproof_parent_hashes = deserializer.ReadVec<mw::Hash>();
        BitSet leafset = deserializer.Read<BitSet>();
        std::vector<mw::Hash> pruned_parent_hashes = deserializer.ReadVec<mw::Hash>();

        return State{
            std::move(utxos),
            std::move(kernels),
            std::move(output_parent_hashes),
            std::move(rangeproof_parent_hashes),
            std::move(leafset),
            std::move(pruned_parent_hashes)
        };
    }

    std::vector<uint8_t> Serialized() const
    {
        return Serializer()
            .AppendVec(utxos)
            .AppendVec(kernels)
            .AppendVec(output_parent_hashes)
            .AppendVec(rangeproof_parent_hashes)
            .Append(leafset)
            .AppendVec(pruned_parent_hashes)
            .vec();
    }

    std::vector<UTXO::CPtr> utxos;
    std::vector<Kernel> kernels;
    std::vector<mw::Hash> output_parent_hashes;
    std::vector<mw::Hash> rangeproof_parent_hashes;
    BitSet leafset;
    std::vector<mw::Hash> pruned_parent_hashes;
};

END_NAMESPACE