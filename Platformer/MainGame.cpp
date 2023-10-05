#include "MainGame.h"

enum GameObjectType
{
	TYPE_PLAYER,
	TYPE_PLATFORM,
	TYPE_BACKGROUND,
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
	float animationspeedjump{ 0.2f };
	float animationspeedfall{ 0.2f };
	float animationspeedland { 0.2f };
	float animationspeedatk{ 0.2f };

	float friction;
	float slidingfriction{0.8f};
	float runningandjumpingfriction{0.8f};


	float runspeed{ 4.5f };
	float slidespeedCounter{ 1.2f };
	float slidetimer{ 2.0f };
	const float slidespeed{ 1.2f };
	float jumpspeed{ -10.0f };
	float fallspeed{ 3.5f };
	const float terminalvelocity{ 6.0f };

	float scale{ 2.0f };
	float gravity{ 0.3f};

	bool playerleftofplatform;
	bool headboxleftofplatform;
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

struct Platform
{
	int type = TYPE_PLATFORM;
	Point2D pos;
	Vector2D AABB{ 32,32 };
	
};

struct PlatformInfo
{
	Point2D CeilingCollidedPos;
	int levelheight;
	int levelwidth;
};

struct Background
{
	int type = TYPE_BACKGROUND;
	Point2D pos;
};


struct GameState
{
	float elapsedTime = 0;
	PlayerState playerstate = STATE_JUMPINGDOWN;
	std::vector<Platform> vPlatforms;
};

VariableJump variablejump;
CoyoteJump coyotejump;
JumpBuffer jumpbuffer;
Background background;
PlayerInfo playerinfo;
Platform platform;
PlatformInfo platforminfo;
GameState gamestate;


void UpdatePlayer();
void HandleGroundedControls();

void HandleSlidingControls();

void HandleAirBorneControls();

void HandleFallingControls();
void HandleGroundedAttackControls();
void HandleAirAttackControls();


void CreatePlatform(int x, int y);
void CreatePlatformRow(int tiles, int x, int y);
void CreatePlatformColumn(int tiles, int x, int y);
void CreatePlatformFloor();

bool IsGrounded();
bool FloorCollisionStarted();
bool CeilingCollisionStarted();
bool IsUnderCeiling();
bool WillCollideWithWall();
bool IsInsideWall();
void CheckPlayerIsLeftOfPLatform(Platform& platform);
void CheckHeadboxIsLeftOfPlatform(Platform& platform);


void CameraFollow();

void Draw();
void DrawPlatforms();
void DrawPlatformsAABB();
void DrawAllGameObjectsByTypeRotated(GameObjectType type);
void DrawAllGameObjectsByType(GameObjectType type);
void DrawObjectAABB(Point2D objcentre, Vector2D objAABB);
void DrawPlayerAABB();
void DrawPlayerNextPositionAABB();
void DrawDebug();



// The entry point for a PlayBuffer program
void MainGameEntry(PLAY_IGNORE_COMMAND_LINE)
{
	Play::CreateManager(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE);
	Play::CentreAllSpriteOrigins();
	Play::LoadBackground("Data\\Backgrounds\\background.png");
	Play::CreateGameObject(TYPE_PLAYER, { DISPLAY_WIDTH / 2,DISPLAY_HEIGHT / 2 }, 0, "idle_right");
	
	int platforminfo.levelwidth = Play::GetSpriteWidth("platformer");
	int platforminfo.levelheight = Play::GetSpriteHeight("platformer");

	CreatePlatformRow(18, levelwidth/platform.AABB, DISPLAY_HEIGHT / 53 * 30); // Floor
	CreatePlatformColumn(3, DISPLAY_WIDTH / 40 * 10, DISPLAY_HEIGHT / 23 * 20); // Wall
	CreatePlatformRow(5, DISPLAY_WIDTH / 40 * 20, DISPLAY_HEIGHT / 23 * 19); // Tunnel	
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

void UpdatePlayer()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);

	Platform platform;

	obj_player.scale = playerinfo.scale;

	obj_player.velocity.x *= playerinfo.friction; // Friction

