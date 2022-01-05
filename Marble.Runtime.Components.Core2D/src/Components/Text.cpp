#include "Text.h"

#include <cmath>
#include <numeric>
#include <tuple>
#include <Mathematics.h>
#include <Core/CoreEngine.h>
#include <Core/Debug.h>
#include <EntityComponentSystem/RectTransform.h>
#include <Rendering/Renderer.h>

using namespace Marble;
using namespace Marble::Internal;
using namespace Marble::Mathematics;
using namespace Marble::Typography;
using namespace Marble::GL;
using namespace Marble::PackageSystem;

robin_hood::unordered_map<PackageSystem::TrueTypeFontPackageFile*, Text::RenderData*> Text::textFonts;

inline static float getAlignOffsetJustify(float endDiff, float mul)
{
    return endDiff * mul;
}
inline static float getAlignOffsetMajor(float endDiff, float)
{
    return endDiff;
}
inline static float getAlignOffsetCenter(float endDiff, float)
{
    return endDiff / 2;
}

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
void Text::setHorizontalAlign(TextAlign value)
{
    switch (value)
    {
    case TextAlign::Justify:
        this->getHorizontalAlignOffset = getAlignOffsetJustify;
        break;
    case TextAlign::Major:
        this->getHorizontalAlignOffset = getAlignOffsetMajor;
        break;
    case TextAlign::Center:
        this->getHorizontalAlignOffset = getAlignOffsetCenter;
        break;
    case TextAlign::Minor:
        this->getHorizontalAlignOffset = Text::getAlignOffsetMinor;
        break;
    }
}
void Text::setVerticalAlign(TextAlign value)
{
    switch (value)
    {
    case TextAlign::Justify:
        this->getVerticalAlignOffset = getAlignOffsetJustify;
        break;
    case TextAlign::Major:
        this->getVerticalAlignOffset = getAlignOffsetMajor;
        break;
    case TextAlign::Center:
        this->getVerticalAlignOffset = getAlignOffsetCenter;
        break;
    case TextAlign::Minor:
        this->getVerticalAlignOffset = Text::getAlignOffsetMinor;
        break;
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
                const RectFloat rect = thisRect->rect;

                float accAdv = 0;
                for (auto it = this->textData.begin(); it != this->textData.end(); ++it)
                    accAdv += it->metrics.advanceWidth;

                const float rectWidth = thisRect->rect().right - thisRect->rect().left;
                const float rectHeight = thisRect->rect().top - thisRect->rect().bottom;

                // NB: Calculate optimistic font size - the largest possible with given text.
                //     This is done to be generally performant with the following iterative
                //     font sizing algorithm.
                // TODO: Is it possible to eliminate the costly sqrt?
                float set = (font.ascent - font.descent) * (rectWidth / accAdv);
                this->_fontSize = set * sqrtf(rectHeight / set);

                const float rot = deg2RadF(thisRect->rotation);
                const float asc = this->data->file->fontHandle().ascent;
                const float lineHeight = asc - this->data->file->fontHandle().descent;

                Debug::LogInfo("Optimistic font size: ", this->_fontSize, '.');

                while (this->_fontSize != 0)
                {
                    float glyphScale = float(this->_fontSize) / lineHeight;
                    float lineHeightScaled = lineHeight * glyphScale;
                    
                    if (lineHeightScaled > rectHeight)
                    {
                        --this->_fontSize;
                        continue;
                    }
                    
                    float lineDiff = this->data->file->fontHandle().lineGap * glyphScale + lineHeightScaled;
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
                            advanceLengths.push_back(float(this->textData[i].metrics.advanceWidth) * glyphScale);

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
                            accXAdvance += this->textData[i].metrics.advanceWidth * glyphScale;
                            break;
                        case U'\t':
                            accXAdvance += this->textData[i].metrics.advanceWidth * glyphScale * 8;
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
        const float lineHeightScaled = lineHeight * glyphScale * scale.y;
        const float lineDiff = (this->data->file->fontHandle().lineGap + lineHeight) * glyphScale * scale.y;
        const uint32_t maxLines = uint32_t(rectHeight > 0 ? rectHeight : 0) / (this->_fontSize > 0 ? this->_fontSize : 1);
        const float effectiveHeight = float(maxLines) * this->_fontSize;

        float accYAdvance = 0.0f;
        float spaceAdv = 0.0f;

        std::vector<float> advanceLengths;
        decltype(advanceLengths)::iterator advanceLenIt;

        struct Character
        {
            CharacterRenderData* character;
            float xOffset;
        };
        struct Word
        {
            std::vector<Character> characters;
        };
        struct Line
        {
            std::vector<Word> words;
            float width;
            float yOffset;
        };
        std::vector<Line> lines
        ({{ { }, 0.0f, 0.0f }});

        float curLineIndex = 0;
        float curVertAlignOffset = this->getVerticalAlignOffset(rectHeight - effectiveHeight, 0.0f);
        
        auto pushCharAndIterNext = [&, this](size_t i)
        {
            auto c = this->data->characters.find(this->textData[i].glyphIndex);
            if (c != this->data->characters.end())
                lines.back().words.back().characters.push_back({ c->second, lines.back().width });

            lines.back().width += *advanceLenIt;
            ++advanceLenIt;
        };

        auto pushCurrentLine = [&, this]
        {
            accYAdvance += lineDiff;
            lines.push_back({ { }, 0.0f, accYAdvance });
        };
        
        size_t beg = 0;
        size_t end;

        // NB: Horrific code that avoids multiple calls of find_first_*.
        //     I don't even know if this was optimal.
        if
        (
            size_t _end = this->_text.find_first_of(U" \t\r\n", 0);
            _end > (end = this->_text.find_first_not_of(U" \t\r\n", 0))
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
            advanceLengths.reserve(end - beg);
            for (size_t i = beg; i < end; i++)
                advanceLengths.push_back(float(this->textData[i].metrics.advanceWidth) * glyphScale * scale.x);

            advanceLenIt = advanceLengths.begin();

            float wordLen = std::accumulate(advanceLenIt, advanceLengths.end(), 0.0f);
            if (lines.back().width + spaceAdv + wordLen > rectWidth) [[unlikely]]
            {
                if (wordLen > rectWidth) [[unlikely]]
                {
                    lines.back().words.push_back({ { } });
                    lines.back().words.back().characters.reserve(end - beg);
                    goto SplitDrawWord;
                }
                else
                {
                    if (accYAdvance + lineDiff + lineHeightScaled > rectHeight)
                        goto ExitTextHandling;
                    pushCurrentLine();
                    lines.back().words.push_back({ { } });
                    lines.back().words.back().characters.reserve(end - beg);
                }
            }
            else
            {
                lines.back().width += spaceAdv;
                lines.back().words.push_back({ { } });
                lines.back().words.back().characters.reserve(end - beg);
                goto InlineDrawWord;
            }

            // NB: Draw a word split across lines. This is used when
            //     a word is longer than the width of a line.
            SplitDrawWord:
            for (size_t i = beg; i < end; i++)
            {
                if (lines.back().width + *advanceLenIt > rectWidth) [[unlikely]]
                {
                    if (accYAdvance + lineDiff + lineHeightScaled > rectHeight)
                        goto ExitTextHandling;
                    pushCurrentLine();
                    lines.back().words.push_back({ { } });
                    lines.back().words.back().characters.reserve(end - i);
                }
                pushCharAndIterNext(i);
            }
            goto ExitDrawWord;

            // NB: Draw a word in one line. This makes the assumption
            //     that the whole word will fit within a line.
            InlineDrawWord:
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
        {
            spaceAdv = 0.0f;
            for (size_t i = beg; i < end; i++)
            {
                switch (this->_text[i])
                {
                    case U' ':
                        spaceAdv += this->textData[i].metrics.advanceWidth * glyphScale * scale.x;
                        break;
                    case U'\t':
                        spaceAdv += this->textData[i].metrics.advanceWidth * glyphScale * scale.x * 8;
                        break;
                    case U'\r':
                        break;
                    case U'\n':
                    Newline:
                    if (accYAdvance + lineDiff + lineHeightScaled > rectHeight)
                        goto ExitTextHandling;
                    spaceAdv = 0.0f;
                    pushCurrentLine();
                    break;
                }
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
        std::vector<std::pair<CharacterRenderData*, ColoredTransformHandle>> charsToDraw;
        for (auto it1 = lines.begin(); it1 != lines.end(); ++it1)
        {
            for (auto it2 = it1->words.begin(); it2 != it1->words.end(); ++it2)
            {
                charsToDraw.reserve(charsToDraw.size() + it2->characters.size());
                for (auto it3 = it2->characters.begin(); it3 != it2->characters.end(); ++it3)
                {
                    ColoredTransformHandle transform;
                    transform.setPosition(pos.x, pos.y);
                    transform.setOffset
                    (
                        rect.left * scale.x + it3->xOffset +
                        this->getHorizontalAlignOffset(rectWidth - it1->width, float(it2 - it1->words.begin()) / (it1->words.size() > 1 ? it1->words.size() - 1 : 1)),
                        (rect.top - asc * glyphScale) * scale.y - it1->yOffset -
                        this->getVerticalAlignOffset(rectHeight - lines.size() * lineDiff, float(it1 - lines.begin()) / (lines.size() > 1 ? lines.size() - 1 : 1))
                    );
                    transform.setScale(glyphScale * scale.x, glyphScale * scale.y);
                    transform.setRotation(rot);
                    transform.setColor(1.0f, 1.0f, 1.0f, 1.0f);
                    charsToDraw.push_back(std::make_pair(it3->character, transform));
                }
            }
        }
        CoreEngine::queueRenderJobForFrame
        (
            [charsToDraw = std::move(charsToDraw)]
            {
                for (auto it = charsToDraw.begin(); it != charsToDraw.end(); ++it)
                    Renderer::drawPolygon(it->first->polygon, it->second);
            }
        );
    }
}