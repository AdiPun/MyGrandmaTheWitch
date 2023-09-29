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
	STATE_JUMPING,
	STATE_FALLING,
	STATE_LANDING,
	STATE_ATTACK,
};

struct PlayerInfo
{
	Vector2D AABB{ 10,20 };
	Vector2D maxoffsety{ 0,40 };
	Vector2D maxoffsetx{ 20,0 };
	Vector2D groundingboxAABB{ 20,1 };
	Vector2D edgeboxAABB{ 1,30 };
	
	bool facingright = true;
	float animationspeedidle{ 0.2f };
	float animationspeedrun{ 0.2f };
	float animationspeedjump{ 0.2f };
	float animationspeedfall{ 0.2f };
	float animationspeedland { 0.15f };
	float animationspeedatk{ 0.2f };

	float friction{ 0.8f };

	float runspeed{ 4.5f };
	float jumpspeed{ -10.0f };
	float fallspeed{ 3.5f };

	float scale{ 2.5f };
	float gravity{ 0.3f};
};

struct CoyoteJump
{
	const float coyoteTime = 0.2f;
	float coyoteTimeCounter;
	bool coyoteJumped = false;
};

struct Platform
{
	int type = TYPE_PLATFORM;
	Point2D pos;
	const Vector2D AABB{ 32,24 };
};

struct Background
{
	int type = TYPE_BACKGROUND;
	Point2D pos;
};


struct GameState
{
	float time = 0;
	PlayerState playerstate = STATE_IDLE;
	std::vector<Platform> vPlatforms;
};

CoyoteJump coyotejump;
Background background;
PlayerInfo playerinfo;
GameState gamestate;

void UpdatePlayer();
void HandleGroundedControls();
void HandleJumpingControls();

void HandleFallingControls();
void HandleGroundedAttackControls();
void HandleAirAttackControls();


void CreatePlatform(int x, int y);
void CreatePlatformRow(int tiles, int x, int y);
void CreatePlatformFloor();

void CreateBackground();

bool IsGrounded();
bool FloorCollisionStarted();
bool IsCollidingWithWall();


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
	CreatePlatform(DISPLAY_WIDTH / 6*5, DISPLAY_HEIGHT / 6*5);
	CreatePlatform(DISPLAY_WIDTH*0.60f, DISPLAY_HEIGHT * 0.60f);
	CreatePlatformRow(5, DISPLAY_WIDTH / 2, DISPLAY_HEIGHT / 4 * 3);
}

// Called by PlayBuffer every frame (60 times a second!)
bool MainGameUpdate(float elapsedTime)
{
	gamestate.time += elapsedTime/60;
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

	Platform platform;

	obj_player.scale = playerinfo.scale;

	obj_player.velocity.x *= playerinfo.friction;



	// Wall interactions
	if (IsCollidingWithWall())
	{
		if (obj_player.velocity.x > 0)
		{
			// Moving right, adjust X position and stop horizontal movement
			obj_player.pos.x = obj_player.oldPos.x;
			obj_player.velocity.x = 0;
		}
		else if (obj_player.velocity.x < 0)
		{
			// Moving left, adjust X position and stop horizontal movement
			obj_player.pos.x = obj_player.oldPos.x;
			obj_player.velocity.x = 0;
		}
	}

	switch (gamestate.playerstate)
	{
	case STATE_IDLE:

		HandleGroundedControls();

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

		if (!playerinfo.facingright)
		{
			Play::SetSprite(obj_player, "run_left", playerinfo.animationspeedrun);

		}
		else if (playerinfo.facingright)
		{
			Play::SetSprite(obj_player, "run_right", playerinfo.animationspeedrun);
		}

		if (IsGrounded() == false)
		{
			gamestate.playerstate = STATE_FALLING;
		}

		break;

	case STATE_JUMPING:

		HandleJumpingControls();

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
			gamestate.playerstate = STATE_FALLING;
		}

		if (FloorCollisionStarted())
		{
			gamestate.playerstate = STATE_LANDING;
			obj_player.pos.y = obj_player.oldPos.y;
		}
		
		break;

	case STATE_FALLING:

		HandleFallingControls();

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
			gamestate.playerstate = STATE_LANDING;
			obj_player.pos.y = obj_player.oldPos.y;
		}
		
		break;

	case STATE_LANDING:

		//HandlePlayerControls();

		obj_player.velocity.y = 0;
		obj_player.acceleration.y = 0;

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
		break;	

	case STATE_ATTACK:

		HandleGroundedAttackControls();

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

	if (Play::KeyPressed('E'))
	{
		gamestate.playerstate = STATE_ATTACK;
	}

	// Jump
	if (Play::KeyPressed('W'))
	{
		obj_player.velocity.y = playerinfo.jumpspeed;
		gamestate.playerstate = STATE_JUMPING;
	}
}

void HandleJumpingControls()
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

