#include "UILabel.h"
#include "../Renderer.h"
#include "../Font.h"

UILabel::UILabel()
    : mTextR(220), mTextG(220), mTextB(220)
{
}

void UILabel::onRender(Renderer& renderer, Font* font)
{
    if (mText.empty() || font == nullptr) return;

    float wx, wy;
    getWorldPosition(wx, wy);

    SDL_Texture* tex = font->renderText(renderer, mText, mTextR, mTextG, mTextB);
    if (tex != nullptr) {
        float tw, th;
        SDL_GetTextureSize(tex, &tw, &th);

        // 如果元素尺寸未设置，自动根据文字大小调整
        if (mWidth <= 0.0f) mWidth = tw;
        if (mHeight <= 0.0f) mHeight = th;

        renderer.drawTexture(tex, wx, wy);
        SDL_DestroyTexture(tex);
    }

    // 渲染子元素
    for (auto& child : mChildren) {
        if (child->mVisible) {
            child->onRender(renderer, font);
        }
    }
}
