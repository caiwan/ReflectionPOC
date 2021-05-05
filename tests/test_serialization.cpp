#include <cmath>
#include <list>
#include <queue>
#include <vector>
//#include <dequeue>
#include <set>
#include <stack>
//#include <multiset>
#include <map>

//
#include <gtest/gtest.h>
#include <refl.h>
//
#include <Serialization/Serialization.h>

// TODO: https://www.sandordargo.com/blog/2019/04/24/parameterized-testing-with-gtest

using Serializable = Grafkit::Attributes::Serializable;

struct Point
{
	float x = 0.f;
	float y = 0.f;
	[[nodiscard]] float magnitude() const { return std::sqrt(x * x + y * y); }

	bool operator==(const Point & rhs) const { return x == rhs.x && y == rhs.y; }
	bool operator!=(const Point & rhs) const { return !(rhs == *this); }
};

REFL_TYPE(Point, bases<>)
REFL_FIELD(x, Serializable())
REFL_FIELD(y, Serializable())
REFL_FUNC(magnitude)
REFL_END

struct Line
{
	Point start = {};
	Point end = {};

	[[nodiscard]] float length() const { return Point{end.x - start.x, end.y - start.y}.magnitude(); }

	bool operator==(const Line & rhs) const { return start == rhs.start && end == rhs.end; }
	bool operator!=(const Line & rhs) const { return !(rhs == *this); }
};

REFL_TYPE(Line, bases<>)
REFL_FIELD(start, Serializable())
REFL_FIELD(end, Serializable())
REFL_FUNC(length)
REFL_END

struct ValidateBinarySerializer
{
	using Serializer = Grafkit::BinarySerializer;

	template <class T> void VerifySerialization(const T & testValue)
	{
		std::stringstream stringstream;
		Serializer serializer(Grafkit::Stream<std::stringstream>{stringstream});
		T readValue{};
		serializer << testValue;
		serializer >> readValue;
		ASSERT_EQ(testValue, readValue) << typeid(T).name();
	}
	template <class T, size_t N> void VerifyArraySerialization(T (&testValue)[N])
	{
		std::stringstream stringstream;
		Serializer serializer(Grafkit::Stream<std::stringstream>{stringstream});

		T readValue[N] = {};

		serializer << testValue;
		serializer >> readValue;

		for (size_t i = 0; i < N; ++i) ASSERT_EQ(readValue[i], testValue[i]);
	}
};

struct ValidateJsonSerializer
{
	using Serializer = Grafkit::JsonSerializer;
	using Json = Grafkit::Json;

	template <class T> void VerifySerialization(const T & testValue)
	{
		Json outJson;
		Serializer outSerializer(outJson);

		outSerializer << testValue;

		std::stringstream stringstream;
		Serializer::DumpJson(Grafkit::Stream<std::stringstream>{stringstream}, outJson);

		Json inJson = Serializer::ParseJson(Grafkit::Stream<std::stringstream>{stringstream});
		Serializer inSerializer(inJson);

		//std::cout << outJson.dump() << ' ' << inJson.dump() << '\n';
		
		T readValue{};

		inSerializer >> readValue;
		ASSERT_EQ(testValue, readValue) << typeid(T).name();
	}

	template <class T, size_t N> void VerifyArraySerialization(T (&testValue)[N])
	{
		Json outJson;
		Serializer outSerializer(outJson);

		outSerializer << testValue;

		std::stringstream stringstream;
		Serializer::DumpJson(Grafkit::Stream<std::stringstream>{stringstream}, outJson);

		Json inJson = Serializer::ParseJson(Grafkit::Stream<std::stringstream>{stringstream});
		Serializer inSerializer(inJson);

		T readValue[N] = {};

		inSerializer >> readValue;

		for (size_t i = 0; i < N; ++i) ASSERT_EQ(readValue[i], testValue[i]);
	}
};

template <class SerializerValidator> class TestSerialization : public testing::Test, public SerializerValidator
{
};

typedef testing::Types<
	ValidateBinarySerializer,
	ValidateJsonSerializer>
	SerializerTestImplementations;

TYPED_TEST_CASE(TestSerialization, SerializerTestImplementations);

TYPED_TEST(TestSerialization, PlainOldData)
{
	VerifySerialization((bool)true);
	VerifySerialization((char)123);
	VerifySerialization((unsigned char)123);
	VerifySerialization((short)123);
	VerifySerialization((unsigned short)123);
	VerifySerialization((int)123);
	VerifySerialization((unsigned int)123);
	VerifySerialization((long)123);
	VerifySerialization((unsigned long)123);
	VerifySerialization((long long)123);
	VerifySerialization((unsigned long long)123);
	VerifySerialization(std::string("salut"));
	VerifySerialization(123.456f);
	VerifySerialization(1.6180f);
	VerifySerialization(123.456);
}

