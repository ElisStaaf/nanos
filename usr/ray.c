#include "lib/usersp.h"
#include "lib/kernel.h"
#include "lib/bool.h"
#include "lib/math.h"
#include "lib/rng.h"

typedef float vec2[2];
typedef float vec3[3];
typedef float vec4[4];

struct ray {
	vec3 origin;
	vec3 direction;
};

struct hit {
	vec3 p;
	vec3 n;
	float t;
	float min, max;
	size_t mat;
};

typedef bool (*hit_function)(void *object, struct ray *ray, struct hit *hit);

struct object {
	hit_function hit;
};

struct world {
	struct object object;
	size_t amt;
	struct object **objs;
};

struct sphere {
	struct object object;
	vec3 center;
	float radius;
	size_t mat;
};

#define REP3_(SPLT, FUN, ...) FUN(0, __VA_ARGS__) SPLT FUN(1, __VA_ARGS__) SPLT FUN(2, __VA_ARGS__)
#define REP3(FUN, ...) REP3_(;, FUN, __VA_ARGS__)
#define VEC_OP_AUN_(IDX, VEC, OP) VEC[IDX] = OP VEC[IDX]
#define VEC_OP_ABIN_(IDX, VECA, VECB, OP) VECA[IDX] = VECA[IDX] OP VECB[IDX]
#define VEC_OP_ABINC_(IDX, VECA, VECB, OP) VECA[IDX] = OP VECB[IDX]
#define VEC_OP_BIN_(IDX, VECA, VECB, OP) VECA[IDX] OP VECB[IDX]
#define VEC_OP_ABINCONST_(IDX, VEC, OP, CONST) VEC[IDX] = VEC[IDX] OP CONST
#define VEC_OP_AUNCONST_(IDX, VEC, OP, CONST) VEC[IDX] = OP CONST

static inline void v3_add_v3(vec3 a, vec3 b) { REP3(VEC_OP_ABIN_, a, b, +); }
static inline void v3_sub_v3(vec3 a, vec3 b) { REP3(VEC_OP_ABIN_, a, b, -); }
static inline void v3_add_f(vec3 a, float b) { REP3(VEC_OP_ABINCONST_, a, +, b); }
static inline void v3_sub_f(vec3 a, float b) { REP3(VEC_OP_ABINCONST_, a, -, b); }
static inline void v3_mul_f(vec3 a, float b) { REP3(VEC_OP_ABINCONST_, a, *, b); }
static inline void v3_div_f(vec3 a, float b) { v3_mul_f(a, 1 / b); } /* might cause prec. issues */
static inline void v3_neg(vec3 v) { REP3(VEC_OP_AUN_, v, -); }
static inline void v3_cpy(vec3 a, vec3 b) { REP3(VEC_OP_ABINC_, a, b, ); }
static inline float v3_dot(vec3 a, vec3 b) { return REP3_(+, VEC_OP_BIN_, a, b, *); }
static inline float v3_mag2(vec3 v) { return v3_dot(v, v); }
static inline float v3_mag(vec3 v) { return sqrtspf(v3_mag2(v)); }

static inline void v3_zero(vec3 v) { REP3(VEC_OP_AUNCONST_, v, , 0); }
static inline void grngspf3(vec3 v) { REP3(VEC_OP_AUNCONST_, v, , grngspf()); }
static inline void grngspf3n(vec3 v) { grngspf3(v); v3_div_f(v, v3_mag(v)); }

struct vec3 { float x, y, z; };

void ray_at(vec3 at, struct ray *ray, float t) {
	v3_cpy(at, ray->direction);
	v3_mul_f(at, t);
	v3_add_v3(at, ray->origin);
}

bool hit_world(struct world *world, struct ray *ray, struct hit *hit) {
	struct hit tmphit;
	bool didhit = false;
	float tmin = hit->max;

	for(size_t i = 0; i < world->amt; ++i) {
		tmphit.min = hit->min;
		tmphit.max = tmin;
		if(world->objs[i]->hit(world->objs[i], ray, &tmphit)) {
			didhit = true;
			tmin = tmphit.t;
			v3_cpy(hit->n, tmphit.n);
			v3_cpy(hit->p, tmphit.p);
			hit->mat = tmphit.mat;
			hit->t = tmphit.t;
		}
	}

	return didhit;
}

bool hit_sphere(struct sphere *sphere, struct ray *ray, struct hit *hit) {
	vec3 oc; v3_cpy(oc, ray->origin);
	v3_sub_v3(oc, sphere->center);
	float a = v3_mag2(ray->direction);
	float b2 = v3_dot(oc, ray->direction);
	float c = v3_mag2(oc) - sphere->radius * sphere->radius;
	float d = b2 * b2 - a * c;
	if(d < 0) return false;
	float q = sqrtspf(d);
	float r = (-b2 - q) / a; /* min. root */
	if(r < hit->min || r > hit->max) {
		r = (-b2 + q) / a; /* max. root */
		if(r < hit->min || r > hit->max)
			return false;
	}
	hit->t = r;
	ray_at(hit->p, ray, hit->t);
	v3_cpy(hit->n, hit->p);
	v3_sub_v3(hit->n, sphere->center);
	v3_div_f(hit->n, sphere->radius);
	hit->mat = sphere->mat;
	return true;
}

void reflect(vec3 v, vec3 n) {
	vec3 n2; v3_cpy(n2, n);
	v3_mul_f(n2, v3_dot(v, n) * 2.0f);
	v3_sub_v3(v, n2);
}

vec3 SUN = { -1.0f, 5.0f, 0.25f };
float SUN_RADIUS = 0.01f;

