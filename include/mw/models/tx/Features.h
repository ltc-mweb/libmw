#pragma once
#pragma warning(disable: 4505) // Unreferenced local function has been removed

// Copyright (c) 2018-2019 David Burkett
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

#include <string>
#include <mw/exceptions/DeserializationException.h>

class Features
{
public:
    Features(const uint8_t features) noexcept : m_features(features) { }

    bool IsSet(const uint8_t feature) const noexcept { return (m_features & feature) == feature; }
    uint8_t Get() const noexcept { return m_features; }

private:
    uint8_t m_features;
};

enum EOutputFeatures : uint8_t
{
    // No Flags
    DEFAULT_OUTPUT = 0,

    // Output is a pegged-in output, must not be spent until maturity
    PEGGED_IN = 1
};

namespace OutputFeatures
{
    static std::string ToString(const EOutputFeatures& features) noexcept
    {
        switch (features)
        {
            case DEFAULT_OUTPUT:
                return "Plain";
            case PEGGED_IN:
                return "PeggedIn";
        }

        return "";
    }

    static EOutputFeatures FromString(const std::string& string)
    {
        if (string == "Plain")
        {
            return EOutputFeatures::DEFAULT_OUTPUT;
        }
        else if (string == "PeggedIn")
        {
            return EOutputFeatures::PEGGED_IN;
        }

        ThrowDeserialization_F("Failed to deserialize output feature: {}", string);
    }
}

//enum EKernelFeatures : uint8_t
//{
//    // No flags
//    DEFAULT_KERNEL = 0,
//
//    // Kernel matching a coinbase output
//    COINBASE_KERNEL = 1,
//
//    // Absolute height locked kernel; has fee and lock_height
//    HEIGHT_LOCKED = 2
//};
//
//namespace KernelFeatures
//{
//    static std::string ToString(const EKernelFeatures& features) noexcept
//    {
//        switch (features)
//        {
//            case DEFAULT_KERNEL:
//                return "Plain";
//            case COINBASE_KERNEL:
//                return "Coinbase";
//            case HEIGHT_LOCKED:
//                return "HeightLocked";
//        }
//
//        return "";
//    }
//
//    static EKernelFeatures FromString(const std::string& string)
//    {
//        if (string == "Plain")
//        {
//            return EKernelFeatures::DEFAULT_KERNEL;
//        }
//        else if (string == "Coinbase")
//        {
//            return EKernelFeatures::COINBASE_KERNEL;
//        }
//        else if (string == "HeightLocked")
//        {
//            return EKernelFeatures::HEIGHT_LOCKED;
//        }
//
//        ThrowDeserialization_F("Failed to deserialize kernel feature: {}", string);
//    }
//}