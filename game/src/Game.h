#pragma once
#include "Physics.h"
#include "Nodes.h" 
#include <array>

class Entity : public Rigidbody
{
public:
	Entity(float maxHealth, float speed, float radius) : mMaxHealth(maxHealth), mSpeed(speed), mRadius(radius)
	{
		health = maxHealth;
	}

	Circle Collider() const { return { pos, mRadius }; }
	float Radius() const { return mRadius; }
	float Speed() const { return mSpeed; }

	float MaxHealth() const { return mMaxHealth; }
	float HealthPercent() const { return health / mMaxHealth; }

	float health;

protected:
	const float mMaxHealth;
	const float mSpeed;
	const float mRadius;
};

class Bullet : public Entity
{
public:
	enum Type
	{
		NONE,
		PLAYER,
		ENEMY
	};

	Bullet(Type type = NONE, float maxHealth = 10.0f, float speed = 250.0f, float radius = 5.0f) :
		Entity(maxHealth, speed, radius), mType(type) {}

	Bullet(const Bullet& bullet) : Entity(bullet.mMaxHealth, bullet.mSpeed, bullet.mRadius), mType(bullet.mType)
	{
		pos = bullet.pos;
		vel = bullet.vel;
		acc = bullet.acc;
		dir = bullet.dir;
		angularSpeed = bullet.angularSpeed;
	}

	Bullet& operator=(const Bullet& bullet)
	{
		pos = bullet.pos;
		vel = bullet.vel;
		acc = bullet.acc;
		dir = bullet.dir;
		angularSpeed = bullet.angularSpeed;
		return *this;
	}

	Type GetType() const { return mType; }
	bool isActive = true;
private:
	const Type mType;
};
using Bullets = std::vector<Bullet>;

struct World
{
	Circles obstacles;
	Points waypoints;
	Bullets bullets;
};

class Player : public Entity
{
public:
	Player(float maxHealth = 100.0f, float speed = 500.0f, float radius = 25.0f) :
		Entity(maxHealth, speed, radius) {}

	// Inside the Player class's Update function
	void Update(float dt, World& world)
	{

		if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
		{
			Bullet newBullet(Bullet::PLAYER);

			// Calculate the offset distance (adjust this value as needed)
			float offsetDistance = mRadius + newBullet.Radius() + 10.0f; // Example offset distance

			// Calculate the offset position based on player's direction and offset distance
			Vector2 offset = dir * offsetDistance;

			// Set the bullet's position to the player's position plus the offset
			newBullet.pos = pos + offset;

			// Set a fixed speed for the bullet
			newBullet.vel = Normalize(GetMousePosition() - newBullet.pos) * 500.0f; // Adjust speed as needed

			world.bullets.push_back(newBullet);
		}

		// Rest of the player's update logic
		acc = Seek(GetMousePosition(), pos, vel, mSpeed);
		::Update(*this, dt);
		ResolveCircles(pos, mRadius, world.obstacles);
	}


};

// Handle transitions at enemy-level, handle everything else at tree-level
class Enemy : public Entity
{
public:
	enum State
	{
		OFFENSIVE,
		DEFENSIVE,
		NEUTRAL,
		COUNT
	};

	Enemy(World& world,
		float neutralDuration = 4.0f, float offensiveDuration = 6.0f, float defensiveDuration = 8.0f,
		float maxHealth = 100.0f, float speed = 1000.0f, float radius = 25.0f, float proximity = 50.0f) :
		mWorld(world), mProximity(proximity),
		Entity(maxHealth, speed, radius)
	{
		mTimers[NEUTRAL].duration = neutralDuration;
		mTimers[OFFENSIVE].duration = offensiveDuration;
		mTimers[DEFENSIVE].duration = defensiveDuration;

		// I went a little over the top with the constructor arguments,
		// so I'm not going to both with customizable avoidance probes since these will suffice.
		mProbes.resize(4);
		mProbes[0].angle = 30.0f;
		mProbes[1].angle = 15.0f;
		mProbes[2].angle = -15.0f;
		mProbes[3].angle = -30.0f;
		mProbes[0].length = mProbes[3].length = 100.0f;
		mProbes[1].length = mProbes[2].length = 250.0f;
	}

