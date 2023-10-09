#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"

const int DISPLAY_WIDTH = 1280;
const int DISPLAY_HEIGHT = 720;
const int DISPLAY_SCALE = 1;

enum GameObjectType
{
	TYPE_PLAYER,
	TYPE_PLATFORM,
	TYPE_SLIME,
	TYPE_AXE,
};

enum PlayerState
{
	STATE_IDLE = 0,
	STATE_RUNNING,
	STATE_SLIDING,
	STATE_JUMPING,
	STATE_JUMPINGDOWN,
	STATE_FALLING,
	STATE_LANDING,
	STATE_ATTACK,
};

enum SlimeState
{
	STATE_SLIME_IDLE = 0,
	STATE_SLIME_HURT,
	STATE_SLIME_DEAD,
};

enum SPLAT_TYPE
{
	SLIME_SPLAT = 0,
};


struct PlayerInfo
{
	Vector2D verticalcollisionAABB{ 15,30 };
	Vector2D wallcollisionAABB{ 15,20 };
	Vector2D slidingAABB{ 15,0 };
	Vector2D standingAABB{ 15,20 };
	Vector2D damageAABB{ 5,10 };

	Vector2D headboxAABB{ 15,1 };
	Vector2D headboxoffset{ 0,25 };


	Vector2D PlatformToPlayerDistance;

	bool facingright;
	float animationspeedidle{ 0.2f };
	float animationspeedrun{ 0.2f };
	float animationspeedslide{ 0.2f };
	float animationspeedjump{ 0.2f };
	float animationspeedfall{ 0.2f };
	float animationspeedland{ 0.2f };
	float animationspeedatk{ 0.2f };

	float friction;
	float slidingfriction{ 1.0f };
	float runningandjumpingfriction{ 0.8f };
	float fallingfriction{ 0.97f };


	float runspeed{ 4.5f };
	float slidetimerCounter{ 0.5f };
	float slidetimer{ 0.3f };
	
	float jumpspeed{ -10.0f };
	float slidespeed{ 8.0f };
	float fallspeed{ 3.5f };
	const float terminalvelocity{ 8.0f };

	float scale{ 2.0f };
	float gravity{ 0.3f };

	bool playerleftofplatform;
	bool headboxleftofplatform;
	bool hasaxe = false;

	Vector2D axeattackoffset{ 0,7};
	Vector2D runoffset{ 0,4 };
	Vector2D slideoffset{ 0,4 };

	Point2D axehitboxoffset{10,0};
	const Point2D constaxehitboxoffset{10,0};
	Vector2D axehitboxAABB{7,7};
};



struct VariableJump
{
	float jumpStartTime;
	float jumpTime;
	bool isJumping;
};

struct CoyoteJump
{
	const float coyoteTime = 0.2f;
	float coyoteTimeCounter;
};

struct JumpBuffer
{
	const float jumpbufferTime = 0.2f;
	float jumpbufferTimeCounter;
};

struct SlideBuffer
{
	const float slidebufferTime = 0.2f;
	float slidebufferTimeCounter;
};

struct Platform
{
	int type = TYPE_PLATFORM;
	int id;
	Point2D pos;
	Vector2D AABB{ 32,32 };

};

struct Slime
{
	int type = TYPE_SLIME;
	Vector2D AABB{ 20,11 };
	Point2D pos;
	float runspeed = 1.0f;
	float animationspeed{ 0.2f };
	float sightrangehorizontal = 200.0f;
	float sightrangevertical = 100.0f;
};


struct PlatformInfo
{
	Point2D CeilingCollidedPos;
	int levelheight;
	int levelwidth;
};

struct LevelLayoutInfo
{
	int width = 40;
	int height = 22;
	int levellayout[880] = {
		
		5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2,
		2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 2,
		2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0, 2,
		2, 0, 0, 0, 5, 0, 0, 0, 0, 0, 1, 1, 2, 0, 0, 0, 2, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 2,
		2, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 1, 1, 2,
		2, 3, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 5, 0, 0, 0, 0, 0, 2, 0, 0, 0, 2,
		2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 2, 0, 0, 0, 2,
		2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 1, 0, 2,
		2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 2, 0, 0, 0, 2,
		2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 2,
		2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 0, 0, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 0, 0, 0, 2, 0, 0, 1, 2,
		2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 2,
		2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 1, 0, 2,
		2, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 2,
		2, 0, 0, 0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 2, 2, 2, 2, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 2,
		2, 0, 0, 0, 0, 1, 2, 2, 1, 0, 0, 0, 0, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 2,
		2, 0, 0, 1, 1, 2, 2, 2, 2, 1, 1, 1, 1, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 0, 0, 0, 2,
		2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 2, 2, 0, 5, 0, 0, 0, 2, 2, 2, 0, 0, 0, 2, 0, 0, 0, 0, 2, 2, 2, 1, 0, 1, 2,
		2, 1, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2,
		2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 2,
		2, 2, 2, 1, 1, 1, 1, 1, 1, 2, 1, 1, 2, 0, 2, 1, 1, 1, 2, 0, 0, 2, 1, 1, 1, 2, 0, 2, 1, 2, 0, 2, 1, 1, 1, 1, 1, 1, 1, 2,
	};
};




struct GameState
{
	float elapsedTime = 0;
	PlayerState playerstate = STATE_JUMPINGDOWN;
	std::vector<Platform> vPlatforms;
};

Slime slime;

VariableJump variablejump;
CoyoteJump coyotejump;
JumpBuffer jumpbuffer;
SlideBuffer slidebuffer;
PlayerInfo playerinfo;
Platform platform;
PlatformInfo platforminfo;
GameState gamestate;


void UpdatePlayer();
void HandleGroundedControls();

void HandleSlidingControls();

void HandleAirBorneControls();

void HandleFallingControls();
void HandleLandingControls();

void UpdateSlimes();
void UpdateItemAxe();


void CreatePlatform(int x, int y);
void CreatePlatformRow(int tiles, int x, int y);
void CreatePlatformColumn(int tiles, int x, int y);
void CreatePlatformFloor();
void CreateLevelFromArray();

bool IsGrounded();
bool FloorCollisionStarted();
bool CeilingCollisionStarted();
bool IsUnderCeiling();
bool WillCollideWithWall(int obj_type, Vector2D obj_AABB);
bool IsInsideWall();
void CheckPlayerIsLeftOfPLatform(Platform& platform);
void CheckHeadboxIsLeftOfPlatform(Platform& platform);
bool IsCollidingAABB(Point2D obj_a_pos, Vector2D obj_a_dimensions, Point2D obj_b_pos, Vector2D obj_b_dimensions);


void CameraFollow();

void Draw();
void DrawGrassPlatforms();
void DrawRockPlatforms();
void DrawPlatforms();
void DrawPlatformsAABB();
void DrawAllGameObjectsByTypeRotated(GameObjectType type);
void DrawAllGameObjectsByType(GameObjectType type);
void DrawObjectAABB(Point2D objcentre, Vector2D objAABB);
void DrawPlayerAABB();
void DrawPlayerNextPositionAABB();
void DrawAllObjectAABB(GameObjectType type, Vector2D obj_dimensions);
void DrawDebug();

