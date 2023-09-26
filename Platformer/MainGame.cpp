#include "MainGame.h"

enum PlayerState
{
	STATE_GROUND = 0,
	STATE_AIRBORNE,
	Point2D playerAABB = {112,133},
};

struct GameState
{
	PlayerState playerstate = STATE_GROUND,
};

GameState gamestate;

void Draw();
void DrawAllGameObjectsOfTypeRotated(GameObjectType type);


// The entry point for a PlayBuffer program
void MainGameEntry(PLAY_IGNORE_COMMAND_LINE)
{
	Play::CreateManager(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE);
	Play::LoadBackground("Data\\Backgrounds\\background.png");
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
	Play::PresentDrawingBuffer();
}

void DrawAllGameObjectsOfTypeRotated(GameObjectType type)
{
	for (int id : Play::CollectGameObjectIDsByType(type))
	{
		GameObject& obj = Play::GetGameObject(id);
		Play::DrawObjectRotated(obj);
	}
}