TYPED_TEST(TestSerialization, Array)
{
	// Array
	int ii[] = {1, 2, 3, 4, 5, 6};
	VerifyArraySerialization(ii);

	Point points[] = {
		{1, 2},
		{3, 4},
		{5, 6},
		{7, 8},
		{9, 10},
	};

	VerifyArraySerialization(points);

	Line lines[] = {
		{{1, 2}, {3, 4}},
		{{5, 6}, {7, 8}},
	};

	VerifyArraySerialization(lines);
}

TYPED_TEST(TestSerialization, Struct)
{
	// User type
	VerifySerialization(Point{1., 1.});
	VerifySerialization(Line{{2., 2.}, {1., 1.}});
}

TYPED_TEST(TestSerialization, STLContainers)
{
	std::set<int> c;
	c.insert(1);

	// STL
	VerifySerialization(std::vector<int>({1, 2, 3, 4, 5}));
	VerifySerialization(std::vector<std::string>({"a", "bb", "ccc", "dddd"}));
	VerifySerialization(std::deque<int>({1, 2, 3, 4, 5}));
	VerifySerialization(std::deque<std::string>({"a", "bb", "ccc", "dddd"}));
	VerifySerialization(std::list<int>({1, 2, 3, 4, 5}));
	VerifySerialization(std::list<std::string>({"a", "bb", "ccc", "dddd"}));
	VerifySerialization(std::set<int>({1, 2, 3, 4, 5}));
	VerifySerialization(std::set<std::string>({"a", "bb", "ccc", "dddd"}));
	VerifySerialization(std::multiset<int>({1, 2, 3, 4, 5}));
	VerifySerialization(std::multiset<std::string>({"a", "bb", "ccc", "dddd"}));
	VerifySerialization(std::map<int, std::string>({std::make_pair(1, "a"), std::make_pair(2, "bb")}));
	VerifySerialization(std::unordered_map<int, std::string>({std::make_pair(1, "a"), std::make_pair(2, "bb")}));
	VerifySerialization(std::multimap<int, std::string>({std::make_pair(1, "a"), std::make_pair(2, "bb")}));
	VerifySerialization(std::unordered_multimap<int, std::string>({std::make_pair(1, "a"), std::make_pair(2, "bb")}));
}

// TODO ... the rest of the tests

// Trait tests
namespace Traits = Grafkit::Traits;
static_assert(true == Traits::is_shared_ptr<std::shared_ptr<void>>::value); // NOLINT {"OCSimplifyInspection"}
static_assert(false == Traits::is_shared_ptr<void>::value);                 // NOLINT {"OCSimplifyInspection"}

static_assert(true == Traits::is_unique_ptr<std::unique_ptr<void>>::value); // NOLINT {"OCSimplifyInspection"}
static_assert(false == Traits::is_unique_ptr<void>::value);                 // NOLINT {"OCSimplifyInspection"}

static_assert(true == Traits::is_iterable<std::vector<int>>::value);                          // NOLINT {"OCSimplifyInspection"}
static_assert(true == Traits::is_iterable<std::vector<std::string>>::value);                  // NOLINT {"OCSimplifyInspection"}
static_assert(true == Traits::is_iterable<std::deque<int>>::value);                           // NOLINT {"OCSimplifyInspection"}
static_assert(true == Traits::is_iterable<std::deque<std::string>>::value);                   // NOLINT {"OCSimplifyInspection"}
static_assert(true == Traits::is_iterable<std::list<int>>::value);                            // NOLINT {"OCSimplifyInspection"}
static_assert(true == Traits::is_iterable<std::list<std::string>>::value);                    // NOLINT {"OCSimplifyInspection"}
static_assert(true == Traits::is_iterable<std::set<int>>::value);                             // NOLINT {"OCSimplifyInspection"}
static_assert(true == Traits::is_iterable<std::set<std::string>>::value);                     // NOLINT {"OCSimplifyInspection"}
static_assert(true == Traits::is_iterable<std::multiset<int>>::value);                        // NOLINT {"OCSimplifyInspection"}
static_assert(true == Traits::is_iterable<std::multiset<std::string>>::value);                // NOLINT {"OCSimplifyInspection"}
static_assert(true == Traits::is_iterable<std::map<int, std::string>>::value);                // NOLINT {"OCSimplifyInspection"}
static_assert(true == Traits::is_iterable<std::unordered_map<int, std::string>>::value);      // NOLINT {"OCSimplifyInspection"}
static_assert(true == Traits::is_iterable<std::multimap<int, std::string>>::value);           // NOLINT {"OCSimplifyInspection"}
static_assert(true == Traits::is_iterable<std::unordered_multimap<int, std::string>>::value); // NOLINT {"OCSimplifyInspection"}

