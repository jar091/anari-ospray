// Copyright 2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "Pathtracer.h"
// std
#include <algorithm>

namespace anari_ospray {

Pathtracer::Pathtracer(OSPRayGlobalState *s) : Renderer(s, "pathtracer") {}

void Pathtracer::commitParameters()
{
  Renderer::commitParameters();
  m_lightSamples = getParam<int>("lightSamples", -1);
  m_roulettePathLength = getParam<int>("roulettePathLength", 5);
  m_maxScatteringEvents = getParam<int>("maxScatteringEvents", 20);
  m_maxContribution = getParam<float>("maxContribution", 1e20f);
  m_backgroundRefraction = getParam<bool>("backgroundRefraction", false);

  auto limit = [&](const char *name) {
    int value = -1;
    return getParam(name, ANARI_INT32, &value) ? std::max(value, 0) : -1;
  };
  m_maxBounce = limit("maxBounce");
  m_maxDiffuseBounce = limit("maxDiffuseBounce");
  m_clampDirect = std::max(getParam<float>("clampDirect", 0.f), 0.f);
  m_clampIndirect = std::max(getParam<float>("clampIndirect", 0.f), 0.f);
  int roulette = 0;
  m_hasRoulettePathLength = getParam("roulettePathLength", ANARI_INT32, &roulette);
}

void Pathtracer::finalize()
{
  Renderer::finalize();

  auto r = osprayRenderer();
  ospSetInt(r, "lightSamples", m_lightSamples);
  ospSetInt(r, "roulettePathLength", m_roulettePathLength);
  ospSetInt(r, "maxScatteringEvents", m_maxScatteringEvents);
  ospSetFloat(r, "maxContribution", m_maxContribution);
  ospSetBool(r, "backgroundRefraction", m_backgroundRefraction);

  // Path length limits with the names and the counting of Cycles (bounces
  // after the camera hit), mapped to what OSPRay's path tracer has. They take
  // precedence over the OSPRay-named parameters when they are set.
  if (m_maxBounce >= 0) {
    // OSPRay counts the path segments which are shaded: the camera hit is 1.
    ospSetInt(r, "maxPathLength", m_maxBounce + 1);
    // No Russian roulette within the limit, unless the application asks for it.
    if (!m_hasRoulettePathLength)
      ospSetInt(r, "roulettePathLength", m_maxBounce + 1);
  }
  // OSPRay limits the non-specular scattering events of a path, the hit of
  // the camera ray included: diffuse and rough glossy bounces share the limit
  // (specular ones are not counted).
  if (m_maxDiffuseBounce >= 0)
    ospSetInt(r, "maxScatteringEvents", m_maxDiffuseBounce + 1);
  // OSPRay has one clamp, of the radiance of a whole pixel sample (direct and
  // indirect light together, per channel).
  if (m_clampIndirect > 0.f) {
    ospSetFloat(r, "maxContribution", m_clampDirect > 0.f ? m_clampDirect + m_clampIndirect : m_clampIndirect);
  }
  ospCommit(r);
}

} // namespace anari_ospray
