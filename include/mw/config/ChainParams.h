#pragma once

#include <mw/common/Macros.h>
#include <string>

MW_NAMESPACE

class ChainParams
{
public:
    static void Initialize(const std::string& hrp)
    {
        s_hrp = hrp;
    }

    static const std::string& GetHRP() { return s_hrp; }

private:
    inline static std::string s_hrp = "tmwltc";
};

END_NAMESPACE