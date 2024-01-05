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
	Play::MoveSpriteOrigin("witch_idle", 0, witchinfo.idlespriteoffset.y);
	Play::MoveSpriteOrigin("witch_talking", 0, witchinfo.talkingspriteoffset.y);
	Play::LoadBackground("Data\\Backgrounds\\background.png");
	Play::CreateGameObject(TYPE_PLAYER, { DISPLAY_WIDTH,DISPLAY_HEIGHT }, 16, "idle_right");
	Play::StartAudioLoop("music");
	CreateLevelFromArray();
}

// Called by PlayBuffer every frame (60 times a second!)
bool MainGameUpdate(float elapsedTime)
{
	gamestate.elapsedTime = elapsedTime;
	UpdatePlayer();
	UpdateItemAxe();
	UpdateSlimes();
	UpdateCreep();
	UpdateDroplets();
	UpdateWitch();
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
	if (WillCollideWithPlatform(obj_player, playerinfo.wallcollisionAABB))
	{
		obj_player.pos.x = obj_player.oldPos.x;
		obj_player.velocity.x = 0;

		if (IsPlayerInsideWall() && playerinfo.playerleftofplatform)
		{
			obj_player.pos.x -= 1;
		}
		else if (IsPlayerInsideWall() && playerinfo.playerleftofplatform == false)
		{
			obj_player.pos.x += 1;
		}
	}

	// Ceiling interactions
	if (CeilingCollisionStarted(obj_player, playerinfo.verticalcollisionAABB))
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
			Play::SetSprite(obj_player, "player_idle_right", playerinfo.animationspeedidle); //Idle
		}
		else if (!playerinfo.facingright)
		{
			Play::SetSprite(obj_player, "player_idle_left", playerinfo.animationspeedidle); //Idle
		}

		if (IsObjGrounded(obj_player, playerinfo.verticalcollisionAABB) == false)
		{
			gamestate.playerstate = STATE_FALLING;
		}

		break;

	case STATE_RUNNING:

		HandleGroundedControls();



		playerinfo.friction = playerinfo.runningandjumpingfriction;

		if (!playerinfo.facingright)
		{
			Play::SetSprite(obj_player, "player_run_left", playerinfo.animationspeedrun);

		}
		else if (playerinfo.facingright)
		{
			Play::SetSprite(obj_player, "player_run_right", playerinfo.animationspeedrun);
		}



		if (IsObjGrounded(obj_player, playerinfo.verticalcollisionAABB) == false)
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

		if (playerinfo.slidetimerCounter < 0 && IsPlayerUnderCeiling())
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
		else if (playerinfo.slidetimerCounter < 0 && IsPlayerUnderCeiling() == false)
		{
			gamestate.playerstate = STATE_IDLE;
			playerinfo.slidetimerCounter = playerinfo.slidetimer;
		}


		if (playerinfo.slidetimerCounter < 0 && IsObjGrounded(obj_player, playerinfo.verticalcollisionAABB) == false)
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

		if (Play::IsAnimationComplete(obj_player))
		{
			gamestate.playerstate = STATE_JUMPINGDOWN;
		}

		if (FloorCollisionStarted(obj_player, playerinfo.verticalcollisionAABB))
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

		if (FloorCollisionStarted(obj_player, playerinfo.verticalcollisionAABB))
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

		if (FloorCollisionStarted(obj_player, playerinfo.verticalcollisionAABB))
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
	if (Play::KeyPressed('W') && IsPlayerUnderCeiling() == false)
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


void UpdateWitch()
{
	GameObject& obj_witch = Play::GetGameObjectByType(TYPE_WITCH);
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);

	Play::SetSprite(obj_witch, "witch_idle", witchinfo.animationspeedidle);

	if (IsCollidingAABB(obj_player.pos, playerinfo.verticalcollisionAABB,
		obj_witch.pos + witchinfo.talkingrangeoffset, witchinfo.talkingrangeAABB))
	{
		witchinfo.intalkingrange = true;
	}
	else
	{
		witchinfo.intalkingrange = false;
	}

	Play::UpdateGameObject(obj_witch);
}

