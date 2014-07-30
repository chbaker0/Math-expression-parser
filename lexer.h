#ifndef LEXER_H_INCLUDED
#define LEXER_H_INCLUDED

#include <string>

#include <cstdlib>
#include <utility>
#include <exception>

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
//    boost::variant<boost::blank, double, char, std::string> value;

    union _val
    {
        double d;
        char c;
        std::string str;

        _val() {}
        ~_val() {}
    } val;

    void destroy()
    {
        switch(type)
        {
        case token_tag::Identifier:
            using std::string;
            val.str.~string();
            break;

        default:
            break;
        }
    }

    token(): type(token_tag::Invalid) {}
    token(token_tag t): type(t) {}

    token(token_tag t, std::string str)
    {
        type = t;
        new (&val.str) std::string(std::move(str));
    }
    token(token_tag t, char c)
    {
        type = t;
        val.c = c;
    }
    token(token_tag t, double d)
    {
        type = t;
        val.d = d;
    }

    token(const token& cp)
    {
        switch(cp.type)
        {
        case token_tag::Identifier:
            new (&val.str) std::string(cp.val.str);
            break;

        case token_tag::Character:
            val.c = cp.val.c;
            break;

        case token_tag::Number:
            val.d = cp.val.d;
            break;

        default:
            break;
        }

        type = cp.type;
    }

    token(token&& mv)
    {
        switch(mv.type)
        {
        case token_tag::Identifier:
            new (&val.str) std::string(std::move(mv.val.str));
            break;

        case token_tag::Character:
            val.c = mv.val.c;
            break;

        case token_tag::Number:
            val.d = mv.val.d;
            break;

        default:
            break;
        }

        type = mv.type;
    }

    ~token()
    {
        destroy();
    }

    token& operator=(token mv)
    {
        this->~token();
        new (this) token(std::move(mv));
        return *this;
    }
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

    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j': case 'k': case 'l': case 'm':
    case 'n': case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
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
