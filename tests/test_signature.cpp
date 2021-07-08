#include <list>
#include <queue>
#include <vector>
//#include <dequeue>
#include <set>
#include <stack>
//#include <multiset>
#include <map>
#include <unordered_map>

//
#include <Serialization/Signature.h>
#include <gtest/gtest.h>
#include <refl.h>

using Serializable = Grafkit::Attributes::Serializable;
using Property = Grafkit::Attributes::Property;

struct Point
{
	float x = 0.f;
	float y = 0.f;
};

REFL_TYPE(Point, bases<>)
REFL_FIELD(x, Serializable())
REFL_FIELD(y, Serializable())
REFL_END

struct Line
{
	Point p0;
	Point p1;
	float w;
};

REFL_TYPE(Line, bases<>)
REFL_FIELD(p0, Serializable())
REFL_FIELD(p1, Serializable())
REFL_FIELD(w, Serializable())
REFL_END

struct TestingBaseTypes
{
	bool b;
	char c;
	unsigned char uc;
	short s;
	unsigned short us;
	int i;
	unsigned int ui;
	long l;
	long long ll;
	unsigned long ul;
	unsigned long long ull;
	float f;
	double d;
	std::string str;
};

REFL_TYPE(TestingBaseTypes, bases<>)
REFL_FIELD(b, Serializable())
REFL_FIELD(c, Serializable())
REFL_FIELD(uc, Serializable())
REFL_FIELD(s, Serializable())
REFL_FIELD(us, Serializable())
REFL_FIELD(i, Serializable())
REFL_FIELD(ui, Serializable())
REFL_FIELD(l, Serializable())
REFL_FIELD(ll, Serializable())
REFL_FIELD(ul, Serializable())
REFL_FIELD(ull, Serializable())
REFL_FIELD(f, Serializable())
REFL_FIELD(d, Serializable())
REFL_FIELD(str, Serializable())
REFL_END

struct TestingSTLTypes
{
	std::vector<int> vi;
	std::vector<std::string> vs;
	std::deque<int> di;
	std::deque<std::string> ds;
	std::list<int> li;
	std::list<std::string> ls;
	std::set<int> si;
	std::set<std::string> ss;
	std::multiset<int> msi;
	std::multiset<std::string> mss;
	std::map<int, std::string> mis;
	// std::unordered_map<int, std::string> umis;
	std::multimap<int, std::string> mmis;
	// std::unordered_multimap<int, std::string> ummis; // TODO: Missing
};

REFL_TYPE(TestingSTLTypes, bases<>)
REFL_FIELD(vi, Serializable())
REFL_FIELD(vs, Serializable())
REFL_FIELD(di, Serializable())
REFL_FIELD(ds, Serializable())
REFL_FIELD(li, Serializable())
REFL_FIELD(ls, Serializable())
REFL_FIELD(si, Serializable())
REFL_FIELD(ss, Serializable())
REFL_FIELD(msi, Serializable())
REFL_FIELD(mss, Serializable())
REFL_FIELD(mis, Serializable())
// REFL_FIELD(umis, Serializable())
REFL_FIELD(mmis, Serializable())
// REFL_FIELD(ummis, Serializable()) // TODO: Missing
REFL_END

class SimpleClazz
{
public:
	int A() const { return mA; }
	void SetA(const int a) { mA = a; }
	int B() const { return mB; }
	void SetB(const int b) { mB = b; }
	int C() const { return mC; }
	void SetC(const int c) { mC = c; }
	std::string Str1() const { return mStr1; }
	void SetStr1(const std::string & str1) { mStr1 = str1; }
	std::string Str2() const { return mStr2; }
	void SetStr2(const std::string & str2) { mStr2 = str2; }

private:
	int mA{}, mB{}, mC{};
	std::string mStr1, mStr2;
};

REFL_TYPE(SimpleClazz, bases<>)
REFL_FUNC(A, Property("a"), Serializable())
REFL_FUNC(SetA, Property("a"), Serializable())
REFL_FUNC(B, Property("b"), Serializable())
REFL_FUNC(SetB, Property("b"), Serializable())
REFL_FUNC(C, Property("c"), Serializable())
REFL_FUNC(SetC, Property("c"), Serializable())
REFL_FUNC(Str1, Property("str1"), Serializable())
REFL_FUNC(SetStr1, Property("str1"), Serializable())
REFL_FUNC(Str2, Property("str2"), Serializable())
REFL_FUNC(SetStr2, Property("str2"), Serializable())
REFL_END

using Grafkit::Utils::Checksum;

