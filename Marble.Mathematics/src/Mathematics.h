#pragma once

#include <inc.h>
#include <Utility/ManagedArray.h>

namespace Marble
{
	namespace Mathematics
	{
		#pragma region Vector
		class Vector2Int;
		
		struct coreapi Vector2
		{
			static const Vector2 zero;
			static const Vector2 one;

			float x;
			float y;

			Vector2();
			Vector2(float x, float y);

			bool operator== (const Vector2& rhs);
			bool operator!= (const Vector2& rhs);
			Vector2 operator+ (const Vector2& rhs) const;
			Vector2 operator- (const Vector2& rhs) const;
			Vector2 operator* (const Vector2& rhs) const;
			Vector2 operator/ (const Vector2& rhs) const;
			Vector2& operator+= (const Vector2& rhs);
			Vector2& operator-= (const Vector2& rhs);
			Vector2& operator*= (const Vector2& rhs);
			Vector2& operator/= (const Vector2& rhs);

			operator std::string () const;
			inline friend std::ostream& operator<<(std::ostream& stream, const Vector2& rhs)
			{
				return stream << static_cast<std::string>(rhs);
			}
		};

		struct coreapi Vector2Int
		{
			static const Vector2Int zero;
			static const Vector2Int one;

			int x;
			int y;

			Vector2Int();
			Vector2Int(int x, int y);

			bool operator== (const Vector2Int& rhs) const;
			bool operator!= (const Vector2Int& rhs) const;
			Vector2Int operator+ (const Vector2Int& rhs) const;
			Vector2Int operator- (const Vector2Int& rhs) const;
			Vector2Int operator* (const Vector2Int& rhs) const;
			Vector2Int operator/ (const Vector2Int& rhs) const;
			Vector2Int& operator+= (const Vector2Int& rhs);
			Vector2Int& operator-= (const Vector2Int& rhs);

			operator std::string () const;
			inline friend std::ostream& operator<<(std::ostream& stream, const Vector2Int& rhs)
			{
				return stream << static_cast<std::string>(rhs);
			}
		};

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
			Matrix(const uint& rows, const uint& columns, const float& value = 0);
			Matrix(const std::initializer_list<std::initializer_list<float>>& matrix);

			float& operator()(const uint& row, const uint& column);
			float& operator[](uint const (&index)[2]);

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

			uint rowsCount();
			uint columnsCount();
		private:
			uint rows;
			uint columns;
			ManagedArray<float> values;
		};

		#define Matrix4x4(initialValue) Matrix(4, 4, initialValue)
		//#define Matrix4x4()
		#pragma endregion
	}
}