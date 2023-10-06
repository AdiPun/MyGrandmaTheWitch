#include "MainGame.h"
#include "PlayerStates.h"

// The entry point for a PlayBuffer program
void MainGameEntry(PLAY_IGNORE_COMMAND_LINE)
{
	Play::CreateManager(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE);
	Play::CentreAllSpriteOrigins();
	Play::LoadBackground("Data\\Backgrounds\\background.png");
	Play::CreateGameObject(TYPE_PLAYER, { DISPLAY_WIDTH,DISPLAY_HEIGHT}, 0, "idle_right");
	Play::PlayAudio("music");
	CreateLevelFromArray();
}

// Called by PlayBuffer every frame (60 times a second!)
bool MainGameUpdate(float elapsedTime)
{
	gamestate.elapsedTime = elapsedTime;
	UpdatePlayer();
	CameraFollow();
	Draw();
	return Play::KeyDown(VK_ESCAPE);
}

// Gets called once when the player quits the game 
int MainGameExit(void)
{
	Play::DestroyManager();
	return PLAY_OK;
}




// Creates a single platform tile
void CreatePlatform(int x, int y, int id)
{
	Platform platform;
	gamestate.vPlatforms.push_back(platform);
	gamestate.vPlatforms.back().pos = Point2D{x,y};
	gamestate.vPlatforms.back().id = id;
}

void CreateLevelFromArray()
{
	LevelLayoutInfo levellayout;

		for (int y = 0; y < levellayout.height; y++)
		{
			for (int x = 0; x < levellayout.width; x++)
			{
				int tileIndex = y * levellayout.width + x; // This is giving us a number, a position 0 to 880

				if (levellayout.levellayout[tileIndex] == 1) // If that number has a 1 in it create a platform
				{
					// Create an object at this position (x, y)
					CreatePlatform((x * platform.AABB.x*2) + platform.AABB.x / 2, (y * platform.AABB.y*2) + platform.AABB.y / 2, levellayout.levellayout[tileIndex]);
				}

				if (levellayout.levellayout[tileIndex] == 2)
				{
					CreatePlatform((x * platform.AABB.x * 2) + platform.AABB.x / 2, (y * platform.AABB.y * 2) + platform.AABB.y / 2, levellayout.levellayout[tileIndex]);
				}

				if (levellayout.levellayout[tileIndex] == 3)
				{
					CreatePlatform((x * platform.AABB.x * 2) + platform.AABB.x / 2, (y * platform.AABB.y * 2) + platform.AABB.y / 2, levellayout.levellayout[tileIndex]);
				}

				if (levellayout.levellayout[tileIndex] == 4)
				{
					CreatePlatform((x * platform.AABB.x * 2) + platform.AABB.x / 2, (y * platform.AABB.y * 2) + platform.AABB.y / 2, levellayout.levellayout[tileIndex]);
				}

				if (levellayout.levellayout[tileIndex] == 5)
				{
					CreatePlatform((x * platform.AABB.x * 2) + platform.AABB.x / 2, (y * platform.AABB.y * 2) + platform.AABB.y / 2, levellayout.levellayout[tileIndex]);
				}
			}
		}
}


// Checks player's AABBmaxY and if it's collided with a platform's minY
bool FloorCollisionStarted()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);

	Point2D playerTopLeft = obj_player.pos - playerinfo.verticalcollisionAABB;
	Point2D playerBottomRight = obj_player.pos + playerinfo.verticalcollisionAABB;

	Point2D playerOldTopLeft = obj_player.oldPos - playerinfo.verticalcollisionAABB;
	Point2D playerOldBottomRight = obj_player.oldPos + playerinfo.verticalcollisionAABB;

	// Iterate through all platforms to check for collisions
	for (const Platform& platform : gamestate.vPlatforms)
	{
		// Calculate the platform's AABB
		Point2D platformTopLeft = platform.pos - platform.AABB;
		Point2D platformBottomRight = platform.pos + platform.AABB;

		// Check for collision between player's grounding box and the platform
		if (playerBottomRight.x > platformTopLeft.x &&
			playerTopLeft.x  < platformBottomRight.x &&
			playerBottomRight.y > platformTopLeft.y &&
			playerTopLeft.y < platformBottomRight.y)
		{


			// Checks if previous frame playermaxY was above the platformminY
			if (playerOldBottomRight.y < platformTopLeft.y)
			{
				return true; // Player is grounded
			}
		}
		
	}

	return false; // Player is not grounded
}

