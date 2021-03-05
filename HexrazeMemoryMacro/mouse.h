//........HEXRAZE | From GitHub:https://github.com/Barracuda1900/HexrazeMemoryMacro_Rust

#pragma once
#include <Windows.h>
#include <iostream>

void set_aim_param(float coeff);
void set_macro_param(float _sensitivity, float _fov);
void mousemove(float dx, float dy);
void zero_accumulation();
void set_game_param(float _sensitivity, float _fov);
