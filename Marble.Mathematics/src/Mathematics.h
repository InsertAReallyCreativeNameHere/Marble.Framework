#pragma once

#include "inc.h"

#include <Utility/ManagedArray.h>

namespace Marble
{
	namespace Mathematics
	{
		#pragma region Vector
		class Vector2Int;
		
		struct Vector2
		{
			static const Vector2 zero;
			static const Vector2 one;

			float x;
			float y;

			constexpr Vector2() : x(0), y(0)
			{
			}
			constexpr Vector2(float x, float y) : x(x), y(y)
			{
			}

			constexpr bool operator== (const Vector2& rhs) const
			{
				return this->x == rhs.x && this->y == rhs.y;
			}
			constexpr bool operator!= (const Vector2& rhs) const
			{
				return this->x != rhs.x || this->y != rhs.y;
			}
			constexpr Vector2 operator+ (const Vector2& rhs) const
			{
				return Vector2(this->x + rhs.x, this->y + rhs.y);
			}
			constexpr Vector2 operator- (const Vector2& rhs) const
			{
				return Vector2(this->x - rhs.x, this->y - rhs.y);
			}
			constexpr Vector2 operator* (const Vector2& rhs) const
			{
				return Vector2(this->x * rhs.x, this->y * rhs.y);
			}
			constexpr Vector2 operator/ (const Vector2& rhs) const
			{
				return Vector2(this->x / rhs.x, this->y / rhs.y);
			}
			constexpr Vector2& operator+= (const Vector2& rhs)
			{
				this->x += rhs.x;
				this->y += rhs.y;
				return *this;
			}
			constexpr Vector2& operator-= (const Vector2& rhs)
			{
				this->x -= rhs.x;
				this->y -= rhs.y;
				return *this;
			}
			constexpr Vector2& operator*= (const Vector2& rhs)
			{
				this->x *= rhs.x;
				this->y *= rhs.y;
				return *this;
			}
			constexpr Vector2& operator/= (const Vector2& rhs)
			{
				this->x /= rhs.x;
				this->y /= rhs.y;
				return *this;
			}

			inline operator std::string () const;
			inline friend std::ostream& operator<<(std::ostream& stream, const Vector2& rhs)
			{
				return stream << static_cast<std::string>(rhs);
			}
		};

		inline const Vector2 Vector2::zero { 0.0f, 0.0f };
		inline const Vector2 Vector2::one { 1.0f, 1.0f };
		
		Vector2::operator std::string() const
		{
			return std::string("{ ").append(std::to_string(this->x)).append(", ").append(std::to_string(this->y)).append(" }");
		}

		struct Vector2Int
		{
			static const Vector2Int zero;
			static const Vector2Int one;

			int32_t x;
			int32_t y;

			constexpr Vector2Int() : x(0), y(0)
			{
			}
			constexpr Vector2Int(int32_t x, int32_t y) : x(x), y(y)
			{
			}

			constexpr bool operator== (const Vector2Int& rhs) const
			{
				return this->x == rhs.x && this->y == rhs.y;
			}
			constexpr bool operator!= (const Vector2Int& rhs) const
			{
				return this->x != rhs.x || this->y != rhs.y;
			}
			constexpr Vector2Int operator+ (const Vector2Int& rhs) const
			{
				return Vector2Int(this->x + rhs.x, this->y + rhs.y);
			}
			constexpr Vector2Int operator- (const Vector2Int& rhs) const
			{
				return Vector2Int(this->x - rhs.x, this->y - rhs.y);
			}
			constexpr Vector2Int operator* (const Vector2Int& rhs) const
			{
				return Vector2Int(this->x * rhs.x, this->y * rhs.y);
			}
			constexpr Vector2Int operator/ (const Vector2Int& rhs) const
			{
				return Vector2Int(this->x / rhs.x, this->y / rhs.y);
			}
			constexpr Vector2Int& operator+= (const Vector2Int& rhs)
			{
				this->x += rhs.x;
				this->y += rhs.y;
				return *this;
			}
			constexpr Vector2Int& operator-= (const Vector2Int& rhs)
			{
				this->x -= rhs.x;
				this->y -= rhs.y;
				return *this;
			}
			constexpr Vector2Int& operator*= (const Vector2Int& rhs)
			{
				this->x *= rhs.x;
				this->y *= rhs.y;
				return *this;
			}
			constexpr Vector2Int& operator/= (const Vector2Int& rhs)
			{
				this->x /= rhs.x;
				this->y /= rhs.y;
				return *this;
			}

			inline operator std::string () const;
			inline friend std::ostream& operator<<(std::ostream& stream, const Vector2Int& rhs)
			{
				return stream << static_cast<std::string>(rhs);
			}
		};

		inline const Vector2Int Vector2Int::zero { 0, 0 };
		inline const Vector2Int Vector2Int::one { 1, 1 };

		Vector2Int::operator std::string() const
		{
			return std::string("{ ").append(std::to_string(this->x)).append(", ").append(std::to_string(this->y)).append(" }");
		}

		struct coreapi Vector3
		{
			float x;
			float y;
			float z;

			Vector3();
			Vector3(float x, float y, float z);
		};

		struct coreapi Vector4
		{
			float x;
			float y;
			float z;
			float w;

			Vector4();
			Vector4(float x, float y, float z, float w);
		};

		typedef Vector2 vec2;
		typedef Vector2Int vec2i;

		typedef Vector3 vec3;
		
		typedef Vector4 vec4;
		#pragma endregion

		#pragma region Matrix
		struct coreapi Matrix final
		{
			Matrix(const size_t& rows, const size_t& columns, const float& value = 0);
			Matrix(const std::initializer_list<std::initializer_list<float>>& matrix);

			float& operator()(const size_t& row, const size_t& column);
			float& operator[](size_t const (&index)[2]);

			template<typename Func>
			void map(const Func& func)
			{
				for (int i = 0; i < this->values.length(); i++)
					this->values[i] = func(this->values[i]);
			}

			Matrix transpose();

			Matrix operator+(const float& rhs);
			Matrix operator+(Matrix rhs);
			
			Matrix operator-(const float& rhs);
			Matrix operator-(Matrix rhs);

			Matrix operator*(const float& rhs);
			Matrix operator*(Matrix rhs);

			size_t rowsCount();
			size_t columnsCount();
		private:
			size_t rows;
			size_t columns;
			ManagedArray<float> values;
		};

		#define Matrix4x4(initialValue) Matrix(4, 4, initialValue)
		//#define Matrix4x4()
		#pragma endregion
	}
}

#define piF 3.14159265358979323846264338327950288f
#define deg2RadF(degrees) 3.14159265358979323846264338327950288f / 180 * (degrees)
