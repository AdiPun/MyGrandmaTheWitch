#include "MainGame.h"

enum GameObjectType
{
	TYPE_PLAYER,
};

enum PlayerState
{
	STATE_GROUND = 0,
	STATE_AIRBORNE,
};

struct PlayerInfo
{
	Vector2D AABB{ 4,8 };
	float animationspeedidle{ 0.15f };
	float animationspeedrun{ 0.2f };
	float animationspeedjump{ 0.02f };
};

struct GameState
{
	PlayerState playerstate = STATE_GROUND;
};

PlayerInfo playerinfo;
GameState gamestate;

void UpdatePlayer();
void HandlePlayerControls();
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
	Play::CreateGameObject(TYPE_PLAYER, { DISPLAY_WIDTH / 2,DISPLAY_HEIGHT / 4 }, 0, "idle_right");
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
	HandlePlayerControls();
	Play::UpdateGameObject(obj_player);
}

void HandlePlayerControls()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);
	if (Play::KeyDown(VK_RIGHT))
	{
		Play::SetSprite(obj_player, "run_right", playerinfo.animationspeedrun); //Run right
	}
	else if (Play::KeyDown(VK_LEFT))
	{
		Play::SetSprite(obj_player, "run_left", playerinfo.animationspeedrun); //Run left
	}
	else /*if (Play::IsAnimationComplete(obj_player))*/
	{
		Play::SetSprite(obj_player, "idle", playerinfo.animationspeedidle); //Idle
	}

	if (Play::KeyPressed(VK_SPACE))
	{
		//Add sound effect!
	}

	if (Play::KeyPressed(VK_UP))
	{
		obj_player.velocity.y = -9;
		Play::SetSprite(obj_player, "jump", playerinfo.animationspeedjump);
		//Add sound effect!
	}
}

void Draw()
{
	Play::DrawBackground();
	DrawAllGameObjectsByType(TYPE_PLAYER);
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