template <typename Type> std::string ReferenceCalcSignature()
{
	std::string ret;

	constexpr auto fields = filter(refl::type_descriptor<Type>::members, [](auto member) { return Grafkit::Traits::is_serializable_field(member); });

	refl::util::for_each(fields, [&](const auto member) {
		typedef decltype(member) DescriptorType;
		if constexpr (refl::descriptor::is_field(member))
		{
			using ValueType = DescriptorType::value_type;
			ret.append(Grafkit::Utils::Signature::type_name<ValueType>::value.c_str());
			ret.append(" ");
			ret.append(member.name.c_str());
			ret.append("; ");
		}
	});

	constexpr auto properties = filter(refl::type_descriptor<Type>::members, [](auto member) { return Grafkit::Traits::is_serializable_getter(member); });

	refl::util::for_each(properties, [&](const auto member) {
		typedef decltype(member) DescriptorType;
		if constexpr (refl::descriptor::is_property(member) && refl::descriptor::is_function(member) && refl::descriptor::is_readable(member))
		{
			using ReturnType = DescriptorType::return_type<Type>;

			ret.append(Grafkit::Utils::Signature::type_name<ReturnType>::value.c_str());
			ret.append(" ");
			ret.append(refl::descriptor::get_display_name(member));
			ret.append("; ");
		}
	});
	return ret;
}

TEST(SignatureTest, SimpleStruct)
{
	constexpr auto signature = Grafkit::Utils::Signature::CalcString<Point>();
	ASSERT_STREQ("float x; float y; ", signature.c_str());
}

TEST(SignatureTest, NestedStruct)
{
	constexpr auto signature = Grafkit::Utils::Signature::CalcString<Line>();
	ASSERT_STREQ("Point p0; Point p1; float w; ", signature.data);
}

TEST(SignatureTest, BaseTypes)
{
	constexpr auto signature = Grafkit::Utils::Signature::CalcString<TestingBaseTypes>();
	ASSERT_STREQ(
		"bool b; char c; unsigned char uc; short s; unsigned short us; int i; unsigned int ui; long l; long long ll; unsigned long ul; unsigned long long ull; "
		"float f; double d; std::string str; ",
		signature.data);
}

TEST(SignatureTest, STLs)
{
	constexpr auto signature = Grafkit::Utils::Signature::CalcString<TestingSTLTypes>();
	ASSERT_STREQ(
		"std::vector<int> vi; std::vector<std::string> vs; std::deque<int> di; std::deque<std::string> ds; std::list<int> li; std::list<std::string> ls; "
		"std::set<int> si; std::set<std::string> ss; std::multiset<int> msi; std::multiset<std::string> mss; std::map<int, std::string> mis; "
		"std::multimap<int, std::string> mmis; ",
		signature.data);
}

TEST(SignatureTest, SimpleClass)
{
	constexpr auto signature = Grafkit::Utils::Signature::CalcString<SimpleClazz>();
	const auto reference = ReferenceCalcSignature<SimpleClazz>();
	
 	ASSERT_STREQ("int a; int b; int c; std::string str1; std::string str2; ", signature.data);
}

//  ---

TEST(ChecksumTests, SimpleStruct)
{
	const auto signature = ReferenceCalcSignature<Point>();
	const auto checksum = Grafkit::Utils::Signature::CalcChecksum<Point>();
	ASSERT_EQ(checksum, Checksum(signature.c_str(), signature.size()));
}

TEST(ChecksumTests, NestedStruct)
{
	const auto signature = ReferenceCalcSignature<Line>();
	const auto checksum = Grafkit::Utils::Signature::CalcChecksum<Line>();
	ASSERT_EQ(checksum, Checksum(signature.c_str(), signature.size()));
}

TEST(ChecksumTests, BaseTypes)
{
	const auto signature = ReferenceCalcSignature<TestingBaseTypes>();
	const auto checksum = Grafkit::Utils::Signature::CalcChecksum<TestingBaseTypes>();
	ASSERT_EQ(checksum, Checksum(signature.c_str(), signature.size()));
}

TEST(ChecksumTests, STLs)
{
	const auto signature = ReferenceCalcSignature<TestingSTLTypes>();
	const auto checksum = Grafkit::Utils::Signature::CalcChecksum<TestingSTLTypes>();
	ASSERT_EQ(checksum, Checksum(signature.c_str(), signature.size()));
}

// ...
TEST(ChecksumTests, SimpleClass)
{
	const auto signature = ReferenceCalcSignature<SimpleClazz>();
	const auto checksum = Grafkit::Utils::Signature::CalcChecksum<SimpleClazz>();
	ASSERT_EQ(checksum, Checksum(signature.c_str(), signature.size()));
}
