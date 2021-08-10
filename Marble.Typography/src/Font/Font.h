#pragma once

#include "inc.h"

#include <mapbox/earcut.hpp>
#include <map>
#undef STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

namespace Marble
{
    namespace Typography
    {
        struct GlyphOutline;
        struct GlyphMetrics;

        class coreapi Font final
        {
            stbtt_fontinfo fontInfo;
        public:
            int ascent, descent, lineGap;

            Font(unsigned char* fontData);
            Font(const Font&) = delete;
            Font(Font&&) = delete;

            GlyphOutline getCodepointOutline(char32_t codepoint);
            GlyphMetrics getCodepointMetrics(char32_t codepoint);
        };

        struct coreapi GlyphOutline final
        {
            stbtt_vertex* verts;
            int vertsSize;

            GlyphOutline(const GlyphOutline&) = delete; // Laziness.
            GlyphOutline(GlyphOutline&& other);
            ~GlyphOutline();

            template <typename VertType>
            std::pair<std::vector<VertType>, std::vector<uint16_t>> createGeometryBuffers()
            {
                constexpr auto pointsAreClockwise = [](stbtt_vertex* points, size_t pointsSize) -> bool
                {
                    int32_t sign = 0;
                    for (size_t i = 0; i < pointsSize - 1; i++)
                        sign += (points[i + 1].x - points[i].x) * (points[i + 1].y + points[i].y);
                    sign += (points[pointsSize - 1].x - points[0].x) * (points[pointsSize - 1].y + points[0].y);
                    return sign >= 0;
                };

                GlyphOutline& glyph = *this;

                VertType ringBegin { .x = float(glyph.verts[0].x), .y = float(glyph.verts[0].y) };
                std::vector<std::vector<std::vector<VertType>>> points { { { ringBegin } } };
                std::vector<std::vector<uint16_t>> indexes;

                size_t firstClockwiseLoopIndex = SIZE_MAX;
                size_t ringSize = 1;
                while (glyph.verts[0].x != glyph.verts[ringSize].x || glyph.verts[0].y != glyph.verts[ringSize].y)
                    ++ringSize;
                ++ringSize;
                
                if (pointsAreClockwise(glyph.verts, ringSize))
                    firstClockwiseLoopIndex = 0;
                
                uint16_t indexOffset = 0;
                for (int j = 1; j < glyph.vertsSize; j++)
                {
                    auto& p = glyph.verts[j];

                    switch (p.type)
                    {
                    [[unlikely]] case STBTT_vmove:
                        {
                            ringSize = j + 1;
                            while (glyph.verts[j].x != glyph.verts[ringSize].x || glyph.verts[j].y != glyph.verts[ringSize].y)
                                ++ringSize;
                            ringSize -= j - 1;

                            if (pointsAreClockwise(glyph.verts + j, ringSize))
                            {
                                switch (firstClockwiseLoopIndex)
                                {
                                [[likely]] case 0:
                                    break;
                                case SIZE_MAX:
                                    firstClockwiseLoopIndex = points.back().size();
                                    goto IgnoreClockwise;
                                    break;
                                default: // What a hack, no ranged cases needed. Switch statements ftw.
                                    std::iter_swap(points.back().begin(), points.back().begin() + firstClockwiseLoopIndex);
                                    firstClockwiseLoopIndex = 0;
                                }

                                auto polyIndexes = mapbox::template earcut<uint16_t>(points.back());
                                for (auto it = polyIndexes.begin(); it != polyIndexes.end(); ++it)
                                    *it += indexOffset;
                                indexes.push_back(std::move(polyIndexes));

                                for (auto it = points.back().begin(); it != points.back().end(); ++it)
                                    indexOffset += it->size();

                                points.push_back({ });
                            }
                            IgnoreClockwise:

                            points.back().push_back({ });
                            ringBegin = { (float)glyph.verts[j].x, (float)glyph.verts[j].y };
                            points.back().back().push_back(ringBegin);
                        }
                        break;
                    case STBTT_vcurve:
                        {
                            VertType begin { .x = float(glyph.verts[j - 1].x), .y = float(glyph.verts[j - 1].y) };
                            VertType control { .x = float(p.cx), .y = float(p.cy) };
                            VertType end { .x = float(p.x), .y = float(p.y) };

                            constexpr auto approxQuadBezierLen =
                            [](const VertType& begin, const VertType& control, const VertType& end) -> float
                            {
                                float d1 = fabs(begin.x - control.x) + fabs(control.x - end.x) + fabs(begin.x - end.x);
                                float d2 = fabs(begin.y - control.y) + fabs(control.y - end.y) + fabs(begin.y - end.y);
                                return sqrt((d1 * d1) + (d2 * d2));
                            };

                            #define QUADRATIC_BEZIER_SEGMENT_LENGTH 32
                            float segments = float(size_t((approxQuadBezierLen(begin, control, end) / float(QUADRATIC_BEZIER_SEGMENT_LENGTH)) + 0.5f));
                            for (float k = 1; k < segments; k++)
                            {
                                VertType p =
                                {
                                    .x =
                                    (begin.x + (control.x - begin.x) / segments * k) +
                                    ((end.x + (control.x - end.x) / segments * (segments - k)) -
                                    (begin.x + (control.x - begin.x) / segments * k)) /
                                    segments * k,
                                    .y =
                                    (begin.y + (control.y - begin.y) / segments * k) +
                                    ((end.y + (control.y - end.y) / segments * (segments - k)) -
                                    (begin.y + (control.y - begin.y) / segments * k)) /
                                    segments * k
                                };
                                points.back().back().push_back(std::move(p));
                            }
                        }
                    [[fallthrough]];
                    case STBTT_vcubic: // Can't be stuffed converting cubic beziers to their quadratic approximations right now.
                    case STBTT_vline:
                        if (p.x == ringBegin.x && p.y == ringBegin.y) [[unlikely]]
                            continue;
                        points.back().back().push_back(VertType { .x = float(p.x), .y = float(p.y) });
                        break;
                    }
                }

                if (firstClockwiseLoopIndex != 0) [[unlikely]]
                {
                    std::iter_swap(points.back().begin(), points.back().begin() + firstClockwiseLoopIndex);
                    firstClockwiseLoopIndex = 0;
                }

                auto polyIndexes = mapbox::template earcut<uint16_t>(points.back());
                for (auto it = polyIndexes.begin(); it != polyIndexes.end(); ++it)
                    *it += indexOffset;
                indexes.push_back(std::move(polyIndexes));

                std::vector<VertType> pointsFlattened;
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

                return std::make_pair(std::move(pointsFlattened), std::move(indexesFlattened));
            }
            
            friend class Marble::Typography::Font;
        private:
            GlyphOutline();
        };
        struct coreapi GlyphMetrics final
        {
            int advanceWidth, leftSideBearing;

            friend class Marble::Typography::Font;
        private:
            GlyphMetrics();
        };
    }
}