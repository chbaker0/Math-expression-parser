#ifndef TREE_TRANSFORM_H_INCLUDED
#define TREE_TRANSFORM_H_INCLUDED

#include <utility>

#include <boost/variant.hpp>
#include <boost/optional.hpp>

#include "tree.h"

template <typename Child, typename NumType, typename ResultType>
struct tree_transform : public boost::static_visitor<ResultType>
{
    typedef NumType num_type;

    t_expression<NumType>& node;

    tree_transform(t_expression<NumType>& _node): node(_node) {}

    template <typename Arg>
    ResultType operator()(Arg&& arg)
    {
        return static_cast<Child&>(*this)(std::forward<Arg>(arg));
    }
};

template <typename TransformType>
typename TransformType::result_type apply_transform(t_expression<typename TransformType::num_type>& tree)
{
    tree_transform<TransformType, typename TransformType::num_type, typename TransformType::result_type> transform(tree);
    return boost::apply_visitor(transform, tree);
}

template <typename NumType>
struct tree_fold : tree_transform<tree_fold<NumType>, NumType, boost::optional<NumType>>
{
    typedef tree_transform<tree_fold<NumType>, NumType, boost::optional<NumType>> parent;
    boost::optional<NumType> operator()(NumType n)
    {
        return n;
    }
    boost::optional<NumType> operator()(t_add<NumType>& t)
    {
        auto lhs = apply_transform<tree_fold>(t.ops[0]),
             rhs = apply_transform<tree_fold>(t.ops[1]);

        if(lhs && rhs)
        {
            auto result = *lhs + *rhs;
            parent::node = result;
            return result;
        }
        else
            return boost::optional<NumType>();
    }
    boost::optional<NumType> operator()(t_subtract<NumType>& t)
    {
        auto lhs = apply_transform<tree_fold>(t.ops[0]),
             rhs = apply_transform<tree_fold>(t.ops[1]);

        if(lhs && rhs)
        {
            auto result = *lhs - *rhs;
            parent::node = result;
            return result;
        }
        else
            return boost::optional<NumType>();
    }
    boost::optional<NumType> operator()(t_multiply<NumType>& t)
    {
        auto lhs = apply_transform<tree_fold>(t.ops[0]),
             rhs = apply_transform<tree_fold>(t.ops[1]);

        if(lhs && rhs)
        {
            auto result = *lhs * *rhs;
            parent::node = result;
            return result;
        }
        else
            return boost::optional<NumType>();
    }
    boost::optional<NumType> operator()(t_divide<NumType>& t)
    {
        auto lhs = apply_transform<tree_fold>(t.ops[0]),
             rhs = apply_transform<tree_fold>(t.ops[1]);

        if(lhs && rhs)
        {
            auto result = *lhs / *rhs;
            parent::node = result;
            return result;
        }
        else
            return boost::optional<NumType>();
    }
    boost::optional<NumType> operator()(t_exponentiate<NumType>& t)
    {
        auto lhs = apply_transform<tree_fold>(t.ops[0]),
             rhs = apply_transform<tree_fold>(t.ops[1]);

        using std::pow;

        if(lhs && rhs)
        {
            auto result = pow(*lhs, *rhs);
            parent::node = result;
            return result;
        }
        else
            return boost::optional<NumType>();
    }
    template <typename Arg>
    boost::optional<NumType> operator()(Arg& arg)
    {
        return boost::optional<NumType>();
    }
};

#endif // TREE_TRANSFORM_H_INCLUDED
