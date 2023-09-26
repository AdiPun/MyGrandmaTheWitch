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

struct Player
{
	Point2D AABB{100,120}
};
struct GameState
{
	PlayerState playerstate = STATE_GROUND,
};

GameState gamestate;

void Draw();
void DrawAllGameObjectsByTypeRotated(GameObjectType type);
void DrawAllGameObjectsByType(GameObjectType type);


// The entry point for a PlayBuffer program
void MainGameEntry(PLAY_IGNORE_COMMAND_LINE)
{
	Play::CreateManager(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE);
	Play::LoadBackground("Data\\Backgrounds\\background.png");
	Play::CreateGameObject(TYPE_PLAYER, { DISPLAY_WIDTH / 2,DISPLAY_HEIGHT / 4 }, 0, "idle_right";
}

// Called by PlayBuffer every frame (60 times a second!)
bool MainGameUpdate(float elapsedTime)
{
	Draw();
}

// Gets called once when the player quits the game 
int MainGameExit(void)
{
	Play::DestroyManager();
	return PLAY_OK;
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
