#include "Text.h"

#include <Mathematics.h>
#include <numeric>
#undef STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>
#include <tuple>
#include <Core/CoreEngine.h>
#include <Core/Components/RectTransform.h>
#include <Rendering/Core/Renderer.h>

using namespace Marble;
using namespace Marble::Internal;
using namespace Marble::Mathematics;
using namespace Marble::Typography;
using namespace Marble::GL;
using namespace Marble::PackageSystem;

std::unordered_map<PackageSystem::TrueTypeFontPackageFile*, Text::RenderData*> Text::textFonts;

Text::Text() :
data(nullptr),
_text(U""),
_fontSize(11),
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
                GlyphOutline glyph = this->data->file->fontHandle().getCodepointOutline(*it);
                
                if (glyph.verts != nullptr) [[likely]]
                {
                    auto buffers = glyph.createGeometryBuffers<Vertex2D>();
                    this->data->characters[*it] = new CharacterRenderData { 1, { } };
                    CoreEngine::queueRenderJobForFrame
                    (
                        [=, pointsFlattened = std::move(buffers.first), indexesFlattened = std::move(buffers.second), data = this->data->characters[*it]]
                        {
                            data->polygon.create(std::move(pointsFlattened), std::move(indexesFlattened));
                        }
                    );
                }
            }
        }
        for (auto it = needToErase.begin(); it != needToErase.end(); ++it)
        {
            if (it->second)
            {
                CoreEngine::queueRenderJobForFrame([=, data = this->data->characters[it->first]] { data->polygon.destroy(); delete data; });
                this->data->characters.erase(it->first);
            }
        }
        
        this->_text = std::move(str);
    }
}),
fontSize
({
    [&]() -> uint32_t
    {
        return this->_fontSize;
    },
    [&](uint32_t value) -> void
    {
        switch (value)
        {
        case FontSize::RecalculateForRectSize:
            {
                if (this->data->file != nullptr) [[likely]]
                {
                    RectTransform* thisRect = this->rectTransform();
                    Font& font = this->data->file->font;

                    float accAdv = 0;
                    for (auto it = this->_text.begin(); it != this->_text.end(); ++it)
                        accAdv += font.getCodepointMetrics(*it).advanceWidth;

                    // NB: Calculate optimistic font size - the largest possible with given text.
                    //     This is done to be generally performant with the following iterative
                    //     font sizing algorithm.
                    // TODO: Is it possible to eliminate the costly sqrt?
                    this->_fontSize = (font.ascent - font.descent) *
                    ((thisRect->rect().right - thisRect->rect().left) / accAdv);
                    this->_fontSize = this->_fontSize * sqrt((thisRect->rect().top - thisRect->rect().bottom) / this->_fontSize);

                    const Vector2& pos = thisRect->position;
                    const Vector2& scale = thisRect->scale;
                    const RectFloat& rect = thisRect->rect;
                    float rectWidth = (rect.right - rect.left) * scale.x;
                    float rectHeight = (rect.top - rect.bottom) * scale.y;
                    float rot = deg2RadF(thisRect->rotation);
                    float asc = this->data->file->fontHandle().ascent;
                    float lineHeight = asc - this->data->file->fontHandle().descent;

                    while (this->_fontSize != 0)
                    {
                        float glyphScale = float(this->_fontSize) / lineHeight;
                        float lineHeightScaled = lineHeight * glyphScale * scale.y;
                        float lineDiff = (this->data->file->fontHandle().lineGap + lineHeight) * glyphScale * scale.y;
                        float accXAdvance = 0;
                        float accYAdvance = 0;
                        float spaceAdv = this->data->file->fontHandle().getCodepointMetrics(U' ').advanceWidth * glyphScale * scale.x;

                        size_t beg = 0;
                        size_t end;

                        if (lineHeightScaled > rectHeight)
                        {
                            --this->_fontSize;
                            continue;
                        }
                        
                        // NB: Horrific code that avoids multiple calls of find_first_*.
                        //     I don't even know if this was optimal.
                        if
                        (
                            size_t _end = this->_text.find_first_of(U" \t\r\n", 0);
                            _end < (end = this->_text.find_first_not_of(U" \t\r\n", 0))
                        )
                        {
                            end = _end;
                            goto HandleWord;
                        }
                        else goto HandleSpace;

                        // NB: Ohhhhhhh the goto abuse...
                        //     I am going to hell.
                        HandleWord:
                        {
                            std::vector<float> advanceLengths;
                            advanceLengths.reserve(end - beg);
                            for (size_t i = beg; i < end; i++)
                                advanceLengths.push_back(float(font.getCodepointMetrics(this->_text[i]).advanceWidth) * glyphScale * scale.x);

                            auto advanceLenIt = advanceLengths.begin();

                            float wordLen = std::accumulate(advanceLenIt, advanceLengths.end(), 0.0f);
                            if (accXAdvance + wordLen > rectWidth) [[unlikely]]
                            {
                                if (wordLen > rectWidth) [[unlikely]]
                                    goto SplitCalcWord;
                                else
                                {
                                    accXAdvance = 0;
                                    accYAdvance += lineDiff;
                                    if (accYAdvance + lineHeightScaled > rectHeight)
                                    {
                                        --this->_fontSize;
                                        continue;
                                    }
                                    goto InlineCalcWord;
                                }
                            }
                            else goto InlineCalcWord;

                            SplitCalcWord:
                            for (auto it = advanceLengths.begin(); it != advanceLengths.end(); ++it)
                            {
                                if (accXAdvance + *it > rectWidth) [[unlikely]]
                                {
                                    accXAdvance = 0;
                                    accYAdvance += lineDiff;
                                    if (accYAdvance + lineHeightScaled > rectHeight)
                                    {
                                        --this->_fontSize;
                                        continue;
                                    }
                                }
                                accXAdvance += *it;
                            }
                            goto ExitCalcWord;

                            InlineCalcWord:
                            for (auto it = advanceLengths.begin(); it != advanceLengths.end(); ++it)
                                accXAdvance += *it;
                            // NB: No goto here, jump would go to the same place anyways.

                            ExitCalcWord:;
                        }
                        beg = end;
                        if (end != this->_text.size()) [[likely]]
                        {
                            size_t _end = this->_text.find_first_not_of(U" \t\r\n", beg + 1);
                            end = (_end == std::u32string::npos ? this->_text.size() : _end);
                            goto HandleSpace;
                        }
                        else goto ExitTextHandling;

                        HandleSpace:
                        for (size_t i = beg; i < end; i++)
                        {
                            switch (this->_text[i])
                            {
                            case U' ':
                                accXAdvance += spaceAdv;
                                break;
                            case U'\t':
                                accXAdvance += spaceAdv * 8;
                                break;
                            case U'\r':
                                break;
                            case U'\n':
                                accXAdvance = 0;
                                accYAdvance += lineDiff;
                                if (accYAdvance + lineHeightScaled > rectHeight)
                                {
                                    --this->_fontSize;
                                    continue;
                                }
                                break;
                            }
                        }
                        beg = end;
                        if (end != this->_text.size()) [[likely]]
                        {
                            size_t _end = this->_text.find_first_of(U" \t\r\n", beg + 1);
                            end = (_end == std::u32string::npos ? this->_text.size() : _end);
                            goto HandleWord;
                        }
                        // NB: No else here, it just goes to the same place anyways.

                        ExitTextHandling: break;
                    }
                }
            }
            break;
        default:
            this->_fontSize = value;
        }
    }
})
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
                CoreEngine::queueRenderJobForFrame([=, data = this->data->characters[*it]] { data->polygon.destroy(); delete data; });
            this->data->characters.erase(*it);
        }
    }
    --this->data->accessCount;
    if (this->data->accessCount == 0)
        delete this->data;
}

