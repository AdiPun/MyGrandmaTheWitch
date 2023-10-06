#include "MainGame.h"
#include "PlayerStates.h"
#include "PlatformFunctions.h"

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