	obj_player.velocity.y = std::clamp(obj_player.velocity.y, -20.0f, playerinfo.terminalvelocity);// Terminal velocity

	float timer = gamestate.elapsedTime;


	if (gamestate.playerstate == STATE_SLIDING)
	{
		playerinfo.wallcollisionAABB = playerinfo.slidingAABB;
	}
	else
	{
		playerinfo.wallcollisionAABB = playerinfo.standingAABB;
	}

	if (Play::KeyDown('W')) // When you hold down jump, the counter goes down
	{
		jumpbuffer.jumpbufferTimeCounter = jumpbuffer.jumpbufferTime;
	}
	else
	{
		jumpbuffer.jumpbufferTimeCounter -= timer;
	}


	// Wall interactions
	if (WillCollideWithWall())
	{
		obj_player.pos.x = obj_player.oldPos.x;
		obj_player.velocity.x = 0;

		if (IsInsideWall() && playerinfo.playerleftofplatform)
		{
			obj_player.pos.x -= 1;
		}
		else if (IsInsideWall() && playerinfo.playerleftofplatform == false)
		{
			obj_player.pos.x += 1;
		}
	}

	// Ceiling interactions
	if (CeilingCollisionStarted())
	{
		obj_player.pos.y = obj_player.oldPos.y;
		obj_player.velocity.y *= 0.9f;
	}

	switch (gamestate.playerstate)
	{
	case STATE_IDLE:

		HandleGroundedControls();


		playerinfo.friction = playerinfo.runningandjumpingfriction;

		// Idle animation
		if (playerinfo.facingright)
		{
			Play::SetSprite(obj_player, "idle_right", playerinfo.animationspeedidle); //Idle
		}
		else if (!playerinfo.facingright)
		{
			Play::SetSprite(obj_player, "idle_left", playerinfo.animationspeedidle); //Idle
		}

		if (IsGrounded() == false)
		{
			gamestate.playerstate = STATE_FALLING;
		}

		break;

	case STATE_RUNNING:

		HandleGroundedControls();



		playerinfo.friction = playerinfo.runningandjumpingfriction;

		if (!playerinfo.facingright)
		{
			Play::SetSprite(obj_player, "run_left", playerinfo.animationspeedrun);

		}
		else if (playerinfo.facingright)
		{
			Play::SetSprite(obj_player, "run_right", playerinfo.animationspeedrun);
		}

		if (Play::KeyPressed('S'))
		{
			gamestate.playerstate = STATE_SLIDING;
			Play::PlayAudio("slide");
		}

		if (IsGrounded() == false)
		{
			gamestate.playerstate = STATE_FALLING;
		}

		break;

	case STATE_SLIDING:

		HandleSlidingControls();

		//playerinfo.friction = playerinfo.slidingfriction;

		playerinfo.slidespeedCounter -= gamestate.elapsedTime;


		if (playerinfo.facingright == false && obj_player.velocity.x < 0)
		{

			obj_player.velocity.x -= playerinfo.slidespeedCounter;
			Play::SetSprite(obj_player, "slide_left", playerinfo.animationspeedrun);

		}
		else if (playerinfo.facingright == true && obj_player.velocity.x > 0)
		{

			obj_player.velocity.x += playerinfo.slidespeedCounter;
			Play::SetSprite(obj_player, "slide_right", playerinfo.animationspeedrun);

		}

		if (playerinfo.slidespeedCounter < 0 && IsUnderCeiling())
		{
			if (playerinfo.facingright == false)
			{
				obj_player.pos.x -= playerinfo.slidespeed;
			}
			else if (playerinfo.facingright)
			{
				obj_player.pos.x += playerinfo.slidespeed;
			}
		}
		else if (playerinfo.slidespeedCounter < 0 && IsUnderCeiling() == false)
		{
			gamestate.playerstate = STATE_IDLE;
			playerinfo.slidespeedCounter = playerinfo.slidespeed;
		}


		if (IsGrounded() == false)
		{
			gamestate.playerstate = STATE_FALLING;
			playerinfo.slidespeedCounter = playerinfo.slidespeed;
		}


		
		break;

	case STATE_JUMPING:

		HandleAirBorneControls();



		playerinfo.friction = playerinfo.runningandjumpingfriction;

		obj_player.acceleration.y = playerinfo.gravity;

		if (!playerinfo.facingright)
		{
			Play::SetSprite(obj_player, "jump_left", playerinfo.animationspeedjump);
		}
		else if (playerinfo.facingright)
		{
			Play::SetSprite(obj_player, "jump_right", playerinfo.animationspeedjump);
		}

		if (Play::IsAnimationComplete(obj_player)) // Change when velocity.y = 0
		{
			gamestate.playerstate = STATE_JUMPINGDOWN;
		}

		if (FloorCollisionStarted())
		{
			Play::PlayAudio("Landing");
			obj_player.pos.y = obj_player.oldPos.y;
			gamestate.playerstate = STATE_LANDING;
		}

		break;

	case STATE_JUMPINGDOWN:

		HandleAirBorneControls();



		playerinfo.friction = playerinfo.runningandjumpingfriction;

		obj_player.acceleration.y = playerinfo.gravity;

		if (!playerinfo.facingright)
		{
			Play::SetSprite(obj_player, "fall_left", playerinfo.animationspeedfall);
		}
		else if (playerinfo.facingright)
		{
			Play::SetSprite(obj_player, "fall_right", playerinfo.animationspeedfall);
		}

		if (FloorCollisionStarted())
		{
			Play::PlayAudio("Landing");
			obj_player.pos.y = obj_player.oldPos.y;
			gamestate.playerstate = STATE_LANDING;
		}

		break;


	case STATE_FALLING:

		HandleFallingControls();



		playerinfo.friction = playerinfo.runningandjumpingfriction;

		obj_player.acceleration.y = playerinfo.gravity;



		if (!playerinfo.facingright)
		{
			Play::SetSprite(obj_player, "fall_left", playerinfo.animationspeedfall);
		}
		else if (playerinfo.facingright)
		{
			Play::SetSprite(obj_player, "fall_right", playerinfo.animationspeedfall);
		}

		if (FloorCollisionStarted())
		{
			Play::PlayAudio("Landing");
			obj_player.pos.y = obj_player.oldPos.y;
			gamestate.playerstate = STATE_LANDING;
		}

		break;

	case STATE_LANDING:


		obj_player.velocity.y = 0;
		obj_player.acceleration.y = 0;

		playerinfo.friction = playerinfo.runningandjumpingfriction;

		coyotejump.coyoteTimeCounter = coyotejump.coyoteTime; // Reset coyotetimecounter when landing

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
			gamestate.playerstate = STATE_IDLE;
		}

