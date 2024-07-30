// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
// 
// Released under the BSD License
// See LICENSE in root directory for full details.
// ----------------------------------------------------------------

#include "Game.h"

const int thickness = 15;
const float paddleH = 100.0f;

Game::Game()
	:mWindow(nullptr)
	, mRenderer(nullptr)
	, mTicksCount(0)
	, mIsRunning(true)
	, mPaddleDir{ 0,0 }
{

}

bool Game::Initialize()
{
	// Initialize SDL
	int sdlResult = SDL_Init(SDL_INIT_VIDEO);
	//SDL_Initは初期化に成功すると整数０を返す
	if (sdlResult != 0)
	{
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
		return false;
	}

	// Create an SDL Window
	mWindow = SDL_CreateWindow(
		"Game Programming in C++ (Chapter 1)", // Window title
		100,	// Top left x-coordinate of window
		100,	// Top left y-coordinate of window
		1024,	// Width of window
		768,	// Height of window
		0		// Flags (0 for no flags set)
	);

	if (!mWindow)	//もしnullptrなら
	{
		SDL_Log("Failed to create window: %s", SDL_GetError());
		return false;
	}

	//// Create SDL renderer
	mRenderer = SDL_CreateRenderer(
		mWindow, // Window to create renderer for 描画対象となるウィンドウ
		-1,		 // Usually -1
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
	);

	if (!mRenderer)
	{
		SDL_Log("Failed to create renderer: %s", SDL_GetError());
		return false;
	}
	// パドルは二つ分
	mPaddlePos[0].x = 10.0f;
	mPaddlePos[0].y = 768.0f / 2.0f;
	mPaddlePos[1].x = 1024.0f - 10.0f - thickness;
	mPaddlePos[1].y = 768.0f / 2.0f;
	mBallPos.x = 1024.0f / 2.0f;
	mBallPos.y = 768.0f / 2.0f;
	mBallVel.x = -200.0f;
	mBallVel.y = 235.0f;
	return true;
}

void Game::RunLoop()
{
	while (mIsRunning)
	{
		ProcessInput();
		UpdateGame();
		GenerateOutput();
	}
}

void Game::ProcessInput()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			// If we get an SDL_QUIT event, end loop
		case SDL_QUIT:
			mIsRunning = false;
			break;
		}
	}

	// Get state of keyboard
	// SDL_GetKeyboardStateは「キーボードの現在の状態が格納された配列」へのポインタを返す関数
	const Uint8* state = SDL_GetKeyboardState(NULL);
	// If escape is pressed, also end loop
	// SDL_SCANCODE_ESCAPEは列挙型の値
	if (state[SDL_SCANCODE_ESCAPE])
	{
		mIsRunning = false;
	}

	// Update paddle direction based on W/S keys
	mPaddleDir[0] = 0;
	if (state[SDL_SCANCODE_W])
	{
		mPaddleDir[0] -= 1;
	}
	if (state[SDL_SCANCODE_S])
	{
		mPaddleDir[0] += 1;
	}
	// Update paddle direction based on I/K keys
	mPaddleDir[1] = 0;
	if (state[SDL_SCANCODE_I])
	{
		mPaddleDir[1] -= 1;
	}
	if (state[SDL_SCANCODE_K])
	{
		mPaddleDir[1] += 1;
	}
}

