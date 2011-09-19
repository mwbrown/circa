// Copyright (c) Paul Hodge. See LICENSE file for license terms.

#include "Box2D/Box2D.h"

#include "circa.h"
#include "gc.h"
#include "list_shared.h"
#include "types/handle.h"

using namespace circa;

#define LOG_HANDLE_CREATION 0

b2World *g_world = NULL;
b2Body *g_groundBody = NULL; // used for mouse joints

double c_timestep = 1.0/60.0;
int c_velocityIterations = 6;
int c_positionIterations = 2;

// Scale between screen & world coordinates. Box2d says that it likes objects
// to have a size between 0.1 and 10 units.
double screen_to_world(double screen) { return screen / 10.0; }
double world_to_screen(double world) { return world * 10.0; }

double radians_to_degrees(double radians) { return radians * 180.0 / M_PI; }
double degrees_to_radians(double unit) { return unit * M_PI / 180.0; }

void b2Vec2_to_point(b2Vec2 const& vec, TaggedValue* point)
{
    set_point(point, world_to_screen(vec.x), world_to_screen(vec.y));
}

b2Vec2 point_to_b2Vec2(TaggedValue* point)
{
    float x,y;
    get_point(point, &x, &y);
    return b2Vec2(screen_to_world(x), screen_to_world(y));
}

// The Body type is used to hold on to b2Body objects.
Type* g_body_t;
Type* g_mouseJoint_t;

ObjectList g_bodyHandles;
ObjectList g_mouseJoints;

struct Body
{
    ObjectHeader header;
    ObjectListElement bodyHandleList;
    b2Body* body;

    Body() : header("Body"), body(NULL)
    {
        append_to_object_list(&g_bodyHandles, &bodyHandleList, &header);
    }

    ~Body() {
        #if LOG_HANDLE_CREATION
        std::cout << "Destroyed " << body << std::endl;
        #endif
        g_world->DestroyBody(body);

        remove_from_object_list(&g_bodyHandles, &bodyHandleList);
    }
};

struct MouseJoint
{
    ObjectHeader header;
    b2MouseJoint* joint;
    ObjectListElement jointList;

    MouseJoint(b2MouseJoint* _joint) : header("MouseJoint"), joint(_joint) {
        append_to_object_list(&g_mouseJoints, &jointList, &header);
    }

    ~MouseJoint() {
        if (joint)
            g_world->DestroyJoint(joint);
        remove_from_object_list(&g_mouseJoints, &jointList);
    }
};

class DestructionListener : public b2DestructionListener
{
    virtual void SayGoodbye(b2Joint* joint)
    {
        std::cout << "SayGoodbye: " << joint << std::endl;
        ObjectListElement* element = g_mouseJoints.first;
        while (element != NULL) {
            MouseJoint* jointWrapper = (MouseJoint*) element->obj;
            if (jointWrapper->joint == joint)
                jointWrapper->joint = NULL;

            element = element->next;
        }
    }
    virtual void SayGoodbye(b2Fixture* fixture)
    {
    }
} g_destructionListener;

