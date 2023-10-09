#include "MainGame.h"

// The entry point for a PlayBuffer program
void MainGameEntry(PLAY_IGNORE_COMMAND_LINE)
{
	Play::CreateManager(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE);
	Play::CentreAllSpriteOrigins();
	Play::MoveSpriteOrigin("axe_left", 0, playerinfo.axeattackoffset.y);
	Play::MoveSpriteOrigin("axe_right", 0, playerinfo.axeattackoffset.y);
	Play::MoveSpriteOrigin("run_left", 0, playerinfo.runoffset.y);
	Play::MoveSpriteOrigin("run_right", 0, playerinfo.runoffset.y);
	Play::MoveSpriteOrigin("slide_left", 0, playerinfo.slideoffset.y);
	Play::MoveSpriteOrigin("slide_right", 0, playerinfo.slideoffset.y);
	Play::MoveSpriteOrigin("droplet", 0, slime.dropletcentreoffset.y);
	Play::LoadBackground("Data\\Backgrounds\\background.png");
	Play::CreateGameObject(TYPE_PLAYER, { DISPLAY_WIDTH,DISPLAY_HEIGHT}, 16, "idle_right");
	//Play::StartAudioLoop("music");
	CreateLevelFromArray();
}

// Called by PlayBuffer every frame (60 times a second!)
bool MainGameUpdate(float elapsedTime)
{
	gamestate.elapsedTime = elapsedTime;
	UpdatePlayer();
	UpdateItemAxe();
	UpdateSlimes();
	UpdateDroplets();
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
	
	if (Play::KeyDown('S')) // When you hold down slide, the counter goes down
	{
		slidebuffer.slidebufferTimeCounter = slidebuffer.slidebufferTime;
	}
	else
	{
		slidebuffer.slidebufferTimeCounter -= timer;
	}

	// Wall interactions
	if (WillCollideWithWall(obj_player, playerinfo.wallcollisionAABB))
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

		

		if (IsGrounded() == false)
		{
			gamestate.playerstate = STATE_FALLING;
		}

		break;

	case STATE_SLIDING:

		HandleSlidingControls();

		playerinfo.friction = playerinfo.slidingfriction;   

		playerinfo.slidetimerCounter -= gamestate.elapsedTime;


		if (playerinfo.facingright == false)
		{
		
			Play::SetSprite(obj_player, "slide_left", playerinfo.animationspeedslide);


		}
		else if (playerinfo.facingright == true)
		{

			Play::SetSprite(obj_player, "slide_right", playerinfo.animationspeedslide);

		}

		if (playerinfo.slidetimerCounter < 0 && IsUnderCeiling())
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
		else if (playerinfo.slidetimerCounter < 0 && IsUnderCeiling() == false)
		{
			gamestate.playerstate = STATE_IDLE;
			playerinfo.slidetimerCounter = playerinfo.slidetimer;
		}


		if (playerinfo.slidetimerCounter < 0 && IsGrounded() == false)
		{
			gamestate.playerstate = STATE_FALLING;
			playerinfo.slidetimerCounter = playerinfo.slidetimer;
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

		if (FloorCollisionStarted(obj_player))
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

		if (FloorCollisionStarted(obj_player))
		{
			Play::PlayAudio("Landing");
			obj_player.pos.y = obj_player.oldPos.y;
			gamestate.playerstate = STATE_LANDING;
		}

		break;


	case STATE_FALLING:

		HandleFallingControls();



		playerinfo.friction = playerinfo.fallingfriction;

		obj_player.acceleration.y = playerinfo.gravity;



		if (!playerinfo.facingright)
		{
			Play::SetSprite(obj_player, "fall_left", playerinfo.animationspeedfall);
		}
		else if (playerinfo.facingright)
		{
			Play::SetSprite(obj_player, "fall_right", playerinfo.animationspeedfall);
		}

		if (FloorCollisionStarted(obj_player))
		{
			Play::PlayAudio("Landing");
			obj_player.pos.y = obj_player.oldPos.y;
			playerinfo.slidetimerCounter = playerinfo.slidetimer;
			gamestate.playerstate = STATE_LANDING;
		}

		break;

	case STATE_LANDING:

		HandleLandingControls();

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

		if (jumpbuffer.jumpbufferTimeCounter > 0.0f) // If the W button is being held, the jumpbuffertimecounter is set to 0.2f so you jump when you hold your jump button.
		{
			obj_player.velocity.y = playerinfo.jumpspeed;
			gamestate.playerstate = STATE_JUMPING;
			Play::PlayAudio("jump");

		}

		if (slidebuffer.slidebufferTimeCounter > 0.0f) // If the S button is being held, the slidebuffertimecounter is set to 0.2f so you slide when you hold your slide button.
		{
			if (playerinfo.facingright == true)
			{
				gamestate.playerstate = STATE_SLIDING;
				obj_player.velocity.x = playerinfo.slidespeed;
				Play::PlayAudio("slide");
			}
			else if (playerinfo.facingright == false)
			{
				gamestate.playerstate = STATE_SLIDING;
				obj_player.velocity.x = -playerinfo.slidespeed;
				Play::PlayAudio("slide");
			}

		}
		break;

	case STATE_ATTACK:

		
		playerinfo.friction = playerinfo.runningandjumpingfriction;

		if (!playerinfo.facingright)
		{
			Play::SetSprite(obj_player, "axe_left", playerinfo.animationspeedatk);
			playerinfo.axehitboxoffset.x = -playerinfo.constaxehitboxoffset.x;
		}
		else if (playerinfo.facingright)
		{
			Play::SetSprite(obj_player, "axe_right", playerinfo.animationspeedatk);
			playerinfo.axehitboxoffset.x = playerinfo.constaxehitboxoffset.x;
		}
		if (Play::IsAnimationComplete(obj_player))
		{
			playerinfo.axeanimationcomplete = true;
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


	if (Play::KeyPressed('L') && playerinfo.hasaxe == true)
	{
		Play::PlayAudio("axe_swing");
		playerinfo.axeanimationcomplete = false;
		gamestate.playerstate = STATE_ATTACK;
	}

	if (Play::KeyPressed('S') && gamestate.playerstate == STATE_RUNNING)
	{
		if (playerinfo.facingright == true)
		{
			gamestate.playerstate = STATE_SLIDING;
			obj_player.velocity.x = playerinfo.slidespeed;
			Play::PlayAudio("slide");
		}
		else if (playerinfo.facingright == false)
		{
			gamestate.playerstate = STATE_SLIDING;
			obj_player.velocity.x = -playerinfo.slidespeed;
			Play::PlayAudio("slide");
		}
		
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

	// Jump
	if (Play::KeyPressed('W') && IsUnderCeiling() == false)
	{
		obj_player.velocity.y = playerinfo.jumpspeed;
		gamestate.playerstate = STATE_JUMPING;
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

void HandleLandingControls()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);
	
	if (Play::KeyDown('A'))
	{
		playerinfo.facingright = false;

		obj_player.velocity.x = -playerinfo.runspeed;
	}
	else if (Play::KeyDown('D'))
	{
		playerinfo.facingright = true;

		obj_player.velocity.x = playerinfo.runspeed;
	}

	if (Play::KeyPressed('S'))
	{
		if (playerinfo.facingright == true)
		{
			gamestate.playerstate = STATE_SLIDING;
			obj_player.velocity.x = playerinfo.slidespeed;
			Play::PlayAudio("slide");
		}
		else if (playerinfo.facingright == false)
		{
			gamestate.playerstate = STATE_SLIDING;
			obj_player.velocity.x = -playerinfo.slidespeed;
			Play::PlayAudio("slide");
		}

	}
}


void UpdateSlimes()
{
	Slime slime;

	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);
	
	std::vector<int> vSlimes = Play::CollectGameObjectIDsByType(TYPE_SLIME);
	
	for (int slime_id : vSlimes)
	{
		GameObject& obj_slime = Play::GetGameObject(slime_id);

		Play::SetSprite(obj_slime, "slime_idle", slime.animationspeed);
		
		obj_slime.acceleration.y = playerinfo.gravity;

		bool isdead = false;

		// IsGrounded for Slimes
		Point2D slimeTopLeft = obj_slime.pos - slime.AABB;
		Point2D slimeBottomRight = obj_slime.pos + slime.AABB;

		for (const Platform& platform : gamestate.vPlatforms)
		{
			Point2D platformTopLeft = platform.pos - platform.AABB;
			Point2D platformBottomRight = platform.pos + platform.AABB;

			if (slimeBottomRight.x > platformTopLeft.x &&
				slimeTopLeft.x  < platformBottomRight.x &&
				slimeBottomRight.y > platformTopLeft.y &&
				slimeTopLeft.y < platformBottomRight.y)
			{
				obj_slime.velocity.y = 0;
				obj_slime.acceleration.y = 0;
			}
		}

		// If the player is to the left or right of the slime, it runs away
		if (obj_player.pos.x < obj_slime.pos.x &&
			obj_player.pos.x > obj_slime.pos.x - Play::RandomRollRange(slime.sightrangehorizontal, 250) &&
			obj_player.pos.y > obj_slime.pos.y - slime.sightrangevertical &&
			obj_player.pos.y < obj_slime.pos.y + slime.sightrangevertical)

		{
			obj_slime.velocity.x = slime.runspeed;
		}
		else if(obj_player.pos.x > obj_slime.pos.x &&
			obj_player.pos.x < obj_slime.pos.x + Play::RandomRollRange(slime.sightrangehorizontal, 250) &&
			obj_player.pos.y > obj_slime.pos.y - slime.sightrangevertical &&
			obj_player.pos.y < obj_slime.pos.y + slime.sightrangevertical)
		{
			obj_slime.velocity.x = -slime.runspeed;
		}
		else
		{
			obj_slime.velocity.x = 0;
		}

		if (obj_slime.velocity.x > 0)
		{
			Play::SetSprite(obj_slime, "slime_idle_right", slime.animationspeed);
		}
		if (obj_slime.velocity.x < 0)
		{
			Play::SetSprite(obj_slime, "slime_idle_left", slime.animationspeed);
		}
	
		Play::UpdateGameObject(obj_slime);

		if (gamestate.playerstate == STATE_ATTACK &&
			obj_player.frame >= 8 &&
			IsCollidingAABB(obj_player.pos + playerinfo.axehitboxoffset, playerinfo.axehitboxAABB,obj_slime.pos , slime.AABB))
		{
			CreateDroplet(obj_slime.pos);
			Play::PlayAudio("hit");
			Play::SetSprite(obj_slime, "slime_melt", slime.animationspeed);
			isdead = true;
		}

		if (isdead)
		{
			Play::DestroyGameObject(slime_id);
		}
	} 
}

void UpdateItemAxe()
{
	GameObject& obj_axe = Play::GetGameObjectByType(TYPE_AXE);
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);

	obj_axe.pos.y += sin(gamestate.elapsedTime)*0.1; // Make the axe bob up and down

	Play::UpdateGameObject(obj_axe);

	if (Play::IsColliding(obj_axe,obj_player))
	{
		Play::DestroyGameObjectsByType(TYPE_AXE);
		Play::PlayAudio("axe_get");
		playerinfo.hasaxe = true;
	}
}

void CreateDroplet(Point2D pos)
{
	DropletParticleInfo dropletinfo;


	for (int i = 0; i < dropletinfo.max_particles; i++)
	{
		pos.y -= 10;

		Play::CreateGameObject(TYPE_DROPLET, pos, 0, "droplet");

		std::vector<int> vdroplets = Play::CollectGameObjectIDsByType(TYPE_DROPLET);

		for (int id_droplet : vdroplets)
		{
			GameObject& obj_droplet = Play::GetGameObject(id_droplet);
			
			obj_droplet.rotation = Play::DegToRad(Play::RandomRollRange(270, 90));

			Play::SetGameObjectDirection(obj_droplet, dropletinfo.initialvelocity.x, obj_droplet.rotation);

			obj_droplet.velocity.y = dropletinfo.initialvelocity.y;
		}
			
	}
}

void UpdateDroplets()
{
	DropletParticleInfo dropletinfo;

	std::vector<int> vDroplets = Play::CollectGameObjectIDsByType(TYPE_DROPLET);

	for (int id_droplet : vDroplets)
	{
		GameObject& obj_droplet = Play::GetGameObject(id_droplet);
		
		obj_droplet.acceleration.y = dropletinfo.gravity;

		SetGameObjectRotationToDirection(obj_droplet);

		if (FloorCollisionStarted(obj_droplet))
		{
			obj_droplet.velocity.y *= -1;
		}

		Play::UpdateGameObject(obj_droplet);
			
	}
}
// Creates a single platform tile
void CreatePlatform(int x, int y, int id)
{
	Platform platform;

	gamestate.vPlatforms.push_back(platform);
	gamestate.vPlatforms.back().pos = Point2D{x,y};
	gamestate.vPlatforms.back().id = id;
}

void CreateLevelFromArray()
{
	LevelLayoutInfo levellayout;

		for (int y = 0; y < levellayout.height; y++)
		{
			for (int x = 0; x < levellayout.width; x++)
			{
				int tileIndex = y * levellayout.width + x; // This is giving us a number, a position 0 to 880

				if (levellayout.levellayout[tileIndex] == 1) // If that number has a 1 in it create a platform
				{
					// Create an object at this position (x, y)
					CreatePlatform((x * platform.AABB.x*2) + platform.AABB.x / 2, (y * platform.AABB.y*2) + platform.AABB.y / 2, levellayout.levellayout[tileIndex]);
				}

				if (levellayout.levellayout[tileIndex] == 2)
				{
					CreatePlatform((x * platform.AABB.x * 2) + platform.AABB.x / 2, (y * platform.AABB.y * 2) + platform.AABB.y / 2, levellayout.levellayout[tileIndex]);
				}

				if (levellayout.levellayout[tileIndex] == 3)
				{
					CreatePlatform((x * platform.AABB.x * 2) + platform.AABB.x / 2, (y * platform.AABB.y * 2) + platform.AABB.y / 2, levellayout.levellayout[tileIndex]);
				}

				if (levellayout.levellayout[tileIndex] == 4)
				{
					CreatePlatform((x * platform.AABB.x * 2) + platform.AABB.x / 2, (y * platform.AABB.y * 2) + platform.AABB.y / 2, levellayout.levellayout[tileIndex]);
				}

				if (levellayout.levellayout[tileIndex] == 5)
				{
					Play::CreateGameObject(TYPE_SLIME, { (x * platform.AABB.x * 2) + platform.AABB.x / 2, (y * platform.AABB.y * 2) + platform.AABB.y / 2 }, 8, "slime_idle");
				}

				if (levellayout.levellayout[tileIndex] == 6)
				{
					Play::CreateGameObject(TYPE_AXE, { (x * platform.AABB.x * 2) + platform.AABB.x / 2, (y * platform.AABB.y * 2) + platform.AABB.y / 2 }, 8, "item_axe");
				}
			}
		}
}


// Checks player's AABBmaxY and if it's collided with a platform's minY
bool FloorCollisionStarted(GameObject& obj)
{
	Point2D playerTopLeft = obj.pos - playerinfo.verticalcollisionAABB;
	Point2D playerBottomRight = obj.pos + playerinfo.verticalcollisionAABB;

	Point2D playerOldTopLeft = obj.oldPos - playerinfo.verticalcollisionAABB;
	Point2D playerOldBottomRight = obj.oldPos + playerinfo.verticalcollisionAABB;

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
bool WillCollideWithWall(GameObject& obj, Vector2D obj_AABB)

{
	Point2D playerTopLeft = obj.pos - obj_AABB;
	Point2D playerBottomRight = obj.pos + obj_AABB;

	Vector2D playernextPosition = obj.pos + obj.velocity;

	Point2D playernextposTopLeft = playernextPosition - obj_AABB;
	Point2D playernextposBottomRight = playernextPosition + obj_AABB;

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

bool IsCollidingAABB(Point2D obj_a_pos, Vector2D obj_a_dimensions, Point2D obj_b_pos, Vector2D obj_b_dimensions)
{
	Point2D obj_a_TopLeft = obj_a_pos - obj_a_dimensions;
	Point2D obj_a_BottomRight = obj_a_pos + obj_a_dimensions;

	Point2D obj_b_TopLeft = obj_b_pos - obj_b_dimensions;
	Point2D obj_b_BottomRight = obj_b_pos + obj_b_dimensions;

	if (obj_a_BottomRight.x > obj_b_TopLeft.x &&
		obj_a_TopLeft.x  < obj_b_BottomRight.x &&
		obj_a_BottomRight.y  > obj_b_TopLeft.y &&
		obj_a_TopLeft.y < obj_b_BottomRight.y)
	{
		return true;
	}

	return false;
}


void SetGameObjectRotationToDirection(GameObject& obj)
{
	obj.rotation = atan2(-obj.velocity.x, obj.velocity.y);
}


void CameraFollow()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);
	
	Play::SetCameraPosition({ obj_player.pos.x-DISPLAY_WIDTH/2, obj_player.pos.y-DISPLAY_HEIGHT/2 });
	
}

void Draw()
{
	Play::DrawBackground();
	
	Play::DrawSprite("BG", { DISPLAY_WIDTH-16,DISPLAY_HEIGHT-32}, 1);

	DrawPlatforms(); 

	DrawAllGameObjectsByTypeRotated(TYPE_PLAYER);

	DrawAllGameObjectsByType(TYPE_AXE);

	DrawAllGameObjectsByType(TYPE_SLIME);

	DrawAllGameObjectsByTypeRotated(TYPE_DROPLET);

	DrawDebug();
	
	Play::PresentDrawingBuffer();
}



void DrawPlatforms()
{
	Platform platform;

	for (Platform& platform : gamestate.vPlatforms)
	{

		if (platform.id == 1)
		{
			Play::DrawSprite(Play::GetSpriteId("tile"), platform.pos, 0);
		}
		else if (platform.id == 2)
		{
			Play::DrawSprite(Play::GetSpriteId("rock"), platform.pos, 0);
		}
		else if (platform.id == 3)
		{
			Play::DrawSprite(Play::GetSpriteId("wood"), platform.pos, 0);
		}
		else if (platform.id == 4)
		{
			Play::DrawSprite(Play::GetSpriteId("tree"), platform.pos, 0);
		}
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

void DrawAllObjectAABB(GameObjectType type, Vector2D obj_dimensions)
{
	for (int id : Play::CollectGameObjectIDsByType(type))
	{
		GameObject& obj = Play::GetGameObject(id);
		DrawObjectAABB(obj.pos, obj_dimensions);
	}
}

void DrawDebug()
{
	/*DrawPlayerNextPositionAABB();

	DrawPlatformsAABB();

	DrawPlayerAABB();

	DrawObjectAABB(Play::GetGameObjectByType(TYPE_PLAYER).pos - playerinfo.headboxoffset, playerinfo.headboxAABB);*/

	DrawObjectAABB(Play::GetGameObjectByType(TYPE_PLAYER).pos + playerinfo.axehitboxoffset, playerinfo.axehitboxAABB); // Axe hitbox

	DrawAllObjectAABB(TYPE_SLIME, slime.AABB);

	Play::DrawFontText("64px", "CENTRE", gamestate.centrepos, Play::CENTRE);
}

