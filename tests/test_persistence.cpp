#include <fstream>
#include <gtest/gtest.h>
//
#include <Serialization/Serialization.h>

using Serializable = Grafkit::Attributes::Serializable;
using Property = Grafkit::Attributes::Property;
using DynamicObject = Grafkit::Serializer::DynamicObject;

struct UseBinarySerializer
{
	using Serializer = Grafkit::BinarySerializer;

	template <class T> void Serialize(const T & obj, std::stringstream & s)
	{
		Serializer serializer(Grafkit::OutputStream<std::stringstream>{s});
		serializer << obj;
	}

	template <class T> void Deserialize(const std::stringstream & s, T & obj)
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

// --

template <class SerializerAdapter> class TestPersistence : public testing::Test, public SerializerAdapter
{
};

typedef testing::Types<UseBinarySerializer, UseJsonSerializer> PersistenceTestImplementations;

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

	std::shared_ptr<SimpleClass> obj = std::make_shared<SimpleClass>(42, "some parameter");

	std::stringstream s;
	Serialize(obj, s);

	std::shared_ptr<SimpleClass> readObj;
	Deserialize(s, readObj);

	ASSERT_TRUE(readObj);
	ASSERT_NE(obj, readObj);

	ASSERT_EQ(obj->Integer(), readObj->Integer());
	ASSERT_STREQ(obj->String().c_str(), readObj->String().c_str());
}
//
//// --
//
// class NestedClass : public DynamicObject
//{
// public:
//	NestedClass() {}
//
//	NestedClass(const std::shared_ptr<SimpleClass> & simpleClass, const std::shared_ptr<SimpleClass> & simpleClass1) : m_obj1(simpleClass), m_obj2(simpleClass1)
//	{
//	}
//
//	std::shared_ptr<SimpleClass> GetObj1() const { return m_obj1; }
//	std::shared_ptr<SimpleClass> GetObj2() const { return m_obj2; }
//
// private:
//	std::shared_ptr<SimpleClass> m_obj1;
//	std::shared_ptr<SimpleClass> m_obj2;
//
//	DYNAMICS_DECL(NestedClass)
//};
//
// REFL_TYPE(NestedClass, bases<>)
// REFL_FIELD(m_obj1, Serializable())
// REFL_FIELD(m_obj2, Serializable())
// REFL_END
//
// DYNAMICS_IMPL(NestedClass)
//
// TEST(Persistence, NestedClassTest)
//{
//	//// given
//	// std::stringstream s;
//	// Archive a(std::make_unique<Stream<std::stringstream>>(s));
//	// std::shared_ptr<NestedClass> obj = std::shared_ptr<NestedClass>(
//	//	new NestedClass(std::shared_ptr<SimpleClass>(new SimpleClass(666, "Devil")), std::shared_ptr<SimpleClass>(new SimpleClass(42, "The ultimate answer"))));
//	// std::shared_ptr<NestedClass> objNull = std::shared_ptr<NestedClass>(new NestedClass(nullptr, nullptr));
//
//	//// when
//	// a << obj << objNull;
//
//	//// then
//	// std::shared_ptr<NestedClass> readObj, readObjNull;
//	// a >> readObj >> readObjNull;
//
//	// ASSERT_TRUE(readObj);
//	// ASSERT_TRUE(readObjNull);
//	// ASSERT_NE(obj, readObj);
//	// ASSERT_NE(objNull, readObjNull);
//
//	//// ... MORE ASSERTS
//
//	// ASSERT_FALSE(readObjNull->GetObj1());
//	// ASSERT_FALSE(readObjNull->GetObj2());
//}
//
// TEST(Persistence, BadTypeTest)
//{
//	//// given
//	// std::stringstream s;
//	// Archive a(std::make_unique<Stream<std::stringstream>>(s));
//	// std::shared_ptr<SimpleClass> obj = std::shared_ptr<SimpleClass>(new SimpleClass(42, "some parameter"));
//
//	//// when
//	// a << obj;
//
//	//// then
//	// std::shared_ptr<NestedClass> readObj;
//
//	//// will it blend?
//	// ASSERT_THROW(a >> readObj, std::exception);
//}
//
///* ****************************************************************************************************************************************
// *
// **************************************************************************************************************************************** */
//
// class SimpleBaseClass : public DynamicObject
//{
// public:
//	SimpleBaseClass() : m_i(0) {}
//
//	SimpleBaseClass(const int i, const std::string & str) : m_i(i), m_str(str) {}
//
//	int GetI() const { return m_i; }
//	std::string GetStr() const { return m_str; }
//
//	virtual std::string GetSomeIntern() const = 0;
//
// protected:
//	// TODO: generate for base calss
//	// template <class AR> void Serialize(AR & ar) { ar & m_i & m_str; }
//
// private:
//	int m_i;
//	std::string m_str;
//};
//
// REFL_TYPE(SimpleBaseClass, bases<>)
// REFL_FIELD(m_i, Serializable())
// REFL_FIELD(m_str, Serializable())
// REFL_END
//
// class DerivedClassA : public SimpleBaseClass
//{
// public:
//	DerivedClassA() {}
//
//	DerivedClassA(int i, const std::string & str, const std::string & str1) : SimpleBaseClass(i, str), m_str1(str1) {}
//
//	std::string GetSomeIntern() const override { return m_str1; }
//
// private:
//	std::string m_str1;
//
//	DYNAMICS_DECL(DerivedClassA)
//};
//
// REFL_TYPE(DerivedClassA, bases<SimpleBaseClass>)
// REFL_FIELD(m_str1, Serializable())
// REFL_END
//
// DYNAMICS_IMPL(DerivedClassA)
//
// class DerivedClassB : public SimpleBaseClass
//{
// public:
//	DerivedClassB() {}
//
//	DerivedClassB(int i, const std::string & str, const std::string & str2) : SimpleBaseClass(i, str), m_str2(str2) {}
//
//	std::string GetSomeIntern() const override { return m_str2; }
//
// private:
//	std::string m_str2;
//
//	DYNAMICS_DECL(DerivedClassB)
//};
//
// REFL_TYPE(DerivedClassB, bases<SimpleBaseClass>)
// REFL_FIELD(m_str2, Serializable())
// REFL_END
//
// DYNAMICS_IMPL(DerivedClassB)
//
// TEST(Persistence, PolimorphClassTest)
//{
//	//// given
//	// std::stringstream s;
//	// Archive a(std::make_unique<Stream<std::stringstream>>(s));
//	// std::shared_ptr<SimpleBaseClass> objA = std::shared_ptr<SimpleBaseClass>(new DerivedClassA(42, "Hello", "World"));
//	// std::shared_ptr<SimpleBaseClass> objB = std::shared_ptr<SimpleBaseClass>(new DerivedClassB(666, "This is a", "test message"));
//
//	//// when
//	// a << objA << objB;
//
//	//// then
//	// std::shared_ptr<SimpleBaseClass> readObjA, readObjB;
//	// a >> readObjA >> readObjB;
//
//	// ASSERT_TRUE(readObjA);
//	// ASSERT_TRUE(readObjB);
//	// ASSERT_NE(objA, readObjA);
//	// ASSERT_NE(objB, readObjB);
//
//	// ASSERT_TRUE(readObjA);
//	// ASSERT_TRUE(dynamic_cast<DerivedClassA *>(readObjA.get()));
//	// ASSERT_TRUE(objA->GetI() == readObjA->GetI());
//	// ASSERT_TRUE(objA->GetStr().compare(readObjA->GetStr()) == 0);
//	// ASSERT_TRUE(objA->GetSomeIntern().compare(readObjA->GetSomeIntern()) == 0);
//
//	// ASSERT_TRUE(readObjB);
//	// ASSERT_TRUE(dynamic_cast<DerivedClassB *>(readObjB.get()));
//	// ASSERT_TRUE(objB->GetI() == readObjB->GetI());
//	// ASSERT_TRUE(objB->GetStr().compare(readObjB->GetStr()) == 0);
//	// ASSERT_TRUE(objB->GetSomeIntern().compare(readObjB->GetSomeIntern()) == 0);
//}
//
// void Verify(const std::shared_ptr<SimpleBaseClass> & expected, const std::shared_ptr<SimpleBaseClass> & actual)
//{
//	ASSERT_TRUE(actual);
//	ASSERT_NE(expected, actual);
//	ASSERT_EQ(expected->GetI(), actual->GetI());
//	ASSERT_STREQ(expected->GetStr().c_str(), actual->GetStr().c_str());
//	ASSERT_STREQ(expected->GetSomeIntern().c_str(), actual->GetSomeIntern().c_str());
//}
//
// TEST(Persistence, STLContainerObjects)
//{
//	//// given
//	// std::stringstream s;
//	// Archive a(std::make_unique<Stream<std::stringstream>>(s));
//
//	// std::array<std::shared_ptr<SimpleBaseClass>, 256> array;
//	// std::vector<std::shared_ptr<SimpleBaseClass>> vector;
//	// std::list<std::shared_ptr<SimpleBaseClass>> list;
//	// for (size_t i = 0; i < 256; ++i)
//	//{
//	//	array[i] = std::shared_ptr<DerivedClassA>(new DerivedClassA(42 + i, "Hello", "Array"));
//	//	vector.push_back(std::shared_ptr<DerivedClassB>(new DerivedClassB(64 + i, "Hello", "List")));
//	//	list.push_back(std::shared_ptr<DerivedClassB>(new DerivedClassB(128 + i, "Hello", "List")));
//	//}
//
//	//// when
//	// a << array << vector << list;
//
//	//// then
//	// std::array<std::shared_ptr<SimpleBaseClass>, 256> readArray;
//	// std::vector<std::shared_ptr<SimpleBaseClass>> readVector;
//	// std::list<std::shared_ptr<SimpleBaseClass>> readList;
//
//	// a >> readArray >> readVector >> readList;
//
//	// ASSERT_FALSE(readArray.empty());
//	// ASSERT_FALSE(readVector.empty());
//	// ASSERT_FALSE(readList.empty());
//
//	//// ... + asserts
//
//	// auto readIt = readList.begin();
//	// auto it = list.begin();
//	// for (size_t i = 0; i < 256; ++i)
//	//{
//	//	Verify(array[i], readArray[i]);
//	//	Verify(vector[i], readVector[i]);
//	//	Verify(*it, *readIt);
//	//	++it, ++readIt;
//	//}
//}
//
//// TODO (1) + map
//// TODO (1) + pair
//
//// TODO (1) + queue
//// TODO (1) + set, multiset
//// TODO (1) + multimap