static_assert(true == Traits::has_push_back<std::vector<int>, int>::value);                                                   // NOLINT {"OCSimplifyInspection"}
static_assert(true == Traits::has_push_back<std::vector<std::string>, std::string>::value);                                   // NOLINT {"OCSimplifyInspection"}
static_assert(true == Traits::has_push_back<std::deque<int>, int>::value);                                                    // NOLINT {"OCSimplifyInspection"}
static_assert(true == Traits::has_push_back<std::deque<std::string>, std::string>::value);                                    // NOLINT {"OCSimplifyInspection"}
static_assert(true == Traits::has_push_back<std::list<int>, int>::value);                                                     // NOLINT {"OCSimplifyInspection"}
static_assert(true == Traits::has_push_back<std::list<std::string>, std::string>::value);                                     // NOLINT {"OCSimplifyInspection"}
static_assert(false == Traits::has_push_back<std::set<int>, int>::value);                                                     // NOLINT {"OCSimplifyInspection"}
static_assert(false == Traits::has_push_back<std::set<std::string>, std::string>::value);                                     // NOLINT {"OCSimplifyInspection"}
static_assert(false == Traits::has_push_back<std::multiset<int>, int>::value);                                                // NOLINT {"OCSimplifyInspection"}
static_assert(false == Traits::has_push_back<std::multiset<std::string>, std::string>::value);                                // NOLINT {"OCSimplifyInspection"}
static_assert(false == Traits::has_push_back<std::map<int, std::string>, std::pair<int, std::string>>::value);                // NOLINT {"OCSimplifyInspection"}
static_assert(false == Traits::has_push_back<std::unordered_map<int, std::string>, std::pair<int, std::string>>::value);      // NOLINT {"OCSimplifyInspection"}
static_assert(false == Traits::has_push_back<std::multimap<int, std::string>, std::pair<int, std::string>>::value);           // NOLINT {"OCSimplifyInspection"}
static_assert(false == Traits::has_push_back<std::unordered_multimap<int, std::string>, std::pair<int, std::string>>::value); // NOLINT {"OCSimplifyInspection"}

static_assert(false == Traits::has_insert<std::vector<int>, int>::value);                                                 // NOLINT {"OCSimplifyInspection"}
static_assert(false == Traits::has_insert<std::vector<std::string>, std::string>::value);                                 // NOLINT {"OCSimplifyInspection"}
static_assert(false == Traits::has_insert<std::deque<int>, int>::value);                                                  // NOLINT {"OCSimplifyInspection"}
static_assert(false == Traits::has_insert<std::deque<std::string>, std::string>::value);                                  // NOLINT {"OCSimplifyInspection"}
static_assert(false == Traits::has_insert<std::list<int>, int>::value);                                                   // NOLINT {"OCSimplifyInspection"}
static_assert(false == Traits::has_insert<std::list<std::string>, std::string>::value);                                   // NOLINT {"OCSimplifyInspection"}
static_assert(true == Traits::has_insert<std::set<int>, int>::value);                                                     // NOLINT {"OCSimplifyInspection"}
static_assert(true == Traits::has_insert<std::set<std::string>, std::string>::value);                                     // NOLINT {"OCSimplifyInspection"}
static_assert(true == Traits::has_insert<std::multiset<int>, int>::value);                                                // NOLINT {"OCSimplifyInspection"}
static_assert(true == Traits::has_insert<std::multiset<std::string>, std::string>::value);                                // NOLINT {"OCSimplifyInspection"}
static_assert(true == Traits::has_insert<std::map<int, std::string>, std::pair<int, std::string>>::value);                // NOLINT {"OCSimplifyInspection"}
static_assert(true == Traits::has_insert<std::unordered_map<int, std::string>, std::pair<int, std::string>>::value);      // NOLINT {"OCSimplifyInspection"}
static_assert(true == Traits::has_insert<std::multimap<int, std::string>, std::pair<int, std::string>>::value);           // NOLINT {"OCSimplifyInspection"}
static_assert(true == Traits::has_insert<std::unordered_multimap<int, std::string>, std::pair<int, std::string>>::value); // NOLINT {"OCSimplifyInspection"}

template <typename T, typename U> struct DummyPair
{
	T first;
	U second;
};

static_assert(true == Traits::is_pair<std::pair<int, int>>::value); // NOLINT {"OCSimplifyInspection"}
static_assert(true == Traits::is_pair<DummyPair<int, int>>::value); // NOLINT {"OCSimplifyInspection"}

static_assert(true == Traits::is_pair<std::pair<const int, std::string>>::value); // NOLINT {"OCSimplifyInspection"}

// Utility tests
typedef DummyPair<int, int> DummyIntPair;

REFL_TYPE(DummyIntPair, bases<>)
REFL_FIELD(first, Serializable())
REFL_FIELD(second, Serializable())
REFL_END

// namespace Detail = Grafkit::Detail;
// static_assert(refl::make_const_string("HelloWorld") == Detail::ConcatConstString("Hello", refl::make_const_string("World")));
// static_assert("first second" == Detail::ConcatFieldNames(refl::reflect(DummyIntPair({})).members));
