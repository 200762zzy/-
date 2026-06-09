#ifndef AUDIO_H
#define AUDIO_H

/*
 * 音频模块 —— 程序化生成短音效并播放
 *
 * 使用 raylib 音频 API，在内存中生成正弦波 WAV
 * 无需外部音频文件
 */

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 初始化音频设备并加载所有音效。
 * 返回 0 成功，-1 失败。
 */
int audio_init(void);

/*
 * 播放吃食物音效。
 */
void audio_play_eat(void);

/*
 * 播放死亡音效。
 */
void audio_play_death(void);

/*
 * 播放菜单确认音效。
 */
void audio_play_menu(void);

/*
 * 释放所有音效并关闭音频设备。
 */
void audio_close(void);

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_H */
