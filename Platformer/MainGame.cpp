#include "MainGame.h"

enum GameObjectType
{
	TYPE_PLAYER,
};

enum PlayerState
{
	STATE_GROUND = 0,
	STATE_AIRBORNE,
	STATE_LANDING,
	STATE_JUMPING,
};

struct PlayerInfo
{
	Vector2D AABB{ 15,20 };
	bool facingright = true;
	bool hasTurned = true;
	float animationspeedidle{ 0.15f };
	float animationspeedrun{ 0.3f };
	float animationspeedjump{ 0.2f };
	float runspeed{ 4.5f };
	float scale{ 2.5f };
	float gravity{ 0.0f};
};

struct GameState
{
	PlayerState playerstate = STATE_GROUND;
};

PlayerInfo playerinfo;
GameState gamestate;

void UpdatePlayer();
void HandlePlayerControls();
void HandleAirborneControls();

void Draw();
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
	obj_player.velocity.x *= 0.9f;
	obj_player.acceleration.y = playerinfo.gravity;
	switch (gamestate.playerstate)
	{
	case STATE_LANDING:
	{

	}
	case STATE_GROUND:
		HandlePlayerControls();
		break;
	case STATE_JUMPING:
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
		HandleAirborneControls();
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
		//Add sound effect!
	}

	if (Play::KeyPressed(VK_UP))
	{
		//obj_player.velocity.y -= 5;
		gamestate.playerstate = STATE_JUMPING;
	}
}

void HandleAirborneControls()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);
	// Running animation
	if (Play::KeyDown(VK_LEFT))
	{
		playerinfo.facingright = false;
		Play::SetSprite(obj_player, "test", playerinfo.animationspeedrun);
		obj_player.velocity.x = -playerinfo.runspeed;
	}
	else if (Play::KeyDown(VK_RIGHT))
	{
		playerinfo.facingright = true;
		Play::SetSprite(obj_player, "test", playerinfo.animationspeedrun);
		obj_player.velocity.x = playerinfo.runspeed;
	}
}

void Draw()
{
	Play::DrawBackground();
	DrawAllGameObjectsByTypeRotated(TYPE_PLAYER);
	DrawObjectAABB(Play::GetGameObjectByType(TYPE_PLAYER).pos, playerinfo.AABB);
	Play::PresentDrawingBuffer();
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
