#include <fstream>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
//
#include <Serialization/Serialization.h>

using ::testing::ElementsAreArray;

using Serializable = Grafkit::Attributes::Serializable;
using Property = Grafkit::Attributes::Property;
using DynamicObject = Grafkit::Serializer::DynamicObject;

// ---

struct UseBinarySerializer
{
	using Serializer = Grafkit::BinarySerializer;

	template <class T> void Serialize(const T & obj, std::stringstream & s)
	{
		Serializer serializer(Grafkit::OutputStream<std::stringstream>{s});
		serializer << obj;
	}

	template <class T> void Deserialize(std::stringstream & s, T & obj)
	{
		Serializer serializer(Grafkit::InputStream<std::stringstream>{s});
		serializer >> obj;
	}
};

struct UseJsonSerializer
{
	using Serializer = Grafkit::JsonSerializer;

	template <class T> void Serialize(const T & obj, std::stringstream & s) {}

	template <class T> void Deserialize(const std::stringstream & s, T & obj) {}
};

// ---

template <class SerializerAdapter> class TestPersistence : public testing::Test, public SerializerAdapter
{
};

typedef testing::Types<UseBinarySerializer /*,UseJsonSerializer*/> PersistenceTestImplementations;

// ---

TYPED_TEST_CASE(TestPersistence, PersistenceTestImplementations);

class SimpleClass : public DynamicObject
{
public:
	SimpleClass() : m_integer(0) {}

	SimpleClass(int integer, const std::string & string) : m_integer(integer), m_string(string) {}

	int Integer() const { return m_integer; }
	void SetInteger(const int integer) { m_integer = integer; }

	std::string String() const { return m_string; }
	void SetString(const std::string & string) { m_string = string; }

private:
	int m_integer;
	std::string m_string;

	DYNAMICS_DECL(SimpleClass)
};

REFL_TYPE(SimpleClass, bases<>)
REFL_FUNC(Integer, Property("integer"), Serializable())
REFL_FUNC(SetInteger, Property("integer"), Serializable())
REFL_FUNC(String, Property("string"), Serializable())
REFL_FUNC(SetString, Property("string"), Serializable())
REFL_END

DYNAMICS_IMPL(SimpleClass)


	TYPED_TEST(TestPersistence, SimpleObjectTest)
{
	// Given
	std::shared_ptr<SimpleClass> obj = std::make_shared<SimpleClass>(42, "some parameter");

	// When
	std::stringstream s;
	Serialize(obj, s);

	// Then
	std::shared_ptr<SimpleClass> readObj;
	Deserialize(s, readObj);

	ASSERT_TRUE(readObj);
	ASSERT_NE(obj, readObj);

	ASSERT_EQ(obj->Integer(), readObj->Integer());
	ASSERT_STREQ(obj->String().c_str(), readObj->String().c_str());
}

// ---

class NestedClass : public DynamicObject
{
public:
	NestedClass() {}

	NestedClass(const std::shared_ptr<SimpleClass> & simpleClass, const std::shared_ptr<SimpleClass> & simpleClass1) : mObj1(simpleClass), mObj2(simpleClass1)
	{
	}

	[[nodiscard]] std::shared_ptr<SimpleClass> Obj1() const { return mObj1; }
	void SetObj1(const std::shared_ptr<SimpleClass> & simpleClass) { mObj1 = simpleClass; }

	std::shared_ptr<SimpleClass> Obj2() const { return mObj2; }
	void SetObj2(const std::shared_ptr<SimpleClass> & simpleClass) { mObj2 = simpleClass; }

private:
	std::shared_ptr<SimpleClass> mObj1;
	std::shared_ptr<SimpleClass> mObj2;

	DYNAMICS_DECL(NestedClass)
};

REFL_TYPE(NestedClass, bases<>)
REFL_FUNC(Obj1, Property("obj1"), Serializable())
REFL_FUNC(SetObj1, Property("obj1"), Serializable())
REFL_FUNC(Obj2, Property("obj2"), Serializable())
REFL_FUNC(SetObj2, Property("obj2"), Serializable())
REFL_END

DYNAMICS_IMPL(NestedClass)

TYPED_TEST(TestPersistence, NestedClassTest)
{
	// Given
	std::shared_ptr<NestedClass> obj =
		std::make_shared<NestedClass>(std::make_shared<SimpleClass>(666, "Devil"), std::make_shared<SimpleClass>(42, "The ultimate answer"));
	std::shared_ptr<NestedClass> objNull = std::make_shared<NestedClass>(nullptr, nullptr);

	// When
	std::stringstream s;
	Serialize(obj, s);
	Serialize(objNull, s);

	// Then
	std::shared_ptr<NestedClass> readObj;
	std::shared_ptr<NestedClass> readObjNull;
	Deserialize(s, readObj);
	Deserialize(s, readObjNull);

	ASSERT_TRUE(readObj);
	ASSERT_TRUE(readObjNull);
	ASSERT_NE(obj, readObj);
	ASSERT_NE(objNull, readObjNull);

	ASSERT_TRUE(readObj->Obj1());
	ASSERT_EQ(666, readObj->Obj1()->Integer());
	ASSERT_STREQ("Devil", readObj->Obj1()->String().c_str());

	ASSERT_TRUE(readObj->Obj2());
	ASSERT_EQ(42, readObj->Obj2()->Integer());
	ASSERT_STREQ("The ultimate answer", readObj->Obj2()->String().c_str());

	ASSERT_FALSE(readObjNull->Obj1());
	ASSERT_FALSE(readObjNull->Obj2());
}

