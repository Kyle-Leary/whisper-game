#include "rigid_body.h"

#include "cglm/quat.h"
#include "cglm/vec3.h"
#include "global.h"
#include "helper_math.h"
#include "physics/body/body.h"
#include "physics/dynamics.h"
#include "physics/physics.h"
#include "physics/response.h"
#include "printers.h"
#include "util.h"
#include "whisper/queue.h"
#include <stdio.h>

// apply various central forces that all affect the object about its center of
// mass.
static void rb_apply_etc_forces(RigidBody *rb) {
  vec3 net_force;
  glm_vec3_zero(net_force); // starts at no change in acceleration.

  { // gravity
    vec3 grav_force = {0, -1, 0};

    // F = mg
    glm_vec3_scale(grav_force, rb->mass * GRAVITY_SCALE, grav_force);
    glm_vec3_add(net_force, grav_force, net_force);
  }

  { // damping
    vec3 damping_force;
    // linear damping is a force applied in the opposite direction of the
    // VELOCITY vector on the physics object.
    glm_vec3_scale(rb->velocity, -rb->linear_damping, damping_force);
    glm_vec3_add(net_force, damping_force, net_force);
  }

  {
#define GETRAND (((float)rand() / RAND_MAX) - 0.5)
    vec3 random = {GETRAND, GETRAND, GETRAND};
    glm_vec3_scale(random, 0.05, random);

#undef GETRAND
    glm_vec3_add(net_force, random, net_force);
  }

  float mag = glm_vec3_distance((vec3){0}, net_force);
  glm_vec3_normalize(net_force);

  rb_apply_force(rb, net_force, mag, rb->position);
  glm_vec3_scale(rb->acceleration, 0.85F, rb->acceleration);
}

static void rk4_position(RigidBody *rb) {
  vec3 k1_vel, k2_vel, k3_vel, k4_vel;
  vec3 k1_pos, k2_pos, k3_pos, k4_pos;
  vec3 tmp_vel, tmp_accel;

  // K1
  glm_vec3_scale(rb->acceleration, delta_time, k1_vel);
  glm_vec3_scale(rb->velocity, delta_time, k1_pos);

  // K2
  glm_vec3_scale(k1_vel, 0.5f, tmp_vel);
  glm_vec3_add(rb->velocity, tmp_vel, tmp_vel);
  glm_vec3_scale(k1_vel, 0.5f, tmp_accel);
  glm_vec3_add(rb->acceleration, tmp_accel, tmp_accel);
  glm_vec3_scale(tmp_accel, delta_time, k2_vel);
  glm_vec3_scale(tmp_vel, delta_time, k2_pos);

  // K3
  glm_vec3_scale(k2_vel, 0.5f, tmp_vel);
  glm_vec3_add(rb->velocity, tmp_vel, tmp_vel);
  glm_vec3_scale(k2_vel, 0.5f, tmp_accel);
  glm_vec3_add(rb->acceleration, tmp_accel, tmp_accel);
  glm_vec3_scale(tmp_accel, delta_time, k3_vel);
  glm_vec3_scale(tmp_vel, delta_time, k3_pos);
  // K4
  glm_vec3_add(rb->velocity, k3_vel, tmp_vel);
  glm_vec3_add(rb->acceleration, k3_vel, tmp_accel);
  glm_vec3_scale(tmp_accel, delta_time, k4_vel);
  glm_vec3_scale(tmp_vel, delta_time, k4_pos);

  // Combine
  for (int i = 0; i < 3; ++i) {
    rb->velocity[i] +=
        (k1_vel[i] + 2 * k2_vel[i] + 2 * k3_vel[i] + k4_vel[i]) / 6.0f;
    rb->position[i] +=
        (k1_pos[i] + 2 * k2_pos[i] + 2 * k3_pos[i] + k4_pos[i]) / 6.0f;
  }
}

