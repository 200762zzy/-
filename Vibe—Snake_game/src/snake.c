#include "snake.h"
#include <stdlib.h>

/*
 * 蛇逻辑实现
 *
 * 移动算法：
 *   1. 根据当前方向计算新蛇头坐标
 *   2. 撞墙检测：新头超出网格范围则死亡
 *   3. 撞自身检测：新头碰触已有身体（非尾部）则死亡
 *   4. 链表操作：push_front(新头) + 若未增长则 pop_back(尾部)
 *
 * 初始蛇身布局（以长度 3、方向向右为例）：
 *   尾 ← 中 ← 头
 *   (9,10) (10,10) (11,10)
 *   push_front 顺序：9→10→11，所以头 = list_front = (11,10)
 */

/* 方向偏移量：{dx, dy} */
static const int DIR_OFFSET[4][2] = {
    { 0, -1 },   /* DIR_UP */
    { 0,  1 },   /* DIR_DOWN */
    { -1, 0 },   /* DIR_LEFT */
    { 1,  0 }    /* DIR_RIGHT */
};

/* ---------- 创建与销毁 ---------- */

Snake *snake_create(int start_x, int start_y, int length, Direction dir)
{
    Snake *snake = (Snake *)malloc(sizeof(Snake));
    if (!snake) return NULL;

    snake->body = list_create();
    if (!snake->body) {
        free(snake);
        return NULL;
    }

    snake->dir = dir;
    snake->next_dir = dir;
    snake->alive = 1;
    snake->grow_pending = 0;

    /*
     * 从中心向左侧展开蛇身，头在右侧。
     * start_x 为蛇的中心位置，蛇身占据 [start_x-length/2, start_x+length/2]。
     * 例如 length=3, start_x=10: 身体占据 (9,10)(10,10)(11,10)，头在 (11,10)。
     */
    int first_x = start_x - length / 2;
    for (int i = 0; i < length; i++)
    {
        list_push_front(snake->body, first_x + i, start_y);
    }

    return snake;
}

void snake_destroy(Snake *snake)
{
    if (!snake) return;
    list_destroy(snake->body);
    free(snake);
}

/* ---------- 移动 ---------- */

int snake_move(Snake *snake)
{
    if (!snake->alive) return -1;

    /* 应用缓冲方向 */
    snake->dir = snake->next_dir;

    /* 计算新蛇头坐标 */
    ListNode *head = list_front(snake->body);
    int new_x = head->x + DIR_OFFSET[snake->dir][0];
    int new_y = head->y + DIR_OFFSET[snake->dir][1];

    /* ---- 撞墙检测 ---- */
    if (new_x < 0 || new_x >= GRID_COLS ||
        new_y < 0 || new_y >= GRID_ROWS)
    {
        snake->alive = 0;
        return -1;
    }

    /* ---- 撞自身检测 ---- */
    /*
     * 遍历所有身体节点（不包括即将移除的尾部，如果未增长的话）。
     * 如果新蛇头坐标与某个身体节点重叠，则死亡。
     */
    {
        ListNode *tail = list_back(snake->body);
        for (ListNode *p = snake->body->sentinel.next;
             p != &snake->body->sentinel;
             p = p->next)
        {
            /* 未增长时尾部即将被移除，跳过 */
            if (snake->grow_pending == 0 && p == tail) continue;

            if (p->x == new_x && p->y == new_y)
            {
                snake->alive = 0;
                return -1;
            }
        }
    }

    /* ---- 执行移动 ---- */
    list_push_front(snake->body, new_x, new_y);

    if (snake->grow_pending > 0)
    {
        snake->grow_pending--;
    }
    else
    {
        list_pop_back(snake->body);
    }

    return 0;
}

/* ---------- 转向 ---------- */

void snake_turn(Snake *snake, Direction new_dir)
{
    /*
     * 禁止 180° 反向掉头：
     *   UP <-> DOWN, LEFT <-> RIGHT
     */
    if ((snake->dir == DIR_UP    && new_dir == DIR_DOWN) ||
        (snake->dir == DIR_DOWN  && new_dir == DIR_UP)   ||
        (snake->dir == DIR_LEFT  && new_dir == DIR_RIGHT) ||
        (snake->dir == DIR_RIGHT && new_dir == DIR_LEFT))
    {
        return;
    }

    snake->next_dir = new_dir;
}

/* ---------- 增长 ---------- */

void snake_grow(Snake *snake)
{
    snake->grow_pending++;
}

/* ---------- 查询 ---------- */

void snake_head_pos(const Snake *snake, int *out_x, int *out_y)
{
    ListNode *head = list_front(snake->body);
    if (head) {
        *out_x = head->x;
        *out_y = head->y;
    }
}

Direction snake_direction(const Snake *snake)
{
    return snake->dir;
}

Direction snake_next_direction(const Snake *snake)
{
    return snake->next_dir;
}

int snake_length(const Snake *snake)
{
    return list_size(snake->body);
}

int snake_is_alive(const Snake *snake)
{
    return snake->alive;
}
