#include "Nodes.h"
#include "raylib.h"
#include "Game.h"
#include <cassert>

Node::Node(Entity& enemy, Entity& player, World& world) :
    mEnemy(enemy), mPlayer(player), mWorld(world) {}

ActionNode::ActionNode(Entity& enemy, Entity& player, World& world) :
    Node(enemy, player, world) {}

DecisionNode::DecisionNode(Entity& enemy, Entity& player, World& world) :
    Node(enemy, player, world) {}

ProximityNode::ProximityNode(Entity& enemy, Entity& player, World& world, float proximity) :
    DecisionNode(enemy, player, world), mProximity(proximity) {}

VisibilityNode::VisibilityNode(Entity& enemy, Entity& player, World& world, float viewDistance) :
    DecisionNode(enemy, player, world), mViewDistance(viewDistance) {}

SeekAction::SeekAction(Entity& enemy, Entity& player, World& world, float speed) :
    ActionNode(enemy, player, world), mSpeed(speed) {}

FleeAction::FleeAction(Entity& enemy, Entity& player, World& world, float speed) :
    ActionNode(enemy, player, world), mSpeed(speed) {}

VisibilityAction::VisibilityAction(Entity& enemy, Entity& player, World& world, float viewDistance) :
    ActionNode(enemy, player, world), mViewDistance(viewDistance) {}

CoverAction::CoverAction(Entity& enemy, Entity& player, World& world, float viewDistance) :
    ActionNode(enemy, player, world), mViewDistance(viewDistance) {}

ShootAction::ShootAction(Entity& enemy, Entity& player, World& world, float cooldown) :
    ActionNode(enemy, player, world)
{
    mTimer.duration = cooldown;
}

Node* ProximityNode::Evaluate()
{
    return DistanceSqr(mEnemy.pos, mPlayer.pos) <= mProximity * mProximity ? yes : no;
}

Node* VisibilityNode::Evaluate()
{
    return IsVisible(mPlayer.pos, mViewDistance, mEnemy.pos, mEnemy.Radius(), mWorld.obstacles) ? yes : no;
}

void Traverse(Node* node)
{
    if (node != nullptr)
    {
        Traverse(node->Evaluate());
    }
}

Node* SeekAction::Evaluate()
{
    mEnemy.acc = mEnemy.acc + Seek(mPlayer.pos, mEnemy.pos, mEnemy.vel, mSpeed);
    return ActionNode::Evaluate();
}

Node* FleeAction::Evaluate()
{
    mEnemy.acc = mEnemy.acc + Flee(mPlayer.pos, mEnemy.pos, mEnemy.vel, mSpeed);
    return ActionNode::Evaluate();
}

Node* VisibilityAction::Evaluate()
{
    // Find PoV's nearest to player since tree requires enemy to be near player before finding visibility
    Points visiblePoints = mWorld.waypoints;
    DistanceSort(mPlayer.pos, visiblePoints);
    for (const Vector2& point : visiblePoints)
    {
        if (IsVisible(point, mViewDistance, mPlayer.pos, mPlayer.Radius(), mWorld.obstacles))
        {
            mEnemy.acc = mEnemy.acc + Seek(point, mEnemy.pos, mEnemy.vel, mEnemy.Speed());
            break;
        }
    }
    return ActionNode::Evaluate();
}

Node* CoverAction::Evaluate()
{
    // Seek furthest cover point from player
    Points coverPoints = mWorld.waypoints;
    DistanceSort(mPlayer.pos, coverPoints);
    for (size_t i = coverPoints.size() - 1; i > 0; i--)
    {
        const Vector2& point = coverPoints[i];
        if (!IsVisible(point, mViewDistance, mPlayer.pos, mPlayer.Radius(), mWorld.obstacles))
        {
            mEnemy.acc = mEnemy.acc + Seek(point, mEnemy.pos, mEnemy.vel, mEnemy.Speed());
            break;
        }
    }
    return ActionNode::Evaluate();
}

Node* ShootAction::Evaluate()
{
    mTimer.Tick(GetFrameTime());
    if (mTimer.Expired())
    {
        mTimer.Reset();
        Vector2 direction = Normalize(mPlayer.pos - mEnemy.pos);

        Bullet bullet(Bullet::ENEMY);
        bullet.dir = direction;
        bullet.vel = direction * bullet.Speed();
        bullet.pos = mEnemy.pos + direction * (mEnemy.Radius() + bullet.Radius());
        mWorld.bullets.push_back(bullet);
    }
    return ActionNode::Evaluate();
}
