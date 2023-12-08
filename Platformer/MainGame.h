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
	TYPE_DROPLET,
	TYPE_WITCH,
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

struct PlayerInfo
{
	Vector2D verticalcollisionAABB{ 15 , 30 };
	Vector2D wallcollisionAABB{ 15 , 20 };
	Vector2D slidingAABB{ 15 , 0 };
	Vector2D standingAABB{ 15 , 20 };

	Vector2D headboxAABB{ 15 , 1 };
	Vector2D headboxoffset{ 0 , 25 };

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

	Vector2D axeattackoffset{ 0 , 7 };
	Vector2D runoffset{ 0 , 4 };
	Vector2D slideoffset{ 0 , 4 };

	Point2D axehitboxoffset{ 40 , 0 };
	const Point2D constaxehitboxoffset{ 40 , 0 };
	Vector2D axehitboxAABB{ 30 , 30 };
	bool axeanimationcomplete = false;
};

struct PlayerInventory
{
	int slimeteardrops{ 0 };
};

struct WitchInfo
{
	float animationspeedidle{ 0.2f };
	float animationspeedtalking{ 0.02f };
	Vector2D idlespriteoffset{ 0 , 90 };
	Vector2D talkingspriteoffset{ 0 , 90 };
	Vector2D talkingrangeAABB{ 200 , 200 };
	Vector2D talkingrangeoffset {-150 , 0 };
	Vector2D speechbubbleoffset{ -200 , -275 };
	bool intalkingrange = false;
	int slimeteardropsneeded{ 40 };
	std::string dialogue1 = "Collect " + std::to_string(slimeteardropsneeded) + " slime tears for me.";
	std::string dialogue2 = "I spotted an axe North West of here";
	std::string dialogue3 = "Go fetch that first.";
	std::string dialoguehowtouseaxe = "Now Press L to smash these slimes up!";
	std::string dialogueslimescollected = "Excellent work dear.";
};

struct BannerInfo
{
	Vector2D AABB{ 360 , 110 };
	Vector2D nextlineoffset{ 0 , 48 };
};

struct SlimeInfo
{
	int type = TYPE_SLIME;
	Vector2D AABB{ 7,7 };
	Point2D pos;
	float runspeed = 1.0f;
	float animationspeed{ 0.2f };
	float sightrangehorizontal = 200.0f;
	float sightrangevertical = 100.0f;

};

struct DropletParticleInfo
{
	int max_particles{ 10 };
	Vector2D AABB{ 5,7 };
	Vector2D initialvelocity{ 3.0f , -5.0f };
	
	float gravity{ 0.1f };

	Vector2D minvelocity{ 0.1f,0.1f };
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
		2, 6, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 1, 1, 2,
		2, 3, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 5, 0, 0, 0, 0, 0, 2, 0, 0, 0, 2,
		2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 2, 0, 0, 0, 2,
		2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 1, 0, 2,
		2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 2, 0, 0, 0, 2,
		2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 2,
		2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 0, 0, 0, 2, 0, 0, 1, 2,
		2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 2,
		2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 1, 0, 2,
		2, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 2,
		2, 0, 0, 0, 0, 0, 1, 2, 0, 0, 0, 0, 0, 2, 2, 2, 2, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 2,
		2, 0, 0, 0, 0, 1, 2, 2, 1, 0, 0, 0, 0, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 2, 0, 0, 0, 2,
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
	Point2D centrepos = { DISPLAY_WIDTH,DISPLAY_HEIGHT};
	PlayerState playerstate = STATE_JUMPINGDOWN;
	std::vector<Platform> vPlatforms;
};


PlayerInfo playerinfo;
PlayerInventory inventory;
WitchInfo witchinfo;
SlimeInfo slimeinfo;
DropletParticleInfo dropletinfo;
BannerInfo bannerinfo;

VariableJump variablejump;
CoyoteJump coyotejump;
JumpBuffer jumpbuffer;
SlideBuffer slidebuffer;
Platform platform;
PlatformInfo platforminfo;
GameState gamestate;


void UpdatePlayer();
void HandleGroundedControls();

void HandleSlidingControls();

void HandleAirBorneControls();

void HandleFallingControls();
void HandleLandingControls();

void UpdateWitch();
void UpdateSlimes();
void UpdateItemAxe();

void CreateDroplet(Point2D pos);
void UpdateDroplets();

void CreateLevelFromArray();

void CreatePlatform(int x, int y);
void CreatePlatformRow(int tiles, int x, int y);
void CreatePlatformColumn(int tiles, int x, int y);

bool IsObjGrounded(GameObject& obj, Vector2D obj_AABB);
bool FloorCollisionStarted(GameObject& obj, Vector2D obj_AABB);
bool CeilingCollisionStarted(GameObject& obj, Vector2D obj_AABB);
bool IsPlayerUnderCeiling();
bool WillCollideWithWall(GameObject& obj, Vector2D obj_AABB);
bool IsObjInsideWall(GameObject& obj, Vector2D obj_AABB);
bool IsPlayerInsideWall();
bool IsCollidingAABB(Point2D obj_a_pos, Vector2D obj_a_dimensions, Point2D obj_b_pos, Vector2D obj_b_dimensions);

void CheckPlayerIsLeftOfPLatform(Platform& platform);
void CheckHeadboxIsLeftOfPlatform(Platform& platform);

void SetGameObjectRotationToDirection(GameObject& obj);

void CameraFollow();

void Draw();
void DrawUI();
void DrawDialogue();

void DrawPlatforms();
void DrawPlatformsAABB();
void DrawAllGameObjectsByTypeRotated(GameObjectType type);
void DrawAllGameObjectsByType(GameObjectType type);


void DrawDebug();
void DrawObjectAABB(Point2D objcentre, Vector2D objAABB);
void DrawPlayerAABB();
void DrawPlayerNextPositionAABB();
void DrawAllObjectAABB(GameObjectType type, Vector2D obj_dimensions);