// Checks if player is under ceiling
bool IsUnderCeiling()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);
	Point2D headBoxPos = obj_player.pos - playerinfo.headboxoffset;
		
	Vector2D headboxAABB = playerinfo.headboxAABB;

	// Iterate through all platforms to check for collisions
	for (Platform& platform : gamestate.vPlatforms)
	{
		// Calculate the platform's AABB
		Point2D platformTopLeft = platform.pos - platform.AABB;
		Point2D platformBottomRight = platform.pos + platform.AABB;

		// Check for collision between player's edge box and the platform side
		if (headBoxPos.x + headboxAABB.x > platformTopLeft.x &&
			headBoxPos.x - headboxAABB.x  < platformBottomRight.x &&
			headBoxPos.y + headboxAABB.y  > platformTopLeft.y &&
			headBoxPos.y - headboxAABB.y < platformBottomRight.y)
		{
			CheckHeadboxIsLeftOfPlatform(platform);
			return true; // Player is colliding with platform side
		}
			
	}

	return false; // Player is not colliding with platform side
}

bool CeilingCollisionStarted()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);
	PlatformInfo platforminfo;

	Point2D playerTopLeft = obj_player.pos - playerinfo.verticalcollisionAABB;
	Point2D playerBottomRight = obj_player.pos + playerinfo.verticalcollisionAABB;

	Point2D playerOldTopLeft = obj_player.oldPos - playerinfo.verticalcollisionAABB;
	Point2D playerOldBottomRight = obj_player.oldPos + playerinfo.verticalcollisionAABB;

	// Iterate through all platforms to check for collisions
	for (const Platform& platform : gamestate.vPlatforms)
	{
		// Calculate the platform's AABB
		Point2D platformTopLeft = platform.pos - platform.AABB;
		Point2D platformBottomRight = platform.pos + platform.AABB;

		// Check for collision between player's collision box and the platform
		if (playerBottomRight.x > platformTopLeft.x &&
			playerTopLeft.x  < platformBottomRight.x &&
			playerBottomRight.y > platformTopLeft.y &&
			playerTopLeft.y < platformBottomRight.y)
		{


			// Checks if previous frame was below the platform
			if (playerOldTopLeft.y > platformBottomRight.y)
			{
				
				return true; // Player is hitting head
			}
		}

	}

	return false; // Player is not hitting head
}


bool IsGrounded()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);

	Point2D playerTopLeft = obj_player.pos - playerinfo.verticalcollisionAABB;
	Point2D playerBottomRight = obj_player.pos + playerinfo.verticalcollisionAABB;

	// Iterate through all platforms to check for collisions
	for (const Platform& platform : gamestate.vPlatforms)
	{
		// Calculate the platform's AABB
		Point2D platformTopLeft = platform.pos - platform.AABB;
		Point2D platformBottomRight = platform.pos + platform.AABB;

		// Check for collision between player's grounding box and the platform
		if (playerBottomRight.x > platformTopLeft.x &&
			playerTopLeft.x  < platformBottomRight.x &&
			playerBottomRight.y > platformTopLeft.y &&
			playerTopLeft.y < platformBottomRight.y)
		{

			return true; // Player is grounded

		}

	}

	return false; // Player is not grounded
}

// Check's player's edgebox and if it's going to collide with the sides of a platform
bool WillCollideWithWall()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);

	Point2D playerTopLeft = obj_player.pos - playerinfo.wallcollisionAABB;
	Point2D playerBottomRight = obj_player.pos + playerinfo.wallcollisionAABB;

	Vector2D playernextPosition = obj_player.pos + obj_player.velocity;

	Point2D playernextposTopLeft = playernextPosition - playerinfo.wallcollisionAABB;
	Point2D playernextposBottomRight = playernextPosition + playerinfo.wallcollisionAABB;

	// Iterate through all platforms to check for collisions
	for (Platform& platform : gamestate.vPlatforms)
	{
		// Calculate the platform's AABB
		Point2D platformTopLeft = platform.pos - platform.AABB;
		Point2D platformBottomRight = platform.pos + platform.AABB;

		// Check for collision between player's edge box and the platform
		if (playernextposBottomRight.x > platformTopLeft.x &&
			playernextposTopLeft.x  < platformBottomRight.x &&
			playernextposBottomRight.y  > platformTopLeft.y &&
			playernextposTopLeft.y < platformBottomRight.y)
		{
		
			return true; // Player is colliding with platform side
			
		}
		
	}

	return false; // Player is not colliding with platform side
}

