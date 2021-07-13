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

        inline GetterReturnType operator=(SetterInputType value)
        {
            this->setter(value);
            return this->getter();
        }
        inline GetterReturnType operator=(const Property<GetterReturnType, SetterInputType>& rhs) = delete;
        inline GetterReturnType operator=(Property<GetterReturnType, SetterInputType>&&) = delete;

        #pragma region Arithmetic
        inline GetterReturnType operator+(SetterInputType rhs)
        {
            return (this->getter() + rhs);
        }
        inline GetterReturnType operator-(SetterInputType rhs)
        {
            return (this->getter() - rhs);
        }
        inline GetterReturnType operator*(SetterInputType rhs)
        {
            return (this->getter() * rhs);
        }
        inline GetterReturnType operator/(SetterInputType rhs)
        {
            return (this->getter() / rhs);
        }
        inline GetterReturnType operator%(SetterInputType rhs)
        {
            return (this->getter() % rhs);
        }
        
        inline GetterReturnType operator++()
        {
            return (this->getter()++);
        }
        inline GetterReturnType operator--()
        {
            return (this->getter()--);
        }
        #pragma endregion

        #pragma region Assignment
        inline GetterReturnType operator+=(SetterInputType rhs)
        {
            GetterReturnType type = this->getter();
            type += rhs;
            this->setter((SetterInputType)type);
            return this->getter();
        }
        inline GetterReturnType operator-=(SetterInputType rhs)
        {
            GetterReturnType type = this->getter();
            type -= rhs;
            this->setter((SetterInputType)type);
            return this->getter();
        }
        inline GetterReturnType operator*=(SetterInputType rhs)
        {
            GetterReturnType type = this->getter();
            type *= rhs;
            this->setter((SetterInputType)type);
            return this->getter();
        }
        inline GetterReturnType operator/=(SetterInputType rhs)
        {
            GetterReturnType type = this->getter();
            type /= rhs;
            this->setter((SetterInputType)type);
            return this->getter();
        }
        inline GetterReturnType operator%=(SetterInputType rhs)
        {
            GetterReturnType type = this->getter();
            type %= rhs;
            this->setter((SetterInputType)type);
            return this->getter();
        }
        #pragma endregion

        #pragma region Comparison
        inline GetterReturnType operator==(SetterInputType rhs)
        {
            return this->getter() == rhs;
        }
        inline GetterReturnType operator!=(SetterInputType rhs)
        {
            return this->getter() != rhs;
        }
        inline GetterReturnType operator>=(SetterInputType rhs)
        {
            return this->getter() >= rhs;
        }
        inline GetterReturnType operator<=(SetterInputType rhs)
        {
            return this->getter() <= rhs;
        }
        inline GetterReturnType operator>(SetterInputType rhs)
        {
            return this->getter() > rhs;
        }
        inline GetterReturnType operator<(SetterInputType rhs)
        {
            return this->getter() < rhs;
        }
        #pragma endregion

        inline GetterReturnType operator()()
        {
            return this->getter();
        }
        inline operator GetterReturnType ()
        {
            return this->getter();
        }
    private:
        skarupke::function<GetterReturnType()> getter = nullptr;
        skarupke::function<void(SetterInputType)> setter = nullptr;
    };
}
