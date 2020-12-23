#include "tetris.h"

t_bool UpdateGame(game_state *state, unsigned char keys[][ALLEGRO_KEY_MAX])
{
	START_TIMED_BLOCK(UpdateGame);

    if(!state->isInitialized)
    {
    	LoadRotationOffsets();
    	InitHighScores(state);

    	state->borderWidth = DISPLAY_WIDTH - GRID_WIDTH;
    	state->borderHeight = DISPLAY_HEIGHT - GRID_HEIGHT;

    	// initialize the grid
    	memset(state->grid, 0, sizeof(t_bool) * BLOCKS_DOWN * BLOCKS_ACROSS);

    	state->dropsPerSecond = 1.7f;
    	state->dropInterval = 1.0f / state->dropsPerSecond;
    	state->halfDropInterval = state->dropInterval * 0.5f;

    	state->playspace.x = state->borderWidth * 0.33f;
    	state->playspace.y =  state->borderHeight * 0.5f;
    	state->playspace.w = GRID_WIDTH;
    	state->playspace.h = GRID_HEIGHT ;

    	state->lineCount = 0;
    	state->level = 1;
    	state->score = 0;
    	int tmpRamp[5] = { 0, 100, 250, 500, 1000 };
    	memcpy(state->scoreRamp, tmpRamp, sizeof(state->scoreRamp));
    	state->scoreIndex = 2;
    	state->levelThreshold = 10; // number of lines needed to advance to next level
    	state->linesThisLevel = 0;

    	grid_coord spawnCoord = {5, 1};
    	state->tet = NewRandomTetronimo(state, spawnCoord, UP);
    	state->nextTet = NewRandomTetronimo(state, spawnCoord, UP);

    	al_start_timer(state->dropTimer);
    	al_start_timer(state->moveTimer);

    	state->isInitialized = VERY_TRUE;
    }
    else if(state->tet)
    {
    	grid_coord coord;
    	t_rotation newRotation;

    	t_bool moved = NOT_TRUE_AT_ALL;
		t_bool pressedLeft = NOT_TRUE_AT_ALL;
		t_bool pressedRight = NOT_TRUE_AT_ALL;
		t_bool heldLeft = NOT_TRUE_AT_ALL;
		t_bool heldRight = NOT_TRUE_AT_ALL;

		if ((pressedLeft = keys[JUST_PRESSED][ALLEGRO_KEY_LEFT]) ||
			(pressedRight = keys[JUST_PRESSED][ALLEGRO_KEY_RIGHT]))
		{
			if(!(pressedLeft && pressedRight))
			{
				coord.x = pressedLeft ? state->tet->pivot->x - 1 : state->tet->pivot->x + 1;
				coord.y = state->tet->pivot->y;
				if(!ValidateTetronimoPlacement(state, state->tet->shape, coord, state->tet->rotation))
           		{
					MoveTetronimo(state, pressedLeft ? LEFT : RIGHT);
					moved = VERY_TRUE;
				}
			}
		}
		else if ((heldLeft = keys[HELD_DOWN][ALLEGRO_KEY_LEFT]) ||
				 (heldRight = keys[HELD_DOWN][ALLEGRO_KEY_RIGHT]))
		{
			if(!(heldLeft && heldRight))
			{
				coord.x = heldLeft ? state->tet->pivot->x - 1 : state->tet->pivot->x + 1;
				coord.y = state->tet->pivot->y;
				if(!ValidateTetronimoPlacement(state, state->tet->shape, coord, state->tet->rotation) && state->moveTime)
           		{
					MoveTetronimo(state, heldLeft ? LEFT : RIGHT);
					moved = VERY_TRUE;
				}
			}
		}

		if(keys[JUST_PRESSED][ALLEGRO_KEY_D])
    	{
    		newRotation = GetAlteredRotation(state->tet->rotation, RIGHT);
    		if(!ValidateTetronimoPlacement(state, state->tet->shape, *state->tet->pivot, newRotation))
    			RotateTetronimo(state, state->tet, RIGHT_ROTATION);
    	}
    	if(keys[JUST_PRESSED][ALLEGRO_KEY_A])
    	{
    		newRotation= GetAlteredRotation(state->tet->rotation, LEFT);
    		if(!ValidateTetronimoPlacement(state, state->tet->shape, *state->tet->pivot, newRotation))
    			RotateTetronimo(state, state->tet, LEFT_ROTATION);
        }

        if(keys[RELEASED][ALLEGRO_KEY_DOWN] && al_get_timer_speed(state->dropTimer) < state->dropInterval)
            al_set_timer_speed(state->dropTimer, state->dropInterval);

        if(keys[HELD_DOWN][ALLEGRO_KEY_DOWN] && al_get_timer_speed(state->dropTimer) > state->halfDropInterval)
            al_set_timer_speed(state->dropTimer, state->halfDropInterval);

        if(state->dropTime)
        {
        	if(!DropTetronimo(state))
        		return VERY_TRUE;
        }
        if(moved)
        {
			state->moveTime = NOT_TRUE_AT_ALL;
			al_stop_timer(state->moveTimer);
			al_start_timer(state->moveTimer);
        }
    }

    STOP_TIMED_BLOCK(UpdateGame);

    return NOT_TRUE_AT_ALL;
}