void Text::renderOffload()
{
    if (this->data->file != nullptr && !this->_text.empty()) [[likely]]
    {
        RectTransform* thisRect = this->rectTransform();
        const Vector2& pos = thisRect->position;
        const Vector2& scale = thisRect->scale;
        const RectFloat& rect = thisRect->rect;
        float rectWidth = (rect.right - rect.left) * scale.x;
        float rectHeight = (rect.top - rect.bottom) * scale.y;
        float rot = deg2RadF(thisRect->rotation);
        float asc = this->data->file->fontHandle().ascent;
        float lineHeight = asc - this->data->file->fontHandle().descent;
        float glyphScale = float(this->fontSize) / lineHeight;
        float lineDiff = (this->data->file->fontHandle().lineGap + lineHeight) * glyphScale * scale.y;
        float accXAdvance = 0;
        float accYAdvance = 0;
        float spaceAdv = this->data->file->fontHandle().getCodepointMetrics(U' ').advanceWidth * glyphScale * scale.x;

        size_t beg = 0;
        size_t end;

        // NB: Horrific code that avoids multiple calls of find_first_*.
        //     I don't even know if this was optimal.
        if
        (
            size_t _end = this->_text.find_first_of(U" \t\r\n", 0);
            _end < (end = this->_text.find_first_not_of(U" \t\r\n", 0))
        )
        {
            end = _end;
            goto HandleWord;
        }
        else goto HandleSpace;

        // NB: Ohhhhhhh the goto abuse...
        //     I am going to hell.
        HandleWord:
        {
            std::vector<float> advanceLengths;
            advanceLengths.reserve(end - beg);
            for (size_t i = beg; i < end; i++)
                advanceLengths.push_back(float(this->data->file->fontHandle().getCodepointMetrics(this->_text[i]).advanceWidth) * glyphScale * scale.x);

            auto advanceLenIt = advanceLengths.begin();

            auto drawCharAndIterNext = [&](size_t i)
            {
                auto c = this->data->characters.find(this->_text[i]);
                if (c != this->data->characters.end())
                {
                    ColoredTransformHandle transform;
                    transform.setPosition(pos.x, pos.y);
                    transform.setOffset(rect.left * scale.x + accXAdvance, rect.top * scale.y - asc * glyphScale - accYAdvance);
                    transform.setScale(glyphScale * scale.x, glyphScale * scale.y);
                    transform.setRotation(rot);
                    transform.setColor(1.0f, 1.0f, 1.0f, 1.0f);
                    CoreEngine::queueRenderJobForFrame([=, data = c->second] { Renderer::drawPolygon(data->polygon, transform); });
                }

                accXAdvance += *advanceLenIt;
                ++advanceLenIt;
            };
            
            float wordLen = std::accumulate(advanceLenIt, advanceLengths.end(), 0.0f);
            if (accXAdvance + wordLen > rectWidth) [[unlikely]]
            {
                if (wordLen > rectWidth) [[unlikely]]
                    goto SplitDrawWord;
                else
                {
                    accXAdvance = 0;
                    accYAdvance += lineDiff;
                    goto InlineDrawWord;
                }
            }
            else goto InlineDrawWord;

            // NB: Draw a word split across lines. This is used when
            //     a word is longer than the width of a line.
            SplitDrawWord:
            for (size_t i = beg; i < end; i++)
            {
                if (accXAdvance + *advanceLenIt > rectWidth) [[unlikely]]
                {
                    accXAdvance = 0;
                    accYAdvance += lineDiff;
                }

                drawCharAndIterNext(i);
            }
            goto ExitDrawWord;

            // NB: Draw a word in one line. This makes the assumption
            //     that the whole word will fit within a line.
            InlineDrawWord:
            for (size_t i = beg; i < end; i++)
                drawCharAndIterNext(i);
            // NB: No goto here, jump would go to the same place anyways.

            ExitDrawWord:;
        }
        beg = end;
        if (end != this->_text.size()) [[likely]]
        {
            size_t _end = this->_text.find_first_not_of(U" \t\r\n", beg + 1);
            end = (_end == std::u32string::npos ? this->_text.size() : _end);
            goto HandleSpace;
        }
        else goto ExitTextHandling;

        HandleSpace:
        for (size_t i = beg; i < end; i++)
        {
            switch (this->_text[i])
            {
            case U' ':
                accXAdvance += spaceAdv;
                break;
            case U'\t':
                accXAdvance += spaceAdv * 8;
                break;
            case U'\r':
                break;
            case U'\n':
                accXAdvance = 0;
                accYAdvance += lineDiff;
                break;
            }
        }
        beg = end;
        if (end != this->_text.size()) [[likely]]
        {
            size_t _end = this->_text.find_first_of(U" \t\r\n", beg + 1);
            end = (_end == std::u32string::npos ? this->_text.size() : _end);
            goto HandleWord;
        }
        // NB: No else here, it just goes to the same place anyways.

        ExitTextHandling:;
    }
}