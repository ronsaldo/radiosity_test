#ifndef RADIOSITY_TEST_OBJECT_HPP
#define RADIOSITY_TEST_OBJECT_HPP

#include <assert.h>
#include <memory>

namespace RadiosityTest
{
#define DECLARE_SMART_POINTERS(className) \
    typedef std::shared_ptr<className> className ## Ptr; \
    typedef std::weak_ptr<className> className ## WeakPtr; \

#define DECLARE_CLASS(className) \
    class className; \
    DECLARE_SMART_POINTERS(className)

#define DECLARE_INTERFACE(className) \
    struct className; \
    DECLARE_SMART_POINTERS(className)

DECLARE_INTERFACE(Object);

/**
 * The object base class
 */
struct Object
{
    virtual ~Object() {}
};

} // End of namespace RadiosityTest

#endif //RADIOSITY_TEST_GPU_PROGRAM_HPP
