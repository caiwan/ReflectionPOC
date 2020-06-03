#include <Reflection.h>
#include <gtest/gtest.h>

struct TestNode
{

	int integerValue;
	float floatValue;
	double doubleValue;

	/*
	    SERIALIZER_FOR_POD(bool);
	    SERIALIZER_FOR_POD(char);
	    SERIALIZER_FOR_POD(unsigned char);
	    SERIALIZER_FOR_POD(short);
	    SERIALIZER_FOR_POD(unsigned short);
	    SERIALIZER_FOR_POD(int);
	    SERIALIZER_FOR_POD(unsigned int);
	    SERIALIZER_FOR_POD(long);
	    SERIALIZER_FOR_POD(unsigned long);
	    SERIALIZER_FOR_POD(long long);
	    SERIALIZER_FOR_POD(unsigned long long);
	    SERIALIZER_FOR_POD(float);
	    SERIALIZER_FOR_POD(double);
	 */

	REFLECT_STRUCT();
};

REFLECT_STRUCT_BEGIN(TestNode)
REFLECT_END()

TEST(Hello, Test)
{

	Reflect::TypeDescriptor * typeDesc = Reflect::TypeResolver<TestNode>::Descriptor();

	ASSERT_TRUE(typeDesc);
	
	SUCCEED();
}
