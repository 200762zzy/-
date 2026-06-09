#ifndef FOOD_H
#define FOOD_H

#include "list.h"
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 食物系统
 *
 * 20 种食物，分为 4 个稀有度等级。
 * 每次地图上同时存在 1 个食物，吃掉后按权重池随机刷新下一个。
 */

#define FOOD_COUNT 20   /* 食物种类总数 */

/* 食物形状 */
typedef enum {
    FOOD_SHAPE_CIRCLE,
    FOOD_SHAPE_SQUARE,
    FOOD_SHAPE_DIAMOND,
    FOOD_SHAPE_STAR
} FoodShape;

/* 食物属性定义 */
typedef struct FoodDef {
    int type;              /* 类型编号 0~19 */
    const char *name;      /* 名称 */
    unsigned char r, g, b; /* 颜色 RGB */
    FoodShape shape;       /* 形状 */
    int score;             /* 吃后得分 */
    int growth;            /* 吃后增长节数 */
    int weight;            /* 出现权重（越大越常见） */
} FoodDef;

/*
 * 获取食物定义。
 * type: 0~19
 * 返回 FoodDef*，type 越界返回 NULL。
 */
const FoodDef *food_get_def(int type);

/*
 * 按权重随机选择一种食物类型。
 * 返回 0~19 的类型编号。
 */
int food_random_type(void);

/*
 * 在空白格生成食物。
 * occupied: 已被占用的格子链表（蛇身），可以为 NULL
 * out_x, out_y: 返回食物坐标
 * out_type: 返回食物类型
 * 返回 0 成功，-1 失败（网格已满）。
 */
int food_spawn(const List *occupied, int *out_x, int *out_y, int *out_type);

#ifdef __cplusplus
}
#endif

#endif /* FOOD_H */