void RenderGame(ALLEGRO_FONT *basicFont, game_state *state)
{
	START_TIMED_BLOCK(RenderGame);

	al_draw_textf(basicFont, al_map_rgb(255, 255, 255), DISPLAY_WIDTH - (state->borderWidth * 0.6), state->borderHeight * 2.5f, 0, "Lines - %d", state->lineCount);
	al_draw_textf(basicFont, al_map_rgb(255, 255, 255), DISPLAY_WIDTH - (state->borderWidth * 1.6), state->borderHeight * 0.3f, 0, "Score - %d", state->score);
	al_draw_textf(basicFont, al_map_rgb(255, 255, 255), DISPLAY_WIDTH - (state->borderWidth * 0.62), state->borderHeight * 6.2f, 0, "Level - %d", state->level);
	al_draw_textf(basicFont, al_map_rgb(255, 255, 255), DISPLAY_WIDTH - (state->borderWidth * 0.82), state->borderHeight * 0.3f, 0, "Hi-Score %d- %d", state->scoreIndex+1, state->highScores[state->scoreIndex]);

	DebugRenderGrid(state);
    if(state->tet)
        DebugDrawTetronimo(state);
    if(state->nextTet)
    	DebugDrawRogueTetronimo(state, DISPLAY_WIDTH - state->borderWidth * 0.38f, DISPLAY_HEIGHT * 0.5f, state->nextTet->shape, al_map_rgb(140, 150, 255));

    STOP_TIMED_BLOCK(RenderGame);
}

void DebugRenderGrid(game_state *state)
{
	ALLEGRO_COLOR gridColor = al_map_rgb(0, 65, 255);
    for(int curry = state->playspace.y + (2 * BLOCK_SIZE); curry <= state->playspace.y + state->playspace.h; curry += BLOCK_SIZE)
    {
		al_draw_line(state->playspace.x, curry, state->playspace.x + state->playspace.w, curry, gridColor, 1);
		for(int currx = state->playspace.x; currx <= state->playspace.x + state->playspace.w; currx += BLOCK_SIZE)
		{
			al_draw_line(currx, state->playspace.y + (2 * BLOCK_SIZE), currx, state->playspace.y + GRID_HEIGHT, gridColor, 1);
		}
	}

	for(int downIndex = 0; downIndex < BLOCKS_DOWN; ++downIndex)
	{
		for(int acrossIndex = 0; acrossIndex < BLOCKS_ACROSS; ++ acrossIndex)
		{
			if(state->grid[downIndex][acrossIndex] == VERY_TRUE)
			{
				grid_coord drawCoord = { acrossIndex, downIndex };
				DebugFillBlock(state, drawCoord, al_map_rgb(155, 155, 205));
			}
		}
	}
}

void DebugFillBlock(game_state *state, grid_coord coords, ALLEGRO_COLOR color)
{
	Assert(coords.x < BLOCKS_ACROSS && coords.x >= 0);
	Assert(coords.y < BLOCKS_DOWN && coords.y >= 0);

	float currx = state->playspace.x + coords.x * BLOCK_SIZE;
	float curry = state->playspace.y + coords.y * BLOCK_SIZE;

	al_draw_filled_rectangle(currx, curry, currx + BLOCK_SIZE, curry + BLOCK_SIZE, color);
}

t_shape GetRandomShape()
{
	int shapeIndex = rand() % (O + 1);
	t_shape result = (t_shape)shapeIndex;
	return result;
}

tetronimo* NewRandomTetronimo(game_state *state, grid_coord pivotBlock, t_rotation rotation)
{
	tetronimo *piece = malloc(sizeof(tetronimo));

	piece->shape = GetRandomShape();

	piece->rotation = rotation;
	AssembleTetronimo(state, piece, pivotBlock);
	piece->pivot = &piece->blocks[0];

	return piece;
}

tetronimo* NewTetronimo(game_state *state, grid_coord pivotBlock, t_rotation rotation, t_shape shape)
{
	tetronimo *piece = malloc(sizeof(tetronimo));

	piece->shape = shape;

	piece->rotation = rotation;
	AssembleTetronimo(state, piece, pivotBlock);
	piece->pivot = &piece->blocks[0];

	return piece;
}