		if (jumpbuffer.jumpbufferTimeCounter > 0.0f) // If the W button is being held, the jumpbuffertimecounter is set to 0.2f so you jump when you hold your jump.
		{
			obj_player.velocity.y = playerinfo.jumpspeed;
			gamestate.playerstate = STATE_JUMPING;
			Play::PlayAudio("jump");

		}
		break;

	case STATE_ATTACK:

		HandleGroundedAttackControls();



		playerinfo.friction = playerinfo.runningandjumpingfriction;

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
			gamestate.playerstate = STATE_IDLE;
		}

		break;

	}

	Play::UpdateGameObject(obj_player);
}

// Controls when player is in a state where their grounding box is on the top of a platform 
void HandleGroundedControls()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);
	
	if (Play::KeyDown('A') == false && Play::KeyDown('D') == false)
	{
		gamestate.playerstate = STATE_IDLE;
	}

	if (Play::KeyDown('A'))
	{
		playerinfo.facingright = false;

		gamestate.playerstate = STATE_RUNNING;

		obj_player.velocity.x = -playerinfo.runspeed;

	}
	else if (Play::KeyDown('D'))
	{
		playerinfo.facingright = true;

		gamestate.playerstate = STATE_RUNNING;

		obj_player.velocity.x = playerinfo.runspeed;
	}


	if (Play::KeyPressed('L'))
	{
		gamestate.playerstate = STATE_ATTACK;
	}

	// Jump
	if (Play::KeyPressed('W'))
	{
		obj_player.velocity.y = playerinfo.jumpspeed;

		gamestate.playerstate = STATE_JUMPING;

		Play::PlayAudio("jump");
	}
}

