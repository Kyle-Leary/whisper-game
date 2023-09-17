#pragma once

#include "body.h"
#include "physics/detection.h"
#include "whisper/queue.h"

void single_rb_dynamics(RigidBody *rb);
void single_rb_response(RigidBody *rb, WQueue collider_events);

void rb_apply_torque(RigidBody *rb, vec3 direction, float magnitude,
                     vec3 contact_pt);
void rb_apply_impulse(RigidBody *rb, vec3 impulse, vec3 contact_pt);
void rb_apply_force(RigidBody *rb, vec3 direction, float magnitude,
                    vec3 contact_pt);