void AssembleTetronimo(game_state *state, tetronimo *piece, grid_coord pivot)
{
    piece->blocks[0] = pivot;
    ApplyRotationOffsets(state, piece);
}

void DebugDrawTetronimo(game_state *state)
{
	for(int blockIndex = 0; blockIndex < BLOCKS_PER_TETRONIMO; ++blockIndex)
	{
		if(state->tet->blocks[blockIndex].y > 1)
			DebugFillBlock(state, state->tet->blocks[blockIndex], al_map_rgb(65, 65, 255));
	}
}

void ApplyRotationOffsets(game_state *state, tetronimo *piece)
{
    int pivotIndex = 0;
	for(int blockIndex = 0; blockIndex < BLOCKS_PER_TETRONIMO; ++blockIndex)
	{
		piece->blocks[blockIndex].x = piece->blocks[pivotIndex].x + state->tetRotationOffsets[piece->shape][piece->rotation][blockIndex].x;
		piece->blocks[blockIndex].y = piece->blocks[pivotIndex].y + state->tetRotationOffsets[piece->shape][piece->rotation][blockIndex].y;
	}
}

void RotateTetronimo(game_state *state, tetronimo *piece, int rotationDir)
{
    t_rotation newRotation = GetAlteredRotation(piece->rotation, rotationDir);
    piece->rotation = newRotation;
    ApplyRotationOffsets(state, piece);
}

t_rotation GetAlteredRotation(t_rotation original, int rotationDir)
{
	int rotIndex = (int)original;
    rotIndex += rotationDir;
    if(rotIndex > LEFT)
    	rotIndex = UP;
    if(rotIndex < UP)
    	rotIndex = LEFT;

    t_rotation newRotation = (t_rotation)rotIndex;

    return newRotation;
}

void MoveTetronimo(game_state *state, t_direction dir)
{
	if(dir == LEFT)
		--state->tet->pivot->x;
	if(dir == RIGHT)
		++state->tet->pivot->x;
	if(dir == DOWN)
		++state->tet->pivot->y;

	AssembleTetronimo(state, state->tet, *state->tet->pivot);
}

t_bool DropTetronimo(game_state *state)
{
	grid_coord coord;
	coord.x = state->tet->pivot->x;
    coord.y = state->tet->pivot->y + 1;

    t_direction collisionDirection = ValidateTetronimoPlacement(state, state->tet->shape, coord, state->tet->rotation);
    if(!collisionDirection)
    {
    	MoveTetronimo(state, DOWN);
    }
    else if(collisionDirection == DOWN)
    {
    	SolidifyTetronimo(state);
    	free(state->tet);
    	grid_coord gc = {5, 2};
    	state->tet = state->nextTet;
    	state->nextTet = NewRandomTetronimo(state, gc, UP);

    	if(ValidateTetronimoPlacement(state, state->tet->shape, *state->tet->pivot, state->tet->rotation))
    	{
    		// If we get invalid placement immediately after spawning the piece it's Game Over
    		FILE *scoreFile = fopen("hiscores.scr", "wb");
    		if(scoreFile)
    		{
    			int size = ArrayCount(state->highScores);
    			t_bool done = NOT_TRUE_AT_ALL;
    			for(int i = size-1; i >= 0; --i)
    			{
    				if(state->score >= state->highScores[i] && !done)
    				{
    					fwrite(&state->score, sizeof(state->score), 1, scoreFile);
    					done = VERY_TRUE;
    				}
    				else
    				{
    					fwrite(&state->highScores[i], sizeof(state->highScores[i]), 1, scoreFile);
    				}
    			}
    			fclose(scoreFile);
    		}

    		return NOT_TRUE_AT_ALL;
    	}

    	int linesCleared = 0;
    	for(int i = 0; i < BLOCKS_DOWN; ++i)
    	{
    		if(DetectCompleteLine(state, i))
    		{
    			ClearLine(state, i);
    			DropBlocks(state, i-1);
    			++state->lineCount;
    			++linesCleared;
    			++state->linesThisLevel;
    			if(state->linesThisLevel >= state->levelThreshold)
    			{
    				state->levelThreshold = (int)((float)state->levelThreshold * 1.1f);
    				++state->level;
    				state->linesThisLevel = 0;
    				state->dropInterval *= 0.8f;
    				state->halfDropInterval = state->dropInterval * 0.5f;
    				al_stop_timer(state->dropTimer);
    				al_set_timer_speed(state->dropTimer, state->dropInterval);
    				al_start_timer(state->dropTimer);
    			}
    		}
    	}

    	Assert(linesCleared >= 0 && linesCleared <= 4);
    	state->score += state->scoreRamp[linesCleared];
    	if(state->score >= state->highScores[state->scoreIndex])
    	{
    		if(state->scoreIndex)
    			state->scoreIndex--;
    	}
    }

    return VERY_TRUE;
}

