#include "app.h"
#include "u8g2.h"
#include "u8g2_port.h"
#include "shell_port.h"

#include <stdio.h>
#include <string.h>

/* ================================================================== */
/* 配置                                                                */
/* ================================================================== */
#define FRAME_MS     250          /* 每帧时长 */
#define YELLOW_H     16           /* 顶部黄区高度 */

#define STAR_N       26

/* ================================================================== */
/* 图形资源                                                            */
/* ================================================================== */
static const uint8_t bmp_star[7]  = {0x08, 0x08, 0x08, 0x7F, 0x08, 0x08, 0x08};

/* sin(2*pi*i/64) * 127 */
static const int8_t k_sin[64] = {
       0,   12,   25,   37,   49,   60,   71,   81,
      90,   98,  106,  112,  117,  122,  125,  126,
     127,  126,  125,  122,  117,  112,  106,   98,
      90,   81,   71,   60,   49,   37,   25,   12,
       0,  -12,  -25,  -37,  -49,  -60,  -71,  -81,
     -90,  -98, -106, -112, -117, -122, -125, -126,
    -127, -126, -125, -122, -117, -112, -106,  -98,
     -90,  -81,  -71,  -60,  -49,  -37,  -25,  -12,
};

static int isin(int ph) { return k_sin[ph & 63]; }
static int icos(int ph) { return k_sin[(ph + 16) & 63]; }

/* ================================================================== */
/* 通用绘制辅助                                                        */
/* ================================================================== */
static void draw_header(uint8_t tick)
{
    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_DrawBox(&u8g2, 0, 0, 128, YELLOW_H);

    u8g2_SetDrawColor(&u8g2, 0);
    u8g2_SetFont(&u8g2, u8g2_font_7x13B_tr);
    u8g2_DrawStr(&u8g2, 3, 12, "STM32F103VCT6");

    for (uint8_t i = 0; i < 3; i++)
    {
        u8g2_uint_t x = 108 + i * 7;
        if ((tick % 3) == i)
        {
            u8g2_DrawBox(&u8g2, x, 6, 4, 4);
        }
        else
        {
            u8g2_DrawFrame(&u8g2, x, 6, 4, 4);
        }
    }

    u8g2_SetDrawColor(&u8g2, 1);
}

static void draw_row(uint8_t y, const char *label, const char *value)
{
    u8g2_uint_t lw, vw, vx, x;

    u8g2_SetFont(&u8g2, u8g2_font_6x10_tf);
    u8g2_DrawStr(&u8g2, 6, y, label);

    vw = u8g2_GetStrWidth(&u8g2, value);
    vx = 122 - vw;
    u8g2_DrawStr(&u8g2, vx, y, value);

    lw = u8g2_GetStrWidth(&u8g2, label);
    for (x = 6 + lw + 3; x + 3 < vx; x += 3)
    {
        u8g2_DrawPixel(&u8g2, x, y - 3);
    }
}

/* ================================================================== */
/* shell <-> UI 消息                                                   */
/* ================================================================== */
static char s_msg[40];

void app_set_message(const char *msg)
{
    strncpy(s_msg, msg, sizeof(s_msg) - 1);
    s_msg[sizeof(s_msg) - 1] = '\0';
}

/* ================================================================== */
/* 场景 1：信息面板                                                    */
/* ================================================================== */
static void scene_dashboard(uint16_t frame, uint8_t tick)
{
    char uptime[16];
    uint32_t seconds = HAL_GetTick() / 1000U;

    (void)frame;

    snprintf(uptime, sizeof(uptime), "%02u:%02u:%02u",
             (unsigned)(seconds / 3600U),
             (unsigned)((seconds / 60U) % 60U),
             (unsigned)(seconds % 60U));

    u8g2_ClearBuffer(&u8g2);
    draw_header(tick);
    draw_row(29, "Cortex-M3", "72 MHz");
    draw_row(41, "I2C1", "400 kHz");

    if (s_msg[0] != '\0')
    {
        u8g2_SetFont(&u8g2, u8g2_font_6x10_tf);
        u8g2_DrawStr(&u8g2, 6, 53, s_msg);
    }
    else
    {
        draw_row(53, "Uptime", uptime);
    }

    u8g2_SendBuffer(&u8g2);
}

