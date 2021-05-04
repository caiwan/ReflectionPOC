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

struct Point
{
	float x = 0.f;
	float y = 0.f;
};

REFL_TYPE(Point, bases<>)
REFL_FIELD(x)
REFL_FIELD(y)
REFL_END

struct Line
{
	Point p0;
	Point p1;
	float w;
};

REFL_TYPE(Line, bases<>)
REFL_FIELD(p0)
REFL_FIELD(p1)
REFL_FIELD(w)
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
REFL_FIELD(b)
REFL_FIELD(c)
REFL_FIELD(uc)
REFL_FIELD(s)
REFL_FIELD(us)
REFL_FIELD(i)
REFL_FIELD(ui)
REFL_FIELD(l)
REFL_FIELD(ll)
REFL_FIELD(ul)
REFL_FIELD(ull)
REFL_FIELD(f)
REFL_FIELD(d)
REFL_FIELD(str)
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
	std::unordered_map<int, std::string> umis;
	std::multimap<int, std::string> mmis;
	std::unordered_multimap<int, std::string> ummis;
};

REFL_TYPE(TestingSTLTypes, bases<>)
REFL_FIELD(vi)
REFL_FIELD(vs)
REFL_FIELD(di)
REFL_FIELD(ds)
REFL_FIELD(li)
REFL_FIELD(ls)
REFL_FIELD(si)
REFL_FIELD(ss)
REFL_FIELD(msi)
REFL_FIELD(mss)
REFL_FIELD(mis)
REFL_FIELD(umis)
REFL_FIELD(mmis)
REFL_FIELD(ummis)
REFL_END

// TODO: use static_assert here

template <typename Type> std::string ReferenceCalcSignature()
{
	std::string ret;

	refl::util::for_each(refl::reflect<Type>().members, [&](const auto member) {
		typedef decltype(member) MemberDescriptorType;
		using MemberType = MemberDescriptorType::value_type;
		ret.append(Grafkit::Utils::Signature::type_name<MemberType>::value.c_str());
		ret.append(" ");
		ret.append(member.name.c_str());
		ret.append("; ");
	});
	return ret;
}

template <typename Type> Grafkit::Utils::Checksum ReferenceCalcChecksum()
{
	auto checksum = Grafkit::Utils::initialChecksum;

	refl::util::for_each(refl::reflect<Type>().members, [&](const auto member) {
		typedef decltype(member) MemberDescriptorType;
		using MemberType = MemberDescriptorType::value_type;

		checksum = Grafkit::Utils::Crc32Rec(Grafkit::Utils::Signature::type_name<MemberType>::value.data, checksum);
		checksum = Grafkit::Utils::Crc32Rec(" ", checksum);
		checksum = Grafkit::Utils::Crc32Rec(member.name.data, checksum);
		checksum = Grafkit::Utils::Crc32Rec("; ", checksum);
	});
	return checksum;
}

TEST(SignatureTest, Signature)
{
	//constexpr auto simpleStruct = Grafkit::Utils::Signature::CalcString<Point>();
	//const auto simpleStructReference = ReferenceCalcSignature<Point>();
	//ASSERT_STREQ(simpleStructReference.c_str(), simpleStruct.c_str());
	//ASSERT_STREQ("float x; float y; ", simpleStruct.c_str());

	// constexpr auto compositeStruct = Grafkit::Utils::Signature::CalcString<Line>();
	// ASSERT_STREQ("Point p0; Point p1; float w; ", compositeStruct.data);

	// constexpr auto baseTypes = Grafkit::Utils::Signature::CalcString<TestingBaseTypes>();
	// ASSERT_STREQ(
	//    "bool b; char c; unsigned char uc; short s; unsigned short us; int i; unsigned int ui; long l; long long ll; unsigned long ul; unsigned long long ull;
	//    " "float f; double d; std::string str; ", baseTypes.data);

	// constexpr auto stlTypes = Grafkit::Utils::Signature::CalcString<TestingSTLTypes>();
	// ASSERT_STREQ(
	//    "std::vector<int> vi; std::vector<std::string> vs; std::deque<int> di; std::deque<std::string> ds; std::list<int> li; std::list<std::string> ls; "
	//    "std::set<int> si; std::set<std::string> ss; std::multiset<int> msi; std::multiset<std::string> mss; std::map<int, std::string> mis; <unknown> umis; "
	//    "std::multimap<int, std::string> mmis; <unknown> ummis; ",
	//    stlTypes.data);
}

TEST(SignatureTest, Checksum)
{
	//constexpr auto simpleStructChecksum = Grafkit::Utils::Signature::CalcChecksum<Point>();
	//constexpr auto simpleStructSgn = Grafkit::Utils::Signature::CalcString<Point>();
	//const auto simpleStructReference = ReferenceCalcChecksum<Point>();

	//ASSERT_EQ(Grafkit::Utils::Crc32(simpleStructSgn.data), simpleStructReference) << "Bazdmeg";
	//ASSERT_EQ(simpleStructReference, simpleStructChecksum) << "BBBB";

	// constexpr auto compositeStructSgn = Grafkit::Utils::Signature::CalcString<Line>();
	// constexpr auto compositeStructChk = Grafkit::Utils::Signature::CalcChecksum<Line>();
	// ASSERT_EQ(Grafkit::Utils::Crc32Rec(compositeStructSgn.data), compositeStructChk);

	// constexpr auto baseTypesSgn = Grafkit::Utils::Signature::CalcString<TestingBaseTypes>();
	// constexpr auto baseTypesChk = Grafkit::Utils::Signature::CalcChecksum<TestingBaseTypes>();
	// ASSERT_EQ(Grafkit::Utils::Crc32Rec(baseTypesSgn.data), baseTypesChk);

	// constexpr auto stlTypesSgn = Grafkit::Utils::Signature::CalcString<TestingSTLTypes>();
	// constexpr auto stlTypesChk = Grafkit::Utils::Signature::CalcChecksum<TestingSTLTypes>();
	// ASSERT_EQ(Grafkit::Utils::Crc32Rec(stlTypesSgn.data), stlTypesChk);
}

// static_assert(Grafkit::Utils::Crc32Rec("World", Grafkit::Utils::Crc32Rec("Hello")) == "HelloWorld"_crc32);

// This is a sample how the CRC32 tables are generated
// TODO: create at least a py script to programmatically do that on the fly.

constexpr size_t tableLength = 32;

//TEST(DummyCrc, Combine) { ASSERT_EQ("Hello\0\0\0\0\0\0"_crc32 ^ "World"_crc32, "HelloWorld"_crc32); }
