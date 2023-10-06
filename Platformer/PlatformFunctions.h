#pragma once

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
				CreatePlatform((x * platform.AABB.x * 2) + platform.AABB.x / 2, (y * platform.AABB.y * 2) + platform.AABB.y / 2, levellayout.levellayout[tileIndex]);
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

