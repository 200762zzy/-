/*
 * 音频模块实现
 *
 * 核心思路：在内存中构建 WAV 文件头 + 正弦波 PCM 数据，
 * 通过 raylib 的 LoadWaveFromMemory + LoadSoundFromWave 加载为 Sound。
 */

#include "audio.h"
#include "raylib.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ---------- 常量 ---------- */

#define SAMPLE_RATE 44100
#define SAMPLE_SIZE 16    /* 16-bit */
#define NUM_CHANNELS 1    /* 单声道 */

/* ---------- 辅助：生成正弦波 WAV 内存 ---------- */

/*
 * WAV 文件结构（PCM，16-bit 单声道）：
 *   [RIFF 头 12 字节] ＋ [fmt 块 24 字节] ＋ [data 块 8 + PCM 数据]
 *   总共 = 44 字节头部 + PCM 数据
 */

/* WAV 文件头总大小 */
#define WAV_HEADER_SIZE 44

/* 生成包含指定频率和时长的正弦波 WAV，返回 malloc 缓冲区，len 写入总字节数 */
static unsigned char *gen_sine_wave(float frequency, float duration, int *out_len)
{
    int num_samples = (int)(SAMPLE_RATE * duration);
    int data_size = num_samples * (SAMPLE_SIZE / 8);   /* 16-bit = 2 bytes */
    int total_size = WAV_HEADER_SIZE + data_size;

    unsigned char *buf = (unsigned char *)malloc(total_size);
    if (!buf) return NULL;

    /* 清零 */
    memset(buf, 0, total_size);

    /* ---- RIFF 头 ---- */
    memcpy(buf, "RIFF", 4);
    /* 文件大小 - 8 */
    int chunk_size = total_size - 8;
    memcpy(buf + 4, &chunk_size, 4);
    memcpy(buf + 8, "WAVE", 4);

    /* ---- fmt 块 ---- */
    memcpy(buf + 12, "fmt ", 4);
    int fmt_size = 16;           /* PCM fmt 块大小 */
    memcpy(buf + 16, &fmt_size, 4);
    short audio_format = 1;      /* PCM */
    memcpy(buf + 20, &audio_format, 2);
    short channels = NUM_CHANNELS;
    memcpy(buf + 22, &channels, 2);
    int sample_rate = SAMPLE_RATE;
    memcpy(buf + 24, &sample_rate, 4);
    int byte_rate = SAMPLE_RATE * NUM_CHANNELS * (SAMPLE_SIZE / 8);
    memcpy(buf + 28, &byte_rate, 4);
    short block_align = (short)(NUM_CHANNELS * (SAMPLE_SIZE / 8));
    memcpy(buf + 32, &block_align, 2);
    short bits_per_sample = SAMPLE_SIZE;
    memcpy(buf + 34, &bits_per_sample, 2);

    /* ---- data 块 ---- */
    memcpy(buf + 36, "data", 4);
    memcpy(buf + 40, &data_size, 4);

    /* ---- PCM 数据（16-bit 有符号正弦波） ---- */
    short *samples = (short *)(buf + WAV_HEADER_SIZE);
    double amplitude = 16000.0;   /* 避免削波 */

    for (int i = 0; i < num_samples; i++)
    {
        double t = (double)i / SAMPLE_RATE;
        double sample = amplitude * sin(2.0 * M_PI * frequency * t);
        samples[i] = (short)sample;
    }

    *out_len = total_size;
    return buf;
}

/* ---------- 音效 ---------- */

static Sound sound_eat;
static Sound sound_death;
static Sound sound_menu;
static int audio_ready = 0;   /* 标记设备是否初始化成功 */

int audio_init(void)
{
    if (audio_ready) return 0;

    InitAudioDevice();
    if (!IsAudioDeviceReady()) return -1;

    /* 生成三个音效 */
    unsigned char *wav_data;
    int wav_len;

    /* 吃食物：880Hz，0.12 秒（偏高，明快） */
    wav_data = gen_sine_wave(880.0f, 0.12f, &wav_len);
    if (wav_data)
    {
        Wave wave = LoadWaveFromMemory(".wav", wav_data, wav_len);
        free(wav_data);
        if (IsWaveValid(wave))
        {
            sound_eat = LoadSoundFromWave(wave);
            UnloadWave(wave);
        }
    }

    /* 死亡：220Hz，0.35 秒（低沉） */
    wav_data = gen_sine_wave(220.0f, 0.35f, &wav_len);
    if (wav_data)
    {
        Wave wave = LoadWaveFromMemory(".wav", wav_data, wav_len);
        free(wav_data);
        if (IsWaveValid(wave))
        {
            sound_death = LoadSoundFromWave(wave);
            UnloadWave(wave);
        }
    }

    /* 菜单：440Hz，0.15 秒（中性） */
    wav_data = gen_sine_wave(440.0f, 0.15f, &wav_len);
    if (wav_data)
    {
        Wave wave = LoadWaveFromMemory(".wav", wav_data, wav_len);
        free(wav_data);
        if (IsWaveValid(wave))
        {
            sound_menu = LoadSoundFromWave(wave);
            UnloadWave(wave);
        }
    }

    audio_ready = 1;
    return 0;
}

void audio_play_eat(void)
{
    if (audio_ready) PlaySound(sound_eat);
}

void audio_play_death(void)
{
    if (audio_ready) PlaySound(sound_death);
}

void audio_play_menu(void)
{
    if (audio_ready) PlaySound(sound_menu);
}

void audio_close(void)
{
    if (audio_ready)
    {
        UnloadSound(sound_eat);
        UnloadSound(sound_death);
        UnloadSound(sound_menu);
        CloseAudioDevice();
        audio_ready = 0;
    }
}
