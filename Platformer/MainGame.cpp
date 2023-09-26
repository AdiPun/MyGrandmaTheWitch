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
	Vector2D AABB{ 100,120 };
	float animationspeedidle{ 0.2f };
};

struct GameState
{
	PlayerState playerstate = STATE_GROUND;
};

PlayerInfo playerinfo;
GameState gamestate;

void UpdatePlayer();
void Draw();
void DrawAllGameObjectsByTypeRotated(GameObjectType type);
void DrawAllGameObjectsByType(GameObjectType type);
void DrawObjectAABB(Point2D objcentre, Vector2D objAABB);


// The entry point for a PlayBuffer program
void MainGameEntry(PLAY_IGNORE_COMMAND_LINE)
{
	Play::CreateManager(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE);
	Play::LoadBackground("Data\\Backgrounds\\background.png");
	Play::CreateGameObject(TYPE_PLAYER, { DISPLAY_WIDTH / 2,DISPLAY_HEIGHT / 4 }, 0, "idle_right");
}

// Called by PlayBuffer every frame (60 times a second!)
bool MainGameUpdate(float elapsedTime)
{
	Draw();
	UpdatePlayer();
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
	obj_player.animSpeed = playerinfo.animationspeedidle;
	Play::UpdateGameObject(obj_player);
	DrawObjectAABB(obj_player.pos, playerinfo.AABB);
}

void Draw()
{
	Play::DrawBackground();
	DrawAllGameObjectsByType(TYPE_PLAYER);
	
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
	Play::DrawRect(topLeft, bottomRight, Play::cRed);
}