bool IsInsideWall()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);

	Point2D playerTopLeft = obj_player.pos - playerinfo.wallcollisionAABB;
	Point2D playerBottomRight = obj_player.pos + playerinfo.wallcollisionAABB;

	// Iterate through all platforms to check for collisions
	for (Platform& platform : gamestate.vPlatforms)
	{
		// Calculate the platform's AABB
		Point2D platformTopLeft = platform.pos - platform.AABB;
		Point2D platformBottomRight = platform.pos + platform.AABB;

		// Check for collision between player's edge box and the platform
		if (playerBottomRight.x > platformTopLeft.x &&
			playerTopLeft.x  < platformBottomRight.x &&
			playerBottomRight.y  > platformTopLeft.y &&
			playerTopLeft.y < platformBottomRight.y)
		{
			CheckPlayerIsLeftOfPLatform(platform);
			return true; // Player is inside platform
		}

	}

	return false; // Player is not inside platform
}

void CheckPlayerIsLeftOfPLatform(Platform& platform)
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);

	if (obj_player.pos.x < platform.pos.x)
	{
		playerinfo.playerleftofplatform = true;
	}
	else if (obj_player.pos.x > platform.pos.x)
	{
		playerinfo.playerleftofplatform = false;

	}
}

void CheckHeadboxIsLeftOfPlatform(Platform& platform)
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);

	if (obj_player.pos.x < platform.pos.x)
	{
		playerinfo.headboxleftofplatform = true;
	}
	else if (obj_player.pos.x > platform.pos.x)
	{
		playerinfo.headboxleftofplatform = false;

	}
}


void CameraFollow()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);
	
	Play::SetCameraPosition({ obj_player.pos.x-DISPLAY_WIDTH/2, obj_player.pos.y-DISPLAY_HEIGHT/2 });
	
}

void Draw()
{
	Play::DrawBackground();

	DrawPlatforms();

	DrawAllGameObjectsByTypeRotated(TYPE_PLAYER);

	//DrawDebug();
	
	Play::PresentDrawingBuffer();
}



void DrawPlatforms()
{
	Platform platform;

	for (Platform& platform : gamestate.vPlatforms)
	{

		if (platform.id == 1)
		{
			Play::DrawSprite(Play::GetSpriteId("tile"), platform.pos, 0);
		}
		else if (platform.id == 2)
		{
			Play::DrawSprite(Play::GetSpriteId("rock"), platform.pos, 0);
		}
		else if (platform.id == 3)
		{
			Play::DrawSprite(Play::GetSpriteId("wood"), platform.pos, 0);
		}
		else if (platform.id == 4)
		{
			Play::DrawSprite(Play::GetSpriteId("tree"), platform.pos, 0);
		}
	}
}

void DrawSlideableTiles()
{
	Platform platform;

	for (SlideableTile& tile : gamestate.vSlideabletiles)
	{

		Play::DrawSprite(Play::GetSpriteId("slide_tile"), platform.pos, 0);
		
	}
}

void DrawPlatformsAABB()
{
	Platform platform;

	for (Platform& p : gamestate.vPlatforms)
	{
		DrawObjectAABB(p.pos, platform.AABB);
	}
}


void DrawAllGameObjectsByTypeRotated(GameObjectType type)
{
	for (int id : Play::CollectGameObjectIDsByType(type))
	{
		GameObject& obj = Play::GetGameObject(id);
		Play::DrawObjectRotated(obj);
	}
}

void DrawAllGameObjectsByType(GameObjectType type)
{
	for (int id : Play::CollectGameObjectIDsByType(type))
	{
		GameObject& obj = Play::GetGameObject(id);
		Play::DrawObject(obj);
	}
}

// Draws a green AABB around the object
void DrawObjectAABB(Point2D objcentre, Vector2D objAABB)
{
	Point2D topLeft = objcentre - objAABB;
	Point2D bottomRight = objcentre + objAABB;
	Play::DrawRect(topLeft, bottomRight, Play::cGreen);
}

void DrawPlayerAABB()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);

	Point2D playerTopLeft = obj_player.pos - playerinfo.wallcollisionAABB;

	Point2D playerBottomRight = obj_player.pos + playerinfo.wallcollisionAABB;


	Play::DrawRect(playerTopLeft, playerBottomRight, Play::cGreen);

}

void DrawPlayerNextPositionAABB()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);
	
	Vector2D playernextPosition = obj_player.pos + obj_player.velocity;

	Point2D playernextposTopLeft = playernextPosition - playerinfo.verticalcollisionAABB;

	Point2D playernextposBottomRight = playernextPosition + playerinfo.verticalcollisionAABB;
	

	Play::DrawRect(playernextposTopLeft, playernextposBottomRight, Play::cBlue);
}

void DrawDebug()
{
	DrawPlayerNextPositionAABB();

	DrawPlatformsAABB();

	DrawPlayerAABB();

	DrawObjectAABB(Play::GetGameObjectByType(TYPE_PLAYER).pos - playerinfo.headboxoffset, playerinfo.headboxAABB);
}