static void versor_to_euler_angles(versor q, vec3 eulerAngles) {
  // 1. Compute some useful values
  float ysqr = q[1] * q[1];
  float t0 = -2.0f * (ysqr + q[2] * q[2]) + 1.0f;
  float t1 = +2.0f * (q[0] * q[1] - q[3] * q[2]);
  float t2 = -2.0f * (q[0] * q[2] + q[3] * q[1]);
  float t3 = +2.0f * (q[1] * q[2] - q[3] * q[0]);
  float t4 = -2.0f * (q[0] * q[0] + ysqr) + 1.0f;

  // 2. Handle singularities
  if (t2 > 1.0f)
    t2 = 1.0f;
  if (t2 < -1.0f)
    t2 = -1.0f;

  // 3. Compute pitch, roll, and yaw
  float pitch = asinf(t2);
  float roll = atan2f(t3, t4);
  float yaw = atan2f(t1, t0);

  // 4. Store in result vector and convert to degrees
  eulerAngles[0] = glm_deg(roll);
  eulerAngles[1] = glm_deg(pitch);
  eulerAngles[2] = glm_deg(yaw);
}

void euler_angles_to_versor(vec3 eulerAngles, versor q) {
  // Convert angles from degrees to radians
  float roll = glm_rad(eulerAngles[0]);
  float pitch = glm_rad(eulerAngles[1]);
  float yaw = glm_rad(eulerAngles[2]);

  // Compute individual quaternion components
  float cy = cosf(yaw * 0.5f);
  float sy = sinf(yaw * 0.5f);
  float cp = cosf(pitch * 0.5f);
  float sp = sinf(pitch * 0.5f);
  float cr = cosf(roll * 0.5f);
  float sr = sinf(roll * 0.5f);

  q[0] = cr * cp * cy + sr * sp * sy;
  q[1] = sr * cp * cy - cr * sp * sy;
  q[2] = cr * sp * cy + sr * cp * sy;
  q[3] = cr * cp * sy - sr * sp * cy;
}

static void rotation_tick(RigidBody *rb) {
  if (!rb->should_roll)
    return;

  // Integrate angular acceleration to get the new angular velocity
  rb->ang_velocity[0] += rb->ang_acceleration[0] * delta_time;
  rb->ang_velocity[1] += rb->ang_acceleration[1] * delta_time;
  rb->ang_velocity[2] += rb->ang_acceleration[2] * delta_time;

  // Apply angular damping
  rb->ang_velocity[0] *= (1 - rb->angular_damping * delta_time);
  rb->ang_velocity[1] *= (1 - rb->angular_damping * delta_time);
  rb->ang_velocity[2] *= (1 - rb->angular_damping * delta_time);

  // Compute the quaternion representing the change in rotation
  float angle = sqrt(rb->ang_velocity[0] * rb->ang_velocity[0] +
                     rb->ang_velocity[1] * rb->ang_velocity[1] +
                     rb->ang_velocity[2] * rb->ang_velocity[2]);

  if (angle == 0) {
    angle = 0.001;
  }

  // so this will always be normalized?
  vec3 axis = {rb->ang_velocity[0] / angle, rb->ang_velocity[1] / angle,
               rb->ang_velocity[2] / angle};

  versor delta_rotation;
  delta_rotation[3] = cos(angle * delta_time / 2);
  delta_rotation[0] = axis[0] * sin(angle * delta_time / 2);
  delta_rotation[1] = axis[1] * sin(angle * delta_time / 2);
  delta_rotation[2] = axis[2] * sin(angle * delta_time / 2);

  glm_quat_mul(rb->rotation, delta_rotation, rb->rotation);
}

static void position_lerp(RigidBody *rb) {
  lerp_vec3(rb->lerp_position, rb->position, rb->position_lerp_speed,
            rb->lerp_position);
}

void single_rb_dynamics(RigidBody *rb) {
  rb_apply_etc_forces(rb);
  rk4_position(rb);
  position_lerp(rb);
  rotation_tick(rb);
}
void single_rb_response(RigidBody *rb, WQueue collider_events) {}

void rb_apply_force(RigidBody *rb, vec3 direction, float magnitude,
                    vec3 contact_pt) {
  if (rb->frozen) {
    return;
  }

  // F = ma <=> F/m = a
  if (rb->mass == 0) {
    fprintf(
        stderr,
        "ERR: division by zero in force application since comp->mass is zero "
        "on object %p. Offsetting the mass slightly...\n",
        rb);
    rb->mass = 0.001F; // lol
  }

  vec3 force;
  glm_vec3_scale(direction, magnitude, force);

  glm_vec3_scale(force, (1 / rb->mass), force);
  glm_vec3_add(force, rb->acceleration, rb->acceleration);
}
