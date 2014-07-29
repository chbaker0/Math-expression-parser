#ifndef TREE_H_INCLUDED
#define TREE_H_INCLUDED

#include <iostream>
#include <vector>
#include <utility>
#include <cmath>

#include <boost/variant.hpp>

template <typename> struct t_unary_op;
template <typename> struct t_binary_op;
template <typename> struct t_nary_op;

template <typename NumType>
struct t_negate : public t_unary_op<NumType>
{
    template <typename... Args>
    t_negate(Args&&... args): t_unary_op<NumType>(std::forward<Args>(args)...) {}
};

template <typename NumType>
struct t_add : public t_binary_op<NumType>
{
    template <typename... Args>
    t_add(Args&&... args): t_binary_op<NumType>(std::forward<Args>(args)...) {}
};

template <typename NumType>
struct t_subtract : public t_binary_op<NumType>
{
    template <typename... Args>
    t_subtract(Args&&... args): t_binary_op<NumType>(std::forward<Args>(args)...) {}
};

template <typename NumType>
struct t_multiply : public t_binary_op<NumType>
{
    template <typename... Args>
    t_multiply(Args&&... args): t_binary_op<NumType>(std::forward<Args>(args)...) {}
};

template <typename NumType>
struct t_divide : public t_binary_op<NumType>
{
    template <typename... Args>
    t_divide(Args&&... args): t_binary_op<NumType>(std::forward<Args>(args)...) {}
};

template <typename NumType>
struct t_exponentiate : public t_binary_op<NumType>
{
    template <typename... Args>
    t_exponentiate(Args&&... args): t_binary_op<NumType>(std::forward<Args>(args)...) {}
};

template <typename NumType>
struct t_var_occurrance
{
    std::string name;

    t_var_occurrance(std::string _name): name(std::move(_name)) {}
};

template <typename>
struct t_func_invocation;

template <typename NumType>
struct t_arg_placeholder
{
    unsigned int index;

    t_arg_placeholder(unsigned int _index): index(_index) {}
};

template <typename NumType>
using t_expression = boost::variant<
                                    NumType,
                                    boost::recursive_wrapper<t_var_occurrance<NumType>>,
                                    boost::recursive_wrapper<t_func_invocation<NumType>>,
                                    boost::recursive_wrapper<t_arg_placeholder<NumType>>,
                                    boost::recursive_wrapper<t_negate<NumType>>,
                                    boost::recursive_wrapper<t_add<NumType>>,
                                    boost::recursive_wrapper<t_subtract<NumType>>,
                                    boost::recursive_wrapper<t_multiply<NumType>>,
                                    boost::recursive_wrapper<t_divide<NumType>>,
                                    boost::recursive_wrapper<t_exponentiate<NumType>>
                                    >;

template <typename NumType>
struct t_func_invocation
{
    std::string name;
    std::vector<t_expression<NumType>> args;

    t_func_invocation(std::string _name, std::vector<t_expression<NumType>> _args): name(std::move(_name)), args(std::move(_args)) {}
};

template <typename NumType>
struct t_unary_op
{
    t_expression<NumType> op;

    template <typename Op>
    t_unary_op(Op&& _op): op(std::forward<Op>(_op)) {}
};

template <typename NumType>
struct t_binary_op
{
    t_expression<NumType> ops[2];

    template <typename Op1, typename Op2>
    t_binary_op(Op1&& op1, Op2&& op2): ops{std::forward<Op1>(op1), std::forward<Op2>(op2)} {}
};

template <typename NumType>
struct t_nary_op
{
    std::vector<t_expression<NumType>> ops;

    template <typename... Ops>
    t_nary_op(Ops&&... _ops): ops{std::forward<Ops>(_ops)...} {}
};

template <typename NumType>
struct t_var_definition
{
    std::string name;
    t_expression<NumType> val;

    t_var_definition(std::string _name, t_expression<NumType> _val): name(std::move(_name)), val(std::move(_val)) {}
};

template <typename NumType>
struct t_func_definition
{
    std::string name;
    unsigned int arity;
    t_expression<NumType> val;

    t_func_definition(std::string _name, unsigned int _arity, t_expression<NumType> _val): name(std::move(_name)), arity(_arity), val(std::move(_val)) {}
};

template <typename NumType>
using t_statement = boost::variant<
                                   t_expression<NumType>,
                                   t_var_definition<NumType>,
                                   t_func_definition<NumType>
                                   >;

enum class statement_type
{
    Expression,
    VarDefinition,
    FuncDefinition
};

template <typename NumType>
statement_type identify_statement(const t_statement<NumType>& t)
{
    switch(t.which())
    {
    case 0:
        return statement_type::Expression;
    case 1:
        return statement_type::VarDefinition;
    case 2:
        return statement_type::FuncDefinition;
    }
}