/* ================================================================== */
/* 场景 2：宿傩 · 伏魔御厨子                                           */
/* ================================================================== */
static void scene_sukuna(uint16_t frame, uint8_t tick)
{
    u8g2_ClearBuffer(&u8g2);
    draw_header(tick);

    /* 鸟居 */
    u8g2_DrawBox(&u8g2, 62, 21, 49, 5);
    u8g2_DrawTriangle(&u8g2, 62, 26, 57, 18, 70, 26);
    u8g2_DrawTriangle(&u8g2, 110, 26, 115, 18, 102, 26);
    u8g2_DrawBox(&u8g2, 66, 33, 41, 4);
    u8g2_DrawBox(&u8g2, 70, 26, 6, 37);
    u8g2_DrawBox(&u8g2, 97, 26, 6, 37);
    u8g2_DrawBox(&u8g2, 83, 26, 7, 8);

    /* 骷髅 */
    for (int s = 0; s < 2; s++)
    {
        int sx = 80 + s * 12;
        u8g2_DrawDisc(&u8g2, sx, 54, 4, U8G2_DRAW_ALL);
        u8g2_SetDrawColor(&u8g2, 0);
        u8g2_DrawDisc(&u8g2, sx - 2, 53, 1, U8G2_DRAW_ALL);
        u8g2_DrawDisc(&u8g2, sx + 2, 53, 1, U8G2_DRAW_ALL);
        u8g2_SetDrawColor(&u8g2, 1);
    }

    /* 文字 */
    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
    u8g2_DrawStr(&u8g2, 4, 30, "RYOMEN");
    u8g2_DrawStr(&u8g2, 4, 40, "SUKUNA");

    /* 咒力火焰 */
    {
        int off = (int)(frame % 3);
        u8g2_DrawLine(&u8g2, 4, 62, 8, 48 + off);
        u8g2_DrawLine(&u8g2, 8, 48 + off, 12, 58);
        u8g2_DrawLine(&u8g2, 12, 58, 16, 46 + off);
        u8g2_DrawLine(&u8g2, 16, 46 + off, 20, 60);
        u8g2_DrawLine(&u8g2, 20, 60, 24, 50 + off);
        u8g2_DrawLine(&u8g2, 24, 50 + off, 28, 62);
    }

    u8g2_SendBuffer(&u8g2);
}

/* ================================================================== */
/* 场景 3：写轮眼 (NARUTO)                                             */
/* ================================================================== */
static void scene_sharingan(uint16_t frame, uint8_t tick)
{
    const int cx = 64;
    const int cy = 40;
    int ph = (int)frame * 3;

    u8g2_ClearBuffer(&u8g2);
    draw_header(tick);

    u8g2_DrawCircle(&u8g2, cx, cy, 20, U8G2_DRAW_ALL);
    u8g2_DrawDisc(&u8g2, cx, cy, 3, U8G2_DRAW_ALL);

    for (int k = 0; k < 3; k++)
    {
        int a  = ph + k * 21;
        int px = cx + 13 * icos(a) / 127;
        int py = cy + 13 * isin(a) / 127;
        int prevx = px;
        int prevy = py;

        u8g2_DrawDisc(&u8g2, (u8g2_uint_t)px, (u8g2_uint_t)py, 3, U8G2_DRAW_ALL);

        for (int s = 1; s <= 3; s++)
        {
            int aa = a - s * 5;
            int rr = 13 + s * 2;
            int qx = cx + rr * icos(aa) / 127;
            int qy = cy + rr * isin(aa) / 127;
            u8g2_DrawLine(&u8g2, (u8g2_uint_t)prevx, (u8g2_uint_t)prevy,
                                 (u8g2_uint_t)qx, (u8g2_uint_t)qy);
            prevx = qx;
            prevy = qy;
        }
    }

    u8g2_SendBuffer(&u8g2);
}

