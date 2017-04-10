#ifndef RADIOSITY_TEST_FLOAT_HPP
#define RADIOSITY_TEST_FLOAT_HPP

namespace RadiosityTest
{
static constexpr auto FloatEpsilon = 0.0001f;

inline bool closeTo(float a, float b)
{
    auto delta = b - a;
    return -FloatEpsilon <= delta && delta <= FloatEpsilon;
}

} // End of namespace RadiosityTest

#endif //RADIOSITY_TEST_FLOAT_HPP
