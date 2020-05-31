#pragma once

#include <mw/common/Macros.h>
#include <mw/models/tx/Transaction.h>

TEST_NAMESPACE

class Tx
{
public:

    const Transaction::CPtr& GetTransaction() const noexcept { return m_pTransaction; }

private:
    Transaction::CPtr m_pTransaction;
};

END_NAMESPACE