# TestFramework

A simple, header-only, C++ unit testing framework.

## Features

- Automatic test registration.
- A rich set of assertions (EQ, NE, TRUE, FALSE, THROWS, etc.).
- Test filtering by name patterns and tags.
- Fixtures via `TestContext` for setup/teardown.
- Color-coded console output.

## How to Use

### CMake Integration

To use this library in your CMake project, you can use `add_subdirectory`.

1.  Add the `test_framework_lib` directory to your project.
2.  In your `CMakeLists.txt`, add the following:

```cmake
# Add the test framework library
add_subdirectory(path/to/test_framework_lib)

# Add your test executable
add_executable(my_tests tests.cpp)

# Link against the framework
target_link_libraries(my_tests PRIVATE TestFramework::test_framework)
```

### Writing a Test

Create a `.cpp` file and include the main header.

```cpp
#include <test_framework/test_framework.h>

// Define a test case
TEST(MyTestCase)
{
    ASSERT_EQ(2 + 2, 4);
}

// Add a main function to run the tests
int main(int argc, char* argv[])
{
    return RUN_ALL_TESTS();
}
```