/* ================================================================== */
/* 场景 4：精灵球 (POKEMON)                                            */
/* ================================================================== */
static void scene_pokeball(uint16_t frame, uint8_t tick)
{
    const int cx = 64;
    const int cy = 40;
    const int r  = 21;
    uint8_t ph = (uint8_t)(frame % 8);
    int wob = ((ph < 4) ? ph : (8 - ph)) - 2;
    int bx = cx + wob;

    u8g2_ClearBuffer(&u8g2);
    draw_header(tick);

    /* 上半球实心 */
    u8g2_DrawDisc(&u8g2, bx, cy, r, U8G2_DRAW_UPPER_LEFT | U8G2_DRAW_UPPER_RIGHT);

    /* 中间的黑色分界带 */
    u8g2_SetDrawColor(&u8g2, 0);
    u8g2_DrawBox(&u8g2, (u8g2_uint_t)(bx - r + 2), (u8g2_uint_t)(cy - 3),
                 (u8g2_uint_t)(2 * r - 4), 6);
    u8g2_SetDrawColor(&u8g2, 1);

    /* 外圈 + 按钮 */
    u8g2_DrawCircle(&u8g2, bx, cy, r, U8G2_DRAW_ALL);
    u8g2_DrawCircle(&u8g2, bx, cy, 7, U8G2_DRAW_ALL);
    u8g2_DrawDisc(&u8g2, bx, cy, 3, U8G2_DRAW_ALL);

    u8g2_SendBuffer(&u8g2);
}

/* ================================================================== */
/* 场景 5：水之呼吸 (鬼灭之刃)                                         */
/* ================================================================== */
static void draw_wave(int base, int amp, int fmul, int speed, uint16_t frame)
{
    int prev = base;

    for (int x = 0; x < 128; x += 2)
    {
        int idx = (x * fmul + (int)frame * speed) & 63;
        int y = base + amp * k_sin[idx] / 127;

        if (x > 0)
        {
            u8g2_DrawLine(&u8g2, (u8g2_uint_t)(x - 2), (u8g2_uint_t)prev,
                                 (u8g2_uint_t)x, (u8g2_uint_t)y);
        }
        prev = y;
    }
}

static void scene_water(uint16_t frame, uint8_t tick)
{
    u8g2_ClearBuffer(&u8g2);
    draw_header(tick);

    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
    u8g2_DrawStr(&u8g2, 4, 26, "SUI NO KOKYU");

    draw_wave(37, 5, 1,  2, frame);
    draw_wave(46, 6, 2, -3, frame);
    draw_wave(55, 5, 1,  3, frame);

    u8g2_SendBuffer(&u8g2);
}

/* ================================================================== */
/* 场景 6：星空                                                        */
/* ================================================================== */
typedef struct
{
    int16_t x;
    int16_t y;
    uint8_t spd;
} star_t;

static star_t   s_stars[STAR_N];
static uint16_t s_seed = 0x1234;

static uint16_t lcg(void)
{
    s_seed = (uint16_t)(s_seed * 25173u + 13849u);
    return s_seed;
}

static void space_enter(void)
{
    for (int i = 0; i < STAR_N; i++)
    {
        s_stars[i].x   = (int16_t)(lcg() % 128);
        s_stars[i].y   = (int16_t)(18 + lcg() % 44);
        s_stars[i].spd = (uint8_t)(1 + lcg() % 3);
    }
}

static void draw_rocket(int x, int y, uint8_t flicker)
{
    u8g2_DrawTriangle(&u8g2, x + 5, y, x, y + 6, x + 10, y + 6);
    u8g2_DrawBox(&u8g2, x, y + 6, 11, 12);
    u8g2_DrawTriangle(&u8g2, x, y + 12, x - 4, y + 20, x, y + 20);
    u8g2_DrawTriangle(&u8g2, x + 10, y + 12, x + 14, y + 20, x + 10, y + 20);

    u8g2_SetDrawColor(&u8g2, 0);
    u8g2_DrawDisc(&u8g2, x + 5, y + 9, 2, U8G2_DRAW_ALL);
    u8g2_SetDrawColor(&u8g2, 1);

    if (flicker)
    {
        u8g2_DrawTriangle(&u8g2, x + 2, y + 18, x + 8, y + 18, x + 5, y + 25);
    }
    else
    {
        u8g2_DrawTriangle(&u8g2, x + 3, y + 18, x + 7, y + 18, x + 5, y + 22);
    }
}

