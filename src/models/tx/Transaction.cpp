#include <mw/models/tx/Transaction.h>
#include <mw/consensus/BlockSumValidator.h>

void mw::Transaction::Validate() const
{
    m_body.Validate();

    BlockSumValidator::ValidateForTx(*this);
}