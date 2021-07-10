#include "Text.h"

#include <Core/CoreEngine.h>
#include <Rendering/Core/Renderer.h>
#undef STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>
#include <tuple>

using namespace Marble;
using namespace Marble::GL;
using namespace Marble::Internal;
using namespace Marble::PackageSystem;
using namespace Marble::Typography;

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
                auto buffers = glyph.createGeometryBuffers<Vertex2D>();
                
                if (glyph.verts != nullptr) [[likely]]
                {
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