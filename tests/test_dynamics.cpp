#include <Serialization/Dynamics.h>
#include <Serialization/Serialization.h>
#include <gtest/gtest.h>

using Serializable = Grafkit::Attributes::Serializable;
using DynamicObject = Grafkit::Serializer::DynamicObject;

class DummyClass : public DynamicObject
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

class DummyBase : public DynamicObject
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

TEST(Dynamics, DynamicsStore)
{
	const auto & dynamics = Grafkit::Serializer::Dynamics::Instance();
	ASSERT_TRUE(dynamic_cast<DummyClass *>(dynamics.Create("DummyClass")));
	ASSERT_TRUE(dynamic_cast<DummyClassInherited *>(dynamics.Create("DummyClassInherited")));
	ASSERT_TRUE(dynamic_cast<DummyB *>(dynamics.Create("DummyB")));
	ASSERT_TRUE(dynamic_cast<DummyA *>(dynamics.Create("DummyA")));
	ASSERT_FALSE(dynamics.Create("Does not exist"));
}

// Serialize dynamic classes

class B
{
public:
	virtual void b() = 0;
	virtual void bc() const = 0;
};

class C : public B
{
};
