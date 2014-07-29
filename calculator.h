#ifndef CALCULATOR_H_INCLUDED
#define CALCULATOR_H_INCLUDED

#include <string>
#include <unordered_map>

#include <exception>
#include <utility>

#include <boost/variant.hpp>

#include "tree.h"

template <typename NumType>
struct calculator_state
{
    std::unordered_map<std::string, NumType> variable_set;
};

class eval_error : public std::exception
{
    std::string msg;

public:

    eval_error(std::string _msg): msg(std::move(_msg)) {}

    const char* what() const noexcept
    {
        return msg.c_str();
    }
};

template <typename NumType>
NumType eval_expression_tree(const calculator_state<double>& c, const t_expression<NumType>& t)
{
    struct visitor_t : public boost::static_visitor<NumType>
    {
        const calculator_state<double>& c;

        visitor_t(const calculator_state<double>& _c): c(_c) {}

        NumType operator()(const NumType& n)
        {
            return n;
        }
        NumType operator()(const t_var_occurrance<NumType>& t)
        {
            auto it = c.variable_set.find(t.name);

            if(it != c.variable_set.end())
            {
                return it->second;
            }
            else
            {
                throw eval_error("Undefined variable");
            }
        }
        NumType operator()(const t_arg_placeholder<NumType>& t)
        {
            throw std::logic_error("t_arg_placeholder encountered while evaluating expression");
        }
        NumType operator()(const t_func_invocation<NumType>& t)
        {
            throw eval_error("Unimplemented");
        }
        NumType operator()(const t_negate<NumType>& t)
        {
            return -boost::apply_visitor(*this, t.op);
        }
        NumType operator()(const t_add<NumType>& t)
        {
            return boost::apply_visitor(*this, t.ops[0]) + boost::apply_visitor(*this, t.ops[1]);
        }
        NumType operator()(const t_subtract<NumType>& t)
        {
            return boost::apply_visitor(*this, t.ops[0]) - boost::apply_visitor(*this, t.ops[1]);
        }
        NumType operator()(const t_multiply<NumType>& t)
        {
            return boost::apply_visitor(*this, t.ops[0]) * boost::apply_visitor(*this, t.ops[1]);
        }
        NumType operator()(const t_divide<NumType>& t)
        {
            return boost::apply_visitor(*this, t.ops[0]) / boost::apply_visitor(*this, t.ops[1]);
        }
        NumType operator()(const t_exponentiate<NumType>& t)
        {
            return std::pow(boost::apply_visitor(*this, t.ops[0]), boost::apply_visitor(*this, t.ops[1]));
        }
    } visitor(c);

    return boost::apply_visitor(visitor, t);
}

template <typename NumType>
void process_variable_definition(calculator_state<NumType>& c, const t_var_definition<NumType>& t)
{
    NumType n = eval_expression_tree(c, t.val);
    c.variable_set[t.name] = n;
}

#endif // CALCULATOR_H_INCLUDED