void UpdateCreep()
{
	CreepInfo creepinfo;

	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);

	std::vector<int> vCreeps = Play::CollectGameObjectIDsByType(TYPE_CREEP);

	for (int creep_id : vCreeps)
	{
		GameObject& obj_creep = Play::GetGameObject(creep_id);
		obj_creep.acceleration.y = playerinfo.gravity;
		bool isfacingright = false;

		// Debug to see creep death animation
		if (Play::KeyPressed('P'))
		{
			gamestate.creepstate = STATE_DYING;
		}
	
		switch (gamestate.creepstate)
		{
		case STATE_CREEP_IDLE:		

			obj_creep.velocity.x *= 0.9;

			if (isfacingright == true)
			{
				Play::SetSprite(obj_creep, "creep_idle_right", creepinfo.animationspeed);
			}
			else if (isfacingright == false)
			{
				Play::SetSprite(obj_creep, "creep_idle_left", creepinfo.animationspeed);
			}

			if (CanGameObjectSeeAnotherGameObject(obj_creep, obj_player, creepinfo.sightrangehorizontal, creepinfo.sightrangeverticalnegative,creepinfo.sightrangeverticalpositive))
			{
				gamestate.creepstate = STATE_CHASING;
			}
			break;
		case STATE_CHASING:

			// Stops infinite acceleration
			SetGameObjectMaxSpeed(obj_creep, creepinfo.maxspeed);

			if (CanGameObjectSeeAnotherGameObject(obj_creep, obj_player, creepinfo.sightrangehorizontal, creepinfo.sightrangeverticalnegative, creepinfo.sightrangeverticalpositive) &&
				IsGameObjectOnLeftOfAnotherGameObject(obj_player, obj_creep) == true)
			{
				isfacingright = false;
				Play::SetSprite(obj_creep, "creep_run_left", creepinfo.animationspeed);
				obj_creep.velocity.x -= creepinfo.runspeed;
			}
			else if (CanGameObjectSeeAnotherGameObject(obj_creep, obj_player, creepinfo.sightrangehorizontal, creepinfo.sightrangeverticalnegative, creepinfo.sightrangeverticalpositive) &&
				IsGameObjectOnLeftOfAnotherGameObject(obj_player, obj_creep) == false)
			{
				isfacingright = true;
				Play::SetSprite(obj_creep, "creep_run_right", creepinfo.animationspeed);
				obj_creep.velocity.x += creepinfo.runspeed;
			}
			else if (CanGameObjectSeeAnotherGameObject(obj_creep, obj_player, creepinfo.sightrangehorizontal, creepinfo.sightrangeverticalnegative, creepinfo.sightrangeverticalpositive) == false)
			{
				gamestate.creepstate = STATE_CREEP_IDLE;
			}
			break;
		case STATE_DYING:
			Play::SetSprite(obj_creep, "creep_dead", creepinfo.animationspeed);
			if (Play::IsAnimationComplete(obj_creep))
			{
				gamestate.creepstate = STATE_DEAD;
			}
			break;
		case STATE_DEAD:
			Play::DestroyGameObject(creep_id);
			break;
		}
		

		// IsGrounded for creeps
		if (IsObjGrounded(obj_creep, creepinfo.AABB))
		{
			obj_creep.velocity.y = 0;
			obj_creep.acceleration.y = 0;
			obj_creep.pos.y = obj_creep.oldPos.y;
		}

		if (WillCollideWithPlatform(obj_creep, creepinfo.AABB))
		{
			obj_creep.velocity.x = 0;
			obj_creep.pos.x = obj_creep.oldPos.x;
		}

		Play::UpdateGameObject(obj_creep);
	}
}