// ---
TYPED_TEST(TestPersistence, BadType)
{
	// Given
	std::shared_ptr<SimpleClass> obj = std::make_shared<SimpleClass>(42, "some parameter");

	// WHen
	std::stringstream s;
	Serialize(obj, s);

	// Then
	std::shared_ptr<NestedClass> readObj;
	ASSERT_THROW(Deserialize(s, readObj), std::exception);
}

// ---
class SimpleBaseClass : public DynamicObject
{
public:
	SimpleBaseClass() : mInteger(0) {}

	SimpleBaseClass(const int i, const std::string & str) : mInteger(i), mString(str) {}

	int Integer() const { return mInteger; }
	void SetInteger(int integer) { mInteger = integer; }

	std::string String() const { return mString; }
	void SetString(const std::string & string) { mString = string; }

	virtual std::string GetSomeIntern() const = 0;

private:
	int mInteger;
	std::string mString;
};

//
REFL_TYPE(SimpleBaseClass, bases<>)
REFL_FUNC(Integer, Property("integer"), Serializable())
REFL_FUNC(SetInteger, Property("integer"), Serializable())
REFL_FUNC(String, Property("string"), Serializable())
REFL_FUNC(SetString, Property("string"), Serializable())
REFL_END

class DerivedClassA : public SimpleBaseClass
{
public:
	DerivedClassA() {}

	DerivedClassA(int i, const std::string & str, const std::string & str1) : SimpleBaseClass(i, str), mString1(str1) {}

	std::string GetSomeIntern() const override { return mString1; }

	std::string String1() const { return mString1; }
	void SetString1(const std::string & string1) { mString1 = string1; }

private:
	std::string mString1;

	DYNAMICS_DECL(DerivedClassA)
};

REFL_TYPE(DerivedClassA, bases<SimpleBaseClass>)
REFL_FUNC(String1, Property("string1"), Serializable())
REFL_FUNC(SetString1, Property("string1"), Serializable())
REFL_END

DYNAMICS_IMPL(DerivedClassA)

class DerivedClassB : public SimpleBaseClass
{
public:
	DerivedClassB() {}

	DerivedClassB(int i, const std::string & str, const std::string & str2) : SimpleBaseClass(i, str), mString2(str2) {}

	std::string GetSomeIntern() const override { return mString2; }

	std::string String2() const { return mString2; }
	void SetString2(const std::string & string2) { mString2 = string2; }

private:
	std::string mString2;

	DYNAMICS_DECL(DerivedClassB)
};

REFL_TYPE(DerivedClassB, bases<SimpleBaseClass>)
REFL_FUNC(String2, Property("string2"), Serializable())
REFL_FUNC(SetString2, Property("string2"), Serializable())
REFL_END

DYNAMICS_IMPL(DerivedClassB)

TYPED_TEST(TestPersistence, PolimorphClassTest)
{
	std::stringstream s;

	// Given
	std::shared_ptr<SimpleBaseClass> objA = std::make_shared<DerivedClassA>(42, "Hello", "World");
	std::shared_ptr<SimpleBaseClass> objB = std::make_shared<DerivedClassB>(666, "This is a", "test message");

	// When
	Serialize(objA, s);
	Serialize(objB, s);

	// Then
	std::shared_ptr<SimpleBaseClass> readObjA, readObjB;

	Deserialize(s, readObjA);
	Deserialize(s, readObjB);

	ASSERT_TRUE(readObjA);
	ASSERT_TRUE(readObjB);
	ASSERT_NE(objA, readObjA);
	ASSERT_NE(objB, readObjB);

	ASSERT_TRUE(readObjA);
	ASSERT_TRUE(dynamic_cast<DerivedClassA *>(readObjA.get()));
	ASSERT_EQ(42, readObjA->Integer());
	ASSERT_STREQ("Hello", readObjA->String().c_str());
	ASSERT_STREQ("World", readObjA->GetSomeIntern().c_str());

	ASSERT_TRUE(readObjB);
	ASSERT_TRUE(dynamic_cast<DerivedClassB *>(readObjB.get()));
	ASSERT_EQ(666, readObjB->Integer());
	ASSERT_STREQ("This is a", readObjB->String().c_str());
	ASSERT_STREQ("test message", readObjB->GetSomeIntern().c_str());
}

// ---
TYPED_TEST(TestPersistence, STLContainerObjects)
{
	std::stringstream s;

	// Given
	std::array<std::shared_ptr<SimpleBaseClass>, 256> array;
	std::vector<std::shared_ptr<SimpleBaseClass>> vector;
	std::list<std::shared_ptr<SimpleBaseClass>> list;
	for (size_t i = 0; i < 256; ++i)
	{
		array[i] = std::make_shared<DerivedClassA>(42 + i, "Hello", "Array");
		vector.push_back(std::make_shared<DerivedClassB>(64 + i, "Hello", "List"));
		list.push_back(std::make_shared<DerivedClassB>(128 + i, "Hello", "List"));
	}

	// When
	Serialize(array, s);
	Serialize(vector, s);
	Serialize(list, s);

	// Then
	std::array<std::shared_ptr<SimpleBaseClass>, 256> readArray;
	std::vector<std::shared_ptr<SimpleBaseClass>> readVector;
	std::list<std::shared_ptr<SimpleBaseClass>> readList;

	Deserialize(s, readArray);
	Deserialize(s, readVector);
	Deserialize(s, readList);

	ASSERT_FALSE(readArray.empty());
	ASSERT_FALSE(readVector.empty());
	ASSERT_FALSE(readList.empty());

	// ... TODO: custom matcher

	// ASSERT_THAT(readArray, ElementsAreArray(array));
	// ASSERT_THAT(readVector, ElementsAreArray(vector));
	// ASSERT_THAT(readList, ElementsAreArray(list));
}

// TODO (1) + map
// TODO (1) + pair

// TODO (1) + queue
// TODO (1) + set, multiset
// TODO (1) + multimap
