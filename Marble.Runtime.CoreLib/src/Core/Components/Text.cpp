#include "Text.h"

#include <Mathematics.h>
#include <numeric>
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
    ProfileFunction();
    for (auto it = text.begin(); it != text.end(); ++it)
    {
        auto c = this->characters.find(it->glyphIndex);
        if (c != this->characters.end())
            ++c->second->accessCount;
        else
        {
            GlyphOutline glyph = this->file->fontHandle().getGlyphOutline(it->glyphIndex);
            if (glyph.verts) [[likely]]
            {
                auto buffers = glyph.createGeometryBuffers<Vertex2D>();
                auto charData = this->characters.insert(robin_hood::pair<int, CharacterRenderData*>(it->glyphIndex, new CharacterRenderData { 1, { } })).first->second;
                CoreEngine::queueRenderJobForFrame
                (
                    [pointsFlattened = std::move(buffers.first), indexesFlattened = std::move(buffers.second), data = charData]
                    { data->polygon.create(std::move(pointsFlattened), std::move(indexesFlattened)); }
                );
            }
        }
    }
}
void Text::RenderData::untrackCharacters(const std::vector<CharacterData>& text)
{
    ProfileFunction();
    for (auto it = text.begin(); it != text.end(); ++it)
    {
        auto charData = this->characters.find(it->glyphIndex);
        if (charData != this->characters.end())
        {
            --charData->second->accessCount;
            if (charData->second->accessCount == 0)
            {
                this->characters.erase(charData);
                CoreEngine::queueRenderJobForFrame([data = charData->second] { data->polygon.destroy(); delete data; });
            }
        }
    }
}

Text::~Text()
{
    ProfileFunction();
    if (this->data)
    {
        this->data->untrackCharacters(this->textData);
        --this->data->accessCount;
        if (this->data->accessCount == 0)
        {
            Text::textFonts.erase(this->data->file);
            delete this->data;
        }
    }
}

