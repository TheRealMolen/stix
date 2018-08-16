#pragma once


const int width = 6;
const int height = 6;

const int bpm = 120;
const int steps_per_beat = 4;
const uint32_t ticks_per_beat = (1000 * 60) / bpm;
const uint32_t ticks_per_step = ticks_per_beat / steps_per_beat;

extern uint32_t ticks_left;
extern uint32_t ticks_left_beat;

extern bool steptrig[steps_per_beat];
