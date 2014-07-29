#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include <sstream>
#include <array>
#include <string>
#include <tuple>
#include <vector>
#include <iterator>
#include <exception>
#include <cmath>

#include <boost/variant.hpp>

#include "lexer.h"
#include "tree.h"

class parse_error : public std::exception
{
    std::string msg;

public:
    parse_error(std::string _msg): exception(), msg(_msg) {}

    const char* what() const noexcept
    {
        return msg.c_str();
    }
};

template <typename Iterator>
struct parser_state
{
    Iterator head, last;
    token lookahead;

    void scan()
    {
        lookahead = get_token(head, last);
    }

    parser_state(Iterator _first, Iterator _last): head(_first), last(_last), lookahead(token_tag::Invalid)
    {
        scan();
    }
};

template <typename Iterator>
parser_state<Iterator> initialize_parser(Iterator first, Iterator last)
{
    return parser_state<Iterator>(first, last);
}

parser_state<std::istreambuf_iterator<char>> initialize_parser(std::istream& is)
{
    return parser_state<std::istreambuf_iterator<char>>(std::istreambuf_iterator<char>(is), std::istreambuf_iterator<char>());
}

template <typename Iterator>
void throw_parse_error(parser_state<Iterator>& s, std::string rule, std::string expected)
{
    using namespace std;

    ostringstream o;
    o << "In rule " << rule << ": expected " << expected << ", got ";

    if(s.lookahead.type == token_tag::EOI)
        o << "end-of-input.";
    else if(s.lookahead.type == token_tag::Character)
        o << '\'' << boost::get<char>(s.lookahead.value) << "'.";
    else if(s.lookahead.type == token_tag::Number)
        o << "number " << boost::get<double>(s.lookahead.value) << '.';
    else
        o << "invalid token.";

    throw parse_error(o.str());
}

template <typename Iterator>
void parse_expression(parser_state<Iterator>&, t_expression<double>&);

template <typename Iterator>
void parse_factor(parser_state<Iterator>&, t_expression<double>&);

template <typename Iterator>
void parse_parenthesized_expression(parser_state<Iterator>& s, t_expression<double>& t)
{
    if(s.lookahead.type != token_tag::Character || boost::get<char>(s.lookahead.value) != '(')
        throw_parse_error(s, "parenthesized-expression", "'('");

    s.scan();

    parse_expression(s, t);

    if(s.lookahead.type != token_tag::Character || boost::get<char>(s.lookahead.value) != ')')
        throw_parse_error(s, "parenthesized-expression", "')'");
}

template <typename Iterator>
void parse_atom(parser_state<Iterator>& s, t_expression<double>& t)
{
    using namespace std;

    char op;
    auto type = s.lookahead.type;

    if(type == token_tag::Number)
    {
        t = boost::get<double>(s.lookahead.value);
    }
    else if(type == token_tag::Identifier)
    {
        t = t_var_occurrance<double>(move(boost::get<string>(s.lookahead.value)));
    }
    else if(type == token_tag::Character)
    {
        parse_parenthesized_expression(s, t);
    }
    else
        throw_parse_error(s, "atom", "number, identifier, or '('");

    s.scan();

    if(s.lookahead.type == token_tag::Character)
    {
        op = boost::get<char>(s.lookahead.value);

        if(op == '^')
        {
            s.scan();

            t_expression<double> c;
            parse_factor(s, c);
            t = t_exponentiate<double>(t, c);
        }
    }
}

template <typename Iterator>
void parse_factor(parser_state<Iterator>& s, t_expression<double>& t)
{
    if(s.lookahead.type == token_tag::Character)
    {
        char op = boost::get<char>(s.lookahead.value);

        if(op == '+')
        {
            s.scan();
            parse_factor(s, t);
        }
        else if(op == '-')
        {
            s.scan();

            t_expression<double> c;
            parse_factor(s, c);
            t = t_negate<double>(c);
        }
        else
            parse_atom(s, t);
    }
    else
        parse_atom(s, t);
}

template <typename Iterator>
void parse_term(parser_state<Iterator>& s, t_expression<double>& t)
{
    parse_factor(s, t);
    while(s.lookahead.type == token_tag::Character)
    {
        char op = boost::get<char>(s.lookahead.value);
        if(op != '*' && op != '/')
            break;

        s.scan();

        t_expression<double> c;
        parse_factor(s, c);
        if(op == '*')
            t = t_multiply<double>(t, c);
        else
            t = t_divide<double>(t, c);
    }
}

template <typename Iterator>
void parse_expression(parser_state<Iterator>& s, t_expression<double>& t)
{
    if(s.lookahead.type == token_tag::Character && boost::get<char>(s.lookahead.value) == ')')
        throw_parse_error(s, "expression", "number, identifier, '+', '-' or '('");

    parse_term(s, t);

    while(s.lookahead.type == token_tag::Character)
    {
        char op = boost::get<char>(s.lookahead.value);

        if(op == '+' || op == '-')
        {
            s.scan();

            t_expression<double> c;
            parse_term(s, c);
            if(op == '+')
                t = t_add<double>(t, c);
            else
                t = t_subtract<double>(t, c);
        }
        else break;
    }
}

struct operator_properties
{
    unsigned int precedence;
    bool left_associative;
};

template <typename Iterator>
void parse_definition(parser_state<Iterator>& s, t_statement<double>& t)
{
    using namespace std;

    string name = move(boost::get<string>(s.lookahead.value));

    s.scan();

    if(s.lookahead.type == token_tag::Character && boost::get<char>(s.lookahead.value) == '=')
    {
        s.scan();

        t_expression<double> e;
        parse_expression(s, e);
        t = t_var_definition<double>(move(name), std::move(e));
    }
}

template <typename Iterator>
void parse_root(parser_state<Iterator>& s, t_statement<double>& t)
{
    if(s.lookahead.type == token_tag::Identifier && boost::get<std::string>(s.lookahead.value) == "define")
    {
        s.scan();
        parse_definition(s, t);
    }
    else
    {
        t_expression<double> e;
        parse_expression(s, e);
        t = std::move(e);
    }

    if(s.lookahead.type != token_tag::EOI)
        throw_parse_error(s, "root", "end-of-input");
}

#endif // PARSER_H_INCLUDED
