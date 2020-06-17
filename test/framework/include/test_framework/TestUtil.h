#pragma once

#include <mw/common/Macros.h>
#include <mw/crypto/Random.h>
#include <mw/file/FilePath.h>

TEST_NAMESPACE

class TestUtil
{
public:
    static FilePath GetTempDir()
    {
        return FilePath(
            fs::temp_directory_path() / ".mimblewimble" / Random::CSPRNG<8>().GetBigInt().ToHex()
        );
    }
};

END_NAMESPACE