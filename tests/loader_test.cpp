#include <gtest/gtest.h>
#include "../src/io/Loader.h"

using namespace planner;

TEST(Loader, SampleSmall) {
  auto r = loadFromJsonFile("data/sample_small.json");
  EXPECT_GE(r.curriculum.courses.size(), 1u);
  EXPECT_GE(r.constraints.maxCreditsPerTerm, 1);
}
