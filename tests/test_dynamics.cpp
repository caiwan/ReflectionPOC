#include <Serialization/Dynamics.h>
#include <Serialization/Serialization.h>
#include <gtest/gtest.h>

using Serializable = Grafkit::Attributes::Serializable;
using RegisteredType = Grafkit::Attributes::RegisteredType;

class DummyClass : public Grafkit::Serializer::DynamicObject
{
public:
	DummyClass() = default;

	// private:
	int x{};
	int y{};

	DYNAMICS_DECL(DummyClass)
};

REFL_TYPE(DummyClass, bases<>)
REFL_FIELD(x, Serializable())
REFL_FIELD(y, Serializable())
REFL_END

DYNAMICS_IMPL(DummyClass)

// ======

class DummyClassInherited : public DummyClass
{
public:
	DummyClassInherited() = default;

	// private:
	int z{};

	DYNAMICS_DECL(DummyClassInherited)
};

REFL_TYPE(DummyClassInherited, bases<DummyClass>)
REFL_FIELD(z, Serializable())
REFL_END

DYNAMICS_IMPL(DummyClassInherited)

// ======

class DummyBase : public Grafkit::Serializer::DynamicObject
{
public:
	virtual int Thing() = 0;
};

REFL_TYPE(DummyBase, bases<>)
REFL_END

class DummyA : public DummyBase
{
public:
	int Thing() override { return 1; }

	DYNAMICS_DECL(DummyA)
};

REFL_TYPE(DummyA, bases<DummyBase>)
REFL_END

DYNAMICS_IMPL(DummyA)

class DummyB : public DummyBase
{
public:
	virtual int Thing() { return 2; }

	DYNAMICS_DECL(DummyB)
};

REFL_TYPE(DummyB, bases<DummyBase>)
REFL_END

DYNAMICS_IMPL(DummyB)

// ======

TEST(Dynamics, Hello0)
{
	DummyClass d;
	refl::runtime::debug(std::cout, d, false);

	std::cout << '\n';

	DummyClassInherited di;
	refl::runtime::debug(std::cout, di, false);

	std::cout << '\n';
}

TEST(Dynamics, Hello1)
{
	DummyBase * da = new DummyA();
	refl::runtime::debug(std::cout, da, false);
	std::cout << '\n';
	delete da;

	std::cout << '\n';

	DummyBase * db = new DummyB();
	refl::runtime::debug(std::cout, db, false);
	std::cout << '\n';
	delete db;

	std::cout << '\n';
}
