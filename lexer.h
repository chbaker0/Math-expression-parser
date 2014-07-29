#ifndef LEXER_H_INCLUDED
#define LEXER_H_INCLUDED

#include <string>

#include <cstdlib>
#include <utility>
#include <exception>

#include <boost/variant.hpp>

class lex_error : public std::exception
{
public:
    const char* what() const noexcept
    {
        return "Unexpected character in input to lexer";
    }
};

enum class token_tag
{
    Invalid,
    Number,
    Character,
    Identifier,
    EOI
};

struct token
{
    token_tag type;
    boost::variant<boost::blank, double, char, std::string> value;

    token(): type(token_tag::Invalid), value() {}
    token(token_tag _type): type(_type), value() {}
    template <typename ValueType>
    token(token_tag _type, ValueType&& _value): type(_type), value(std::forward<ValueType>(_value)) {}
};

template <typename Iterator>
void skip_spaces(Iterator& first, Iterator last)
{
    while(first != last && isspace(*first)) ++first;
}

template <typename Iterator>
token get_token(Iterator& first, Iterator last)
{
    using namespace std;

    skip_spaces(first, last);
    if(first == last) return token(token_tag::EOI);

    string temp;
    bool decimal = false;

    switch(*first)
    {
    case '+':
    case '-':
    case '*':
    case '/':
    case '^':
    case '(':
    case ')':
    case '=':
        goto accept_operator;

    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j': case 'l': case 'm': case 'n':
    case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
        goto id;

    case '.':
        decimal = true;
        // Intentional fall-through
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        goto number;

    default:
        throw lex_error();
    }

    accept_operator:
        return token(token_tag::Character, *first++);

    id:
        do
        {
            temp.push_back(*first++);
        } while(first != last && islower(*first));
        goto accept_id;

    accept_id:
        return token(token_tag::Identifier, std::move(temp));

    number:
        temp.push_back(*first++);
        if(first != last)
        {
            if(isdigit(*first))
                goto number;
            else if(*first == '.')
            {
                if(decimal == false)
                {
                    decimal = true;
                    goto number;
                }
                else
                    throw lex_error();
            }
        }
        goto accept_number;


    accept_number:
        if(decimal && temp.size() == 1)
            throw lex_error();
        char *dummy;
        double val = strtod(temp.c_str(), &dummy);
        return token(token_tag::Number, val);
}

#endif // LEXER_H_INCLUDED
