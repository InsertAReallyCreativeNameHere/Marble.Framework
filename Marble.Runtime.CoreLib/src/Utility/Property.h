#pragma once

#include <inc.h>
#include <Utility/Function.h>

namespace Marble
{
    template
    <
        typename GetterReturnType,
        typename SetterInputType
    >
    struct coreapi Property
    {
        explicit Property(const auto& getter, const auto& setter) :
        getter(getter), setter(setter)
        {
        }
        explicit Property(const Property<GetterReturnType, SetterInputType>& other)
        {
            this->setter(other.getter());
        }
        Property(Property<GetterReturnType, SetterInputType>&& property) = delete;
        
        auto operator=(const Property<GetterReturnType, SetterInputType>& rhs)
        {
            this->setter(rhs.getter());
            return this->getter();
        }
        auto operator=(Property<GetterReturnType, SetterInputType>&&) = delete;
        auto operator=(SetterInputType value)
        {
            this->setter(value);
            return this->getter();
        }
        
        #pragma region Arithmetic
        auto operator+(SetterInputType rhs)
        {
            return (this->getter() + rhs);
        }
        auto operator-(SetterInputType rhs)
        {
            return (this->getter() - rhs);
        }
        auto operator*(SetterInputType rhs)
        {
            return (this->getter() * rhs);
        }
        auto operator/(SetterInputType rhs)
        {
            return (this->getter() / rhs);
        }
        auto operator%(SetterInputType rhs)
        {
            return (this->getter() % rhs);
        }
        
        auto operator++()
        {
            return (this->getter()++);
        }
        auto operator--()
        {
            return (this->getter()--);
        }
        #pragma endregion

        #pragma region Assignment
        auto operator+=(SetterInputType rhs)
        {
            GetterReturnType type = this->getter();
            type += rhs;
            this->setter((SetterInputType)type);
            return this->getter();
        }
        auto operator-=(SetterInputType rhs)
        {
            GetterReturnType type = this->getter();
            type -= rhs;
            this->setter((SetterInputType)type);
            return this->getter();
        }
        auto operator*=(SetterInputType rhs)
        {
            GetterReturnType type = this->getter();
            type *= rhs;
            this->setter((SetterInputType)type);
            return this->getter();
        }
        auto operator/=(SetterInputType rhs)
        {
            GetterReturnType type = this->getter();
            type /= rhs;
            this->setter((SetterInputType)type);
            return this->getter();
        }
        auto operator%=(SetterInputType rhs)
        {
            GetterReturnType type = this->getter();
            type %= rhs;
            this->setter((SetterInputType)type);
            return this->getter();
        }
        #pragma endregion

        #pragma region Comparison
        auto operator==(SetterInputType rhs)
        {
            return this->getter() == rhs;
        }
        auto operator!=(SetterInputType rhs)
        {
            return this->getter() != rhs;
        }
        auto operator>=(SetterInputType rhs)
        {
            return this->getter() >= rhs;
        }
        auto operator<=(SetterInputType rhs)
        {
            return this->getter() <= rhs;
        }
        auto operator>(SetterInputType rhs)
        {
            return this->getter() > rhs;
        }
        auto operator<(SetterInputType rhs)
        {
            return this->getter() < rhs;
        }
        #pragma endregion

        auto operator()()
        {
            return this->getter();
        }
        operator auto ()
        {
            return this->getter();
        }
    protected:
        skarupke::function<GetterReturnType()> getter = nullptr;
        skarupke::function<void(SetterInputType)> setter = nullptr;
    };
}