extern "C" {

void initialize_world()
{
    if (g_world == NULL) {
        g_world = new b2World(b2Vec2(), false);
        g_world->SetDestructionListener(&g_destructionListener);
    }
}

b2Body* get_body_from_handle(TaggedValue* value)
{
    Body* bodyHandle = (Body*) handle_t::get_ptr(value);
    if (bodyHandle == NULL)
        return NULL;
    return bodyHandle->body;
}

bool is_valid_body_handle(TaggedValue* value)
{
    return value->value_type == g_body_t
        && get_body_from_handle(value) != NULL;
}

CA_FUNCTION(box2d__step)
{
    initialize_world();

    g_world->Step(c_timestep, c_velocityIterations, c_positionIterations);
    g_world->ClearForces();
}

CA_FUNCTION(box2d__gravity)
{
    initialize_world();

    Point& gravity = *Point::checkCast(INPUT(0));
    g_world->SetGravity(b2Vec2(gravity.getX(), gravity.getY()));
}

CA_FUNCTION(box2d__create_body)
{
    // Inputs:
    //   int bodyType
    //   Point initialPosition
    //   number initialRotation
    //
    // Outputs:
    //   BodyPtr handle
    
    initialize_world();

    int bodyType = INT_INPUT(0);

    TaggedValue* initialPosition = INPUT(1);
    float initialRotation = as_float(INPUT(2));

    // Create a b2Body
    b2BodyDef bodyDef;

    switch (bodyType) {
    case 0: bodyDef.type = b2_staticBody; break;
    case 1: bodyDef.type = b2_dynamicBody; break;
    case 2: bodyDef.type = b2_kinematicBody; break;
    }
    
    bodyDef.position = point_to_b2Vec2(initialPosition);
    bodyDef.angle = degrees_to_radians(initialRotation);

    Body* bodyHandle = handle_t::create<Body>(OUTPUT, g_body_t);

    bodyHandle->body = g_world->CreateBody(&bodyDef);

    #if LOG_HANDLE_CREATION
    std::cout << "Created " << bodyHandle->body << std::endl;
    #endif
}

CA_FUNCTION(box2d__Body_set_fixtures)
{
    // Inputs:
    //   BodyPtr handle
    //   List fixtureDefs:
    //     int type
    //     either int or Point - type-specific data
    //     float density
    //     float friction
    //     float restitution
    b2Body* body = get_body_from_handle(INPUT(0));

    if (body == NULL)
        return error_occurred(CONTEXT, CALLER, "invalid body handle");

    // Remove any old fixtures
    while (body->GetFixtureList())
        body->DestroyFixture(body->GetFixtureList());

    // Append new fixtures according to the fixtureDefs list
    List& fixtureDefs = *List::checkCast(INPUT(1));

    for (int i=0; i < fixtureDefs.length(); i++) {

        List& fixtureDefIn = *List::checkCast(fixtureDefs[i]);

        int shapeType = as_int(fixtureDefIn[0]);

        b2FixtureDef fixtureDef;
        b2PolygonShape polygonShape;
        b2CircleShape circleShape;

        if (shapeType == 0) {
            Point& size = *Point::checkCast(fixtureDefIn[1]);
            polygonShape.SetAsBox(
                screen_to_world(size.getX()),
                screen_to_world(size.getY()));
            fixtureDef.shape = &polygonShape;
        } else if (shapeType == 1) {
            circleShape.m_radius = screen_to_world(to_float(fixtureDefIn[1]));
            fixtureDef.shape = &circleShape;
        }

        fixtureDef.density = to_float(fixtureDefIn[2]);
        fixtureDef.friction = to_float(fixtureDefIn[3]);
        fixtureDef.restitution = to_float(fixtureDefIn[4]);

        body->CreateFixture(&fixtureDef);
    }
}

CA_FUNCTION(box2d__Body_get_points)
{
    b2Body* body = get_body_from_handle(INPUT(0));
    if (body == NULL)
        return;
    
    List& points = *List::cast(OUTPUT, 0);

    // Copy vertex locations
    for (b2Fixture* fixture = body->GetFixtureList(); fixture; fixture = fixture->GetNext()) {
        b2Shape* shape = fixture->GetShape();

        if (shape->GetType() == b2Shape::e_polygon) {
            b2PolygonShape *poly = (b2PolygonShape*) shape;

            int count = poly->GetVertexCount();

            for (int i = 0; i < count; i++) {
                b2Vec2 vec = body->GetWorldPoint(poly->GetVertex(i));
                b2Vec2_to_point(vec, points.append());
            }
        }
    }
}

CA_FUNCTION(box2d__Body_get_position)
{
    b2Body* body = get_body_from_handle(INPUT(0));
    if (body == NULL)
        return;

    b2Vec2_to_point(body->GetPosition(), OUTPUT);
}

CA_FUNCTION(box2d__Body_set_position)
{
    b2Body* body = get_body_from_handle(INPUT(0));
    if (body == NULL)
        return;

    body->SetTransform(point_to_b2Vec2(INPUT(1)), body->GetAngle());
}

CA_FUNCTION(box2d__Body_get_rotation)
{
    b2Body* body = get_body_from_handle(INPUT(0));
    if (body == NULL)
        return;

    set_float(OUTPUT, radians_to_degrees(body->GetAngle()));
}

CA_FUNCTION(box2d__Body_set_rotation)
{
    b2Body* body = get_body_from_handle(INPUT(0));
    if (body == NULL)
        return;

    float rotation = degrees_to_radians(FLOAT_INPUT(1));

    body->SetTransform(body->GetPosition(), rotation);
}

CA_FUNCTION(box2d__Body_get_linear_velocity)
{
    b2Body* body = get_body_from_handle(INPUT(0));
    if (body == NULL)
        return;

    b2Vec2_to_point(body->GetLinearVelocity(), OUTPUT);
}

CA_FUNCTION(box2d__Body_set_linear_velocity)
{
    b2Body* body = get_body_from_handle(INPUT(0));
    if (body == NULL)
        return;

    body->SetLinearVelocity(point_to_b2Vec2(INPUT(1)));
}

CA_FUNCTION(box2d__Body_apply_force)
{
    b2Body* body = get_body_from_handle(INPUT(0));
    if (body == NULL)
        return;

    body->ApplyForce(point_to_b2Vec2(INPUT(1)), point_to_b2Vec2(INPUT(2)));
}
CA_FUNCTION(box2d__Body_apply_torque)
{
    b2Body* body = get_body_from_handle(INPUT(0));
    if (body == NULL)
        return;

    body->ApplyTorque(FLOAT_INPUT(1));
}
CA_FUNCTION(box2d__Body_apply_linear_impulse)
{
    b2Body* body = get_body_from_handle(INPUT(0));
    if (body == NULL)
        return;

    body->ApplyLinearImpulse(point_to_b2Vec2(INPUT(1)), point_to_b2Vec2(INPUT(2)));
}
CA_FUNCTION(box2d__Body_apply_angular_impulse)
{
    b2Body* body = get_body_from_handle(INPUT(0));
    if (body == NULL)
        return;

    body->ApplyAngularImpulse(FLOAT_INPUT(1));
}

CA_FUNCTION(box2d__body_contains_point)
{
    b2Body* body = get_body_from_handle(INPUT(0));
    if (body == NULL)
        return;

    b2Vec2 p = point_to_b2Vec2(INPUT(1));

    b2Fixture* fixture = body->GetFixtureList();

    while (fixture) {
        if (fixture->TestPoint(p)) {
            set_bool(OUTPUT, true);
            return;
        }
        fixture = fixture->GetNext();
    }

    set_bool(OUTPUT, false);
}

CA_FUNCTION(box2d__Body_create_mouse_joint)
{
    b2Body* body = get_body_from_handle(INPUT(0));
    b2MouseJointDef def;

    if (g_groundBody == NULL) {
        b2BodyDef bodyDef;
        g_groundBody = g_world->CreateBody(&bodyDef);
    }

    def.bodyA = g_groundBody;
    def.bodyB = body;
    def.target = point_to_b2Vec2(INPUT(1));
    def.maxForce = 1000.0f * body->GetMass();

    b2MouseJoint* joint = (b2MouseJoint*) g_world->CreateJoint(&def);
    handle_t::set(OUTPUT, g_mouseJoint_t, (void*) new MouseJoint(joint));

    body->SetAwake(true);
}

CA_FUNCTION(box2d__MouseJoint_set_target)
{
    MouseJoint* jointHandle = (MouseJoint*) handle_t::get_ptr(INPUT(0));
    if (jointHandle->joint == NULL)
        return;
    jointHandle->joint->SetTarget(point_to_b2Vec2(INPUT(1)));
    set_null(OUTPUT);
}

void on_load(Branch* branch)
{
    g_body_t = get_declared_type(branch, "box2d:Body");
    g_mouseJoint_t = get_declared_type(branch, "box2d:MouseJoint");

    handle_t::setup_type<Body>(g_body_t);
    handle_t::setup_type<MouseJoint>(g_mouseJoint_t);
}

} // extern "C"