void UpdateSlimes()
{
	SlimeInfo slimeinfo;

	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);

	std::vector<int> vSlimes = Play::CollectGameObjectIDsByType(TYPE_SLIME);

	for (int slime_id : vSlimes)
	{
		GameObject& obj_slime = Play::GetGameObject(slime_id);



		obj_slime.acceleration.y = playerinfo.gravity;

		bool isdead = false;

		// IsGrounded for Slimes
		if (IsObjGrounded(obj_slime, slimeinfo.AABB))
		{
			obj_slime.velocity.y = 0;
			obj_slime.acceleration.y = 0;
			obj_slime.pos.y = obj_slime.oldPos.y;
		}

		if (WillCollideWithPlatform(obj_slime, slimeinfo.AABB))
		{
			obj_slime.velocity.x = 0;
			obj_slime.pos.x = obj_slime.oldPos.x;
		}

		// If the player is to the left or right of the slime, it runs away
		if (obj_player.pos.x < obj_slime.pos.x &&
			obj_player.pos.x > obj_slime.pos.x - slimeinfo.sightrangehorizontal &&
			obj_player.pos.y > obj_slime.pos.y - slimeinfo.sightrangevertical &&
			obj_player.pos.y < obj_slime.pos.y + slimeinfo.sightrangevertical)

		{
			obj_slime.velocity.x = slimeinfo.runspeed;
		}
		else if (obj_player.pos.x > obj_slime.pos.x &&
			obj_player.pos.x < obj_slime.pos.x + slimeinfo.sightrangehorizontal &&
			obj_player.pos.y > obj_slime.pos.y - slimeinfo.sightrangevertical &&
			obj_player.pos.y < obj_slime.pos.y + slimeinfo.sightrangevertical)
		{
			obj_slime.velocity.x = -slimeinfo.runspeed;
		}
		else
		{
			obj_slime.velocity.x = 0;
			Play::SetSprite(obj_slime, "slime_idle", slimeinfo.animationspeed);
		}

		// Faces the slime in the direction of travel
		if (obj_slime.velocity.x > 0)
		{
			Play::SetSprite(obj_slime, "slime_hop_right", slimeinfo.animationspeed);
		}
		if (obj_slime.velocity.x < 0)
		{
			Play::SetSprite(obj_slime, "slime_hop_left", slimeinfo.animationspeed);
		}

		// Creates droplets if the player attacks a slime
		if (gamestate.playerstate == STATE_ATTACK &&
			obj_player.frame >= 8 &&
			IsCollidingAABB(obj_player.pos + playerinfo.axehitboxoffset, playerinfo.axehitboxAABB, obj_slime.pos, slimeinfo.AABB))
		{
			CreateDroplet(obj_slime.pos);
			Play::PlayAudio("hit");
			Play::SetSprite(obj_slime, "slime_melt", slimeinfo.animationspeed); // not working
			isdead = true;
		}

		Play::UpdateGameObject(obj_slime);

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

	obj_axe.pos.y += sin(gamestate.elapsedTime) * 0.1; // Make the axe bob up and down

	Play::UpdateGameObject(obj_axe);

	if (Play::IsColliding(obj_axe, obj_player))
	{
		Play::DestroyGameObjectsByType(TYPE_AXE);
		Play::PlayAudio("axe_get");
		playerinfo.hasaxe = true;
	}
}


// Creates dropletinfo.max_particles number of droplets and sets their rotation and velocity
void CreateDroplet(Point2D pos)
{
	for (int i = 0; i < dropletinfo.max_particles; i++)
	{
		pos.y -= 2;

		int id_droplet = Play::CreateGameObject(TYPE_DROPLET, pos, 0, "droplet");

		GameObject& obj_droplet = Play::GetGameObject(id_droplet);

		obj_droplet.rotation = Play::DegToRad(Play::RandomRollRange(270, 90));

		Play::SetGameObjectDirection(obj_droplet, dropletinfo.initialvelocity.x, obj_droplet.rotation);

		obj_droplet.velocity.y = dropletinfo.initialvelocity.y;
	}
}

