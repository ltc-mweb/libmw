#pragma once

#include <mw/exceptions/LTCException.h>
#include <mw/util/StringUtil.h>

#define ThrowValidation(type) throw ValidationException(type, __FUNCTION__)

enum class EConsensusError
{
    CUT_THROUGH,
    BLOCK_WEIGHT,
    KERNEL_SUMS,
    KERNEL_SIG,
    BULLETPROOF,
    PEGIN_MISMATCH,
    PEGOUT_MISMATCH
};

class ValidationException : public LTCException
{
public:
    ValidationException(const EConsensusError& type, const std::string& function)
        : LTCException("ValidationException", GetMessage(type), function)
    {

    }

private:
    static std::string GetMessage(const EConsensusError& type)
    {
        return StringUtil::Format("Consensus Error: {}", ToString(type));
    }

    static std::string ToString(const EConsensusError& type)
    {
        switch (type)
        {
            case EConsensusError::CUT_THROUGH:
                return "CUT_THROUGH";
            case EConsensusError::BLOCK_WEIGHT:
                return "BLOCK_WEIGHT";
            case EConsensusError::KERNEL_SUMS:
                return "KERNEL_SUMS";
            case EConsensusError::KERNEL_SIG:
                return "KERNEL_SIG";
            case EConsensusError::BULLETPROOF:
                return "BULLETPROOF";
            case EConsensusError::PEGIN_MISMATCH:
                return "PEGIN_MISMATCH";
            case EConsensusError::PEGOUT_MISMATCH:
                return "PEGOUT_MISMATCH";
        }

        return "UNKNOWN";
    }
};