// returns the direction of the playspace that the piece is 'sticking out' into
t_direction ValidateTetronimoPlacement(game_state *state, t_shape shape, grid_coord pivot, t_rotation rotation)
{
	tetronimo *copy = NewTetronimo(state, pivot, rotation, shape);

	t_direction result = 0;

	// check edges
	for(int blockIndex = 0; blockIndex < BLOCKS_PER_TETRONIMO; ++blockIndex)
	{
		grid_coord *block = &copy->blocks[blockIndex];
		if(block->x < 0)
            result = LEFT;
        if(block->x >= BLOCKS_ACROSS)
            result = RIGHT;
        if(block->y >= BLOCKS_DOWN)
            result = DOWN;

		if(result == 0)
		{
			if(state->grid[block->y][block->x] == VERY_TRUE)
				result = DOWN;
		}
	}

	free(copy);
	copy = 0;
	return result; // use the UP constant to signify success since motion is not ever in the upward direction
}

void SolidifyTetronimo(game_state *state)
{
	for(int blockIndex = 0; blockIndex < BLOCKS_PER_TETRONIMO; ++blockIndex)
	{
		grid_coord block = state->tet->blocks[blockIndex];
		state->grid[block.y][block.x] = VERY_TRUE;
	}
}

void LoadRotationOffsets(game_state *state)
{
	FILE *fp = fopen("piece.tdt", "r");

	if(fp)
	{
		for(int shapeIndex = 0; shapeIndex < TETRONIMO_VARIATIONS; ++shapeIndex)
		{
			for(int rotationIndex = 0; rotationIndex < TETRONIMO_ROTATIONS; ++rotationIndex)
			{
				for(int blockIndex = 0; blockIndex < BLOCKS_PER_TETRONIMO; ++blockIndex)
				{
					char dummy;
					block_offset *block = &state->tetRotationOffsets[shapeIndex][rotationIndex][blockIndex];
					fscanf(fp, "%d", &block->x);
					fscanf(fp, "%c", &dummy);
					fscanf(fp, "%d", &block->y);
				}
			}
		}

		fclose(fp);
	}
	else
	{
		printf("could not open ../piece.tdt\n");
		// TODO :: You call this error handling?
	}
}

t_bool DetectCompleteLine(game_state *state, int y)
{
	for(int xIndex = 0; xIndex < BLOCKS_ACROSS; ++xIndex)
	{
		if(state->grid[y][xIndex] == NOT_TRUE_AT_ALL)
			return NOT_TRUE_AT_ALL;
	}
	return VERY_TRUE;
}

void ClearLine(game_state *state, int y)
{
	for(int xIndex = 0; xIndex < BLOCKS_ACROSS; ++xIndex)
	{
		state->grid[y][xIndex] = NOT_TRUE_AT_ALL;
	}
}

void DropBlocks(game_state *state, int lineToDropFrom)
{
	Assert(lineToDropFrom < BLOCKS_DOWN - 1);
	for(int lineIndex = lineToDropFrom; lineIndex >= 0; --lineIndex)
	{
		for(int xIndex = 0; xIndex < BLOCKS_ACROSS; ++xIndex)
		{
			if(state->grid[lineIndex][xIndex] == VERY_TRUE)
			{
				Assert(!state->grid[lineIndex+1][xIndex]);
				state->grid[lineIndex][xIndex] = NOT_TRUE_AT_ALL;
				state->grid[lineIndex+1][xIndex] = VERY_TRUE;
			}
		}
	}
}

void DebugDrawRogueTetronimo(game_state *state, float pivotX, float pivotY, t_shape shape, ALLEGRO_COLOR color)
{
	al_draw_filled_rectangle(pivotX, pivotY, pivotX + BLOCK_SIZE, pivotY + BLOCK_SIZE, color);
	for(int i = 1; i < BLOCKS_PER_TETRONIMO; ++i)
	{
		float newX = pivotX + state->tetRotationOffsets[shape][UP][i].x * BLOCK_SIZE;
		float newY = pivotY + state->tetRotationOffsets[shape][UP][i].y * BLOCK_SIZE;
		al_draw_filled_rectangle(newX, newY, newX + BLOCK_SIZE, newY + BLOCK_SIZE, color);
	}
}

void InitHighScores(game_state *state)
{
	int tmpHigh[3] ={10000, 5000, 100};
	memcpy(state->highScores, tmpHigh, sizeof(state->highScores));

    FILE *scoreFile = fopen("hiscores.scr", "rb");
    if(scoreFile)
    {
    	int size = ArrayCount(state->highScores);
    	for(int i = size-1; i >= 0; --i)
    	{
    		fread(&state->highScores[i], sizeof(int), 1, scoreFile);    		
    	}
    	fclose(scoreFile);
    }
}