#pragma once
#include "Timer.h"

struct World;
class Entity;
class Player;
class Enemy;

class Node
{
public:
    Node(Entity& enemy, Entity& player, World& world);
    virtual Node* Evaluate() = 0;
protected:
    Entity& mEnemy;
    Entity& mPlayer;
    World& mWorld;
};

class ActionNode : public Node
{
public:
    ActionNode(Entity& enemy, Entity& player, World& world);
    Node* Evaluate() override { return next; }
    ActionNode* next = nullptr;
};

class DecisionNode : public Node
{
public:
    DecisionNode(Entity& enemy, Entity& player, World& world);
    Node* yes = nullptr;
    Node* no = nullptr;
};

class ProximityNode : public DecisionNode
{
public:
    ProximityNode(Entity& enemy, Entity& player, World& world, float proximity);
    Node* Evaluate() final;

    float Proximity() { return mProximity; }
private:
    const float mProximity;
};

class VisibilityNode : public DecisionNode
{
public:
    VisibilityNode(Entity& enemy, Entity& player, World& world, float viewDistance);
    Node* Evaluate() final;

    float ViewDistance() { return mViewDistance; }
private:
    const float mViewDistance;
};

class SeekAction : public ActionNode
{
public:
    SeekAction(Entity& enemy, Entity& player, World& world, float speed);
    Node* Evaluate() final;
private:
    const float mSpeed;
};

class FleeAction : public ActionNode
{
public:
    FleeAction(Entity& enemy, Entity& player, World& world, float speed);
    Node* Evaluate() final;
private:
    const float mSpeed;
};

class VisibilityAction : public ActionNode
{
public:
    VisibilityAction(Entity& enemy, Entity& player, World& world, float viewDistance);
    Node* Evaluate() final;
private:
    const float mViewDistance;
};

class CoverAction : public ActionNode
{
public:
    CoverAction(Entity& enemy, Entity& player, World& world, float viewDistance);
    Node* Evaluate() final;
private:
    const float mViewDistance;
};

class ShootAction : public ActionNode
{
public:
    ShootAction(Entity& enemy, Entity& player, World& world, float cooldown);
    Node* Evaluate() final;
private:
    Timer mTimer;
};

void Traverse(Node* node);