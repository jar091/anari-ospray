// Copyright 2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Renderer.h"

namespace anari_ospray {

struct Pathtracer : public Renderer
{
  Pathtracer(OSPRayGlobalState *s);
  void commitParameters() override;
  void finalize() override;

 private:
  int m_lightSamples{-1};
  int m_roulettePathLength{5};
  int m_maxScatteringEvents{20};
  float m_maxContribution{1e20f};
  bool m_backgroundRefraction{false};
  // Path length limits and clamps with the names of Cycles; -1 and 0: not set
  int m_maxBounce{-1};
  int m_maxDiffuseBounce{-1};
  float m_clampDirect{0.f};
  float m_clampIndirect{0.f};
  bool m_hasRoulettePathLength{false};
};

} // namespace anari_ospray