void Game::UpdateGame()
{
	// Wait until 16ms has elapsed since last frame
	while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTicksCount + 16))
		;

	// Delta time is the difference in ticks from last frame
	// (converted to seconds)
	float deltaTime = (SDL_GetTicks() - mTicksCount) / 1000.0f;

	// Clamp maximum delta time value
	if (deltaTime > 0.05f)
	{
		deltaTime = 0.05f;
	}

	// Update tick counts (for next frame)
	mTicksCount = SDL_GetTicks();

	// Update paddle position based on direction
	for (int i = 0; i < 2; i++)
	{
		if (mPaddleDir[i] != 0)
		{
			mPaddlePos[i].y += mPaddleDir[i] * 300.0f * deltaTime;
			// Make sure paddle doesn't move off screen!
			if (mPaddlePos[i].y < (paddleH / 2.0f + thickness))
			{
				mPaddlePos[i].y = paddleH / 2.0f + thickness;
			}

			else if (mPaddlePos[i].y > (768.0f - paddleH / 2.0f - thickness))
			{
				mPaddlePos[i].y = 768.0f - paddleH / 2.0f - thickness;
			}
		}
	}

	// Update ball position based on ball velocity
	mBallPos.x += mBallVel.x * deltaTime;
	mBallPos.y += mBallVel.y * deltaTime;

	// Bounce if needed
	// Did we intersect with the paddle?
	float diff0 = mPaddlePos[0].y - mBallPos.y;
	float diff1 = mPaddlePos[1].y - mBallPos.y;
	// Take absolute value of difference 絶対値
	diff0 = (diff0 > 0.0f) ? diff0 : -diff0;
	if (
		// Our y-difference is small enough
		diff0 <= paddleH / 2.0f &&
		// We are in the correct x-position
		mBallPos.x <= 25.0f && mBallPos.x >= 20.0f &&
		// The ball is moving to the left
		mBallVel.x < 0.0f)
	{
		mBallVel.x *= -1.0f;
	}
	diff1 = (diff1 > 0.0f) ? diff1 : -diff1;
	if (
		// Our y-difference is small enough
		diff1 <= paddleH / 2.0f &&
		// We are in the correct x-position
		mBallPos.x >= 1024.0f - 10.0f - thickness &&
		mBallPos.x <= 1024.0f - 10.0f - thickness + 5.0f &&
		// The ball is moving to the left
		mBallVel.x > 0.0f)
	{
		mBallVel.x *= -1.0f;
	}
	// Did the ball go off the screen? (if so, end game)
	else if (mBallPos.x <= 0.0f || mBallPos.x >= 1024.0f - thickness)
	{
		mIsRunning = false;
	}
	// Did the ball collide with the right wall?
	else if (mBallPos.x >= (1024.0f - thickness) && mBallVel.x > 0.0f)
	{
		mBallVel.x *= -1.0f;
	}

	// Did the ball collide with the top wall?
	if (mBallPos.y <= thickness && mBallVel.y < 0.0f)
	{
		mBallVel.y *= -1;
	}
	// Did the ball collide with the bottom wall?
	else if (mBallPos.y >= (768 - thickness) &&
		mBallVel.y > 0.0f)
	{
		mBallVel.y *= -1;
	}
}

void Game::GenerateOutput()
{
	// Set draw color to blue
	SDL_SetRenderDrawColor(
		mRenderer,
		0,		// R
		0,		// G 
		255,	// B
		255		// A
	);

	// Clear back buffer
	SDL_RenderClear(mRenderer);

	// Draw walls
	SDL_SetRenderDrawColor(mRenderer, 255, 255, 255, 255);

	// Draw top wall
	SDL_Rect wall{
		0,			// Top left x
		0,			// Top left y
		1024,		// Width
		thickness	// Height
	};
	SDL_RenderFillRect(mRenderer, &wall);

	// Draw bottom wall
	wall.y = 768 - thickness;
	SDL_RenderFillRect(mRenderer, &wall);

	//// Draw right wall
	//wall.x = 1024 - thickness;
	//wall.y = 0;
	//wall.w = thickness;
	//wall.h = 1024;
	//SDL_RenderFillRect(mRenderer, &wall);

	// Draw two paddles
	for (int i = 0; i < 2; i++)
	{
		SDL_Rect paddle{
			static_cast<int>(mPaddlePos[i].x),
			static_cast<int>(mPaddlePos[i].y - paddleH / 2),
			thickness,
			static_cast<int>(paddleH)
		};
		SDL_RenderFillRect(mRenderer, &paddle);
	}

	// Draw ball
	SDL_Rect ball{
		static_cast<int>(mBallPos.x - thickness / 2),
		static_cast<int>(mBallPos.y - thickness / 2),
		thickness,
		thickness
	};
	SDL_RenderFillRect(mRenderer, &ball);

	// Swap front buffer and back buffer
	SDL_RenderPresent(mRenderer);
}

void Game::Shutdown()
{
	SDL_DestroyRenderer(mRenderer);
	SDL_DestroyWindow(mWindow);
	SDL_Quit();
}