void HandleSlidingControls()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);

	

	// Slide Attack
	if (Play::KeyPressed('L') && IsUnderCeiling() == false)
	{
		gamestate.playerstate = STATE_ATTACK;
		playerinfo.slidespeedCounter = playerinfo.slidespeed;
	}

	// Jump
	if (Play::KeyPressed('W') && IsUnderCeiling() == false)
	{
		obj_player.velocity.y = playerinfo.jumpspeed;
		gamestate.playerstate = STATE_JUMPING;
		playerinfo.slidespeedCounter = playerinfo.slidespeed;
		Play::PlayAudio("jump");
	}
}

void HandleAirBorneControls()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);

	if (Play::KeyDown('A'))
	{
		playerinfo.facingright = false;

		obj_player.velocity.x = -playerinfo.fallspeed;

	}
	else if (Play::KeyDown('D'))
	{
		playerinfo.facingright = true;

		obj_player.velocity.x = playerinfo.fallspeed;
	}
}

void HandleLandingControls()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);

	float timer = 0;
	timer += gamestate.elapsedTime;

	if (Play::KeyDown('W')) // Holding W down countsdown your jumpbuffer time
	{
		jumpbuffer.jumpbufferTimeCounter = jumpbuffer.jumpbufferTime;
	}
	else
	{
		jumpbuffer.jumpbufferTimeCounter -= timer;
	}


	//// If there's still jumpbuffertime left, you jump
	//if (jumpbuffer.jumpbufferTimeCounter > 0.0f)
	//{
	//	obj_player.velocity.y = playerinfo.jumpspeed;
	//	
	//	gamestate.playerstate = STATE_JUMPING;
	//	Play::PlayAudio("jump");
	//}
}


// Controls when player is in a state where their grounding box is on the top of a platform
void HandleFallingControls()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);

	float timer = 0.0f;
	timer += gamestate.elapsedTime;

	if (Play::KeyDown('A'))
	{
		playerinfo.facingright = false;
		
		obj_player.velocity.x = -playerinfo.fallspeed;
	}
	else if (Play::KeyDown('D'))
	{
		playerinfo.facingright = true;

		obj_player.velocity.x = playerinfo.fallspeed;	
	}

	if (Play::KeyDown('W')) // Holding W down countsdown your jumpbuffer time
	{
		jumpbuffer.jumpbufferTimeCounter = jumpbuffer.jumpbufferTime;
	}
	else
	{
		jumpbuffer.jumpbufferTimeCounter -= timer;
	}

	
	coyotejump.coyoteTimeCounter -= timer;

	// If there's still coyotetimecounter left AND
	// If there's still jumpbuffertime left, you jump
	if (coyotejump.coyoteTimeCounter > 0.0f && jumpbuffer.jumpbufferTimeCounter > 0.0f)
	{
		obj_player.velocity.y = playerinfo.jumpspeed;
		jumpbuffer.jumpbufferTimeCounter = 0;
		gamestate.playerstate = STATE_JUMPING;
		Play::PlayAudio("jump");
	}

}


void HandleGroundedAttackControls()
{
	// If attack is pressed, play attack 2
	// Set a timer so if attack is pressed within 5? 6? frames? research best timings
	// Play attack 3
}

