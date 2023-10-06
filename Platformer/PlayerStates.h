#pragma once

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

		if (FloorCollisionStarted())
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



	// Slide Attack
	if (Play::KeyPressed('L') && IsUnderCeiling() == false)
	{
		gamestate.playerstate = STATE_ATTACK;
	}

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

	//if (Play::KeyDown('W')) // Holding W down countsdown your jumpbuffer time
	//{
	//	jumpbuffer.jumpbufferTimeCounter = jumpbuffer.jumpbufferTime;
	//}
	//else
	//{
	//	jumpbuffer.jumpbufferTimeCounter -= timer;
	//}


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

void HandleGroundedAttackControls()
{
	// If attack is pressed, play attack 2
	// Set a timer so if attack is pressed within 5? 6? frames? research best timings
	// Play attack 3
}
