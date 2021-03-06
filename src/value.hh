#ifndef value_hh_INCLUDED
#define value_hh_INCLUDED

#include <memory>
#include <unordered_map>

#include "units.hh"

namespace Kakoune
{

struct bad_value_cast {};

struct Value
{
    Value() = default;

    template<typename T>
    Value(const T& val) : m_value{new Model<T>{val}} {}

    template<typename T>
    Value(T&& val) : m_value{new Model<T>{std::move(val)}} {}

    Value(const Value& val)
    {
        if (val.m_value)
            m_value.reset(val.m_value->clone());
    }

    Value(Value&&) = default;

    Value& operator=(const Value& val)
    {
        if (val.m_value)
            m_value.reset(val.m_value->clone());
        else
            m_value.reset();
        return *this;
    }

    Value& operator=(Value&& val) = default;

    explicit operator bool() const { return (bool)m_value; }

    template<typename T>
    bool is_a() const
    {
        return m_value and m_value->type() == typeid(T);
    }

    template<typename T>
    T& as()
    {
        if (not is_a<T>())
            throw bad_value_cast{};
        return static_cast<Model<T>*>(m_value.get())->m_content;
    }

    template<typename T>
    const T& as() const
    {
        return const_cast<Value*>(this)->as<T>();
    }

private:
    struct Concept
    {
        virtual ~Concept() {}
        virtual const std::type_info& type() const = 0;
        virtual Concept* clone() const = 0;
    };

    template<typename T>
    struct Model : public Concept
    {
        Model(const T& val) : m_content(val) {}
        Model(T&& val) : m_content(std::move(val)) {}

        const std::type_info& type() const override { return typeid(T); }
        Concept* clone() const { return new Model(m_content); }

        T m_content;
    };

    std::unique_ptr<Concept> m_value;
};

struct ValueId : public StronglyTypedNumber<ValueId, int>
{
    constexpr ValueId(int value = 0) : StronglyTypedNumber<ValueId>(value) {}

    static ValueId get_free_id()
    {
        static ValueId next;
        return next++;
    }
};

using ValueMap = std::unordered_map<ValueId, Value>;

}

namespace std
{

template<>
struct hash<Kakoune::ValueId>
{
    size_t operator()(Kakoune::ValueId val) const
    {
        return std::hash<int>()((int)val);
    }
};

}


#endif // value_hh_INCLUDED
