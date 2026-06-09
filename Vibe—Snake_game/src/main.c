/*
 * Vibe Snake — 游戏入口
 *
 * 状态机主循环：START → PLAYING ↔ PAUSED → GAME_OVER → PLAYING
 * 输入 → 更新 → 渲染 每个状态分别处理
 */

#include "renderer.h"
#include "game.h"
#include "audio.h"
#include "raylib.h"

/* 应用层状态（GAME_STATE_START 由界面管理，不在 game 模块中） */
typedef enum {
    APP_START,
    APP_PLAYING,
    APP_PAUSED,
    APP_GAME_OVER
} AppState;

/* ---------- 方向输入 ---------- */

static void handle_direction_input(Game *game)
{
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))
        game_turn(game, DIR_RIGHT);
    if (IsKeyPressed(KEY_LEFT)  || IsKeyPressed(KEY_A))
        game_turn(game, DIR_LEFT);
    if (IsKeyPressed(KEY_UP)    || IsKeyPressed(KEY_W))
        game_turn(game, DIR_UP);
    if (IsKeyPressed(KEY_DOWN)  || IsKeyPressed(KEY_S))
        game_turn(game, DIR_DOWN);
}

/* ---------- 辅助：根据游戏总分变化检测吃食物 ---------- */

static int get_level(Game *game)
{
    return game_get_total_eaten(game) / SPEED_EVERY_N + 1;
}

/* ---------- 主入口 ---------- */

int main(void)
{
    if (renderer_init() != 0) return -1;

    audio_init();

    Game *game = game_create();
    if (!game) { audio_close(); renderer_close(); return -1; }

    AppState state = APP_START;
    float move_timer = 0.0f;
    int prev_score = 0;

    while (!renderer_should_close())
    {
        float dt = GetFrameTime();

        /* ======== 输入 + 更新 ======== */

        switch (state)
        {
        case APP_START:
            if (GetKeyPressed() != 0)
            {
                audio_play_menu();
                move_timer = 0.0f;
                prev_score = 0;
                state = APP_PLAYING;
            }
            break;

        case APP_PLAYING:
            if (IsKeyPressed(KEY_P))
            {
                game_set_state(game, GAME_STATE_PAUSED);
                state = APP_PAUSED;
                break;
            }
            handle_direction_input(game);

            move_timer += dt;
            while (move_timer >= game_get_interval(game))
            {
                prev_score = game_get_score(game);
                game_update(game);
                move_timer -= game_get_interval(game);

                /* 吃食物音效 */
                if (game_get_score(game) > prev_score)
                {
                    audio_play_eat();
                }

                if (game_get_state(game) == GAME_STATE_GAME_OVER)
                {
                    audio_play_death();
                    state = APP_GAME_OVER;
                    break;
                }
            }
            break;

        case APP_PAUSED:
            if (IsKeyPressed(KEY_P))
            {
                game_set_state(game, GAME_STATE_PLAYING);
                move_timer = 0.0f;
                state = APP_PLAYING;
            }
            break;

        case APP_GAME_OVER:
            if (IsKeyPressed(KEY_SPACE))
            {
                audio_play_menu();
                game_reset(game);
                move_timer = 0.0f;
                prev_score = 0;
                state = APP_PLAYING;
            }
            break;
        }

        /* ======== 渲染 ======== */

        renderer_begin_draw();
        ClearBackground(BLACK);

        switch (state)
        {
        case APP_START:
            renderer_draw_start_screen();
            break;

        case APP_PLAYING:
            renderer_draw_grid();
            renderer_draw_snake(game_get_snake(game));
            renderer_draw_food(
                game_get_food_x(game), game_get_food_y(game),
                food_get_def(game_get_food_type(game)));
            renderer_draw_hud(
                game_get_score(game), game_get_high_score(game),
                snake_length(game_get_snake(game)),
                get_level(game));
            break;

        case APP_PAUSED:
            renderer_draw_grid();
            renderer_draw_snake(game_get_snake(game));
            renderer_draw_food(
                game_get_food_x(game), game_get_food_y(game),
                food_get_def(game_get_food_type(game)));
            renderer_draw_hud(
                game_get_score(game), game_get_high_score(game),
                snake_length(game_get_snake(game)),
                get_level(game));
            renderer_draw_pause_overlay();
            break;

        case APP_GAME_OVER:
            renderer_draw_game_over_screen(
                game_get_score(game), game_get_high_score(game));
            break;
        }

        renderer_end_draw();
    }

    game_destroy(game);
    audio_close();
    renderer_close();
    return 0;
}
