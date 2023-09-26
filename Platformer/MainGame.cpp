#include "MainGame.h"

enum GameObjectType
{
	TYPE_PLAYER,
	TYPE_PLATFORM,
};

enum PlayerState
{
	STATE_GROUND = 0,
	STATE_AIRBORNE,
	STATE_LANDING,
	STATE_JUMPING,
	STATE_ATTACK,
};

struct PlayerInfo
{
	Vector2D AABB{ 10,20 };
	bool facingright = true;
	float animationspeedidle{ 0.15f };
	float animationspeedrun{ 0.3f };
	float animationspeedjump{ 0.2f };
	float animationspeedfall{ 0.05f };
	float animationspeedland { 0.1f };
	float animationspeedatk{ 0.3f };
	float runspeed{ 4.5f };
	float jumpspeed{ 4.0f };
	float fallspeed{ 3.5f };
	float scale{ 2.5f };
	float gravity{ 0.0f};
};

struct Platform
{
	int type = TYPE_PLATFORM;
	Point2D pos;
	const Vector2D AABB{ 16,16 };
};

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
	
	obj_player.acceleration.y = playerinfo.gravity;
	switch (gamestate.playerstate)
	{
	case STATE_LANDING:

		obj_player.velocity.x *= 0.94f;

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

		obj_player.velocity.x *= 0.85f;
		HandlePlayerControls();
		break;

	case STATE_JUMPING:

		obj_player.velocity.x *= 0.96f;

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
			gamestate.playerstate = STATE_AIRBORNE;
		}
		break;
	case STATE_AIRBORNE:

		obj_player.velocity.x *= 0.96f;

		if (!playerinfo.facingright)
		{
			Play::SetSprite(obj_player, "fall_left", playerinfo.animationspeedfall);
		}
		else if (playerinfo.facingright)
		{
			Play::SetSprite(obj_player, "fall_right", playerinfo.animationspeedfall);
		}
		HandleAirborneControls();
		break;

	case STATE_ATTACK:

		obj_player.velocity.x *= 0.9f;

		if (!playerinfo.facingright)
		{
			Play::SetSprite(obj_player, "atk3_left", playerinfo.animationspeedatk);
		}
		else if (playerinfo.facingright)
		{
			Play::SetSprite(obj_player, "atk3_right", playerinfo.animationspeedatk);
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
	
	// Running animation
	if (Play::KeyDown(VK_LEFT))
	{
		playerinfo.facingright = false;
		Play::SetSprite(obj_player, "run_left", playerinfo.animationspeedrun);
		obj_player.velocity.x = -playerinfo.fallspeed;
	}
	else if (Play::KeyDown(VK_RIGHT))
	{
		playerinfo.facingright = true;
		Play::SetSprite(obj_player, "run_right", playerinfo.animationspeedrun);
		obj_player.velocity.x = playerinfo.fallspeed;
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

	// Jump
	if (Play::KeyPressed(VK_UP))
	{
		//obj_player.velocity.y -= 5;
		gamestate.playerstate = STATE_JUMPING;
	}

	if (Play::KeyPressed(VK_SPACE))
	{
		gamestate.playerstate = STATE_ATTACK;
	}
}

void HandleAirborneControls()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);
	// Falling animation
	if (Play::KeyDown(VK_LEFT))
	{
		playerinfo.facingright = false;
		Play::SetSprite(obj_player, "fall_left", playerinfo.animationspeedfall);
		obj_player.velocity.x = -playerinfo.fallspeed;
	}
	else if (Play::KeyDown(VK_RIGHT))
	{
		playerinfo.facingright = true;
		Play::SetSprite(obj_player, "fall_right", playerinfo.animationspeedfall);
		obj_player.velocity.x = playerinfo.fallspeed;
	}
	if (Play::KeyDown(VK_DOWN))
	{
		gamestate.playerstate = STATE_LANDING;
	}
}

void CreatePlatform(int x, int y)
{
	Platform platform;
	gamestate.vPlatforms.push_back(platform);
	gamestate.vPlatforms.back().pos = Point2D{x,y};
}

void CreatePlatformRow(int tiles, int x, int y)
{
	Platform platform;
	for (int display_x = 0; display_x < DISPLAY_WIDTH / 32; ++display_x)
	{
		int display_fraction = display_x * DISPLAY_WIDTH / 40;
		gamestate.vPlatforms.push_back(platform);
		gamestate.vPlatforms.back().pos = Point2D{ display_fraction,DISPLAY_HEIGHT - 32 };
	}
	gamestate.vPlatforms.push_back(platform);
	gamestate.vPlatforms.back().pos = Point2D{ x,y };
}

void CreatePlatformFloor()
{
	Platform platform;

	// Create floor
	for (int display_x = 0; display_x < DISPLAY_WIDTH / 32; ++display_x)
	{
		int display_fraction = display_x * DISPLAY_WIDTH / 40;
		gamestate.vPlatforms.push_back(platform);
		gamestate.vPlatforms.back().pos = Point2D{ display_fraction,DISPLAY_HEIGHT - 32 };
	}
}

void Draw()
{
	Play::DrawBackground();
	DrawAllGameObjectsByTypeRotated(TYPE_PLAYER);
	DrawObjectAABB(Play::GetGameObjectByType(TYPE_PLAYER).pos, playerinfo.AABB);
	Play::DrawSprite("middle", { DISPLAY_WIDTH / 2,DISPLAY_HEIGHT / 2 }, 0);
	DrawPlatforms();
	Play::PresentDrawingBuffer();
}

void DrawPlatforms()
{

	for (Platform& p : gamestate.vPlatforms)
	{
		Play::DrawSprite(Play::GetSpriteId("tile"),p.pos,0);
		DrawObjectAABB(p.pos, {32,32});
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

void DrawObjectAABB(Point2D objcentre, Vector2D objAABB)
{
	Point2D topLeft = objcentre - objAABB;
	Point2D bottomRight = objcentre + objAABB;
	Play::DrawRect(topLeft, bottomRight, Play::cGreen);
}