void Text::setFontFile(TrueTypeFontPackageFile* value)
{
    ProfileFunction();
    if (this->data)
    {
        this->data->untrackCharacters(this->textData);
        --this->data->accessCount;
        if (this->data->accessCount == 0)
        {
            Text::textFonts.erase(this->data->file);
            delete this->data;
        }
    }
    if (value)
    {
        auto set = Text::textFonts.find(value);
        if (set != Text::textFonts.end())
        {
            this->data = set->second;
            ++this->data->accessCount;
        }
        else
        {
            this->data = Text::textFonts.insert(robin_hood::pair<TrueTypeFontPackageFile*, RenderData*>(value, new RenderData { 1, { }, value })).first->second;
            this->data->trackCharacters(this->textData);
        }
    }
    else this->data = nullptr;
}
void Text::setText(std::u32string value)
{
    ProfileFunction();
    if (this->data)
    {
        this->_text = std::move(value);
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
void Text::setFontSize(uint32_t value)
{
    ProfileFunction();
    switch (value)
    {
    case FontSize::RecalculateForRectSize:
        {
            if (this->data) [[likely]]
            {
                const RectTransform* const thisRect = this->rectTransform();
                Font& font = this->data->file->fontHandle();

                const Vector2 pos = thisRect->position;
                const Vector2 scale = thisRect->scale;
                const RectFloat rect = thisRect->rect;

                float accAdv = 0;
                for (auto it = this->textData.begin(); it != this->textData.end(); ++it)
                    accAdv += it->metrics.advanceWidth;

                // NB: Calculate optimistic font size - the largest possible with given text.
                //     This is done to be generally performant with the following iterative
                //     font sizing algorithm.
                // TODO: Is it possible to eliminate the costly sqrt?
                this->_fontSize = (font.ascent - font.descent) *
                ((thisRect->rect().right - thisRect->rect().left) * scale.x / accAdv);
                this->_fontSize = this->_fontSize * sqrt((thisRect->rect().top - thisRect->rect().bottom) * scale.y / this->_fontSize);

                const float rectWidth = (rect.right - rect.left) * scale.x;
                const float rectHeight = (rect.top - rect.bottom) * scale.y;
                const float rot = deg2RadF(thisRect->rotation);
                const float asc = this->data->file->fontHandle().ascent;
                const float lineHeight = asc - this->data->file->fontHandle().descent;

                Debug::LogInfo("Optimistic font size: ", this->_fontSize, '.');

                while (this->_fontSize != 0)
                {
                    float glyphScale = float(this->_fontSize) / lineHeight;
                    float lineHeightScaled = lineHeight * glyphScale * scale.y;
                    
                    if (lineHeightScaled > rectHeight)
                    {
                        --this->_fontSize;
                        continue;
                    }
                    
                    float lineDiff = this->data->file->fontHandle().lineGap * glyphScale * scale.y + lineHeightScaled;
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
                    // NB: Do calculations for an individual text segment. This matters because
                    //     each word cannot be split across lines, unless there is no space for it to fit in one line.
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
                            // NB: If the word doesn't fit in one line,
                            //     split it across two, otherwise,
                            //     put it on a new line.
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

                        // NB: For when a word doesn't fit within its line,
                        //     and has to be split across two.
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
                                    goto Continue;
                                }
                            }
                            accXAdvance += *it;
                        }
                        goto ExitCalcWord;

                        // NB: More label abuse to allow for continuing
                        //     of the outer loop.
                        Continue:
                        continue;
                        // NB: Never falls through.

                        // NB: For when a text segment fits within its line.
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
                            {
                                float charAdvScaled = this->textData[i].metrics.advanceWidth * glyphScale * scale.x;
                                if (accXAdvance + charAdvScaled > rectWidth) [[unlikely]]
                                    goto NewLine;
                                else accXAdvance += charAdvScaled;
                            }
                            break;
                        case U'\t':
                            {
                                float charAdvScaled = this->textData[i].metrics.advanceWidth * glyphScale * scale.x * 8;
                                if (accXAdvance + charAdvScaled > rectWidth) [[unlikely]]
                                    goto NewLine;
                                else accXAdvance += charAdvScaled;
                            }
                            break;
                        case U'\r':
                            break;
                        case U'\n':
                        Newline:
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

                    ExitTextHandling:
                    Debug::LogInfo("Fitting font size: ", this->_fontSize, '.');
                    break;
                }
            }
        }
        break;
    default:
        this->_fontSize = value;
    }
}

