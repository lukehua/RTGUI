#include <rtgui/rtgui_system.h>
#include <rtgui/dc.h>
#include <rtgui/blit.h>
#include <rtgui/animation.h>

void rtgui_anim_engine_move(struct rtgui_dc *background,
                            struct rtgui_dc_buffer *background_buffer,
                            struct rtgui_dc_buffer *items,
                            int item_cnt,
                            int progress,
                            void *param)
{
    int cx, cy, w, h;
    struct rtgui_anim_engine_move_ctx *ctx = param;
    struct rtgui_rect dc_rect;

    if (!background)
        return;

    if (background_buffer)
        rtgui_dc_blit((struct rtgui_dc*)background_buffer,
                      NULL, background, NULL);

    rtgui_dc_get_rect(background, &dc_rect);

    cx = progress * (ctx->end.x - ctx->start.x) / RTGUI_ANIM_TICK_RANGE;
    cy = progress * (ctx->end.y - ctx->start.y) / RTGUI_ANIM_TICK_RANGE;
    w = rtgui_rect_width(dc_rect);
    h = rtgui_rect_height(dc_rect);

    dc_rect.x1 = cx + ctx->start.x;
    dc_rect.y1 = cy + ctx->start.y;
    dc_rect.x2 = dc_rect.x1 + w;
    dc_rect.y2 = dc_rect.y1 + h;

    /* To prevent overlapping, only one item can be drawn by
     * rtgui_anim_engine_move. */
    if (items)
        rtgui_dc_blit((struct rtgui_dc*)(items), NULL, background, &dc_rect);
}

void rtgui_anim_engine_fade(struct rtgui_dc *background,
                            struct rtgui_dc_buffer *background_buffer,
                            struct rtgui_dc_buffer *items,
                            int item_cnt,
                            int progress,
                            void *param)
{
    int cur_lvl;
    struct rtgui_blit_info info;
    struct rtgui_dc_buffer *buf;
    struct rtgui_anim_engine_fade_ctx *ctx = param;

    if (!background_buffer || !items)
        return;

    RT_ASSERT(background);

    /* NOTE: the underlaying dc only support 5bits(32 levels) alpha value. */
    cur_lvl = progress * 255 / RTGUI_ANIM_TICK_RANGE;
    if (ctx->is_fade_out)
        cur_lvl = 255 - cur_lvl;
    /* Only 5bits of alpha is effective. But always update the dc when alpha is 0 or
     * 255. */
    if ((cur_lvl >> 3) == (ctx->plvl >> 3))
    {
        if (cur_lvl == 255 || cur_lvl == 0)
        {
            if (cur_lvl == ctx->plvl)
                return;
        }
        else
        {
            return;
        }
    }
    ctx->plvl = cur_lvl;

    buf = (struct rtgui_dc_buffer*)rtgui_dc_buffer_create_from_dc(
                                       (struct rtgui_dc*)background_buffer);
    if (!buf)
        return;

    info.a = cur_lvl;

    info.src       = items->pixel;
    info.src_fmt   = items->pixel_format;
    info.src_h     = items->height;
    info.src_w     = items->width;
    info.src_pitch = items->pitch;
    info.src_skip  = info.src_pitch - info.src_w *
                    rtgui_color_get_bpp(items->pixel_format);

    info.dst       = buf->pixel;
    info.dst_fmt   = buf->pixel_format;
    info.dst_h     = buf->height;
    info.dst_w     = buf->width;
    info.dst_pitch = buf->pitch;
    info.dst_skip  = info.dst_pitch - info.dst_w *
                    rtgui_color_get_bpp(buf->pixel_format);

    rtgui_blit(&info);
    rtgui_dc_blit((struct rtgui_dc*)buf, NULL, background, NULL);
    rtgui_dc_destory((struct rtgui_dc*)buf);
}
