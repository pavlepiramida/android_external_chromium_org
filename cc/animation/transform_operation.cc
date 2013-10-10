// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Needed on Windows to get |M_PI| from <cmath>
#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif

#include <algorithm>
#include <cmath>
#include <limits>

#include "base/logging.h"
#include "cc/animation/transform_operation.h"
#include "ui/gfx/box_f.h"
#include "ui/gfx/vector3d_f.h"

namespace {
const SkMScalar kAngleEpsilon = 1e-4f;
}

namespace cc {

bool TransformOperation::IsIdentity() const {
  return matrix.IsIdentity();
}

static bool IsOperationIdentity(const TransformOperation* operation) {
  return !operation || operation->IsIdentity();
}

static bool ShareSameAxis(const TransformOperation* from,
                          const TransformOperation* to,
                          SkMScalar* axis_x,
                          SkMScalar* axis_y,
                          SkMScalar* axis_z,
                          SkMScalar* angle_from) {
  if (IsOperationIdentity(from) && IsOperationIdentity(to))
    return false;

  if (IsOperationIdentity(from) && !IsOperationIdentity(to)) {
    *axis_x = to->rotate.axis.x;
    *axis_y = to->rotate.axis.y;
    *axis_z = to->rotate.axis.z;
    *angle_from = 0;
    return true;
  }

  if (!IsOperationIdentity(from) && IsOperationIdentity(to)) {
    *axis_x = from->rotate.axis.x;
    *axis_y = from->rotate.axis.y;
    *axis_z = from->rotate.axis.z;
    *angle_from = from->rotate.angle;
    return true;
  }

  SkMScalar length_2 = from->rotate.axis.x * from->rotate.axis.x +
                       from->rotate.axis.y * from->rotate.axis.y +
                       from->rotate.axis.z * from->rotate.axis.z;
  SkMScalar other_length_2 = to->rotate.axis.x * to->rotate.axis.x +
                             to->rotate.axis.y * to->rotate.axis.y +
                             to->rotate.axis.z * to->rotate.axis.z;

  if (length_2 <= kAngleEpsilon || other_length_2 <= kAngleEpsilon)
    return false;

  SkMScalar dot = to->rotate.axis.x * from->rotate.axis.x +
                  to->rotate.axis.y * from->rotate.axis.y +
                  to->rotate.axis.z * from->rotate.axis.z;
  SkMScalar error =
      std::abs(SK_MScalar1 - (dot * dot) / (length_2 * other_length_2));
  bool result = error < kAngleEpsilon;
  if (result) {
    *axis_x = to->rotate.axis.x;
    *axis_y = to->rotate.axis.y;
    *axis_z = to->rotate.axis.z;
    // If the axes are pointing in opposite directions, we need to reverse
    // the angle.
    *angle_from = dot > 0 ? from->rotate.angle : -from->rotate.angle;
  }
  return result;
}

static SkMScalar BlendSkMScalars(SkMScalar from,
                                 SkMScalar to,
                                 SkMScalar progress) {
  return from * (1 - progress) + to * progress;
}

bool TransformOperation::BlendTransformOperations(
    const TransformOperation* from,
    const TransformOperation* to,
    SkMScalar progress,
    gfx::Transform* result) {
  if (IsOperationIdentity(from) && IsOperationIdentity(to))
    return true;

  TransformOperation::Type interpolation_type =
      TransformOperation::TransformOperationIdentity;
  if (IsOperationIdentity(to))
    interpolation_type = from->type;
  else
    interpolation_type = to->type;

  switch (interpolation_type) {
  case TransformOperation::TransformOperationTranslate: {
    SkMScalar from_x = IsOperationIdentity(from) ? 0 : from->translate.x;
    SkMScalar from_y = IsOperationIdentity(from) ? 0 : from->translate.y;
    SkMScalar from_z = IsOperationIdentity(from) ? 0 : from->translate.z;
    SkMScalar to_x = IsOperationIdentity(to) ? 0 : to->translate.x;
    SkMScalar to_y = IsOperationIdentity(to) ? 0 : to->translate.y;
    SkMScalar to_z = IsOperationIdentity(to) ? 0 : to->translate.z;
    result->Translate3d(BlendSkMScalars(from_x, to_x, progress),
                        BlendSkMScalars(from_y, to_y, progress),
                        BlendSkMScalars(from_z, to_z, progress));
    break;
  }
  case TransformOperation::TransformOperationRotate: {
    SkMScalar axis_x = 0;
    SkMScalar axis_y = 0;
    SkMScalar axis_z = 1;
    SkMScalar from_angle = 0;
    SkMScalar to_angle = IsOperationIdentity(to) ? 0 : to->rotate.angle;
    if (ShareSameAxis(from, to, &axis_x, &axis_y, &axis_z, &from_angle)) {
      result->RotateAbout(gfx::Vector3dF(axis_x, axis_y, axis_z),
                          BlendSkMScalars(from_angle, to_angle, progress));
    } else {
      gfx::Transform to_matrix;
      if (!IsOperationIdentity(to))
        to_matrix = to->matrix;
      gfx::Transform from_matrix;
      if (!IsOperationIdentity(from))
        from_matrix = from->matrix;
      *result = to_matrix;
      if (!result->Blend(from_matrix, progress))
        return false;
    }
    break;
  }
  case TransformOperation::TransformOperationScale: {
    SkMScalar from_x = IsOperationIdentity(from) ? 1 : from->scale.x;
    SkMScalar from_y = IsOperationIdentity(from) ? 1 : from->scale.y;
    SkMScalar from_z = IsOperationIdentity(from) ? 1 : from->scale.z;
    SkMScalar to_x = IsOperationIdentity(to) ? 1 : to->scale.x;
    SkMScalar to_y = IsOperationIdentity(to) ? 1 : to->scale.y;
    SkMScalar to_z = IsOperationIdentity(to) ? 1 : to->scale.z;
    result->Scale3d(BlendSkMScalars(from_x, to_x, progress),
                    BlendSkMScalars(from_y, to_y, progress),
                    BlendSkMScalars(from_z, to_z, progress));
    break;
  }
  case TransformOperation::TransformOperationSkew: {
    SkMScalar from_x = IsOperationIdentity(from) ? 0 : from->skew.x;
    SkMScalar from_y = IsOperationIdentity(from) ? 0 : from->skew.y;
    SkMScalar to_x = IsOperationIdentity(to) ? 0 : to->skew.x;
    SkMScalar to_y = IsOperationIdentity(to) ? 0 : to->skew.y;
    result->SkewX(BlendSkMScalars(from_x, to_x, progress));
    result->SkewY(BlendSkMScalars(from_y, to_y, progress));
    break;
  }
  case TransformOperation::TransformOperationPerspective: {
    SkMScalar from_perspective_depth =
        IsOperationIdentity(from) ? std::numeric_limits<SkMScalar>::max()
                                  : from->perspective_depth;
    SkMScalar to_perspective_depth = IsOperationIdentity(to)
                                         ? std::numeric_limits<SkMScalar>::max()
                                         : to->perspective_depth;
    result->ApplyPerspectiveDepth(BlendSkMScalars(
        from_perspective_depth, to_perspective_depth, progress));
    break;
  }
  case TransformOperation::TransformOperationMatrix: {
    gfx::Transform to_matrix;
    if (!IsOperationIdentity(to))
      to_matrix = to->matrix;
    gfx::Transform from_matrix;
    if (!IsOperationIdentity(from))
      from_matrix = from->matrix;
    *result = to_matrix;
    if (!result->Blend(from_matrix, progress))
      return false;
    break;
  }
  case TransformOperation::TransformOperationIdentity:
    // Do nothing.
    break;
  }

  return true;
}

static void ApplyScaleToBox(float x_scale,
                            float y_scale,
                            float z_scale,
                            gfx::BoxF* box) {
  if (x_scale < 0)
    box->set_x(-box->right());
  if (y_scale < 0)
    box->set_y(-box->bottom());
  if (z_scale < 0)
    box->set_z(-box->front());
  box->Scale(std::abs(x_scale), std::abs(y_scale), std::abs(z_scale));
}

static void UnionBoxWithZeroScale(gfx::BoxF* box) {
  float min_x = std::min(box->x(), 0.f);
  float min_y = std::min(box->y(), 0.f);
  float min_z = std::min(box->z(), 0.f);
  float max_x = std::max(box->right(), 0.f);
  float max_y = std::max(box->bottom(), 0.f);
  float max_z = std::max(box->front(), 0.f);
  *box = gfx::BoxF(
      min_x, min_y, min_z, max_x - min_x, max_y - min_y, max_z - min_z);
}

// If p = (px, py) is a point in the plane being rotated about (0, 0, nz), this
// function computes the angles we would have to rotate from p to get to
// (length(p), 0), (-length(p), 0), (0, length(p)), (0, -length(p)). If nz is
// negative, these angles will need to be reversed.
static void FindCandidatesInPlane(float px,
                                  float py,
                                  float nz,
                                  double* candidates,
                                  int* num_candidates) {
  double phi = atan2(px, py);
  *num_candidates = 4;
  candidates[0] = phi;
  for (int i = 1; i < *num_candidates; ++i)
    candidates[i] = candidates[i - 1] + M_PI_2;
  if (nz < 0.f) {
    for (int i = 0; i < *num_candidates; ++i)
      candidates[i] *= -1.f;
  }
}

static float RadiansToDegrees(float radians) {
  return (180.f * radians) / M_PI;
}

static float DegreesToRadians(float degrees) {
  return (M_PI * degrees) / 180.f;
}

// Div by zero doesn't always result in Inf as you might hope, so we'll do this
// explicitly here.
static float SafeDivide(float numerator, float denominator) {
  if (numerator == 0.f)
    return 0.f;

  if (denominator == 0.f) {
    return numerator > 0.f ? std::numeric_limits<float>::infinity()
                           : -std::numeric_limits<float>::infinity();
  }

  return numerator / denominator;
}

static void BoundingBoxForArc(const gfx::Point3F& point,
                              const TransformOperation* from,
                              const TransformOperation* to,
                              gfx::BoxF* box) {
  const TransformOperation* exemplar = from ? from : to;
  *box = gfx::BoxF();
  gfx::Point3F point_rotated_from = point;
  if (from)
    from->matrix.TransformPoint(&point_rotated_from);
  gfx::Point3F point_rotated_to = point;
  if (to)
    to->matrix.TransformPoint(&point_rotated_to);

  box->set_origin(point_rotated_from);
  box->ExpandTo(point_rotated_to);

  const bool x_is_zero = exemplar->rotate.axis.x == 0.f;
  const bool y_is_zero = exemplar->rotate.axis.y == 0.f;
  const bool z_is_zero = exemplar->rotate.axis.z == 0.f;

  // We will have at most 6 angles to test (excluding from->angle and
  // to->angle).
  static const int kMaxNumCandidates = 6;
  double candidates[kMaxNumCandidates];
  int num_candidates = kMaxNumCandidates;

  if (x_is_zero && y_is_zero && z_is_zero)
    return;

  if (x_is_zero && y_is_zero) {
    FindCandidatesInPlane(point.x(),
                          point.y(),
                          exemplar->rotate.axis.z,
                          candidates,
                          &num_candidates);
  } else if (x_is_zero && z_is_zero) {
    FindCandidatesInPlane(point.z(),
                          point.x(),
                          exemplar->rotate.axis.y,
                          candidates,
                          &num_candidates);
  } else if (y_is_zero && z_is_zero) {
    FindCandidatesInPlane(point.y(),
                          point.z(),
                          exemplar->rotate.axis.x,
                          candidates,
                          &num_candidates);
  } else {
    gfx::Vector3dF normal(exemplar->rotate.axis.x,
                          exemplar->rotate.axis.y,
                          exemplar->rotate.axis.z);
    float normal_length = normal.Length();
    if (normal_length == 0.f)
      return;

    normal.Scale(1.f / normal_length);

    // First, find center of rotation.
    gfx::Point3F origin;
    gfx::Vector3dF to_point = point - origin;
    gfx::Point3F center =
        origin + gfx::ScaleVector3d(normal, gfx::DotProduct(to_point, normal));

    // Now we need to find two vectors in the plane of rotation. One pointing
    // towards point and another, perpendicular vector in the plane.
    gfx::Vector3dF v1 = point - center;
    float v1_length = v1.Length();
    if (v1_length == 0.f)
      return;

    v1.Scale(1.f / v1_length);

    gfx::Vector3dF v2 = gfx::CrossProduct(normal, v1);

    // Now figure out where (1, 0, 0) and (0, 0, 1) project on the rotation
    // plane.
    gfx::Point3F px(1.f, 0.f, 0.f);
    gfx::Vector3dF to_px = px - center;
    gfx::Point3F px_projected =
        px - gfx::ScaleVector3d(normal, gfx::DotProduct(to_px, normal));
    gfx::Vector3dF vx = px_projected - origin;

    gfx::Point3F pz(0.f, 0.f, 1.f);
    gfx::Vector3dF to_pz = pz - center;
    gfx::Point3F pz_projected =
        pz - ScaleVector3d(normal, gfx::DotProduct(to_pz, normal));
    gfx::Vector3dF vz = pz_projected - origin;

    double phi_x = atan2(gfx::DotProduct(v2, vx), gfx::DotProduct(v1, vx));
    double phi_z = atan2(gfx::DotProduct(v2, vz), gfx::DotProduct(v1, vz));

    // NB: it is fine if the denominators here are zero and these values go to
    // infinity; atan can handle it.
    double tan_theta1 = SafeDivide(normal.y(), (normal.x() * normal.z()));
    double tan_theta2 = SafeDivide(-normal.z(), (normal.x() * normal.y()));

    candidates[0] = atan(tan_theta1) + phi_x;
    candidates[1] = candidates[0] + M_PI;
    candidates[2] = atan(tan_theta2) + phi_x;
    candidates[3] = candidates[2] + M_PI;
    candidates[4] = atan(-tan_theta1) + phi_z;
    candidates[5] = candidates[4] + M_PI;
  }

  double min_theta = from ? DegreesToRadians(from->rotate.angle) : 0.0;
  double max_theta = to ? DegreesToRadians(to->rotate.angle) : 0.0;

  if (min_theta > max_theta)
    std::swap(min_theta, max_theta);

  gfx::Vector3dF axis(exemplar->rotate.axis.x,
                      exemplar->rotate.axis.y,
                      exemplar->rotate.axis.z);
  axis.Scale(1.f / axis.Length());

  for (int i = 0; i < num_candidates; ++i) {
    double theta = candidates[i];
    while (theta < min_theta)
      theta += 2.0 * M_PI;
    while (theta > max_theta)
      theta -= 2.0 * M_PI;
    if (theta < min_theta)
      continue;

    gfx::Transform rotation;
    rotation.RotateAbout(axis, RadiansToDegrees(theta));
    gfx::Point3F rotated = point;
    rotation.TransformPoint(&rotated);

    box->ExpandTo(rotated);
  }
}

bool TransformOperation::BlendedBoundsForBox(const gfx::BoxF& box,
                                             const TransformOperation* from,
                                             const TransformOperation* to,
                                             SkMScalar min_progress,
                                             SkMScalar max_progress,
                                             gfx::BoxF* bounds) {
  bool is_identity_from = IsOperationIdentity(from);
  bool is_identity_to = IsOperationIdentity(to);
  if (is_identity_from && is_identity_to) {
    *bounds = box;
    return true;
  }

  TransformOperation::Type interpolation_type =
      TransformOperation::TransformOperationIdentity;
  if (is_identity_to)
    interpolation_type = from->type;
  else
    interpolation_type = to->type;

  switch (interpolation_type) {
    case TransformOperation::TransformOperationTranslate: {
      SkMScalar from_x, from_y, from_z;
      if (is_identity_from) {
        from_x = from_y = from_z = 0.0;
      } else {
        from_x = from->translate.x;
        from_y = from->translate.y;
        from_z = from->translate.z;
      }
      SkMScalar to_x, to_y, to_z;
      if (is_identity_to) {
        to_x = to_y = to_z = 0.0;
      } else {
        to_x = to->translate.x;
        to_y = to->translate.y;
        to_z = to->translate.z;
      }
      *bounds = box;
      *bounds += gfx::Vector3dF(BlendSkMScalars(from_x, to_x, min_progress),
                                BlendSkMScalars(from_y, to_y, min_progress),
                                BlendSkMScalars(from_z, to_z, min_progress));
      gfx::BoxF bounds_max = box;
      bounds_max += gfx::Vector3dF(BlendSkMScalars(from_x, to_x, max_progress),
                                   BlendSkMScalars(from_y, to_y, max_progress),
                                   BlendSkMScalars(from_z, to_z, max_progress));
      bounds->Union(bounds_max);
      return true;
    }
    case TransformOperation::TransformOperationScale: {
      SkMScalar from_x, from_y, from_z;
      if (is_identity_from) {
        from_x = from_y = from_z = 1.0;
      } else {
        from_x = from->scale.x;
        from_y = from->scale.y;
        from_z = from->scale.z;
      }
      SkMScalar to_x, to_y, to_z;
      if (is_identity_to) {
        to_x = to_y = to_z = 1.0;
      } else {
        to_x = to->scale.x;
        to_y = to->scale.y;
        to_z = to->scale.z;
      }
      *bounds = box;
      ApplyScaleToBox(
          SkMScalarToFloat(BlendSkMScalars(from_x, to_x, min_progress)),
          SkMScalarToFloat(BlendSkMScalars(from_y, to_y, min_progress)),
          SkMScalarToFloat(BlendSkMScalars(from_z, to_z, min_progress)),
          bounds);
      gfx::BoxF bounds_max = box;
      ApplyScaleToBox(
          SkMScalarToFloat(BlendSkMScalars(from_x, to_x, max_progress)),
          SkMScalarToFloat(BlendSkMScalars(from_y, to_y, max_progress)),
          SkMScalarToFloat(BlendSkMScalars(from_z, to_z, max_progress)),
          &bounds_max);
      if (!bounds->IsEmpty() && !bounds_max.IsEmpty()) {
        bounds->Union(bounds_max);
      } else if (!bounds->IsEmpty()) {
        UnionBoxWithZeroScale(bounds);
      } else if (!bounds_max.IsEmpty()) {
        UnionBoxWithZeroScale(&bounds_max);
        *bounds = bounds_max;
      }

      return true;
    }
    case TransformOperation::TransformOperationIdentity:
      *bounds = box;
      return true;
    case TransformOperation::TransformOperationRotate: {
      bool first_point = true;
      for (int i = 0; i < 8; ++i) {
        gfx::Point3F corner = box.origin();
        corner += gfx::Vector3dF(i & 1 ? box.width() : 0.f,
                                 i & 2 ? box.height() : 0.f,
                                 i & 4 ? box.depth() : 0.f);
        gfx::BoxF box_for_arc;
        BoundingBoxForArc(corner, from, to, &box_for_arc);
        if (first_point)
          *bounds = box_for_arc;
        else
          bounds->Union(box_for_arc);
        first_point = false;
      }
      return true;
    }
    case TransformOperation::TransformOperationSkew:
    case TransformOperation::TransformOperationPerspective:
    case TransformOperation::TransformOperationMatrix:
      return false;
  }
  NOTREACHED();
  return false;
}

}  // namespace cc
