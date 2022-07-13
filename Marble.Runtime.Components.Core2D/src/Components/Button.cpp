#include "Button.h"

#include <cmath>
#include <Mathematics.h>
#include <Core/CoreEngine.h>
#include <Core/Input.h>
#include <Components/RectTransform.h>

using namespace Marble;
using namespace Marble::GL;
using namespace Marble::Internal;
using namespace Marble::Mathematics;

void ColorHighlightButton::renderOffload()
{
    ProfileFunction();
    
    RectTransform* thisRect = this->rectTransform();
    decltype(auto) pos = thisRect->position();
    decltype(auto) rot = thisRect->rotation();
    decltype(auto) scale = thisRect->scale();
    decltype(auto) rect = thisRect->rect();

    ColoredTransformHandle t;
    t.setPosition(pos.x, pos.y);
    t.setOffset((rect.right + rect.left) / 2, (rect.top + rect.bottom) / 2);
    t.setScale(scale.x * (rect.right - rect.left), scale.y * (rect.top - rect.bottom));
    t.setRotation(deg2RadF(rot));

    if (thisRect->queryPointIn((Vector2)Input::mousePosition()))
        t.setColor((float)this->highlightColor.r / 255.0f, (float)this->highlightColor.g / 255.0f, (float)this->highlightColor.b / 255.0f, (float)this->highlightColor.a / 255.0f);
    else t.setColor((float)this->idleColor.r / 255.0f, (float)this->idleColor.g / 255.0f, (float)this->idleColor.b / 255.0f, (float)this->idleColor.a / 255.0f);
    
    CoreEngine::queueRenderJobForFrame
    (
        [t]()
        {
            Renderer::drawUnitSquare(t);
        }
    );
}