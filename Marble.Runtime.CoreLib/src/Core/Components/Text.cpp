#include "Text.h"

#include <Core/CoreEngine.h>
#undef STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>
#include <tuple>
#include <mapbox/earcut.hpp>

using namespace Marble;
using namespace Marble::GL;
using namespace Marble::Internal;
using namespace Marble::PackageSystem;
using namespace Marble::Typography;

static constexpr std::array<float, 2> operator+(const std::array<float, 2>& lhs, const std::array<float, 2>& rhs)
{
    return { lhs[0] + rhs[0], lhs[1] + rhs[1] };
}
static constexpr std::array<float, 2> operator-(const std::array<float, 2>& lhs, const std::array<float, 2>& rhs)
{
    return { lhs[0] - rhs[0], lhs[1] - rhs[1] };
}
static constexpr std::array<float, 2> operator*(const std::array<float, 2>& lhs, const std::array<float, 2>& rhs)
{
    return { lhs[0] * rhs[0], lhs[1] * rhs[1] };
}
static constexpr std::array<float, 2> operator/(const std::array<float, 2>& lhs, const std::array<float, 2>& rhs)
{
    return { lhs[0] / rhs[0], lhs[1] / rhs[1] };
}
static constexpr std::array<float, 2> operator+(const std::array<float, 2>& lhs, float rhs)
{
    return { lhs[0] + rhs, lhs[1] + rhs };
}
static constexpr std::array<float, 2> operator-(const std::array<float, 2>& lhs, float rhs)
{
    return { lhs[0] - rhs, lhs[1] - rhs };
}
static constexpr std::array<float, 2> operator*(const std::array<float, 2>& lhs, float rhs)
{
    return { lhs[0] * rhs, lhs[1] * rhs };
}
static constexpr std::array<float, 2> operator/(const std::array<float, 2>& lhs, float rhs)
{
    return { lhs[0] / rhs, lhs[1] / rhs };
}

static bool pointsAreClockwise(stbtt_vertex* points, size_t pointsSize)
{
    int32_t sign = 0;
    for (size_t i = 0; i < pointsSize - 1; i++)
        sign += (points[i + 1].x - points[i].x) * (points[i + 1].y + points[i].y);
    sign += (points[pointsSize - 1].x - points[0].x) * (points[pointsSize - 1].y + points[0].y);
    return sign >= 0;
}

std::unordered_map<PackageSystem::TrueTypeFontPackageFile*, Text::RenderData*> Text::textFonts;

