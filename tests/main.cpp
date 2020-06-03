#include <gtest/gtest.h>

using ::testing::InitGoogleTest;

int main(int argc, char ** argv)
{
	InitGoogleTest(&argc, argv);
	const int res = RUN_ALL_TESTS();
	return res;
}