static void scene_space(uint16_t frame, uint8_t tick)
{
    uint8_t ph = (uint8_t)(frame % 8);
    int bob = ((ph < 4) ? ph : (8 - ph)) - 2;

    u8g2_ClearBuffer(&u8g2);
    draw_header(tick);

    for (int i = 0; i < STAR_N; i++)
    {
        s_stars[i].x -= s_stars[i].spd;
        if (s_stars[i].x < 0)
        {
            s_stars[i].x += 128;
            s_stars[i].y = (int16_t)(18 + lcg() % 44);
        }
        u8g2_DrawPixel(&u8g2, (u8g2_uint_t)s_stars[i].x, (u8g2_uint_t)s_stars[i].y);
        if (s_stars[i].spd >= 3)
        {
            u8g2_DrawPixel(&u8g2, (u8g2_uint_t)(s_stars[i].x + 1), (u8g2_uint_t)s_stars[i].y);
        }
    }

    if ((frame % 16) < 5)
    {
        int p = (int)(frame % 16);
        int sx = 118 - p * 12;
        int sy = 26 + p * 5;
        for (int k = 0; k < 6; k++)
        {
            u8g2_DrawPixel(&u8g2, (u8g2_uint_t)(sx + k), (u8g2_uint_t)(sy - k));
        }
    }

    u8g2_DrawDisc(&u8g2, 104, 52, 9, U8G2_DRAW_ALL);
    u8g2_SetDrawColor(&u8g2, 0);
    u8g2_DrawDisc(&u8g2, 100, 49, 2, U8G2_DRAW_ALL);
    u8g2_DrawDisc(&u8g2, 108, 55, 3, U8G2_DRAW_ALL);
    u8g2_SetDrawColor(&u8g2, 1);

    draw_rocket(24, 18 + bob, (uint8_t)(frame & 1U));

    u8g2_SendBuffer(&u8g2);
}

/* ================================================================== */
/* 场景 7：木叶护额 (NARUTO)                                           */
/* ================================================================== */
static void draw_spiral(int cx, int cy, int r0, int r1, int turns)
{
    const int steps = 160;
    int prevx = cx;
    int prevy = cy;

    for (int i = 0; i <= steps; i++)
    {
        int t  = i * 256 / steps;
        int ph = t * turns * 64 / 256;
        int r  = r0 + (r1 - r0) * t / 256;
        int x  = cx + r * icos(ph) / 127;
        int y  = cy + r * isin(ph) / 127;

        if (i > 0)
        {
            u8g2_DrawLine(&u8g2, (u8g2_uint_t)prevx, (u8g2_uint_t)prevy,
                                 (u8g2_uint_t)x, (u8g2_uint_t)y);
        }
        prevx = x;
        prevy = y;
    }
}

static void scene_headband(uint16_t frame, uint8_t tick)
{
    (void)frame;

    u8g2_ClearBuffer(&u8g2);
    draw_header(tick);

    /* 布带 + 金属护额 */
    u8g2_DrawFrame(&u8g2, 2, 22, 124, 37);
    u8g2_DrawRBox(&u8g2, 38, 25, 53, 31, 4);

    /* 木叶旋涡 */
    u8g2_SetDrawColor(&u8g2, 0);
    draw_spiral(64, 40, 1, 11, 2);
    u8g2_SetDrawColor(&u8g2, 1);

    u8g2_SendBuffer(&u8g2);
}

/* ================================================================== */
/* 场景 8：草帽 (ONE PIECE)                                            */
/* ================================================================== */
static void scene_strawhat(uint16_t frame, uint8_t tick)
{
    uint8_t ph = (uint8_t)(frame % 8);
    int bob = ((ph < 4) ? ph : (8 - ph)) - 2;
    int cx = 64;
    int cy = 46 + bob;

    u8g2_ClearBuffer(&u8g2);
    draw_header(tick);

    /* 帽檐 */
    u8g2_DrawEllipse(&u8g2, cx, cy, 44, 9, U8G2_DRAW_ALL);

    /* 帽冠（上半圆实心） */
    u8g2_DrawDisc(&u8g2, cx, cy - 2, 18, U8G2_DRAW_UPPER_LEFT | U8G2_DRAW_UPPER_RIGHT);

    /* 帽带 */
    u8g2_SetDrawColor(&u8g2, 0);
    u8g2_DrawBox(&u8g2, cx - 18, cy - 6, 36, 6);
    u8g2_SetDrawColor(&u8g2, 1);

    u8g2_SendBuffer(&u8g2);
}

