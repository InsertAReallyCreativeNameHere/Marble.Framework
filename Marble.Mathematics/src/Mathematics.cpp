#include "Mathematics.h"

#include <inc.h>

using namespace Marble::Mathematics;

#pragma region Vector2
const Vector2 Vector2::zero { 0, 0 };
const Vector2 Vector2::one { 1, 1 };
const Vector2Int Vector2Int::zero { 0, 0 };
const Vector2Int Vector2Int::one { 1, 1 };

Vector2::Vector2() : x(0), y(0)
{
}
Vector2::Vector2(float x, float y) : x(x), y(y)
{
}

bool Vector2::operator== (const Vector2& rhs)
{
	if (this->x == rhs.x && this->y == rhs.y)
		return true;
	else return false;
}
bool Vector2::operator!= (const Vector2& rhs)
{
	if (this->x == rhs.x && this->y == rhs.y)
		return false;
	else return true;
}
Vector2 Vector2::operator+ (const Vector2& rhs) const
{
	return Vector2(this->x + rhs.x, this->y + rhs.y);
}
Vector2 Vector2::operator- (const Vector2& rhs) const
{
	return Vector2(this->x - rhs.x, this->y - rhs.y);
}
Vector2 Vector2::operator* (const Vector2& rhs) const
{
	return Vector2(this->x * rhs.x, this->y * rhs.y);
}
Vector2 Vector2::operator/ (const Vector2& rhs) const
{
	return Vector2(this->x / rhs.x, this->y / rhs.y);
}
Vector2& Vector2::operator+= (const Vector2& rhs)
{
    this->x += rhs.x;
    this->y += rhs.y;
	return *this;
}
Vector2& Vector2::operator-= (const Vector2& rhs)
{
    this->x -= rhs.x;
    this->y -= rhs.y;
	return *this;
}
Vector2& Vector2::operator*= (const Vector2& rhs)
{
    this->x *= rhs.x;
    this->y *= rhs.y;
	return *this;
}
Vector2& Vector2::operator/= (const Vector2& rhs)
{
    this->x /= rhs.x;
    this->y /= rhs.y;
	return *this;
}

Vector2::operator std::string() const
{
    return
    #ifndef MSC_VER
    ((std::ostringstream&)
    #endif
    (std::ostringstream() << "{ " << this->x << ", " << this->y << " }")
    #ifndef MSC_VER
    )
    #endif
    .rdbuf()->str();
}

Vector2Int::Vector2Int() : x(0), y(0)
{
}
Vector2Int::Vector2Int(int x, int y) : x(x), y(y)
{
}

bool Vector2Int::operator== (const Vector2Int& rhs) const
{
	if (this->x == rhs.x && this->y == rhs.y)
		return true;
	else return false;
}
bool Vector2Int::operator!= (const Vector2Int& rhs) const
{
	if (this->x == rhs.x && this->y == rhs.y)
		return false;
	else return true;
}
Vector2Int Vector2Int::operator+ (const Vector2Int& rhs) const
{
	return Vector2Int(this->x + rhs.x, this->y + rhs.y);
}
Vector2Int Vector2Int::operator- (const Vector2Int& rhs) const
{
	return Vector2Int(this->x - rhs.x, this->y - rhs.y);
}
Vector2Int Vector2Int::operator* (const Vector2Int& rhs) const
{
	return Vector2Int(this->x * rhs.x, this->y * rhs.y);
}
Vector2Int Vector2Int::operator/ (const Vector2Int& rhs) const
{
	return Vector2Int(rhs.x != 0 ? this->x / rhs.x : 0, rhs.y != 0 ? this->y / rhs.y : 0);
}
Vector2Int& Vector2Int::operator+= (const Vector2Int& rhs)
{
    this->x += rhs.x;
    this->y += rhs.y;
	return *this;
}
Vector2Int& Vector2Int::operator-= (const Vector2Int& rhs)
{
    this->x -= rhs.x;
    this->y -= rhs.y;
	return *this;
}

Vector2Int::operator std::string() const
{
    return
    #ifndef MSC_VER
    ((std::ostringstream&)
    #endif
    (std::ostringstream() << "{ " << this->x << ", " << this->y << " }")
    #ifndef MSC_VER
    )
    #endif
    .rdbuf()->str();
}
#pragma endregion

#pragma region Vector3
Vector3::Vector3() : x(0), y(0), z(0)
{
}
Vector3::Vector3(float x, float y, float z) : x(x), y(y), z(z)
{
}
#pragma endregion

#pragma region Vector4
Vector4::Vector4() : x(0), y(0), z(0), w(0)
{
}
Vector4::Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w)
{
}
#pragma endregion

