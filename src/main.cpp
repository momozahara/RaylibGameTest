#pragma comment(lib, "winmm")

#include <iostream>
#include <vector>
#include <sstream>

#include <raylib.h>
#include <raymath.h>

#define GLSL_VERSION 330

#define PHYSAC_IMPLEMENTATION
#include <extras/physac.h>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

using namespace std;

int main()
{
	InitWindow((int)(1280), (int)(720), "Game");
	InitAudioDevice();
	InitPhysics();

	SetExitKey(0);
	HideCursor();

	SetWindowState(FLAG_WINDOW_MAXIMIZED);
	SetWindowMinSize(1280/2, 720/2);

	SetPhysicsGravity(0.f, 4.9f);

	// Create Camera
	Camera2D camera = { { 1920.f / 2.f, 1080.f / 2.f }, { 0, -30.f }, 0.f, 2.f };

	// Create Render Texture
	RenderTexture2D texture = LoadRenderTexture(1920, 1080);

	// Start Create Game Objects

	PhysicsBody floor = CreatePhysicsBodyRectangle({ 0.f, 0.f}, 1400.f, 20.f, 10.f);
	floor->enabled = false;

	PhysicsBody floor1 = CreatePhysicsBodyRectangle({ 0.f, -160.f }, 500.f, 20.f, 10.f);
	floor1->enabled = false;

	PhysicsBody floor2 = CreatePhysicsBodyRectangle({ 400.f, -80.f }, 150.f, 20.f, 10.f);
	floor2->enabled = false;

	PhysicsBody floor3 = CreatePhysicsBodyRectangle({ -400.f, -80.f }, 150.f, 20.f, 10.f);
	floor3->enabled = false;

	PhysicsBody player = CreatePhysicsBodyRectangle({ 0.f, -30.f }, 20.f, 40.f, 1.f);
	player->freezeOrient = true;

	// End

	// Start Load Shader
	
	int targetShaders = 1;
	Shader shaders[2] = { 0 };
	shaders[0] = LoadShader("./GameData/Shader/base.vs", "./GameData/Shader/base.fs");
	shaders[1] = LoadShader("./GameData/Shader/base.vs", "./GameData/Shader/distort.fs");

	// End Load Shader

	// Start Load Sound

	Sound jumpSE = LoadSound("./GameData/Audio/jump.wav");
	SetSoundVolume(jumpSE, 0.1f);
	Sound explosionSE = LoadSound("./GameData/Audio/explosion.wav");
	SetSoundVolume(explosionSE, 0.1f);
	Sound clickSE = LoadSound("./GameData/Audio/click.wav");
	SetSoundVolume(clickSE, 0.1f);
	Sound resetSE = LoadSound("./GameData/Audio/reset.wav");
	SetSoundVolume(resetSE, 0.1f);

	// End Load Sound

	int targetFPS = GetMonitorRefreshRate(0);
	SetTargetFPS(targetFPS);

	while (!WindowShouldClose())
	{
		// Set mouse scale
		{
			float x = 1920.f / (float)GetScreenWidth();
			float y = 1080.f / (float)GetScreenHeight();

			SetMouseScale(x, y);
		}

		// Get deltaTime
		float deltaTime = GetFrameTime();

		UpdatePhysics();

		// Start Movement

		// Ground Check
		if (player->isGrounded)
		{
			player->velocity = { 0, player->velocity.y };
		}
		else
		{
			player->velocity = { player->velocity.x, player->velocity.y };
		}

		// Movement
		float moveSpeed = 0.5f;
		if (IsKeyDown(KEY_W) && player->isGrounded)
		{
			PlaySound(jumpSE);
			player->velocity.y = -1.0f;
		}
		if (IsKeyDown(KEY_A) && player->isGrounded)
		{
			player->velocity.x = -moveSpeed ;
		}
		if (IsKeyDown(KEY_D) && player->isGrounded)
		{
			player->velocity.x = moveSpeed;
		}

		// End Movement

		// Start Function

		// Frame Rate Control
		// Max at refresh rate / Unlimited
		if (IsKeyPressed(KEY_F1))
		{
			if (targetFPS == 0)
			{
				targetFPS = GetMonitorRefreshRate(0);
			}
			else
			{
				targetFPS = 0;
			}
			SetTargetFPS(targetFPS);
		}

		// Cycle Shader from list
		if (IsKeyPressed(KEY_F3))
		{
			targetShaders += 1;
			if (targetShaders >= 2) targetShaders = 0;
		}

		// Toggle Fullscreen
		if (IsKeyPressed(KEY_F11))
		{
			static bool fullscreen = false;
			if (fullscreen)
			{
				fullscreen = false;
				ClearWindowState(FLAG_WINDOW_UNDECORATED);
				SetWindowSize(1280, 720);
				SetWindowPosition(GetMonitorWidth(0) / 2 - 1280 / 2, GetMonitorHeight(0) / 2 - 720 / 2);
			}
			else
			{
				fullscreen = true;
				SetWindowState(FLAG_WINDOW_UNDECORATED);
				SetWindowSize(GetMonitorWidth(0), GetMonitorHeight(0));
				SetWindowPosition(0, 0);
			}
		}

		// End Function

		// Reset Game
		if (IsKeyPressed(KEY_R))
		{
			PlaySound(resetSE);
			
			ResetPhysics();

			floor = CreatePhysicsBodyRectangle({ 0.f, 0.f}, 1400.f, 20.f, 10.f);
			floor->enabled = false;

			floor1 = CreatePhysicsBodyRectangle({ 0.f, -160.f }, 500.f, 20.f, 10.f);
			floor1->enabled = false;

			floor2 = CreatePhysicsBodyRectangle({ 400.f, -80.f }, 150.f, 20.f, 10.f);
			floor2->enabled = false;

			floor3 = CreatePhysicsBodyRectangle({ -400.f, -80.f }, 150.f, 20.f, 10.f);
			floor3->enabled = false;

			player = CreatePhysicsBodyRectangle({ 0.f, -30.f }, 20.f, 40.f, 1.f);
			player->freezeOrient = true;
			
			player->velocity = { 0, 0, };
			player->position = { 0, -30.f };

			camera.target = player->position;
		}

		// Left Mouse Click
		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
		{
			Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), camera);
			
			// Get count of physics bodies
			int phyBodyCount = GetPhysicsBodiesCount();
			for (int i = 0; i < phyBodyCount; i++)
			{
				// Get physics body
				PhysicsBody body = GetPhysicsBody(i);
				if (body != NULL)
				{
					vector<Vector2> points;
					// Get physics body verticies count
					int vertexCount = GetPhysicsShapeVerticesCount(i);
					// Loop through all verticies and push into vector
					for (int j = 0; j < vertexCount; j++)
					{
						Vector2 vertexA = GetPhysicsShapeVertex(body, j);

						points.push_back(vertexA);
					}

					// Check if the mouse is inside any of the polygon
					bool inside = false;
					for (int j = 0, k = (int)points.size() - 1; j < (int)points.size(); k = j++)
					{
						if ((points[j].y > mousePos.y) != (points[k].y > mousePos.y) &&
							(mousePos.x < (points[k].x - points[j].x) * (mousePos.y - points[j].y) / (points[k].y - points[j].y) + points[j].x))
						{
							inside = !inside;
						}
					}

					// If the mouse is inside the polygon, destroy the body
					if (inside)
					{
						if (body != player && body != floor)
						{
							cout << "PHYSICSBODY ID: " << body->id << " Destroyed" << endl;
							PlaySound(explosionSE);
							DestroyPhysicsBody(body);
						}
						else
						{
							PlaySound(clickSE);
						}
						break;
					}
				}
			}
		}

		// Smooth follow camera
		float camLerpSpeed = 5.f;
		float lx = lerp(camera.target.x, player->position.x, camLerpSpeed * deltaTime);
		float ly = lerp(camera.target.y, player->position.y, camLerpSpeed * deltaTime);
		camera.target = { lx, ly };
		
		// Begin Drawing Render Texture
		// This is not the main viewport renderer
		BeginTextureMode(texture);
			// Using 2D mode
			BeginMode2D(camera);
				ClearBackground(BLACK);
				{
					// Draw All Physics Bodies
					int phyBodyCount = GetPhysicsBodiesCount();
					for (int i = 0; i < phyBodyCount; i++)
					{
						PhysicsBody body = GetPhysicsBody(i);
						if (body != NULL)
						{
							int vertexCount = GetPhysicsShapeVerticesCount(i);
							for (int j = 0; j < vertexCount; j++)
							{
								Vector2 vertexA = GetPhysicsShapeVertex(body, j);

								int jj = (((j + 1) < vertexCount) ? (j + 1) : 0);   // Get next vertex or first to close the shape
								Vector2 vertexB = GetPhysicsShapeVertex(body, jj);

								// Hardcode time >:D
								Color lineColor = GREEN;
								if (body == player)
									lineColor = WHITE;
								else if (body == floor)
									lineColor = YELLOW;

								DrawLineEx(vertexA, vertexB, 1.5f, lineColor);   // Draw a line between two vertex positions
							}
						}
					}
				}

				// Draw Input Keys above Player
				if (IsKeyDown(KEY_R)) DrawText("R", (int)player->position.x + 14, (int)player->position.y - 60, 20, WHITE);
				if (IsKeyDown(KEY_W)) DrawText("W", (int)player->position.x - 7, (int)player->position.y - 60, 20, WHITE);
				if (IsKeyDown(KEY_A)) DrawText("A", (int)player->position.x - 16, (int)player->position.y - 40, 20, WHITE);
				if (IsKeyDown(KEY_D)) DrawText("D", (int)player->position.x + 4, (int)player->position.y - 40, 20, WHITE);

			EndMode2D();

			// Start Overlay
			{
				stringstream ss;
				ss << "FPS: " << GetFPS();
				DrawText(ss.str().c_str(), 60, 40, 30, WHITE);
			}
			{
				stringstream ss;
				ss << "Shader: " << targetShaders;
				DrawText(ss.str().c_str(), 60, 70, 30, WHITE);
			}
			{
				int i = 60;
				if (targetShaders == 1) i = 70;
				DrawText("DeltaTime", i, 100, 30, WHITE);
				{
					stringstream ss;
					ss << deltaTime;
					DrawText(ss.str().c_str(), i, 130, 30, WHITE);
				}
			}

			// Draw Mouse Position
			{
				Vector2 pos = GetScreenToWorld2D(GetMousePosition(), camera);
				stringstream ss;
				ss << "Mouse: " << (int)pos.x << " " << (int)pos.y;
				DrawText(ss.str().c_str(), GetMouseX(), GetMouseY(), 30, WHITE);
			}

			// End Overlay

		EndTextureMode();
		
		// Main viewport Renderer
		BeginDrawing();
			ClearBackground(BLACK);

			// Begin Scaling Render Texture to fit the viewport
			// ratio 16:9

			float x = (float)GetScreenWidth() / 1920.f;
			float y = (float)GetScreenHeight() / 1080.f;

			if (x > y)
			{
				x = y;
			}
			else if (y > x)
			{
				y = x;
			}

			float outX = 1920.f * x;
			float outY = 1080.f * y;

			float posX = ((float)GetScreenWidth() - outX) / 2.f;
			float posY = ((float)GetScreenHeight() - outY) / 2.f;

			// End Scaling Render Texture

			// Using Shader to draw the Render Texture
			BeginShaderMode(shaders[targetShaders]);
				// Draw Render Texture
				DrawTexturePro(texture.texture, { 0.f, 0.f, (float)texture.texture.width, -(float)texture.texture.height }, { posX, posY, outX, outY }, { 0.f, 0.f, }, 0.f, WHITE);
			EndShaderMode();
		
		EndDrawing();
	}

	// De-Initialization

	for(int i = 0; i < 2; i++)
	{
		UnloadShader(shaders[i]);
	}

	UnloadRenderTexture(texture);

	UnloadSound(jumpSE);
	UnloadSound(explosionSE);
	UnloadSound(clickSE);
	UnloadSound(resetSE);

	ClosePhysics();
	CloseAudioDevice();
	CloseWindow();
}