#include "MainGame.h"

enum GameObjectType
{
	TYPE_PLAYER,
	TYPE_PLATFORM,
};

enum PlayerState
{
	STATE_GROUND = 0,
	STATE_FALLING,
	STATE_LANDING,
	STATE_JUMPING,
	STATE_ATTACK,
};

struct PlayerInfo
{
	Vector2D AABB{ 10,20 };
	Vector2D maxyoffset{ 0,40 };
	Vector2D groundingboxAABB{ 15,1 };
	
	bool facingright = true;
	float animationspeedidle{ 0.2f };
	float animationspeedrun{ 0.2f };
	float animationspeedjump{ 0.2f };
	float animationspeedfall{ 0.2f };
	float animationspeedland { 0.2f };
	float animationspeedatk{ 0.2f };

	float friction{ 0.8f };

	float runspeed{ 4.5f };
	float jumpspeed{ 12.0f };
	float fallspeed{ 3.5f };

	float scale{ 2.5f };
	float gravity{ 0.3f};
};

struct Platform
{
	int type = TYPE_PLATFORM;
	Point2D pos;
	const Vector2D AABB{ 32,32 };
};

Platform platform;

struct GameState
{
	PlayerState playerstate = STATE_GROUND;
	std::vector<Platform> vPlatforms;
};

PlayerInfo playerinfo;
GameState gamestate;

void UpdatePlayer();
void HandlePlayerControls();
void HandleAirborneControls();


void CreatePlatform(int x, int y);
void CreatePlatformRow(int tiles, int x, int y);
void CreatePlatformFloor();

bool IsGrounded();

void Draw();
void DrawPlatforms();
void DrawAllGameObjectsByTypeRotated(GameObjectType type);
void DrawAllGameObjectsByType(GameObjectType type);
void DrawObjectAABB(Point2D objcentre, Vector2D objAABB);


// The entry point for a PlayBuffer program
void MainGameEntry(PLAY_IGNORE_COMMAND_LINE)
{
	Play::CreateManager(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE);
	Play::CentreAllSpriteOrigins();
	Play::LoadBackground("Data\\Backgrounds\\background.png");
	Play::CreateGameObject(TYPE_PLAYER, { DISPLAY_WIDTH / 2,DISPLAY_HEIGHT / 2 }, 0, "idle_right");
	CreatePlatformFloor();
	CreatePlatform(DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 4*3);
	CreatePlatformRow(5, 100, 400);
}

// Called by PlayBuffer every frame (60 times a second!)
bool MainGameUpdate(float elapsedTime)
{
	UpdatePlayer();
	Draw();
	return Play::KeyDown(VK_ESCAPE);
}

// Gets called once when the player quits the game 
int MainGameExit(void)
{
	Play::DestroyManager();
	return PLAY_OK;
}

void UpdatePlayer()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);


	obj_player.scale = playerinfo.scale;
	
	// Grounded interactions
	if (IsGrounded())
	{
		obj_player.acceleration.y = 0;
		obj_player.velocity.y = 0;
	}
	else if (!IsGrounded())
	{
		obj_player.acceleration.y = playerinfo.gravity;
	}

	switch (gamestate.playerstate)
	{
	case STATE_LANDING:

		obj_player.velocity.x *= playerinfo.friction;

		if (!playerinfo.facingright)
		{
			Play::SetSprite(obj_player, "land_left", playerinfo.animationspeedland);
		}
		else if (playerinfo.facingright)
		{
			Play::SetSprite(obj_player, "land_right", playerinfo.animationspeedland);
		}
		if (Play::IsAnimationComplete(obj_player))
		{
			gamestate.playerstate = STATE_GROUND;
		}
		break;

	case STATE_GROUND:

		obj_player.velocity.x *= playerinfo.friction;
	
		HandlePlayerControls();


		if (!IsGrounded()) 
		{
			gamestate.playerstate = STATE_FALLING;
		}

		break;

	case STATE_JUMPING:

		obj_player.velocity.x *= playerinfo.friction;


		HandleAirborneControls();


		if (!playerinfo.facingright)
		{
			Play::SetSprite(obj_player, "jump_left", playerinfo.animationspeedjump);
		}
		else if (playerinfo.facingright)
		{
			Play::SetSprite(obj_player, "jump_right", playerinfo.animationspeedjump);
		}

		if (Play::IsAnimationComplete(obj_player))
		{
			gamestate.playerstate = STATE_FALLING;
		}
		if (IsGrounded())
		{
			gamestate.playerstate = STATE_LANDING;
		}
		break;

	case STATE_FALLING:

		HandleAirborneControls();

		obj_player.velocity.x *= playerinfo.friction;

		if (!playerinfo.facingright)
		{
			Play::SetSprite(obj_player, "fall_left", playerinfo.animationspeedfall);
		}
		else if (playerinfo.facingright)
		{
			Play::SetSprite(obj_player, "fall_right", playerinfo.animationspeedfall);
		}
		if (IsGrounded())
		{
			gamestate.playerstate = STATE_LANDING;
		}

		

		break;

	case STATE_ATTACK:

		obj_player.velocity.x *= 0.9f;

		if (!playerinfo.facingright)
		{
			Play::SetSprite(obj_player, "atk_left", playerinfo.animationspeedatk);
		}
		else if (playerinfo.facingright)
		{
			Play::SetSprite(obj_player, "atk_right", playerinfo.animationspeedatk);
		}
		if (Play::IsAnimationComplete(obj_player))
		{
			gamestate.playerstate = STATE_GROUND;
		}
		break;
	}
	Play::UpdateGameObject(obj_player);
}

