#pragma once

#include <mw/traits/Jsonable.h>
#include <cstdint>

class ScryptParameters : public Traits::IJsonable
{
public:
    uint32_t N;
    uint32_t r;
    uint32_t p;

    ScryptParameters(const uint32_t N_, const uint32_t r_, const uint32_t p_) : N(N_), r(r_), p(p_) { }
    virtual ~ScryptParameters() = default;

    json ToJSON() const noexcept final
    {
        return json({
            {"N", N},
            {"r", r},
            {"p", p}
        });
    }

    static ScryptParameters FromJSON(const Json& json)
    {
        return ScryptParameters(
            json.GetRequired<uint32_t>("N"),
            json.GetRequired<uint32_t>("r"),
            json.GetRequired<uint32_t>("p")
        );
    }
};