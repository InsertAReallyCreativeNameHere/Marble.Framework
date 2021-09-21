#pragma once

#include "inc.h"
#include <skarupke/function.h>

namespace Marble
{
    template
    <
        typename GetterReturnType,
        typename SetterInputType
    >
    struct Property
    {
        inline explicit Property(const std::pair<skarupke::function<GetterReturnType()>, skarupke::function<void(SetterInputType)>>&& constructor) :
        getter(std::move(std::get<0>(constructor))), setter(std::move(std::get<1>(constructor)))
        {
        }
        inline explicit Property(const Property<GetterReturnType, SetterInputType>& other) = delete;
        inline explicit Property(Property<GetterReturnType, SetterInputType>&& other) = delete;

        inline GetterReturnType operator=(SetterInputType value) const
        {
            this->setter(value);
            return this->getter();
        }
        inline GetterReturnType operator=(const Property<GetterReturnType, SetterInputType>& rhs) = delete;
        inline GetterReturnType operator=(Property<GetterReturnType, SetterInputType>&&) = delete;

        #pragma region Arithmetic
        inline GetterReturnType operator+(SetterInputType rhs) const
        {
            return (this->getter() + rhs);
        }
        inline GetterReturnType operator-(SetterInputType rhs) const
        {
            return (this->getter() - rhs);
        }
        inline GetterReturnType operator*(SetterInputType rhs) const
        {
            return (this->getter() * rhs);
        }
        inline GetterReturnType operator/(SetterInputType rhs) const
        {
            return (this->getter() / rhs);
        }
        inline GetterReturnType operator%(SetterInputType rhs) const
        {
            return (this->getter() % rhs);
        }
        
        inline GetterReturnType operator++() const
        {
            return (this->getter()++);
        }
        inline GetterReturnType operator--() const
        {
            return (this->getter()--);
        }
        #pragma endregion

        #pragma region Assignment
        inline GetterReturnType operator+=(SetterInputType rhs) const
        {
            GetterReturnType type = this->getter();
            type += rhs;
            this->setter((SetterInputType)type);
            return this->getter();
        }
        inline GetterReturnType operator-=(SetterInputType rhs) const
        {
            GetterReturnType type = this->getter();
            type -= rhs;
            this->setter((SetterInputType)type);
            return this->getter();
        }
        inline GetterReturnType operator*=(SetterInputType rhs) const
        {
            GetterReturnType type = this->getter();
            type *= rhs;
            this->setter((SetterInputType)type);
            return this->getter();
        }
        inline GetterReturnType operator/=(SetterInputType rhs) const
        {
            GetterReturnType type = this->getter();
            type /= rhs;
            this->setter((SetterInputType)type);
            return this->getter();
        }
        inline GetterReturnType operator%=(SetterInputType rhs) const
        {
            GetterReturnType type = this->getter();
            type %= rhs;
            this->setter((SetterInputType)type);
            return this->getter();
        }
        #pragma endregion

        #pragma region Comparison
        inline GetterReturnType operator==(SetterInputType rhs) const
        {
            return this->getter() == rhs;
        }
        inline GetterReturnType operator!=(SetterInputType rhs) const
        {
            return this->getter() != rhs;
        }
        inline GetterReturnType operator>=(SetterInputType rhs) const
        {
            return this->getter() >= rhs;
        }
        inline GetterReturnType operator<=(SetterInputType rhs) const
        {
            return this->getter() <= rhs;
        }
        inline GetterReturnType operator>(SetterInputType rhs) const
        {
            return this->getter() > rhs;
        }
        inline GetterReturnType operator<(SetterInputType rhs) const
        {
            return this->getter() < rhs;
        }
        #pragma endregion

        inline GetterReturnType operator()() const
        {
            return this->getter();
        }
        inline operator GetterReturnType () const
        {
            return this->getter();
        }
    private:
        const skarupke::function<GetterReturnType()> getter;
        const skarupke::function<void(SetterInputType)> setter;
    };
}