void ray_compute_color(vec3 color, struct ray *ray, struct object *object, int depth) {
	struct hit hit = {0};
	hit.min = 0.001f;
	hit.max = INFINITY;

	if(depth == 0) {
		static vec3 COLOR = { 1.0f, 1.0f, 1.0f };
		v3_cpy(color, COLOR);
		// struct ray sray;
		// v3_cpy(sray.origin, ray->origin);
		// v3_cpy(sray.direction, SUN);
		// vec3 rnd; grngspf3n(rnd);
		// v3_sub_f(rnd, -0.5f);
		// v3_mul_f(rnd, 2.0f * SUN_RADIUS);
		// v3_sub_v3(sray.direction, rnd);
		// v3_sub_v3(sray.direction, ray->origin);
		// if(object->hit(object, &sray, &hit)) {
		// 	v3_mul_f(color, 0.01f);
		// }
		return;
	}

	if(object->hit(object, ray, &hit)) {
		switch(hit.mat) {
		case 0: {
			vec3 tgt = {0};
			grngspf3n(tgt);
			if(v3_dot(tgt, hit.n) < 0.0) v3_neg(tgt);
			v3_add_v3(tgt, hit.n);
			v3_cpy(ray->origin, hit.p);
			v3_cpy(ray->direction, tgt);
		} break;
		case 1: {
			v3_cpy(ray->origin, hit.p);
			reflect(ray->direction, hit.n);
		} break;
		}
		ray_compute_color(color, ray, object, depth - 1);
		v3_mul_f(color, 0.5f);
		return;
	}

	v3_div_f(ray->direction, v3_mag(ray->direction));
	float t = 0.5 * (ray->direction[1] + 1.0);
	vec3 color_bottom = { 1.0f - t, 1.0f - t, 1.0f - t };
	vec3 color_top = { 0.2f * t, 0.3f * t, 0.7f * t };
	v3_cpy(color, color_bottom);
	v3_add_v3(color, color_top);
	return;
}

uint32_t get_u32color(vec3 color) {
	return
		(uint8_t)(sqrtspf(color[0]) * 255) << 16 |
		(uint8_t)(sqrtspf(color[1]) * 255) <<  8 |
		(uint8_t)(sqrtspf(color[2]) * 255) <<  0 ;
}

#define DRAW_2X 1

void render(struct object *object, uint32_t *fb, int offset, int w, int h, int pitch) {
	float aspect = (float)w / (float)h;
	float viewh = 2.0f;
	float vieww = viewh * aspect;
	float focal = 1.0f;
	vec3 origin = { 0.0f, 0.0f, 0.0f };
	vec3 lower_left = {
		origin[0] - vieww / 2.0f,
		origin[1] - viewh / 2.0f,
		origin[2] - focal
	};

	for(int j = 0; j < h; ++j) {
		for(int i = 0; i < w; ++i) {
			vec3 color = { 0, 0, 0 };
			for(int s = 0; s < 8; ++s) {
				float u = ((float)i + grngspf()) / (float)(w - 1);
				float v = 1 - ((float)j + grngspf()) / (float)(h - 1);
				struct ray ray;
				v3_cpy(ray.origin, origin);
				v3_cpy(ray.direction, lower_left);
				ray.direction[0] += u * vieww;
				ray.direction[1] += v * viewh;
				vec3 scolor;
				ray_compute_color(scolor, &ray, object, 10);

				v3_add_v3(color, scolor);
			}
			v3_div_f(color, 8.0f);
			uint32_t color_u32 = get_u32color(color);
#if DRAW_2X
			int i2 = i * 2, j2 = j * 2;
			fb[offset + i2     + j2     * pitch] = color_u32;
			fb[offset + (i2+1) + j2     * pitch] = color_u32;
			fb[offset + i2     + (j2+1) * pitch] = color_u32;
			fb[offset + (i2+1) + (j2+1) * pitch] = color_u32;
#else
			fb[offset + i + j * pitch] = color_u32;
#endif
		}
	}
}

void init_sphere(struct sphere *sphere, int mat, struct vec3 center, float radius) {
	sphere->center[0] = center.x;
	sphere->center[1] = center.y;
	sphere->center[2] = center.z;
	sphere->radius = radius;
	sphere->object.hit = (hit_function)&hit_sphere;
	sphere->mat = mat;
}

void init_world(struct world *world, int amt, struct object *objs[]) {
	world->amt = amt;
	world->objs = objs;
	world->object.hit = (hit_function)&hit_world;
}

int main() {
	struct kernel_functions F;
	sys_funcs(&F);
	srng(5489);

	screen_t *const scr = F.scrdev_getscr(F.DSCR);

	struct world world;
	struct sphere spheres[3];
	struct object *objs[] = { &spheres[0].object, &spheres[1].object, &spheres[2].object };

	init_sphere(&spheres[0], 1, (struct vec3){ 0.0f, 0.0f, -1.0f }, 0.5f);
	init_sphere(&spheres[1], 0, (struct vec3){ 0.0f, -100.5f, -1.0f }, 100.0f);
	init_sphere(&spheres[2], 0, (struct vec3){ 0.4f, -0.4f, -0.6f }, 0.1f);
	init_world(&world, sizeof(objs) / sizeof(struct object *), objs);

	int dw = 256 / (DRAW_2X?2:1), dh = 256 / (DRAW_2X?2:1);

	memset(scr->buf, 0, scr->w * scr->h * 4);
	render(&world.object, scr->buf, 200 + 100 * scr->w, dw, dh, scr->w);
	F.puts(F.DL0, "\n\n\n");
	return 0;
}