void UpdateDroplets()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);

	std::vector<int> vDroplets = Play::CollectGameObjectIDsByType(TYPE_DROPLET);

	for (int id_droplet : vDroplets)
	{
		GameObject& obj_droplet = Play::GetGameObject(id_droplet);

		bool IsCollected = false;

		if (FloorCollisionStarted(obj_droplet, dropletinfo.AABB) == false)
		{
			obj_droplet.acceleration.y = dropletinfo.gravity;
		}

		SetGameObjectRotationToDirection(obj_droplet);

		if (FloorCollisionStarted(obj_droplet, dropletinfo.AABB))
		{
			obj_droplet.velocity.y *= -1;
			obj_droplet.velocity.y *= 0.5;
		}

		if (WillCollideWithPlatform(obj_droplet, dropletinfo.AABB))
		{
			obj_droplet.velocity.x *= -1;
			obj_droplet.velocity.x *= 0.5;
		}

		if (IsObjGrounded(obj_droplet, dropletinfo.AABB) &&
			obj_droplet.velocity.y < dropletinfo.minvelocity.y &&
			obj_droplet.velocity.y > -dropletinfo.minvelocity.y)
		{
			obj_droplet.velocity.y = 0;
			obj_droplet.velocity.x = 0;
			obj_droplet.acceleration.y = 0;
		}

		if (IsObjInsideWall(obj_droplet, dropletinfo.AABB) == true) // Stops droplet getting stuck in walls
		{
			obj_droplet.pos.y -= 1.0f;
		}

		if (gamestate.playerstate == STATE_ATTACK &&
			obj_player.frame >= 8 &&
			IsCollidingAABB(obj_player.pos + playerinfo.axehitboxoffset, playerinfo.axehitboxAABB,
				obj_droplet.pos, dropletinfo.AABB))
		{
			if (obj_player.pos.x < obj_droplet.pos.x) // Player to the left of droplet
			{
				obj_droplet.rotation = Play::DegToRad(Play::RandomRollRange(180, 90));
				Play::SetGameObjectDirection(obj_droplet, dropletinfo.initialvelocity.x, obj_droplet.rotation);
				obj_droplet.velocity.y = dropletinfo.initialvelocity.y;
			}
			else if (obj_player.pos.x > obj_droplet.pos.x)
			{
				obj_droplet.rotation = Play::DegToRad(Play::RandomRollRange(270, 180));
				Play::SetGameObjectDirection(obj_droplet, dropletinfo.initialvelocity.x, obj_droplet.rotation);
				obj_droplet.velocity.y = dropletinfo.initialvelocity.y;
			}

		}

		if (IsCollidingAABB(obj_player.pos, playerinfo.verticalcollisionAABB,
			obj_droplet.pos, dropletinfo.AABB))
		{
			IsCollected = true;
			inventory.slimeteardrops += 1;

			switch (Play::RandomRollRange(1, 3))
			{
			case 1:
				Play::PlayAudio("tear_collect_1");
				break;
			case 2:
				Play::PlayAudio("tear_collect_2");
				break;
			case 3:
				Play::PlayAudio("tear_collect_3");
				break;
			}

		}

		Play::UpdateGameObject(obj_droplet);

		if (IsCollected == true)
		{
			Play::DestroyGameObject(id_droplet);
		}
	}
}


// Creates a single platform tile
void CreatePlatform(int x, int y, int id)
{
	Platform platform;

	gamestate.vPlatforms.push_back(platform);
	gamestate.vPlatforms.back().pos = Point2D{ x,y };
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
				CreatePlatform((x * platform.AABB.x * 2) + platform.AABB.x / 2, (y * platform.AABB.y * 2) + platform.AABB.y / 2, levellayout.levellayout[tileIndex]);
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

			if (levellayout.levellayout[tileIndex] == 7)
			{
				Play::CreateGameObject(TYPE_WITCH, { (x * platform.AABB.x * 2) + platform.AABB.x / 2, (y * platform.AABB.y * 2) + platform.AABB.y / 2 }, 8, "witch_idle");
			}

			if (levellayout.levellayout[tileIndex] == 8)
			{
				Play::CreateGameObject(TYPE_CREEP, { (x * platform.AABB.x * 2) + platform.AABB.x / 2, (y * platform.AABB.y * 2) + platform.AABB.y / 2 }, 8, "creep_idle");
			}
		}
	}
}


