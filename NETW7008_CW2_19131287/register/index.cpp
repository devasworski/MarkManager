/* Copyright(c) 2021  Alexander Sworski
*  All Rights Reserved
*  Brookes ID: 19131287
*/
#include <iostream>
#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTMLClasses.h"
#include "mysql/mysql.h"

using namespace cgicc;
using namespace std;

Cgicc cgi;

/*Page Function
    Prints the HTML code of the registration index page

    @param message
    the message that will be display on the page
    Defaul value empty
*/
void Page(string message = "") {
    try {

        cout << HTTPHTMLHeader() << endl;
        cout << html() << head(title("Mark Manager")).add(link().set("rel", "stylesheet").set("href", "../css/main.css").set("style", "text/css")) << endl;
        cout << body();
        cout << cgicc::div().set("class", "sublogotop") << endl;
        cout << p();
        cout << a("M").set("class", "capital");
        cout << "ark";
        cout << a("M").set("class", "capital");
        cout << "anager";
        cout << p();
        cout << cgicc::div() << endl;
        cout << cgicc::div().set("class", "sublogobot") << endl;
        cout << p("Register");
        cout << cgicc::div() << endl;

        cout << form().set("method", "post").set("action", "/register/confirm.cgi") << endl;
        cout << p("Username").set("class", "lable") << endl;
        cout << input().set("type", "text").set("name", "username").set("class", "input") << endl;
        cout << "<br>" << endl;
        cout << p("Password").set("class", "lable") << endl;
        cout << input().set("type", "password").set("name", "password").set("class", "input") << endl;
        cout << "<br>" << endl;
        cout << p("Email").set("class", "lable") << endl;
        cout << input().set("type", "email").set("name", "email").set("class", "input") << endl;
        cout << "<br>" << endl;
        cout << p("Full Name").set("class", "lable") << endl;
        cout << input().set("type", "text").set("name", "fullname").set("class", "input") << endl;
        cout << "<br>" << endl;
        if (!message.empty()) {
            cout << p(message).set("class", "error") << endl;
            cout << "<br>" << endl;
        }
        cout << input().set("type", "submit").set("value", "Register").set("name", "type").set("class", "button") << endl;
        cout << "<br>" << endl;
        cout << form() << endl;
        cout << body() << endl;
        cout << html();
    }
    catch (exception& e) {
        cout << "This should not have happend. Please try to reload the webpage." << endl; 
    }
}

/* Main Function
    Calls the function startpage and checks if a error message should be shown in the page

    @POSTparam error
    a message directly shwon on the http page
*/
int main()
{
    string error = "";
    form_iterator fi = cgi.getElement("error");
    if (!fi->isEmpty() && fi != (*cgi).end())
        error = **fi;
    Page(error);
}