void HandleAirAttackControls()
{
	// If attack is pressed, play smash down
	// Make player drop down quick y value increase
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

void CreatePlatformColumn(int tiles, int x, int y)
{
	Platform platform;
	for (int i = 0; i < tiles; i++)
	{
		int tilespacing = 64 * i;
		gamestate.vPlatforms.push_back(platform);
		gamestate.vPlatforms.back().pos = Point2D{ x , y - tilespacing };
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


// Checks player's AABBmaxY and if it's collided with a platform's minY
bool FloorCollisionStarted()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);

	Point2D playerTopLeft = obj_player.pos - playerinfo.verticalcollisionAABB;
	Point2D playerBottomRight = obj_player.pos + playerinfo.verticalcollisionAABB;

	Point2D playerOldTopLeft = obj_player.oldPos - playerinfo.verticalcollisionAABB;
	Point2D playerOldBottomRight = obj_player.oldPos + playerinfo.verticalcollisionAABB;

	// Iterate through all platforms to check for collisions
	for (const Platform& platform : gamestate.vPlatforms)
	{
		// Calculate the platform's AABB
		Point2D platformTopLeft = platform.pos - platform.AABB;
		Point2D platformBottomRight = platform.pos + platform.AABB;

		// Check for collision between player's grounding box and the platform
		if (playerBottomRight.x > platformTopLeft.x &&
			playerTopLeft.x  < platformBottomRight.x &&
			playerBottomRight.y > platformTopLeft.y &&
			playerTopLeft.y < platformBottomRight.y)
		{


			// Checks if previous frame playermaxY was above the platformminY
			if (playerOldBottomRight.y < platformTopLeft.y)
			{
				return true; // Player is grounded
			}
		}
		
	}

	return false; // Player is not grounded
}

// Checks if player is under ceiling
bool IsUnderCeiling()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);
	Point2D headBoxPos = obj_player.pos - playerinfo.headboxoffset;
		
	Vector2D headboxAABB = playerinfo.headboxAABB;

	// Iterate through all platforms to check for collisions
	for (Platform& platform : gamestate.vPlatforms)
	{
		// Calculate the platform's AABB
		Point2D platformTopLeft = platform.pos - platform.AABB;
		Point2D platformBottomRight = platform.pos + platform.AABB;

		// Check for collision between player's edge box and the platform side
		if (headBoxPos.x + headboxAABB.x > platformTopLeft.x &&
			headBoxPos.x - headboxAABB.x  < platformBottomRight.x &&
			headBoxPos.y + headboxAABB.y  > platformTopLeft.y &&
			headBoxPos.y - headboxAABB.y < platformBottomRight.y)
		{
			CheckHeadboxIsLeftOfPlatform(platform);
			return true; // Player is colliding with platform side
		}
			
	}

	return false; // Player is not colliding with platform side
}

bool CeilingCollisionStarted()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);
	PlatformInfo platforminfo;

	Point2D playerTopLeft = obj_player.pos - playerinfo.verticalcollisionAABB;
	Point2D playerBottomRight = obj_player.pos + playerinfo.verticalcollisionAABB;

	Point2D playerOldTopLeft = obj_player.oldPos - playerinfo.verticalcollisionAABB;
	Point2D playerOldBottomRight = obj_player.oldPos + playerinfo.verticalcollisionAABB;

	// Iterate through all platforms to check for collisions
	for (const Platform& platform : gamestate.vPlatforms)
	{
		// Calculate the platform's AABB
		Point2D platformTopLeft = platform.pos - platform.AABB;
		Point2D platformBottomRight = platform.pos + platform.AABB;

		// Check for collision between player's collision box and the platform
		if (playerBottomRight.x > platformTopLeft.x &&
			playerTopLeft.x  < platformBottomRight.x &&
			playerBottomRight.y > platformTopLeft.y &&
			playerTopLeft.y < platformBottomRight.y)
		{


			// Checks if previous frame was below the platform
			if (playerOldTopLeft.y > platformBottomRight.y)
			{
				
				return true; // Player is hitting head
			}
		}

	}

	return false; // Player is not hitting head
}


bool IsGrounded()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);

	Point2D playerTopLeft = obj_player.pos - playerinfo.verticalcollisionAABB;
	Point2D playerBottomRight = obj_player.pos + playerinfo.verticalcollisionAABB;

	// Iterate through all platforms to check for collisions
	for (const Platform& platform : gamestate.vPlatforms)
	{
		// Calculate the platform's AABB
		Point2D platformTopLeft = platform.pos - platform.AABB;
		Point2D platformBottomRight = platform.pos + platform.AABB;

		// Check for collision between player's grounding box and the platform
		if (playerBottomRight.x > platformTopLeft.x &&
			playerTopLeft.x  < platformBottomRight.x &&
			playerBottomRight.y > platformTopLeft.y &&
			playerTopLeft.y < platformBottomRight.y)
		{

			return true; // Player is grounded

		}

	}

	return false; // Player is not grounded
}