#pragma region Matrix
Matrix::Matrix
(
    const size_t& rows,
    const size_t& columns,
	const float& value
)
: rows(rows), columns(columns), values(rows * columns)
{
    for (size_t i = 0; i < rows; i++)
        for (size_t j = 0; j < columns; j++)
            this->values[i * this->columns + j] = value;
}
Matrix::Matrix(const std::initializer_list<std::initializer_list<float>>& matrix) : rows(matrix.size()), columns(matrix.begin()->size())
{
    size_t clmCt = matrix.begin()->size();
    this->values = ManagedArray<float>(this->rows * clmCt);

    size_t _i = 0;
    for (std::initializer_list<std::initializer_list<float>>::iterator i = matrix.begin(); i != matrix.end(); ++i)
    {
        if (i->size() != clmCt)
        {
            fputs("Matrix could not be created from std::initializer_list. Column count was inconsistent.", stderr);
            *this = Matrix(0, 0);
            return;
        }
        size_t _j = 0;
        for (std::initializer_list<float>::iterator j = i->begin(); j != i->end(); ++j)
        {
            this->values[_i * this->columns + _j] = *j;
            _j++;
        }
        _i++;
    }
}

float& Matrix::operator()(const size_t& row, const size_t& column)
{
    return values[row * this->columns + column];
}
float& Matrix::operator[](size_t const (&index)[2])
{
    return values[index[0] * this->columns + index[1]];
}

Matrix Matrix::transpose()
{
    Matrix m(this->columns, this->rows);
    #pragma omp parallel for default(none) shared(m)
    for (size_t i = 0; i < this->rows; i++)
        for (size_t j = 0; j < this->columns; j++)
            m(j, i) = this->operator()(i, j);
    return m;
}

Matrix Matrix::operator+(const float& rhs)
{
    Matrix m = Matrix(this->rows, this->columns);
    #pragma omp parallel for default(none) shared(rhs, m)
    for (size_t i = 0; i < this->rows; i++)
        for (size_t j = 0; j < this->columns; j++)
            m(i, j) = this->operator()(i, j) + rhs;
    return m;
}
Matrix Matrix::operator+(Matrix rhs)
{
    if (this->rows != rhs.rows || this->columns != rhs.columns)
    {
        fputs("Matrix could not be added: this->rows != rhs.rows || this->columns != rhs.columns.\n", stderr);
        return Matrix(0, 0);
    }

    #pragma omp parallel for default(none) shared(rhs)
    for (size_t i = 0; i < this->rows; i++)
        for (size_t j = 0; j < this->columns; j++)
            rhs(i, j) = this->operator()(i, j) + rhs(i, j);
    return rhs;
}

Matrix Matrix::operator-(const float& rhs)
{
    Matrix m = Matrix(this->rows, this->columns);
    #pragma omp parallel for default(none) shared(rhs, m)
    for (size_t i = 0; i < this->rows; i++)
        for (size_t j = 0; j < this->columns; j++)
            m(i, j) = this->operator()(i, j) - rhs;
    return m;
}
Matrix Matrix::operator-(Matrix rhs)
{
    if (this->rows != rhs.rows || this->columns != rhs.columns)
    {
        fputs("Matrix could not be subtracted: this->rows != rhs.rows || this->columns != rhs.columns.\n", stderr);
        return Matrix(0, 0);
    }

    #pragma omp parallel for default(none) shared(rhs)
    for (size_t i = 0; i < this->rows; i++)
        for (size_t j = 0; j < this->columns; j++)
            rhs(i, j) = this->operator()(i, j) - rhs(i, j);
    return rhs;
}

Matrix Matrix::operator*(const float& rhs)
{
    Matrix m = Matrix(this->rows, this->columns);
    #pragma omp parallel for default(none) shared(rhs, m)
    for (size_t i = 0; i < this->columns; i++)
        for (size_t j = 0; j < this->rows; j++)
            m(j, i) = this->operator()(j, i) * rhs;
    return m;
}
Matrix Matrix::operator*(Matrix rhs)
{
    if (this->rows != rhs.columns)
    {
        fputs("Matrix could not be added: this->rows != rhs.columns.\n", stderr);
        return Matrix(0, 0);
    }

    Matrix m = Matrix(this->rows, rhs.columns);
    #pragma omp parallel for default(none) shared(rhs, m) schedule(dynamic) collapse(2)
    for (size_t i = 0; i < this->rows; i++)
        for (size_t j = 0; j < rhs.columns; j++)
            for (size_t k = 0; k < rhs.rows; k++)
                m(i, j) += this->operator()(i, j) * rhs(k, j);
    return m;
}

size_t Matrix::rowsCount()
{
    return this->rows;
}
size_t Matrix::columnsCount()
{
    return this->columns;
}
#pragma endregion