void Text::renderOffload()
{
    ProfileFunction();
    if (this->data && !this->_text.empty()) [[likely]]
    {
        const RectTransform* const thisRect = this->rectTransform();
        const Vector2 pos = thisRect->position;
        const Vector2 scale = thisRect->scale;
        const RectFloat rect = thisRect->rect;

        const float rectWidth = (rect.right - rect.left) * scale.x;
        const float rectHeight = (rect.top - rect.bottom) * scale.y;
        const float rot = deg2RadF(thisRect->rotation);
        const float asc = this->data->file->fontHandle().ascent;
        const float lineHeight = asc - this->data->file->fontHandle().descent;
        const float glyphScale = float(this->_fontSize) / lineHeight;
        const float lineDiff = (this->data->file->fontHandle().lineGap + lineHeight) * glyphScale * scale.y;

        float accXAdvance = 0;
        float accYAdvance = 0;

        std::vector<float> advanceLengths;
        decltype(advanceLengths)::iterator advanceLenIt;
        std::vector<std::pair<decltype(this->data->characters)::iterator, float>> curLine;
        
        std::vector<std::pair<PolygonHandle, ColoredTransformHandle>> charsToDraw;

        auto pushCharAndIterNext = [&, this](size_t i)
        {
            auto c = this->data->characters.find(this->textData[i].glyphIndex);
            if (c != this->data->characters.end())
                curLine.push_back(std::make_pair(c, accXAdvance));

            accXAdvance += *advanceLenIt;
            ++advanceLenIt;
        };

        auto pushLineAndResetCurrent = [&, this]
        {
            charsToDraw.reserve(charsToDraw.size() + curLine.size());
            for (auto it = curLine.begin(); it != curLine.end(); ++it)
            {
                ColoredTransformHandle transform;
                charsToDraw.push_back(std::make_pair(it->first->second->polygon, ColoredTransformHandle()));
                charsToDraw.back().second.setPosition(pos.x, pos.y);
                switch (this->horizontalAlign)
                {
                case TextAlign::Justify:
                    charsToDraw.back().second.setOffset(rect.left * scale.x + it->second + (rectWidth - accXAdvance) * (it->second / accXAdvance), (rect.top - asc * glyphScale) * scale.y - accYAdvance);
                    break;
                case TextAlign::Major:
                    charsToDraw.back().second.setOffset(rect.left * scale.x + it->second + rectWidth - accXAdvance, (rect.top - asc * glyphScale) * scale.y - accYAdvance);
                    break;
                case TextAlign::Center:
                    charsToDraw.back().second.setOffset(rect.left * scale.x + it->second + (rectWidth - accXAdvance) / 2, (rect.top - asc * glyphScale) * scale.y - accYAdvance);
                    break;
                case TextAlign::Minor:
                default:
                    charsToDraw.back().second.setOffset(rect.left * scale.x + it->second, (rect.top - asc * glyphScale) * scale.y - accYAdvance);
                }
                charsToDraw.back().second.setScale(glyphScale * scale.x, glyphScale * scale.y);
                charsToDraw.back().second.setRotation(rot);
                charsToDraw.back().second.setColor(1.0f, 1.0f, 1.0f, 1.0f);
            }

            curLine.clear();
            accXAdvance = 0;
            accYAdvance += lineDiff;
        };
        
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
            curLine.reserve(curLine.size() + end - beg);
            advanceLengths.reserve(end - beg);
            for (size_t i = beg; i < end; i++)
                advanceLengths.push_back(float(this->textData[i].metrics.advanceWidth) * glyphScale * scale.x);

            advanceLenIt = advanceLengths.begin();

            float wordLen = std::accumulate(advanceLenIt, advanceLengths.end(), 0.0f);
            if (accXAdvance + wordLen > rectWidth) [[unlikely]]
            {
                if (wordLen > rectWidth) [[unlikely]]
                    goto SplitDrawWord;
                else
                {
                    pushLineAndResetCurrent();
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
                    pushLineAndResetCurrent();
                    curLine.reserve(end - i);
                }
                pushCharAndIterNext(i);
            }
            goto ExitDrawWord;

            // NB: Draw a word in one line. This makes the assumption
            //     that the whole word will fit within a line.
            InlineDrawWord:
            curLine.reserve(curLine.size() + end - beg);
            for (size_t i = beg; i < end; i++)
                pushCharAndIterNext(i);
            // NB: No goto here, jump would go to the same place anyways.

            ExitDrawWord:
            advanceLengths.clear();
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
                    {
                        float charAdvScaled = this->textData[i].metrics.advanceWidth * glyphScale * scale.x;
                        if (accXAdvance + charAdvScaled > rectWidth) [[unlikely]]
                            goto NewLine;
                        else accXAdvance += charAdvScaled;
                    }
                    break;
                case U'\t':
                    {
                        float charAdvScaled = this->textData[i].metrics.advanceWidth * glyphScale * scale.x * 8;
                        if (accXAdvance + charAdvScaled > rectWidth) [[unlikely]]
                            goto NewLine;
                        else accXAdvance += charAdvScaled;
                    }
                    break;
                case U'\r':
                    break;
                case U'\n':
                Newline:
                pushLineAndResetCurrent();
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

        ExitTextHandling:
        pushLineAndResetCurrent();
        CoreEngine::queueRenderJobForFrame
        (
            [charsToDraw = std::move(charsToDraw)]
            {
                for (auto it = charsToDraw.begin(); it != charsToDraw.end(); ++it)
                    Renderer::drawPolygon(it->first, it->second);
            }
        );
    }
}