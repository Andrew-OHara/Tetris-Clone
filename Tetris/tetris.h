#ifndef TETRIS_H_INCLUDED
#define TETRIS_H_INCLUDED

#include <stdio.h>
#include <stdint.h>

#include <allegro.h>
#include <allegro_font.h>
#include <allegro_ttf.h>
#include <allegro_primitives.h>

#include "tetris_tools.h"

//#define WTF

#if defined(WTF)

#define START_TIMED_BLOCK(ID) uint64 debugCycleCount_##ID = rdtsc()
#define STOP_TIMED_BLOCK(ID) cycleCounts[DEBUG_CYCLE_COUNT_##ID].cycleCount += rdtsc() - debugCycleCount_##ID; cycleCounts[DEBUG_CYCLE_COUNT_##ID].hitCount++

#define Assert(expression) if(!(expression)) {*(int *)0 = 0;}

#else

#define START_TIMED_BLOCK(ID)
#define STOP_TIMED_BLOCK(ID)

#define Assert(Expression)

#endif

#define DISPLAY_HEIGHT 740
#define DISPLAY_WIDTH  800

#define UPDATES_PER_SECOND 60
#define FRAMES_PER_SECOND  60

#define DROPS_PER_SECOND 1.5
#define DROP_INTERVAL (1.0 / DROPS_PER_SECOND)
#define HALF_DROP_INTERVAL (DROP_INTERVAL * 0.5)

#define BLOCKS_ACROSS 10
#define BLOCKS_DOWN	22
#define BLOCK_SIZE 30
#define BLOCKS_PER_TETRONIMO 4

#define RIGHT_ROTATION 1
#define LEFT_ROTATION -1

#define GRID_WIDTH (BLOCK_SIZE * BLOCKS_ACROSS)
#define GRID_HEIGHT (BLOCK_SIZE * BLOCKS_DOWN)

#define HELD_DOWN 0
#define RELEASED 1
#define JUST_PRESSED 2


#define TETRONIMO_VARIATIONS 7
#define TETRONIMO_ROTATIONS 4

#define SPAWN_BUFFER -2

#define ArrayCount(array) sizeof(array) / sizeof(array[0])

typedef uint64_t uint64;

#if defined(WTF)

typedef struct debug_block_info
{
	int hitCount;
	uint64 cycleCount;
} debug_block_info;

debug_block_info cycleCounts[32];

typedef enum profiled_block{ DEBUG_CYCLE_COUNT_UpdateGame, DEBUG_CYCLE_COUNT_RenderGame } profiled_block;

#endif

typedef enum t_bool{ NOT_TRUE_AT_ALL, VERY_TRUE } t_bool;

typedef enum t_shape{ L, J, T, Z, S, I, O } t_shape;
typedef enum t_rotation{ UP, RIGHT, DOWN, LEFT } t_rotation, t_direction;

typedef struct block_index
{
    int x;
    int y;
} grid_coord, block_offset;

typedef struct tetronimo
{
	t_shape shape;
	t_rotation rotation;
	grid_coord *pivot;
	grid_coord blocks[BLOCKS_PER_TETRONIMO];
} tetronimo;

// --------------------------------------------------------------------------
typedef struct game_state
{
	tetronimo *tet;
	tetronimo *nextTet;
	block_offset tetRotationOffsets[TETRONIMO_VARIATIONS][TETRONIMO_ROTATIONS][BLOCKS_PER_TETRONIMO];
	t_bool grid[BLOCKS_DOWN][BLOCKS_ACROSS];
	rect playspace;
	float borderWidth;
	float borderHeight;
	int lineCount;
	int score;
	int scoreRamp[5];
	int level;
	int linesThisLevel;
	int levelThreshold;
	float dropsPerSecond;
	float dropInterval;
	float halfDropInterval;
	int highScores[3];
	int scoreIndex;
	t_bool dropTime;
	t_bool moveTime;
	ALLEGRO_TIMER *dropTimer;
	ALLEGRO_TIMER *moveTimer;

	t_bool isInitialized;
} game_state;


t_bool UpdateGame(game_state *state, unsigned char keys[][ALLEGRO_KEY_MAX]);
void RenderGame(ALLEGRO_FONT *basicFont, game_state *state);

void DebugResetCycleCounts();

uint64 rdtsc();

// -------------------------------------------------------------

void DropBlocks();
void DebugFillBlock();
void DebugRenderGrid();
t_shape GetRandomShape();
void SolidifyTetronimo();
void LoadRotationOffsets();
void InitHighScores(game_state *state);
t_bool DropTetronimo(game_state *state);
void ClearLine(game_state *state, int y);
void DebugDrawTetronimo(game_state *state);
t_bool DetectCompleteLine(game_state *state, int y);
void MoveTetronimo(game_state *state, t_direction dir);
void ApplyRotationOffsets(game_state *state, tetronimo *piece);
t_rotation GetAlteredRotation(t_rotation original, int rotationDir);
void RotateTetronimo(game_state *state, tetronimo *piece, int rotationDir);
void AssembleTetronimo(game_state *state, tetronimo *piece, grid_coord pivot);
tetronimo* NewRandomTetronimo(game_state *state, grid_coord pivotBlock, t_rotation rotation);
tetronimo* NewTetronimo(game_state *state, grid_coord pivotBlock, t_rotation rotation, t_shape shape);
void DebugDrawRogueTetronimo(game_state *state, float pivotX, float pivotY, t_shape shape, ALLEGRO_COLOR color);
t_direction ValidateTetronimoPlacement(game_state *state, t_shape shape, grid_coord pivot, t_rotation rotation);

#endif // TETRIS_H_INCLUDED