// Checks player's AABBmaxY and if it's collided with a platform's minY
bool FloorCollisionStarted(GameObject& obj, Vector2D obj_AABB)
{
	Point2D playerTopLeft = obj.pos - obj_AABB;
	Point2D playerBottomRight = obj.pos + obj_AABB;

	Point2D playerOldTopLeft = obj.oldPos - obj_AABB;
	Point2D playerOldBottomRight = obj.oldPos + obj_AABB;

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
bool IsPlayerUnderCeiling()
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

bool CeilingCollisionStarted(GameObject& obj, Vector2D obj_AABB)
{
	PlatformInfo platforminfo;

	Point2D objTopLeft = obj.pos - obj_AABB;
	Point2D objBottomRight = obj.pos + obj_AABB;

	Point2D objOldTopLeft = obj.oldPos - obj_AABB;
	Point2D objOldBottomRight = obj.oldPos + obj_AABB;

	// Iterate through all platforms to check for collisions
	for (const Platform& platform : gamestate.vPlatforms)
	{
		// Calculate the platform's AABB
		Point2D platformTopLeft = platform.pos - platform.AABB;
		Point2D platformBottomRight = platform.pos + platform.AABB;

		// Check for collision between obj's collision box and the platform
		if (objBottomRight.x > platformTopLeft.x &&
			objTopLeft.x  < platformBottomRight.x &&
			objBottomRight.y > platformTopLeft.y &&
			objTopLeft.y < platformBottomRight.y)
		{


			// Checks if previous frame was below the platform
			if (objOldTopLeft.y > platformBottomRight.y)
			{

				return true; // obj is hitting head
			}
		}

	}

	return false; // obj is not hitting head
}


bool IsObjGrounded(GameObject& obj, Vector2D obj_AABB)
{

	Point2D objTopLeft = obj.pos - obj_AABB;
	Point2D objBottomRight = obj.pos + obj_AABB;

	// Iterate through all platforms to check for collisions
	for (const Platform& platform : gamestate.vPlatforms)
	{
		// Calculate the platform's AABB
		Point2D platformTopLeft = platform.pos - platform.AABB;
		Point2D platformBottomRight = platform.pos + platform.AABB;

		// Check for collision between obj's grounding box and the platform
		if (objBottomRight.x > platformTopLeft.x &&
			objTopLeft.x  < platformBottomRight.x &&
			objBottomRight.y > platformTopLeft.y &&
			objTopLeft.y < platformBottomRight.y)
		{

			return true; // obj is grounded

		}

	}

	return false; // obj is not grounded
}

// Check's obj's edgebox and if it's going to collide with the sides of a platform
bool WillCollideWithPlatform(GameObject& obj, Vector2D obj_AABB)

{
	Point2D objTopLeft = obj.pos - obj_AABB;
	Point2D objBottomRight = obj.pos + obj_AABB;

	Vector2D objnextPosition = obj.pos + obj.velocity;

	Point2D objnextposTopLeft = objnextPosition - obj_AABB;
	Point2D objnextposBottomRight = objnextPosition + obj_AABB;

	// Iterate through all platforms to check for collisions
	for (Platform& platform : gamestate.vPlatforms)
	{
		// Calculate the platform's AABB
		Point2D platformTopLeft = platform.pos - platform.AABB;
		Point2D platformBottomRight = platform.pos + platform.AABB;

		// Check for collision between obj's edge box and the platform
		if (objnextposBottomRight.x > platformTopLeft.x &&
			objnextposTopLeft.x  < platformBottomRight.x &&
			objnextposBottomRight.y  > platformTopLeft.y &&
			objnextposTopLeft.y < platformBottomRight.y)

		{

			return true; // obj is colliding with platform side

		}

	}

	return false; // obj is not colliding with platform side
}

bool IsObjInsideWall(GameObject& obj, Vector2D obj_AABB)
{
	Point2D objTopLeft = obj.pos - obj_AABB;
	Point2D objBottomRight = obj.pos + obj_AABB;

	// Iterate through all platforms to check for collisions
	for (Platform& platform : gamestate.vPlatforms)
	{
		// Calculate the platform's AABB
		Point2D platformTopLeft = platform.pos - platform.AABB;
		Point2D platformBottomRight = platform.pos + platform.AABB;

		// Check for collision between obj's edge box and the platform
		if (objBottomRight.x > platformTopLeft.x &&
			objTopLeft.x  < platformBottomRight.x &&
			objBottomRight.y > platformTopLeft.y &&
			objTopLeft.y < platformBottomRight.y)
		{
			return true; // obj is inside platform
		}

	}

	return false; // obj is not inside platform
}

bool IsPlayerInsideWall()
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

// Takes a game object and gives it a sight range and speed at which it chases another game object
void MakeGameObjectChaseAnother(GameObject& obj_chaser, GameObject& obj_gettingchased, float sightrangehorizontal, float sightrangevertical, float runspeed, float maxspeed)
{
	if (obj_gettingchased.pos.x < obj_chaser.pos.x &&
		obj_gettingchased.pos.x > obj_chaser.pos.x - sightrangehorizontal &&
		obj_gettingchased.pos.y > obj_chaser.pos.y - sightrangevertical &&
		obj_gettingchased.pos.y < obj_chaser.pos.y + sightrangevertical &&
		obj_chaser.velocity.x >= -maxspeed)

	{
		obj_chaser.velocity.x -= runspeed;
	}
	else if (obj_gettingchased.pos.x > obj_chaser.pos.x &&
		obj_gettingchased.pos.x < obj_chaser.pos.x + sightrangehorizontal &&
		obj_gettingchased.pos.y > obj_chaser.pos.y - sightrangevertical &&
		obj_gettingchased.pos.y < obj_chaser.pos.y + sightrangevertical &&
		obj_chaser.velocity.x <= maxspeed)
	{
		obj_chaser.velocity.x += runspeed;
	}
	else
	{
		obj_chaser.velocity.x = 0;
	}
}

bool CanGameObjectSeeAnotherGameObject(GameObject& obj_chaser, GameObject& obj_gettingchased, float sightrangehorizontal, float sightrangeverticalnegative, float sightrangeverticalpositive)
{
	
	if (obj_gettingchased.pos.x < obj_chaser.pos.x &&
		obj_gettingchased.pos.x > obj_chaser.pos.x - sightrangehorizontal &&
		obj_gettingchased.pos.y > obj_chaser.pos.y - sightrangeverticalnegative &&
		obj_gettingchased.pos.y < obj_chaser.pos.y + sightrangeverticalpositive)

	{
		return true;
	}
	else if (obj_gettingchased.pos.x > obj_chaser.pos.x &&
		obj_gettingchased.pos.x < obj_chaser.pos.x + sightrangehorizontal &&
		obj_gettingchased.pos.y > obj_chaser.pos.y - sightrangeverticalnegative &&
		obj_gettingchased.pos.y < obj_chaser.pos.y + sightrangeverticalpositive)
	{
		return true;
	}

	return false;
}

bool IsGameObjectOnLeftOfAnotherGameObject(GameObject& obj_inmotion, GameObject& obj_stationary)
{
	if (obj_inmotion.pos.x < obj_stationary.pos.x)
	{
		return true;
	}
	else if (obj_inmotion.pos.x > obj_stationary.pos.x)
	{
		return false;
	}
	
}

void SetGameObjectMaxSpeed(GameObject& obj, float max_velocity)
{
	if (obj.velocity.x <= -max_velocity)
	{
		obj.velocity.x = -max_velocity;
	}
	else if (obj.velocity.x >= max_velocity)
	{
		obj.velocity.x = max_velocity;
	}
}


void CameraFollow()
{
	GameObject& obj_player = Play::GetGameObjectByType(TYPE_PLAYER);

	Play::SetCameraPosition({ obj_player.pos.x - DISPLAY_WIDTH / 2, obj_player.pos.y - DISPLAY_HEIGHT / 2 });

}

void Draw()
{
	Play::DrawBackground();

	Play::DrawSprite("BG", { DISPLAY_WIDTH - 16,DISPLAY_HEIGHT - 32 }, 1); // Mushroom Background

	DrawPlatforms();

	Play::DrawObject(Play::GetGameObjectByType(TYPE_WITCH));

	DrawAllGameObjectsByTypeRotated(TYPE_PLAYER);

	DrawAllGameObjectsByType(TYPE_AXE);

	DrawAllGameObjectsByType(TYPE_SLIME);

	DrawAllGameObjectsByType(TYPE_CREEP);

	DrawAllGameObjectsByTypeRotated(TYPE_DROPLET);


	DrawDialogue();

	DrawUI();

	DrawDebug();

	Play::PresentDrawingBuffer();
}


// Draws how many slime tears you have
void DrawUI()
{
	Play::SetDrawingSpace(Play::SCREEN);
	Play::DrawSprite("ui_tear", Point2D(DISPLAY_WIDTH * 0.1f, DISPLAY_HEIGHT * 0.1f), 1);
	Play::DrawFontText("64px", " : " + std::to_string(inventory.slimeteardrops), Point2D(DISPLAY_WIDTH * 0.15f, DISPLAY_HEIGHT * 0.1f), Play::CENTRE);
	Play::SetDrawingSpace(Play::WORLD);
}

void DrawDialogue()
{
	GameObject& obj_witch = Play::GetGameObjectByType(TYPE_WITCH);

	if (witchinfo.intalkingrange == true)
	{


		if (inventory.slimeteardrops >= witchinfo.slimeteardropsneeded)
		{
			Play::DrawFontText("64px", witchinfo.dialogueslimescollected, obj_witch.pos + witchinfo.speechbubbleoffset, Play::CENTRE);
		}
		else if (inventory.slimeteardrops < witchinfo.slimeteardropsneeded && playerinfo.hasaxe == true)
		{
			Play::DrawFontText("64px", witchinfo.dialoguehowtouseaxe, obj_witch.pos + witchinfo.speechbubbleoffset, Play::CENTRE);
		}
		else if (inventory.slimeteardrops < witchinfo.slimeteardropsneeded)
		{
			Play::DrawFontText("64px", witchinfo.dialogue1, obj_witch.pos + witchinfo.speechbubbleoffset, Play::CENTRE);

			Play::DrawFontText("64px", witchinfo.dialogue2, obj_witch.pos + witchinfo.speechbubbleoffset + bannerinfo.nextlineoffset, Play::CENTRE);

			Play::DrawFontText("64px", witchinfo.dialogue3, obj_witch.pos + witchinfo.speechbubbleoffset + bannerinfo.nextlineoffset * 2, Play::CENTRE);

		}


	}
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

	Point2D playerTopLeft = obj_player.pos - playerinfo.verticalcollisionAABB;

	Point2D playerBottomRight = obj_player.pos + playerinfo.verticalcollisionAABB;


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

	//DrawPlayerNextPositionAABB();

	//DrawPlatformsAABB();

	//DrawObjectAABB(Play::GetGameObjectByType(TYPE_PLAYER).pos - playerinfo.headboxoffset, playerinfo.headboxAABB);
	
	//DrawPlayerAABB();

	//DrawObjectAABB(Play::GetGameObjectByType(TYPE_PLAYER).pos + playerinfo.axehitboxoffset, playerinfo.axehitboxAABB); // Axe hitbox

	//DrawObjectAABB(Play::GetGameObjectByType(TYPE_WITCH).pos + witchinfo.talkingrangeoffset, witchinfo.talkingrangeAABB); // Witch talking hitbox

	//DrawObjectAABB(Play::GetGameObjectByType(TYPE_WITCH).pos + witchinfo.speechbubbleoffset, bannerinfo.AABB); // Dialoguebox pos

	//DrawAllObjectAABB(TYPE_SLIME, slimeinfo.AABB);

	//DrawAllObjectAABB(TYPE_DROPLET, dropletinfo.AABB);

	// DrawAllObjectAABB(TYPE_CREEP, { creepinfo.sightrangehorizontal , creepinfo.sightrangeverticalnegative }); // Creep sightrange

}