// Check's player's edgebox and if it's going to collide with the sides of a platform
bool WillCollideWithWall()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);

	Point2D playerTopLeft = obj_player.pos - playerinfo.wallcollisionAABB;
	Point2D playerBottomRight = obj_player.pos + playerinfo.wallcollisionAABB;

	Vector2D playernextPosition = obj_player.pos + obj_player.velocity;

	Point2D playernextposTopLeft = playernextPosition - playerinfo.wallcollisionAABB;
	Point2D playernextposBottomRight = playernextPosition + playerinfo.wallcollisionAABB;

	// Iterate through all platforms to check for collisions
	for (Platform& platform : gamestate.vPlatforms)
	{
		// Calculate the platform's AABB
		Point2D platformTopLeft = platform.pos - platform.AABB;
		Point2D platformBottomRight = platform.pos + platform.AABB;

		// Check for collision between player's edge box and the platform
		if (playernextposBottomRight.x > platformTopLeft.x &&
			playernextposTopLeft.x  < platformBottomRight.x &&
			playernextposBottomRight.y  > platformTopLeft.y &&
			playernextposTopLeft.y < platformBottomRight.y)
		{
		
			return true; // Player is colliding with platform side
			
		}
		
	}

	return false; // Player is not colliding with platform side
}

bool IsInsideWall()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);

	Point2D playerTopLeft = obj_player.pos - playerinfo.wallcollisionAABB;
	Point2D playerBottomRight = obj_player.pos + playerinfo.wallcollisionAABB;

	// Iterate through all platforms to check for collisions
	for (Platform& platform : gamestate.vPlatforms)
	{
		// Calculate the platform's AABB
		Point2D platformTopLeft = platform.pos - platform.AABB;
		Point2D platformBottomRight = platform.pos + platform.AABB;

		// Check for collision between player's edge box and the platform
		if (playerBottomRight.x > platformTopLeft.x &&
			playerTopLeft.x  < platformBottomRight.x &&
			playerBottomRight.y  > platformTopLeft.y &&
			playerTopLeft.y < platformBottomRight.y)
		{
			CheckPlayerIsLeftOfPLatform(platform);
			return true; // Player is inside platform
		}

	}

	return false; // Player is not inside platform
}

void CheckPlayerIsLeftOfPLatform(Platform& platform)
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);

	if (obj_player.pos.x < platform.pos.x)
	{
		playerinfo.playerleftofplatform = true;
	}
	else if (obj_player.pos.x > platform.pos.x)
	{
		playerinfo.playerleftofplatform = false;

	}
}

void CheckHeadboxIsLeftOfPlatform(Platform& platform)
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);

	if (obj_player.pos.x < platform.pos.x)
	{
		playerinfo.headboxleftofplatform = true;
	}
	else if (obj_player.pos.x > platform.pos.x)
	{
		playerinfo.headboxleftofplatform = false;

	}
}


void CameraFollow()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);

	
	Play::SetCameraPosition({ obj_player.pos.x-DISPLAY_WIDTH/2, obj_player.pos.y-DISPLAY_HEIGHT/2 });
	
}

void Draw()
{
	Play::DrawBackground();
	Play::DrawSprite("platformer", { DISPLAY_WIDTH / 2,DISPLAY_HEIGHT / 2 }, 0);
	DrawPlatforms();

	DrawAllGameObjectsByTypeRotated(TYPE_PLAYER);

	DrawDebug();
	
	Play::PresentDrawingBuffer();
}

// Draws all platforms in vPlatforms
void DrawPlatforms()
{
	Platform platform;

	for (Platform& p : gamestate.vPlatforms)
	{
		Play::DrawSprite(Play::GetSpriteId("tile"),p.pos,0);
	}
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

	Play::DrawFontText("font64px", "y velocity: " + std::to_string(Play::GetGameObjectByType(TYPE_PLAYER).velocity.y), { DISPLAY_WIDTH / 2,DISPLAY_HEIGHT / 6 }, Play::CENTRE);
}