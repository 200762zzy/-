#ifndef SNAKE_H
#define SNAKE_H

#include "list.h"
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 蛇模块 —— 用双向循环链表表示蛇身
 *
 * 蛇头 = 链表第一个数据节点 (list_front)
 * 蛇尾 = 链表最后一个数据节点 (list_back)
 * 方向变化先缓存在 next_dir，下次移动时生效，防止反方向掉头。
 * grow_pending > 0 表示下一次移动时增长一节。
 */

/* 方向枚举 */
typedef enum {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
} Direction;

/* 蛇对象 */
typedef struct Snake {
    List *body;             /* 蛇身链表 */
    Direction dir;          /* 当前移动方向 */
    Direction next_dir;     /* 缓冲方向（下一次移动时应用） */
    int alive;              /* 0=死亡, 1=存活 */
    int grow_pending;       /* 待增长节数 */
} Snake;

/*
 * 创建一条蛇。
 * start_x, start_y: 蛇的中心位置
 * length: 初始长度
 * dir: 初始方向
 * 返回 Snake*，失败返回 NULL。
 */
Snake *snake_create(int start_x, int start_y, int length, Direction dir);

/*
 * 销毁蛇，释放所有内存。
 */
void snake_destroy(Snake *snake);

/*
 * 蛇前进一格。
 * 内部流程：计算新蛇头 → 碰撞检测 → 插入新头 → 移除尾部/增长
 * 返回 0 正常，-1 死亡（碰撞）。
 */
int snake_move(Snake *snake);

/*
 * 蛇转向。如果 new_dir 与当前方向相反（180°），则忽略。
 * 方向会缓存到 next_dir，在下次移动时生效。
 */
void snake_turn(Snake *snake, Direction new_dir);

/*
 * 设置增长标记：下一次移动时长度增加 1 节。
 */
void snake_grow(Snake *snake);

/*
 * 蛇头当前坐标（写回参数）。
 */
void snake_head_pos(const Snake *snake, int *out_x, int *out_y);

/*
 * 当前方向。
 */
Direction snake_direction(const Snake *snake);

/*
 * 缓冲方向（蛇下一帧实际移动的方向）。
 * 在 snake_move 执行前调用以获得正确的预判。
 */
Direction snake_next_direction(const Snake *snake);

/*
 * 蛇身长度（节数）。
 */
int snake_length(const Snake *snake);

/*
 * 蛇是否存活。
 * 返回 1 存活，0 死亡。
 */
int snake_is_alive(const Snake *snake);

#ifdef __cplusplus
}
#endif

#endif /* SNAKE_H */
