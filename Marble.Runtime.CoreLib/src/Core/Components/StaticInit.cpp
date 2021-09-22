#include <Core/Components/Image.h>
#include <Core/Components/Panel.h>
#include <Core/Components/Text.h>
#include <Core/EntityComponentSystem/CoreSystem.h>
#include <Utility/TypeInfo.h>

using namespace Marble;
using namespace Marble::Internal;

namespace Marble::Internal
{
    static struct ComponentCoreStaticInit {
        ComponentCoreStaticInit()
        {
            InternalEngineEvent::OnRenderOffloadForComponent += [](Component* component)
            {
                switch (component->typeIndex())
                {
                case __typeid(Panel).qualifiedNameHash():
                    static_cast<Panel*>(component)->renderOffload();
                    break;
                case __typeid(Image).qualifiedNameHash():
                    static_cast<Image*>(component)->renderOffload();
                    break;
                case __typeid(Text).qualifiedNameHash():
                    static_cast<Text*>(component)->renderOffload();
                    break;
                }
            };
            InternalEngineEvent::OnRenderShutdown += []
            {
                for (auto it = Image::imageTextures.begin(); it != Image::imageTextures.end(); ++it)
                {
                    it->second->internalTexture.destroy();
                    delete it->second;
                }
            };
        };
    } init;
}
