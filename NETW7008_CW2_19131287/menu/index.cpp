/* Copyright(c) 2021  Alexander Sworski
*  All Rights Reserved
*  Brookes ID: 19131287
*/
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTMLClasses.h"
#include "mysql/mysql.h"
#include <cgicc/HTTPRedirectHeader.h>

#include "mysql_connection.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

using namespace cgicc;
using namespace std;

Cgicc cgi;

/* Session Alive Function
    checks if the Session is active and if the username and IP adress are corrrect
    uses db user: stateful

    @param username
    the username

    @param SessionID
    the sessionID

    @return true, if sessionID active and IP and username correct
*/
bool SessionAlive(string username, string SessionID) {
    string DBusername = "";
    bool DBSessionStatus = false;

    if (username.empty() || SessionID.empty()) {
        cout << HTTPRedirectHeader("/?error=Please Login") << endl;
        exit(0);
    }

    const CgiEnvironment& env = cgi.getEnvironment();
    string ENV = env.getRemoteAddr();

    try {
        time_t     zero = time(0);
        struct tm  tstruct;
        char       timenow[80];
        tstruct = *localtime(&zero);
        strftime(timenow, sizeof(timenow), "%Y-%m-%d %X", &tstruct);

        sql::Driver* driver;
        sql::Connection* con;
        sql::Statement* stmt;
        sql::ResultSet* res;

        driver = get_driver_instance();
        con = driver->connect("tcp://127.0.0.1:3306", "stateful", "dajuyihobumasopin7qefiroa");
        con->setSchema("markmanager");

        stmt = con->createStatement();
        string select = "SELECT fk_user,SessionID,active FROM sessions WHERE IP='" + ENV + "' AND SessionID = '" + SessionID + "' AND expires > '" + timenow + "'";
        res = stmt->executeQuery(select);
        while (res->next()) {
            string dbSession = res->getString("SessionID");
            if (dbSession == SessionID) {
                DBusername = res->getString("fk_user");
                DBSessionStatus = res->getBoolean("active");
                delete res;
                delete stmt;
                delete con;
                break;
            }
        }
    }
    catch (sql::SQLException& e) {
        cout << HTTPRedirectHeader("/?error=Session expired") << endl;
        exit(0);
    }

    //if(SessionID exists)
    if (DBusername == username) { //username and SessionID fit
        if (DBSessionStatus) //sessionID active
            if (username == "admin") {
                return true;
            }
            else {
                cout << HTTPRedirectHeader("/auth.cgi") << endl;
                exit(0);
            }
        else {
            cout << HTTPRedirectHeader("/?error=Session expired") << endl;
            exit(0);
        }
    }
    else {
        cout << HTTPRedirectHeader("/?error=Session expired") << endl;
        exit(0);
    }
}

/* escape SQL Funciton
    removes all characters, that could be user to make a valid sql injection
    only leaves alphanumerical and . _ @ characters in the string

    @param input
    the input string that should be stripped

    @return the stripped input string
*/
string escape_sql(string input) {
    for (string::iterator i = input.begin(); i != input.end(); i++)
    {
        char c = input.at(i - input.begin());
        if (!(isalnum(c) || c == '@' || c == '.' || c == '_'))
        {
            input.erase(i);
            i--;
        }
    }
    return input;
}

/* Main Funcion
    the main function
    It receives the Cookies paramters and shows all menu options to the admin

    @COOKIEparam username
    the username
    @COOKIEparam SessionID
    the sesssionID
*/
int main()
{
    string username = "";
    string SessionID = "";

    #pragma region GetCookies
    const_cookie_iterator cci;
    const CgiEnvironment& env = cgi.getEnvironment();

    for (cci = env.getCookieList().begin(); cci != env.getCookieList().end(); ++cci) {
        if (cci->getName() == "SessionID") {
            SessionID = escape_sql(cci->getValue());
        }
        if (cci->getName() == "username") {
            username = escape_sql(cci->getValue());
        }
    }
    #pragma endregion

    if (SessionAlive(username, SessionID)) {

        try {

            cout << HTTPHTMLHeader() << endl;
            cout << html() << head(title("Mark Manager")).add(link().set("rel", "stylesheet").set("href", "/css/main.css").set("style", "text/css")) << endl;
            cout << link().set("rel", "stylesheet").set("href", "/css/list.css").set("style", "text/css") << endl;
            cout << head() << endl;
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
            cout << p("Admin Board");
            cout << cgicc::div() << endl;

            cout << a().set("target", "_self").set("href", "/admin/courses/");
            cout << cgicc::div().set("class", "listelement") << endl;
            cout << "All Courses";
            cout << cgicc::div() << endl;
            cout << a() << endl;
            cout << "<br>" << endl;
            cout << a().set("target", "_self").set("href", "/admin/students.cgi");
            cout << cgicc::div().set("class", "listelement") << endl;
            cout << "All Students";
            cout << cgicc::div() << endl;
            cout << a() << endl;

            cout << body() << endl;
            cout << html();
        }
        catch (exception& e) {
            cout << "This should not have happend. Please try to reload the webpage." << endl;
        }
    }

}