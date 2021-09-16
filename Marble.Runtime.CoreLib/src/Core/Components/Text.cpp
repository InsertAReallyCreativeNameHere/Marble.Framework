#include "Text.h"

#include <Core/CoreEngine.h>
#include <Core/Components/RectTransform.h>
#include <Rendering/Core/Renderer.h>
#include <Mathematics.h>
#undef STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>
#include <tuple>

using namespace Marble;
using namespace Marble::GL;
using namespace Marble::Mathematics;
using namespace Marble::Internal;
using namespace Marble::PackageSystem;
using namespace Marble::Typography;

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
                    CoreEngine::pendingRenderJobBatchesOffload.push_back
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
                CoreEngine::pendingRenderJobBatchesOffload.push_back([=, data = this->data->characters[it->first]] { data->polygon.destroy(); delete data; });
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
                RectTransform* rt = this->rectTransform();
                Font& font = this->data->file->fontHandle();
                Vector2 pos = rt->position();
                Vector2 scale = rt->scale();
                RectFloat rect = rt->rect();
                float rectWidth = (rect.right - rect.left) * scale.x;
                float rectHeight = (rect.top - rect.bottom) * scale.y;
                float rot = deg2RadF(rt->rotation);
                float asc = font.ascent;
                float lineHeight = asc - font.descent;
                float accXAdvance = 0;
                float accYAdvance = 0;

                float accAdv = 0;
                for (auto it = this->_text.begin(); it != this->_text.end(); ++it)
                    accAdv += font.getCodepointMetrics(*it).advanceWidth;

                // Calculate optimistic font size - the largest possible with given text.
                // TODO: Is it possible to eliminate the costly sqrt?
                this->_fontSize = (font.ascent - font.descent) *
                ((this->rectTransform()->rect().right - this->rectTransform()->rect().left) / accAdv);
                this->_fontSize = this->_fontSize * sqrt((this->rectTransform()->rect().top - this->rectTransform()->rect().bottom) / this->_fontSize);

                size_t beg = 0;
                size_t end;

                auto calcNextWord = [&, this]
                {
                    std::vector<float> advanceLengths;
                    advanceLengths.reserve(end - beg);
                    for (size_t i = beg; i < end; i++)
                        advanceLengths.push_back(float(this->data->file->fontHandle().getCodepointMetrics(this->_text[i]).advanceWidth) * glyphScale * scale.x);

                    auto advanceLenIt = advanceLengths.begin();
                    for (size_t i = beg; i < end; i++)
                    {
                        if (accXAdvance + *advanceLenIt > rectWidth) [[unlikely]]
                        {
                            accXAdvance = 0;
                            accYAdvance += lineDiff;
                        }
                        accXAdvance += float(*advanceLenIt);
                        ++advanceLenIt;
                    }
                };

                while (this->_fontSize != 0)
                {
                    float glyphScale = float(this->_fontSize) / lineHeight;
                    float lineDiff = (font.lineGap + lineHeight) * glyphScale * scale.y;
                    float spaceAdv = font.getCodepointMetrics(U' ').advanceWidth * glyphScale * scale.x;

                    while ((end = this->_text.find_first_of(U" \t\r\n", beg + 1)) != std::u32string::npos)
                    {
                        calcNextWord();
                        if (accYAdvance > rectHeight)
                            goto CalcOneLess;

                        beg = end;
                        end = this->_text.find_first_not_of(U" \n", beg + 1);

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
                    }

                    end = this->_text.size();
                    calcNextWord();
                    if (accYAdvance <= rectHeight)
                        break;

                    CalcOneLess:
                    --this->_fontSize;
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
                CoreEngine::pendingRenderJobBatchesOffload.push_back([=, data = this->data->characters[*it]] { data->polygon.destroy(); delete data; });
            this->data->characters.erase(*it);
        }
    }
    --this->data->accessCount;
    if (this->data->accessCount == 0)
        delete this->data;
}