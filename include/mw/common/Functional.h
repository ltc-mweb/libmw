#pragma once

#include <tl/optional.hpp>
#include <functional>
#include <algorithm>
#include <memory>

template<class Container>
struct optional_back_insert_iterator : public std::iterator<std::output_iterator_tag, void, void, void, void>
{
    explicit optional_back_insert_iterator(Container& c) : container(std::addressof(c)) { }

    using value_type = typename Container::value_type;

    optional_back_insert_iterator<Container>& operator=(const tl::optional<value_type> opt)
    {
        if (opt.has_value())
        {
            container->push_back(std::move(opt.value()));
        }

        return *this;
    }

    optional_back_insert_iterator<Container>& operator*() { return *this; }
    optional_back_insert_iterator<Container>& operator++() { return *this; }
    optional_back_insert_iterator<Container>& operator++(int) { return *this; }

protected:
    Container* container;
};

template<class Container>
optional_back_insert_iterator<Container> optional_back_inserter(Container& container)
{
    return optional_back_insert_iterator<Container>(container);
}

template<class InputType, class OutputType>
std::vector<OutputType> transform_if(
    const std::vector<InputType>& inputs,
    const std::function<tl::optional<OutputType>(const InputType&)>& func)
{
    std::vector<OutputType> transformed;
    std::transform(
        inputs.cbegin(), inputs.cend(),
        optional_back_inserter(transformed),
        func
    );
    return transformed;
}

//return transform_if<IKernel::CPtr, Kernel::CPtr>(
//    m_pBlock->GetKernels(),
//    [](const auto& pKernel) -> tl::optional<Kernel::CPtr> {
//        return pKernel->IsCoinbase() ? tl::make_optional(std::dynamic_pointer_cast<const Kernel>(pKernel)) : tl::nullopt;
//    }
//);