template <typename NumType>
void print_expression_tree(const t_expression<NumType>& t)
{
    using namespace std;

    struct visitor_t : public boost::static_visitor<>
    {
        unsigned int offset;

        visitor_t(unsigned int o): offset(o) {}

        void operator()(const NumType& n) const
        {
            for(auto i = offset; i > 0; --i)
                cout.put(' ');
            cout << n << endl;
        }
        void operator()(const t_var_occurrance<NumType>& t) const
        {
            for(auto i = offset; i > 0; --i)
                cout.put(' ');
            cout << "Variable " << t.name << endl;
        }
        void operator()(const t_func_invocation<NumType>& t) const
        {
            for(auto i = offset; i > 0; --i)
                cout.put(' ');
            cout << "Function " << t.name << endl;

            for(const auto& i : t.args)
            {
                boost::apply_visitor(visitor_t(offset+1), i);
            }
        }
        void operator()(const t_arg_placeholder<NumType>& t) const
        {
            for(auto i = offset; i > 0; --i)
                cout.put(' ');
            cout << "Argument " << t.index << endl;
        }
        void operator()(const t_negate<NumType>& t) const
        {
            for(auto i = offset; i > 0; --i)
                cout.put(' ');
            cout << "Negate" << endl;
            boost::apply_visitor(visitor_t(offset+1), t.op);
        }
        void operator()(const t_add<NumType>& t) const
        {
            for(auto i = offset; i > 0; --i)
                cout.put(' ');
            cout << "Add" << endl;
            boost::apply_visitor(visitor_t(offset+1), t.ops[0]);
            boost::apply_visitor(visitor_t(offset+1), t.ops[1]);
        }
        void operator()(const t_subtract<NumType>& t) const
        {
            for(auto i = offset; i > 0; --i)
                cout.put(' ');
            cout << "Subtract" << endl;
            boost::apply_visitor(visitor_t(offset+1), t.ops[0]);
            boost::apply_visitor(visitor_t(offset+1), t.ops[1]);
        }
        void operator()(const t_multiply<NumType>& t) const
        {
            for(auto i = offset; i > 0; --i)
                cout.put(' ');
            cout << "Multiply" << endl;
            boost::apply_visitor(visitor_t(offset+1), t.ops[0]);
            boost::apply_visitor(visitor_t(offset+1), t.ops[1]);
        }
        void operator()(const t_divide<NumType>& t) const
        {
            for(auto i = offset; i > 0; --i)
                cout.put(' ');
            cout << "Divide" << endl;
            boost::apply_visitor(visitor_t(offset+1), t.ops[0]);
            boost::apply_visitor(visitor_t(offset+1), t.ops[1]);
        }
        void operator()(const t_exponentiate<NumType>& t) const
        {
            for(auto i = offset; i > 0; --i)
                cout.put(' ');
            cout << "Exponentiate" << endl;
            boost::apply_visitor(visitor_t(offset+1), t.ops[0]);
            boost::apply_visitor(visitor_t(offset+1), t.ops[1]);
        }
    } visitor(0);

    boost::apply_visitor(visitor, t);
}

template <typename NumType>
void print_statement_tree(const t_statement<NumType>& t)
{
    using namespace std;

    struct visitor_t : public boost::static_visitor<>
    {
        void operator()(const t_expression<NumType>& t) const
        {
            cout << "Expression:" << endl;
            print_expression_tree(t);
        }
        void operator()(const t_var_definition<NumType>& t) const
        {
            cout << "Variable \"" << t.name << "\" definition:" << endl;
            print_expression_tree(t.val);
        }
        void operator()(const t_func_definition<NumType>& t) const
        {
            throw logic_error("Unimplemented");
        }
    } visitor;

    boost::apply_visitor(visitor, t);
}

//template <typename NumType>
//NumType eval_tree(const t_expression<NumType>& t)
//{
//    struct visitor_t : public boost::static_visitor<NumType>
//    {
//        NumType operator()(const NumType& n)
//        {
//            return n;
//        }
//        NumType operator()(const t_var_occurrance<NumType>& t)
//        {
//            return 0;
//        }
//        NumType operator()(const t_negate<NumType>& t)
//        {
//            return -boost::apply_visitor(*this, t.op);
//        }
//        NumType operator()(const t_add<NumType>& t)
//        {
//            return boost::apply_visitor(*this, t.ops[0]) + boost::apply_visitor(*this, t.ops[1]);
//        }
//        NumType operator()(const t_subtract<NumType>& t)
//        {
//            return boost::apply_visitor(*this, t.ops[0]) - boost::apply_visitor(*this, t.ops[1]);
//        }
//        NumType operator()(const t_multiply<NumType>& t)
//        {
//            return boost::apply_visitor(*this, t.ops[0]) * boost::apply_visitor(*this, t.ops[1]);
//        }
//        NumType operator()(const t_divide<NumType>& t)
//        {
//            return boost::apply_visitor(*this, t.ops[0]) / boost::apply_visitor(*this, t.ops[1]);
//        }
//        NumType operator()(const t_exponentiate<NumType>& t)
//        {
//            return std::pow(boost::apply_visitor(*this, t.ops[0]), boost::apply_visitor(*this, t.ops[1]));
//        }
//    } visitor;
//
//    return boost::apply_visitor(visitor, t);
//}

#endif // TREE_H_INCLUDED
