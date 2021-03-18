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

TEST(SignatureTest, SignatureGeneration)
{
	const auto simpleStruct = Grafkit::Utils::Signature::SignatureString<Point>::CalcString();
	ASSERT_TRUE(simpleStruct.size());
	ASSERT_STREQ("float x; float y;", simpleStruct.c_str());

	const auto compositeStruct = Grafkit::Utils::Signature::SignatureString<Line>::CalcString();
	ASSERT_TRUE(compositeStruct.size());
	ASSERT_STREQ("Point p0; Point p1; float w;", compositeStruct.c_str());

	const auto baseTypes = Grafkit::Utils::Signature::SignatureString<TestingBaseTypes>::CalcString();
	ASSERT_TRUE(baseTypes.size());
	ASSERT_STREQ(
	    "bool b; char c; unsigned char uc; short s; unsigned short us; int i; unsigned int ui; long l; long long ll; unsigned long ul; unsigned long long ull; "
	    "float f; double d; std::string str;",
	    baseTypes.c_str());

	const auto stlTypes = Grafkit::Utils::Signature::SignatureString<TestingSTLTypes>::CalcString();
	ASSERT_TRUE(stlTypes.size());
	ASSERT_STREQ(
	    "std::vector<int> vi; std::vector<std::string> vs; std::deque<int> di; std::deque<std::string> ds; std::list<int> li; std::list<std::string> ls; "
	    "std::set<int> si; std::set<std::string> ss; std::multiset<int> msi; std::multiset<std::string> mss; std::map<int, std::string> mis; <unknown> umis; "
	    "std::multimap<int, std::string> mmis; <unknown> ummis;",
	    stlTypes.c_str());
}