/* ================================================================== */
/* 场景 9：NERV (EVA)                                                  */
/* ================================================================== */
static void scene_nerv(uint16_t frame, uint8_t tick)
{
    (void)frame;

    u8g2_ClearBuffer(&u8g2);
    draw_header(tick);

    /* 半叶 */
    u8g2_DrawEllipse(&u8g2, 64, 58, 44, 34, U8G2_DRAW_UPPER_LEFT | U8G2_DRAW_UPPER_RIGHT);
    u8g2_DrawHLine(&u8g2, 20, 58, 88);

    u8g2_SetFont(&u8g2, u8g2_font_7x13B_tr);
    u8g2_DrawStr(&u8g2, 50, 44, "NERV");

    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
    u8g2_DrawStr(&u8g2, 17, 56, "GOD'S IN HIS HEAVEN");

    u8g2_SendBuffer(&u8g2);
}

/* ================================================================== */
/* 场景 10：自由之翼 (进击的巨人)                                      */
/* ================================================================== */
static void draw_wing(int tipx, int tipy, int basex, int basey, int wb, int wt)
{
    int x0 = (tipx < basex) ? tipx : basex;
    int x1 = (tipx > basex) ? tipx : basex;

    for (int x = x0; x <= x1; x++)
    {
        int t = (basex == tipx) ? 0 : (x - basex) * 256 / (tipx - basex);
        int yc, w;

        if (t < 0)   t = 0;
        if (t > 256) t = 256;

        yc = basey + (tipy - basey) * t / 256;
        w  = wb + (wt - wb) * t / 256;
        if (w < 1) w = 1;

        u8g2_DrawVLine(&u8g2, (u8g2_uint_t)x, (u8g2_uint_t)(yc - w / 2), (u8g2_uint_t)w);
    }
}

static void scene_wings(uint16_t frame, uint8_t tick)
{
    (void)frame;

    u8g2_ClearBuffer(&u8g2);
    draw_header(tick);

    draw_wing(112, 24, 66, 58, 16, 3);
    draw_wing(16, 24, 62, 58, 16, 3);

    /* 中缝 + 羽毛分隔线 */
    u8g2_SetDrawColor(&u8g2, 0);
    u8g2_DrawVLine(&u8g2, 64, 20, 39);
    for (int k = 1; k <= 3; k++)
    {
        int xr = 66 + (112 - 66) * k / 4;
        int yr = 58 + (24 - 58) * k / 4;
        u8g2_DrawLine(&u8g2, (u8g2_uint_t)xr, (u8g2_uint_t)(yr - 2),
                             (u8g2_uint_t)(xr - 6), (u8g2_uint_t)(yr + 3));

        int xl = 62 + (16 - 62) * k / 4;
        int yl = 58 + (24 - 58) * k / 4;
        u8g2_DrawLine(&u8g2, (u8g2_uint_t)xl, (u8g2_uint_t)(yl - 2),
                             (u8g2_uint_t)(xl + 6), (u8g2_uint_t)(yl + 3));
    }
    u8g2_SetDrawColor(&u8g2, 1);

    u8g2_SendBuffer(&u8g2);
}

/* ================================================================== */
/* 场景 11：怪盗基德的预告函 (名侦探柯南)                              */
/* ================================================================== */
static void scene_kid(uint16_t frame, uint8_t tick)
{
    (void)frame;

    u8g2_ClearBuffer(&u8g2);
    draw_header(tick);

    /* 卡片 */
    u8g2_DrawRBox(&u8g2, 8, 20, 113, 39, 3);

    /* 礼帽 + 单片眼镜 */
    u8g2_SetDrawColor(&u8g2, 0);
    for (int y = 24; y <= 34; y++)
    {
        int xl = 22 + 2 * (y - 24) / 10;
        int xr = 38 - 2 * (y - 24) / 10;
        u8g2_DrawHLine(&u8g2, (u8g2_uint_t)xl, (u8g2_uint_t)y, (u8g2_uint_t)(xr - xl));
    }
    u8g2_DrawBox(&u8g2, 18, 34, 25, 2);
    u8g2_DrawCircle(&u8g2, 31, 45, 7, U8G2_DRAW_ALL);
    u8g2_DrawLine(&u8g2, 28, 45, 34, 45);

    /* 文字 */
    u8g2_SetFont(&u8g2, u8g2_font_7x13B_tr);
    u8g2_DrawStr(&u8g2, 50, 38, "KAITO KID");
    u8g2_SetFont(&u8g2, u8g2_font_5x7_tf);
    u8g2_DrawStr(&u8g2, 50, 52, "CALLING CARD");
    u8g2_SetDrawColor(&u8g2, 1);

    u8g2_SendBuffer(&u8g2);
}