void HandlePlayerControls()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);
	
	// Jump
	if (Play::KeyPressed(VK_UP))
	{
		obj_player.velocity.y -= playerinfo.jumpspeed;
		gamestate.playerstate = STATE_JUMPING;
	}

	// Running animation
	if (Play::KeyDown(VK_LEFT))
	{
		playerinfo.facingright = false;
		Play::SetSprite(obj_player, "run_left", playerinfo.animationspeedrun);
		obj_player.velocity.x = -playerinfo.runspeed;
	}
	else if (Play::KeyDown(VK_RIGHT))
	{
		playerinfo.facingright = true;
		Play::SetSprite(obj_player, "run_right", playerinfo.animationspeedrun);
		obj_player.velocity.x = playerinfo.runspeed;
	}
	
	// Idle animation
	if (playerinfo.facingright && !Play::KeyDown(VK_LEFT) && !Play::KeyDown(VK_RIGHT))
	{
		Play::SetSprite(obj_player, "idle_right", playerinfo.animationspeedidle); //Idle
	}
	else if (!playerinfo.facingright && !Play::KeyDown(VK_LEFT) && !Play::KeyDown(VK_RIGHT))
	{
		Play::SetSprite(obj_player, "idle_left", playerinfo.animationspeedidle); //Idle
	}


	if (Play::KeyPressed(VK_SPACE))
	{
		gamestate.playerstate = STATE_ATTACK;
	}
}


void HandleAirborneControls()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);

	if (Play::KeyDown(VK_LEFT))
	{
		playerinfo.facingright = false;
		obj_player.velocity.x = -playerinfo.fallspeed;
	}
	else if (Play::KeyDown(VK_RIGHT))
	{
		playerinfo.facingright = true;
		obj_player.velocity.x = playerinfo.fallspeed;
	}
}

// Creates a single platform tile
void CreatePlatform(int x, int y)
{
	Platform platform;
	gamestate.vPlatforms.push_back(platform);
	gamestate.vPlatforms.back().pos = Point2D{x,y};
}

// Creates a row of platform tiles
void CreatePlatformRow(int tiles, int x, int y)
{
	Platform platform;
	for (int i = 0; i < tiles; i++)
	{
		int tilespacing = 64 * i;
		gamestate.vPlatforms.push_back(platform);
		gamestate.vPlatforms.back().pos = Point2D{ x + tilespacing , y };
	}
}

// Creates a floor of platform tiles
void CreatePlatformFloor()
{
	Platform platform;

	for (int display_x = 0; display_x < DISPLAY_WIDTH / 16; display_x++)
	{
		int display_fraction = display_x * DISPLAY_WIDTH / 16;
		gamestate.vPlatforms.push_back(platform);
		gamestate.vPlatforms.back().pos = Point2D{ display_fraction,DISPLAY_HEIGHT - 32 };
	}
}

// Checks player's groundingbox and if it's colliding with a platform
bool IsGrounded()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);
	Point2D groundingBoxPos = obj_player.pos + playerinfo.maxyoffset;
	Vector2D groundingBoxAABB = playerinfo.groundingboxAABB;

	// Iterate through all platforms to check for collisions
	for (const Platform& platform : gamestate.vPlatforms)
	{
		// Calculate the platform's AABB
		Point2D platformTopLeft = platform.pos - platform.AABB;
		Point2D platformBottomRight = platform.pos + platform.AABB;

		// Check for collision between player's grounding box and the platform
		if (groundingBoxPos.x + groundingBoxAABB.x  > platformTopLeft.x &&
			groundingBoxPos.x - groundingBoxAABB.x  < platformBottomRight.x &&
			groundingBoxPos.y + groundingBoxAABB.y  > platformTopLeft.y &&
			groundingBoxPos.y - groundingBoxAABB.y  < platformBottomRight.y)
		{
			return true; // Player is grounded
		}
	}

	return false; // Player is not grounded
}

void Draw()
{
	Play::DrawBackground();
	Play::DrawSprite("middle", { DISPLAY_WIDTH / 2,DISPLAY_HEIGHT / 2 }, 0);
	DrawPlatforms();
	DrawAllGameObjectsByTypeRotated(TYPE_PLAYER);
	//DrawObjectAABB(Play::GetGameObjectByType(TYPE_PLAYER).pos, playerinfo.AABB);
	//DrawObjectAABB(Play::GetGameObjectByType(TYPE_PLAYER).pos + playerinfo.maxyoffset, playerinfo.groundingboxAABB);
	Play::PresentDrawingBuffer();
}

// Draws all platforms in vPlatforms
void DrawPlatforms()
{
	for (Platform& p : gamestate.vPlatforms)
	{
		Play::DrawSprite(Play::GetSpriteId("tile"),p.pos,0);
		//DrawObjectAABB(p.pos, platform.AABB);
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