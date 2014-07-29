#include <iostream>
#include <string>
#include <exception>
#include <iterator>
#include <utility>

#include <boost/variant.hpp>

#include "lexer.h"
#include "parser.h"
#include "tree.h"
#include "calculator.h"

int main()
{
    using namespace std;

    calculator_state<double> calc;

    while(true)
    {
        string input;

        cout << ">> ";
        getline(cin, input);

        try
        {
            auto s = initialize_parser(input.begin(), input.end());

            t_statement<double> t;
            parse_root(s, t);

//            print_statement_tree(t);
//            cout << endl;

            auto type = identify_statement(t);
            if(type == statement_type::Expression)
            {
                cout << eval_expression_tree(calc, boost::get<t_expression<double>>(t)) << endl << endl;
            }
            else if(type == statement_type::VarDefinition)
            {
                process_variable_definition(calc, boost::get<t_var_definition<double>>(t));
            }
            else
                throw logic_error("Unimplemented");
        }
        catch(const exception& e)
        {
            cout << e.what() << endl << endl;
        }
    }

//    while(true)
//    {
//        string input;
//        getline(cin, input);
//
//        try
//        {
//            cout << "Results of tokenization:" << endl;
//
//            auto head = input.begin(), last = input.end();
//            token tok;
//            while((tok = get_token(head, last)).type != token_tag::EOI)
//            {
//                switch(tok.type)
//                {
//                case token_tag::Character:
//                    cout << '\'' << boost::get<char>(tok.value) << "' ";
//                    break;
//
//                case token_tag::Number:
//                    cout << "[Number " << boost::get<double>(tok.value) << "] ";
//                    break;
//
//                default:
//                    cout << "[Identifier " << boost::get<string>(tok.value) << "] ";
//                    break;
//                }
//            }
//
//            cout << endl << endl;
//
//            auto s = initialize_parser(input.begin(), input.end());
//
//            t_expression<double> t;
//            parse_root(s, t);
//
//            cout << "Successfully parsed \"" << string(input.begin(), s.head) << '"' << endl;
//            cout << "Tree: " << endl;
//            print_tree(t);
//            cout << endl << "Result: " << eval_tree(t) << endl << endl;
//        }
//        catch(const exception& e)
//        {
//            cerr << e.what() << endl << endl;
//        }
//    }
}