Text::Text() :
font
({
    []
    {
        return nullptr;
    },
    [this](TrueTypeFontPackageFile* file)
    {
        if (this->data != nullptr)
        {
            --this->data->accessCount;
            if (this->data->accessCount == 0)
            {
                Text::textFonts.erase(this->data->file);
                delete data;
            }
        }
        if (file != nullptr)
        {
            auto set = Text::textFonts.find(file);
            if (set != Text::textFonts.end())
            {
                this->data = set->second;
                ++this->data->accessCount;
            }
            else
            {
                auto& set = Text::textFonts[file];
                set = new RenderData { 1, { }, file };
                this->data = set;
            }
        }
        else this->data = nullptr;
    }
}),
data(nullptr),
text
({
    [this]() -> const std::u32string&
    {
        return this->_text;
    },
    [this](std::u32string str) -> void
    {
        std::map<char32_t, bool> needToErase;
        for (auto it = this->_text.begin(); it != this->_text.end(); ++it)
        {
            auto c = this->data->characters.find(*it);
            if (c != this->data->characters.end())
            {
                --c->second->accessCount;
                if (c->second->accessCount == 0)
                    needToErase[*it] = true;
            }
        }
        for (auto it = str.begin(); it != str.end(); ++it)
        {
            auto c = this->data->characters.find(*it);
            if (c != this->data->characters.end())
            {
                ++c->second->accessCount;
                if (auto erase = needToErase.find(c->first); erase != needToErase.end())
                    erase->second = false;
            }
            else
            {
                GlyphOutline glyph(this->data->file->fontHandle(), *it);

                if (glyph.verts != nullptr) [[likely]]
                {
                    std::array<float, 2> ringBegin { glyph.verts[0].x, glyph.verts[0].y };
                    std::vector<std::vector<std::vector<std::array<float, 2>>>> points { { { ringBegin } } };
                    std::vector<std::vector<uint16_t>> indexes;

                    uint16_t indexOffset = 0;
                    for (int j = 1; j < glyph.vertsSize; j++)
                    {
                        auto& p = glyph.verts[j];

                        switch (p.type)
                        {
                        [[unlikely]] case STBTT_vmove:
                            {
                                size_t ringSize = j + 1;
                                while (glyph.verts[j].x != glyph.verts[ringSize].x || glyph.verts[j].y != glyph.verts[ringSize].y)
                                    ++ringSize;
                                ringSize -= j - 1;

                                if (pointsAreClockwise(glyph.verts + j, ringSize))
                                {
                                    auto polyIndexes = mapbox::earcut<uint16_t>(points.back());
                                    for (auto it = polyIndexes.begin(); it != polyIndexes.end(); ++it)
                                        *it += indexOffset;
                                    indexes.push_back(std::move(polyIndexes));

                                    for (auto it = points.back().begin(); it != points.back().end(); ++it)
                                        indexOffset += it->size();

                                    points.push_back({ });
                                }

                                points.back().push_back({ });
                                ringBegin = { (float)glyph.verts[j].x, (float)glyph.verts[j].y };
                                points.back().back().push_back(ringBegin);
                            }
                            break;
                        case STBTT_vcurve:
                            {
                                std::array<float, 2> begin { glyph.verts[j - 1].x, glyph.verts[j - 1].y };
                                std::array<float, 2> control { p.cx, p.cy };
                                std::array<float, 2> end { p.x, p.y };

                                constexpr auto approxQuadBezierLen =
                                [](const std::array<float, 2>& begin, const std::array<float, 2>& control, const std::array<float, 2>& end) -> float
                                {
                                    float d1 = std::fabsf(begin[0] - control[0]) + std::fabsf(control[0] - end[0]) + std::fabsf(begin[0] - end[0]);
                                    float d2 = std::fabsf(begin[1] - control[1]) + std::fabsf(control[1] - end[1]) + std::fabsf(begin[1] - end[1]);
                                    return std::sqrtf((d1 * d1) + (d2 * d2));
                                };

                                #define QUADRATIC_BEZIER_SEGMENT_LENGTH 32
                                float segments = float(size_t((approxQuadBezierLen(begin, control, end) / float(QUADRATIC_BEZIER_SEGMENT_LENGTH)) + 0.5f));
                                for (float k = 1; k < segments; k++)
                                {
                                    std::array<float, 2> p =
                                    (begin + (control - begin) / segments * k) +
                                    ((end + (control - end) / segments * (segments - k)) -
                                    (begin + (control - begin) / segments * k)) /
                                    segments * k;
                                    points.back().back().push_back(std::move(p));
                                }
                            }
                        [[fallthrough]];
                        case STBTT_vcubic: // Can't be stuffed converting cubic beziers to their quadratic approximations right now.
                        case STBTT_vline:
                            if (p.x == ringBegin[0] && p.y == ringBegin[1]) [[unlikely]]
                                continue;
                            points.back().back().push_back(std::array<float, 2> { float(p.x), float(p.y) });
                            break;
                        }
                    }
                    auto polyIndexes = mapbox::earcut<uint16_t>(points.back());
                    for (auto it = polyIndexes.begin(); it != polyIndexes.end(); ++it)
                        *it += indexOffset;
                    indexes.push_back(std::move(polyIndexes));
                    
                    std::vector<std::array<float, 2>> pointsFlattened;
                    size_t reserveSize = 0;
                    for (auto it1 = points.begin(); it1 != points.end(); ++it1)
                        for (auto it2 = it1->begin(); it2 != it1->end(); ++it2)
                            reserveSize += it2->size();
                    pointsFlattened.reserve(reserveSize);
                    for (auto it1 = points.begin(); it1 != points.end(); ++it1)
                        for (auto it2 = it1->begin(); it2 != it1->end(); ++it2)
                            pointsFlattened.insert(pointsFlattened.end(), std::make_move_iterator(it2->begin()), std::make_move_iterator(it2->end()));

                    std::vector<uint16_t> indexesFlattened;
                    reserveSize = 0;
                    for (auto it = indexes.begin(); it != indexes.end(); ++it)
                        reserveSize += it->size();
                    indexesFlattened.reserve(reserveSize);
                    for (auto it = indexes.begin(); it != indexes.end(); ++it)
                        indexesFlattened.insert(indexesFlattened.end(), std::make_move_iterator(it->begin()), std::make_move_iterator(it->end()));

                    this->data->characters[*it] = new CharacterRenderData { 1, { } };
                    CoreEngine::pendingRenderJobBatchesOffload.push_back
                    (
                        [=, pointsFlattened = std::move(pointsFlattened), indexesFlattened = std::move(indexesFlattened), data = this->data->characters[*it]]
                        {
                            data->polygon.create(pointsFlattened.data(), pointsFlattened.size(), indexesFlattened.data(), indexesFlattened.size(), 0xffffffffu);
                        }
                    );
                }
            }
        }
        for (auto it = needToErase.begin(); it != needToErase.end(); ++it)
        {
            if (it->second)
            {
                CoreEngine::pendingRenderJobBatchesOffload.push_back([=, data = this->data->characters[it->first]] { data->polygon.destroy(); delete data; });
                this->data->characters.erase(it->first);
            }
        }
        
        this->_text = std::move(str);
    }
}),
_text(U""),
fontSize(11)
{
}
Text::~Text()
{
    for (auto it = this->_text.begin(); it != this->_text.end(); ++it)
    {
        decltype(this->data->characters)::iterator c;
        if ((c = this->data->characters.find(*it)) != this->data->characters.end())
        {
            --this->data->characters[*it]->accessCount;
            if (this->data->characters[*it]->accessCount == 0)
                CoreEngine::pendingRenderJobBatchesOffload.push_back([=, data = this->data->characters[*it]] { data->polygon.destroy(); delete data; });
            this->data->characters.erase(*it);
        }
    }
    --this->data->accessCount;
    if (this->data->accessCount == 0)
        delete this->data;
}