	// (Unlocked in lab 8)
	// Player and Enemy must be created before trees can be created, so we need to set them explicity
	void SetTrees(Node* offensive, Node* defensive)
	{
		mTrees[OFFENSIVE] = offensive;
		mTrees[DEFENSIVE] = defensive;
	}

	void SetTree(Node* tree) { mTree = tree; }

	void Update(float dt)
	{
		// 1. State update
		OnStateUpdate(dt);

		// Test decision tree, make your own offensive and defensive trees for this lab!
		if (mTree != nullptr)
			Traverse(mTree);

		// (Unlocked in lab 8)
		// 2. Evaluate decision tree for current state
		if (mState < State::NEUTRAL)
			Traverse(mTrees[mState]);

		// 3. Physics update
		acc = acc + Avoid(*this, dt, mWorld.obstacles, mProbes);
		::Update(*this, dt);
		ResolveCircles(pos, mRadius, mWorld.obstacles);

		// 3.1 Reset acceleration and apply friction
		acc = {};
		vel = Normalize(vel) * Length(vel) * 0.95f;
	}

	void Change(State state)
	{
		OnStateExit();
		mState = state;
		OnStateEnter();
	}

private:
	World& mWorld;
	size_t mWaypointIndex = 0;
	const float mProximity;
	bool mPatrolClockwise = true;

	Probes mProbes;

	State mState = NEUTRAL;

	std::array<Timer, COUNT> mTimers{};
	Node* mTree = nullptr;

	// (Unlocked in lab 8)
	std::array<Node*, 2> mTrees{};

	void OnStateEnter()
	{
		// Reset new state's timer on-enter
		mTimers[mState].Reset();

		// if in NEUTRAL
		//		set patrol direction based on coin-toss
		//		set next waypoint index to that of the nearest waypoint

		// Debug logs:
		switch (mState)
		{
		case Enemy::OFFENSIVE:
			printf("Entering OFFENSIVE\n");
			break;

		case Enemy::DEFENSIVE:
			printf("Entering DEFENSIVE\n");
			break;

		case Enemy::NEUTRAL:
			printf("Entering NEUTRAL\n");
			break;
		}
	}

	void OnStateExit()
	{
		// No state-specific exit code should be necessary for anything in this course,
		// but don't let this comment stop you from getting creative!
	}

	void OnStateUpdate(float dt)
	{
		if (HealthPercent() < 0.25f && (mState == OFFENSIVE || mState == NEUTRAL))
		{
			Change(DEFENSIVE);
		}

		Timer& currentTimer = mTimers[mState];
		currentTimer.Tick(dt);

		if (currentTimer.Expired())
		{
			if (mState == OFFENSIVE || mState == DEFENSIVE)
			{
				Change(NEUTRAL);
			}
			else
			{
				if (std::rand() % 2 == 0)
				{
					Change(OFFENSIVE);
				}
				else
				{
					Change(DEFENSIVE);
				}
			}
		}

	
		if (mState == NEUTRAL)
		{
			if (std::rand() % 2 == 0)
			{
				mPatrolClockwise = true;
			}
			else
			{
				mPatrolClockwise = false;
			}

			size_t nearestWaypointIndex = 0;
			float shortestDistance = Distance(pos, mWorld.waypoints[0]);

			for (size_t i = 1; i < mWorld.waypoints.size(); ++i)
			{
				float distance = Distance(pos, mWorld.waypoints[i]);
				if (distance < shortestDistance)
				{
					shortestDistance = distance;
					nearestWaypointIndex = i;
				}
			}

			mWaypointIndex = nearestWaypointIndex;


			Vector2 desiredDirection = Normalize(mWorld.waypoints[mWaypointIndex] - pos);
			Vector2 steeringForce = Seek(mWorld.waypoints[mWaypointIndex], pos, vel, Speed());
			acc = steeringForce;
		}
	}


};
