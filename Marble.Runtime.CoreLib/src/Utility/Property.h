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
    struct Property
    {
        explicit Property(const std::tuple<skarupke::function<GetterReturnType()>, skarupke::function<void(SetterInputType)>>&& constructor) :
        getter(std::move(std::get<0>(constructor))), setter(std::move(std::get<1>(constructor)))
        {
        }
        Property(const Property<GetterReturnType, SetterInputType>& other) = delete;
        Property(Property<GetterReturnType, SetterInputType>&& other) = delete;

        auto operator=(SetterInputType value)
        {
            this->setter(value);
            return this->getter();
        }
        auto operator=(const Property<GetterReturnType, SetterInputType>& rhs) = delete;
        auto operator=(Property<GetterReturnType, SetterInputType>&&) = delete;

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
    private:
        skarupke::function<GetterReturnType()> getter = nullptr;
        skarupke::function<void(SetterInputType)> setter = nullptr;
    };
}
