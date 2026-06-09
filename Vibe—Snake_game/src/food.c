#include "food.h"
#include <stdlib.h>

/*
 * 20 种食物数据表
 *
 * 稀有度梯度：Common(40) > Uncommon(19) > Epic(7) > Legendary(2)
 * 权重越大越常见，Legendary 出现概率最低。
 */
static const FoodDef FOODS[FOOD_COUNT] = {
    /* ===== Common (8种, 权重合计 40) ===== */
    { 0,  "Tiny Berry",     124, 252, 0,   FOOD_SHAPE_CIRCLE,  1,   1, 8 },
    { 1,  "Small Berry",    50,  205, 50,  FOOD_SHAPE_CIRCLE,  2,   1, 7 },
    { 2,  "Berry",          0,   255, 0,   FOOD_SHAPE_CIRCLE,  3,   1, 6 },
    { 3,  "Big Berry",      0,   204, 0,   FOOD_SHAPE_CIRCLE,  4,   1, 5 },
    { 4,  "Lime",           173, 255, 47,  FOOD_SHAPE_CIRCLE,  5,   1, 5 },
    { 5,  "Lemon",          255, 250, 205, FOOD_SHAPE_CIRCLE,  6,   1, 4 },
    { 6,  "Starfruit",      255, 215, 0,   FOOD_SHAPE_CIRCLE,  7,   1, 3 },
    { 7,  "Yellow Pepper",  255, 255, 0,   FOOD_SHAPE_CIRCLE,  8,   1, 2 },

    /* ===== Uncommon (6种, 权重合计 19) ===== */
    { 8,  "Orange",         255, 165, 0,   FOOD_SHAPE_SQUARE,  10,  1, 6 },
    { 9,  "Tangerine",      255, 140, 0,   FOOD_SHAPE_SQUARE,  12,  1, 5 },
    { 10, "Persimmon",      255, 99,  71,  FOOD_SHAPE_SQUARE,  14,  2, 4 },
    { 11, "Tomato",         255, 69,  0,   FOOD_SHAPE_SQUARE,  16,  2, 2 },
    { 12, "Chili",          220, 20,  60,  FOOD_SHAPE_SQUARE,  18,  2, 1 },
    { 13, "Dragon Chili",   178, 34,  34,  FOOD_SHAPE_SQUARE,  20,  2, 1 },

    /* ===== Epic (4种, 权重合计 7) ===== */
    { 14, "Amethyst",       155, 89,  182, FOOD_SHAPE_DIAMOND, 30,  2, 3 },
    { 15, "Sapphire",       52,  152, 219, FOOD_SHAPE_DIAMOND, 40,  2, 2 },
    { 16, "Topaz",          46,  134, 193, FOOD_SHAPE_DIAMOND, 50,  3, 1 },
    { 17, "Diamond",        26,  82,  118, FOOD_SHAPE_DIAMOND, 60,  3, 1 },

    /* ===== Legendary (2种, 权重合计 2) ===== */
    { 18, "Golden Apple",   255, 215, 0,   FOOD_SHAPE_STAR,    100, 4, 1 },
    { 19, "Rainbow Gem",    255, 105, 180, FOOD_SHAPE_STAR,    200, 5, 1 },
};

/* 总权重 */
#define TOTAL_WEIGHT 68

/* ---------- API 实现 ---------- */

const FoodDef *food_get_def(int type)
{
    if (type < 0 || type >= FOOD_COUNT) return NULL;
    return &FOODS[type];
}

int food_random_type(void)
{
    int roll = rand() % TOTAL_WEIGHT;
    int cumulative = 0;

    for (int i = 0; i < FOOD_COUNT; i++)
    {
        cumulative += FOODS[i].weight;
        if (roll < cumulative)
        {
            return i;
        }
    }

    /* 不应走到这里，但兜底返回最后一个 */
    return FOOD_COUNT - 1;
}

int food_spawn(const List *occupied, int *out_x, int *out_y, int *out_type)
{
    /* 先尝试随机位置（最多 100 次） */
    for (int attempt = 0; attempt < 100; attempt++)
    {
        int x = rand() % GRID_COLS;
        int y = rand() % GRID_ROWS;

        if (!occupied || !list_contains(occupied, x, y))
        {
            *out_x = x;
            *out_y = y;
            *out_type = food_random_type();
            return 0;
        }
    }

    /* 随机失败后线性扫描全网格 */
    for (int x = 0; x < GRID_COLS; x++)
    {
        for (int y = 0; y < GRID_ROWS; y++)
        {
            if (!occupied || !list_contains(occupied, x, y))
            {
                *out_x = x;
                *out_y = y;
                *out_type = food_random_type();
                return 0;
            }
        }
    }

    /* 网格已满 */
    return -1;
}
