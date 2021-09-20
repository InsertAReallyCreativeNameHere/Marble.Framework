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

robin_hood::unordered_map<PackageSystem::TrueTypeFontPackageFile*, Text::RenderData*> Text::textFonts;

void Text::RenderData::trackCharacters(const std::vector<CharacterData>& text)
{
    for (auto it = text.begin(); it != text.end(); ++it)
    {
        auto c = this->characters.find(it->glyphIndex);
        if (c != this->characters.end())
            ++c->second->accessCount;
        else
        {
            GlyphOutline glyph = this->file->fontHandle().getGlyphOutline(it->glyphIndex);
            
            if (glyph.verts != nullptr) [[likely]]
            {
                auto buffers = glyph.createGeometryBuffers<Vertex2D>();
                auto charData = this->characters.insert(robin_hood::pair<int, CharacterRenderData*>(it->glyphIndex, new CharacterRenderData { 1, { } })).first->second;
                CoreEngine::queueRenderJobForFrame
                (
                    [pointsFlattened = std::move(buffers.first), indexesFlattened = std::move(buffers.second), data = charData]
                    {
                        data->polygon.create(std::move(pointsFlattened), std::move(indexesFlattened));
                    }
                );
            }
        }
    }
}
void Text::RenderData::untrackCharacters(const std::vector<CharacterData>& text)
{
    for (auto it = text.begin(); it != text.end(); ++it)
    {
        auto& charData = this->characters[it->glyphIndex];
        --charData->accessCount;
        if (charData->accessCount == 0)
        {
            CoreEngine::queueRenderJobForFrame([data = charData] { data->polygon.destroy(); delete data; });
            this->characters.erase(it->glyphIndex);
        }
    }
}

Text::Text() :
data(nullptr),
_text(U""),
_fontSize(11),
font
({
    [this]
    {
        return this->data ? this->data->file : nullptr;
    },
    [this](TrueTypeFontPackageFile* file)
    {
        if (this->data)
        {
            this->data->untrackCharacters(this->textData);
            --this->data->accessCount;
            if (this->data->accessCount == 0)
                delete this->data;
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
                this->data = Text::textFonts.insert(robin_hood::pair<TrueTypeFontPackageFile*, RenderData*>(file, new RenderData { 1, { }, file })).first->second;
                this->data->trackCharacters(this->textData);
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
        if (this->data)
        {
            this->_text = std::move(str);
            std::vector<CharacterData> oldTextData = std::move(this->textData);

            this->textData.clear();
            this->textData.reserve(this->_text.size());
            Font& font = this->data->file->fontHandle();
            for (auto it = this->_text.begin(); it != this->_text.end(); ++it)
            {
                int glyphIndex = font.getGlyphIndex(*it);
                this->textData.push_back
                ({
                    glyphIndex,
                    font.getGlyphMetrics(glyphIndex)
                });
            }

            this->data->trackCharacters(this->textData);
            this->data->untrackCharacters(oldTextData);
        }
    }
}),
fontSize
({
    [&]() -> uint32_t
    {
        return this->_fontSize;
    },
    [&, this](uint32_t value) -> void
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
                    for (auto it = this->textData.begin(); it != this->textData.end(); ++it)
                        accAdv += it->metrics.advanceWidth;

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
                                advanceLengths.push_back(float(this->textData[i].metrics.advanceWidth) * glyphScale * scale.x);

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
                                accXAdvance += this->textData[i].metrics.advanceWidth;
                                break;
                            case U'\t':
                                accXAdvance += this->textData[i].metrics.advanceWidth * 8;
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
    if (this->data)
    {
        this->data->untrackCharacters(this->textData);
        --this->data->accessCount;
        if (this->data->accessCount == 0)
            delete this->data;
    }
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
                advanceLengths.push_back(float(this->textData[i].metrics.advanceWidth) * glyphScale * scale.x);

            auto advanceLenIt = advanceLengths.begin();

            auto drawCharAndIterNext = [&](size_t i)
            {
                auto c = this->data->characters.find(this->textData[i].glyphIndex);
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
                accXAdvance += this->textData[i].metrics.advanceWidth;
                break;
            case U'\t':
                accXAdvance += this->textData[i].metrics.advanceWidth * 8;
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