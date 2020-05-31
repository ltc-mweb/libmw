#pragma once

// Copyright (c) 2018-2019 David Burkett
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <memory>

namespace Traits
{
    class IBatchable
    {
    public:
        IBatchable() : m_dirty(false) { }
        virtual ~IBatchable() = default;

        virtual void Commit() = 0;
        virtual void Rollback() noexcept = 0;

        // This can be overridden
        virtual void OnInitWrite() noexcept { }

        // This can be overridden
        virtual void OnEndWrite() noexcept { }

        bool IsDirty() const noexcept { return m_dirty; }
        void SetDirty(const bool dirty) noexcept { m_dirty = dirty; }

    private:
        bool m_dirty;
    };
}