#pragma once

#include <mw/file/File.h>
#include <mw/crypto/Random.h>

class TestUtil
{
public:
    static File CreateTemp()
    {
        FilePath path(fs::temp_directory_path() / (Random::CSPRNG<2>().GetBigInt().ToHex() + ".tmp"));
        return File(std::move(path));
    }
};