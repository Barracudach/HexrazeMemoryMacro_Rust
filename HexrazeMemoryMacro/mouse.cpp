#include "mouse.h"

float aim_coef = 1;

float game_sensitivity;
float game_fov;

float macro_sensitivity;
float macro_fov;

float accum_x = 0;
float accum_y = 0;

float coef;

void set_aim_param(float coeff)
{
	aim_coef = coeff;
	coef = macro_fov / game_fov * macro_sensitivity / game_sensitivity * aim_coef;
}


void set_game_param(float _sensitivity, float _fov)
{
	game_sensitivity = _sensitivity;
	game_fov = _fov;
	coef = macro_fov / game_fov * macro_sensitivity / game_sensitivity; //оепеялнрперэ
}

void set_macro_param(float _sensitivity, float _fov)
{
	macro_sensitivity = _sensitivity;
	macro_fov = _fov;

	coef = macro_fov / game_fov * macro_sensitivity / game_sensitivity;
}

void mousemove(float dx, float dy)
{

	register float dx_f = dx * coef;
	register float dy_f = dy * coef;

	accum_x += dx_f;
	accum_y += dy_f;
	dx_f = (int)accum_x;
	dy_f = (int)accum_y;
	accum_x = accum_x - dx_f;
	accum_y = accum_y - dy_f;

	mouse_event((0x0001), (int)dx_f, (int)dy_f, 0, 0);
}

void zero_accumulation()
{
	accum_x = 0;
	accum_y = 0;
}