/* ================================================================== */
/* 场景表 + 主循环                                                     */
/* ================================================================== */
typedef struct
{
    uint16_t duration;                    /* 停留帧数 */
    void   (*enter)(void);                /* 进入时调用，可为 NULL */
    void   (*draw)(uint16_t frame, uint8_t tick);
} scene_t;

static const scene_t k_scenes[] = {
    {20, 0,          scene_dashboard},
    {24, 0,          scene_sukuna   },
    {24, 0,          scene_sharingan},
    {24, 0,          scene_pokeball },
    {24, 0,          scene_water    },
    {24, 0,          scene_headband },
    {24, 0,          scene_strawhat },
    {24, 0,          scene_nerv     },
    {24, 0,          scene_wings    },
    {24, 0,          scene_kid      },
    {24, space_enter, scene_space   },
};

#define SCENE_COUNT  (sizeof(k_scenes) / sizeof(k_scenes[0]))

static void splash_play(void)
{
    float p = 0.0f;

    for (uint8_t i = 0; i < 40; i++)
    {
        p += (96.0f - p) * 0.18f;

        u8g2_ClearBuffer(&u8g2);
        draw_header(0);

        u8g2_DrawRBox(&u8g2, 49, 20, 30, 22, 3);
        u8g2_SetDrawColor(&u8g2, 0);
        u8g2_DrawRBox(&u8g2, 53, 24, 22, 14, 2);
        u8g2_SetDrawColor(&u8g2, 1);

        for (uint8_t k = 0; k < 3; k++)
        {
            u8g2_DrawBox(&u8g2, 43, 24 + k * 6, 6, 3);
            u8g2_DrawBox(&u8g2, 79, 24 + k * 6, 6, 3);
        }

        u8g2_SetFont(&u8g2, u8g2_font_6x10_tf);
        u8g2_DrawStr(&u8g2, 34, 52, "Mini Board");

        if ((i % 4) != 3) u8g2_DrawXBMP(&u8g2, 34, 17, 7, 7, bmp_star);
        if ((i % 4) != 1) u8g2_DrawXBMP(&u8g2, 87, 17, 7, 7, bmp_star);
        if ((i % 4) != 2) u8g2_DrawXBMP(&u8g2, 34, 38, 7, 7, bmp_star);

        u8g2_DrawFrame(&u8g2, 14, 56, 100, 6);
        if (p > 0.5f)
        {
            u8g2_DrawBox(&u8g2, 16, 58, (u8g2_uint_t)(p + 0.5f), 2);
        }

        u8g2_SendBuffer(&u8g2);
        HAL_Delay(50);
    }

    HAL_Delay(500);
}

void user_main(void)
{
    uint8_t  tick = 0;
    uint8_t  idx = 0;
    uint16_t frame = 0;
    uint32_t last = 0;
    const scene_t *sc;

    u8g2_port_init();
    shell_port_init();
    splash_play();

    sc = &k_scenes[idx];
    if (sc->enter)
    {
        sc->enter();
    }

    last = HAL_GetTick();

    while (1)
    {
        shell_port_task();

        if ((HAL_GetTick() - last) >= FRAME_MS)
        {
            last = HAL_GetTick();

            sc->draw(frame, tick);

            if (++frame >= sc->duration)
            {
                frame = 0;
                idx = (uint8_t)((idx + 1) % SCENE_COUNT);
                sc = &k_scenes[idx];
                if (sc->enter)
                {
                    sc->enter();
                }
            }

            tick++;
            if (tick & 1U)
            {
                HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
            }
        }

        __WFI();
    }
}
