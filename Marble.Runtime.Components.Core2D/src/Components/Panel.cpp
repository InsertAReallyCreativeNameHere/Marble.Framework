#include "Panel.h"

#include <Mathematics.h>
#include <Core/CoreEngine.h>
#include <Components/RectTransform.h>
#include <Rendering/Renderer.h>

using namespace Marble;
using namespace Marble::Internal;
using namespace Marble::Mathematics;
using namespace Marble::GL;

void Panel::renderOffload()
{
    ProfileFunction();
    
    RectTransform* thisRect = this->rectTransform();
    const Vector2& pos = thisRect->position;
    const Vector2& scale = thisRect->scale;
    const RectFloat& rect = thisRect->rect;

    ColoredTransformHandle t;
    t.setPosition(pos.x, pos.y);
    t.setOffset((rect.right + rect.left) / 2, (rect.top + rect.bottom) / 2);
    t.setScale(scale.x * (rect.right - rect.left), scale.y * (rect.top - rect.bottom));
    t.setRotation(deg2RadF(thisRect->rotation));
    t.setColor(this->_color.r, this->_color.g, this->_color.b, this->_color.a);
    
    CoreEngine::queueRenderJobForFrame([t]
    {
        Renderer::drawUnitSquare(t);
    });
}