// Controls when player is in a state where their grounding box is on the top of a platform
void HandleFallingControls()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);

	float time = gamestate.time;

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

	coyotejump.coyoteTimeCounter = coyotejump.coyoteTime;
	
	coyotejump.coyoteTimeCounter -= time;
	

	// If there's still coyotetimecounter left, you can jump
	if (coyotejump.coyoteTimeCounter > 0.0f && Play::KeyPressed('W'))
	{
		obj_player.velocity.y = playerinfo.jumpspeed;
		coyotejump.coyoteTimeCounter = 0;
		gamestate.playerstate = STATE_JUMPING;
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

void CreateBackground()
{
	Background background;

}

// Checks player's groundingbox and if it's colliding with a platform
bool FloorCollisionStarted()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);
	Point2D groundingBoxPos = obj_player.pos + playerinfo.maxoffsety;
	Vector2D groundingBoxAABB = playerinfo.groundingboxAABB;

	Point2D groundingBoxOldPos = obj_player.oldPos + playerinfo.maxoffsety;

	// Iterate through all platforms to check for collisions
	for (const Platform& platform : gamestate.vPlatforms)
	{
		// Calculate the platform's AABB
		Point2D platformTopLeft = platform.pos - platform.AABB;
		Point2D platformBottomRight = platform.pos + platform.AABB;

		// Check for collision between player's grounding box and the platform
		if (groundingBoxPos.x + groundingBoxAABB.x > platformTopLeft.x &&
			groundingBoxPos.x - groundingBoxAABB.x  < platformBottomRight.x &&
			groundingBoxPos.y + groundingBoxAABB.y > platformTopLeft.y &&
			groundingBoxPos.y - groundingBoxAABB.y < platformBottomRight.y)
		{


			// Checks if previous frame was above the platform
			if (groundingBoxOldPos.y + groundingBoxAABB.y < platformTopLeft.y)
			{
				return true; // Player is grounded
			}
		}
		
	}

	return false; // Player is not grounded
}

bool IsGrounded()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);
	Point2D groundingBoxPos = obj_player.pos + playerinfo.maxoffsety;
	Vector2D groundingBoxAABB = playerinfo.groundingboxAABB;

	Point2D groundingBoxOldPos = obj_player.oldPos + playerinfo.maxoffsety;

	// Iterate through all platforms to check for collisions
	for (const Platform& platform : gamestate.vPlatforms)
	{
		// Calculate the platform's AABB
		Point2D platformTopLeft = platform.pos - platform.AABB;
		Point2D platformBottomRight = platform.pos + platform.AABB;

		// Check for collision between player's grounding box and the platform
		if (groundingBoxPos.x + groundingBoxAABB.x > platformTopLeft.x &&
			groundingBoxPos.x - groundingBoxAABB.x  < platformBottomRight.x &&
			groundingBoxPos.y + groundingBoxAABB.y > platformTopLeft.y &&
			groundingBoxPos.y - groundingBoxAABB.y < platformBottomRight.y)
		{
			return true; // Player is grounded
		}

	}

	return false; // Player is not grounded
}

// Check's player's edgebox and if it's going to collide with the sides of a platform
bool IsCollidingWithWall()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);
	Point2D edgeBoxPosleft = obj_player.pos - playerinfo.maxoffsetx;
	Point2D edgeBoxPosright = obj_player.pos + playerinfo.maxoffsetx;
	Vector2D edgeBoxAABB = playerinfo.edgeboxAABB;
	Vector2D nextPosition = obj_player.pos + obj_player.velocity;

	// Iterate through all platforms to check for collisions
	for (Platform& platform : gamestate.vPlatforms)
	{
		// Calculate the platform's AABB
		Point2D platformTopLeft = platform.pos - platform.AABB;
		Point2D platformBottomRight = platform.pos + platform.AABB;

		// Check for collision between player's edge box and the platform side
		if (edgeBoxPosleft.x + edgeBoxAABB.x > platformTopLeft.x &&
			edgeBoxPosleft.x - edgeBoxAABB.x  < platformBottomRight.x &&
			edgeBoxPosleft.y + edgeBoxAABB.y  > platformTopLeft.y &&
			edgeBoxPosleft.y - edgeBoxAABB.y < platformBottomRight.y)
		{
			return true; // Player is colliding with platform side
		}
		if (edgeBoxPosright.x + edgeBoxAABB.x > platformTopLeft.x &&
			edgeBoxPosright.x - edgeBoxAABB.x  < platformBottomRight.x &&
			edgeBoxPosright.y + edgeBoxAABB.y  > platformTopLeft.y &&
			edgeBoxPosright.y - edgeBoxAABB.y < platformBottomRight.y)
		{
			return true; // Player is colliding with platform side
		}
	}

	return false; // Player is not colliding with platform side
}


void Draw()
{
	Play::DrawBackground();
	Play::DrawSprite("middle", { DISPLAY_WIDTH / 2,DISPLAY_HEIGHT / 2 }, 0);
	DrawPlatforms();
	DrawAllGameObjectsByTypeRotated(TYPE_PLAYER);

	DrawObjectAABB(Play::GetGameObjectByType(TYPE_PLAYER).pos, playerinfo.AABB);

	DrawObjectAABB(Play::GetGameObjectByType(TYPE_PLAYER).pos + playerinfo.maxoffsety, playerinfo.groundingboxAABB);

	DrawObjectAABB(Play::GetGameObjectByType(TYPE_PLAYER).pos + playerinfo.maxoffsetx, playerinfo.edgeboxAABB);

	DrawObjectAABB(Play::GetGameObjectByType(TYPE_PLAYER).pos - playerinfo.maxoffsetx, playerinfo.edgeboxAABB);

	Play::PresentDrawingBuffer();
}

// Draws all platforms in vPlatforms
void DrawPlatforms()
{
	Platform platform;

	for (Platform& p : gamestate.vPlatforms)
	{
		Play::DrawSprite(Play::GetSpriteId("tile"),p.pos,0);
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