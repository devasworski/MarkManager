/* Copyright(c) 2021  Alexander Sworski
*  All Rights Reserved
*  Brookes ID: 19131287
*/
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTMLClasses.h"

#include <cgicc/HTTPRedirectHeader.h>
#include "mysql_connection.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

using namespace cgicc;
using namespace std;

Cgicc cgi;

/* Insert Query Function
    Used to execute alter statements within the database
    uses db user: user

    @param select
    The SQL statement
*/
void InsterQuery(string select) {
    try {
        sql::Driver* driver;
        sql::Connection* con;
        sql::Statement* stmt;
        sql::ResultSet* res;

        driver = get_driver_instance();
        con = driver->connect("tcp://127.0.0.1:3306", "user", "d9pifetoyesad2cekipoyolis");
        con->setSchema("markmanager");

        stmt = con->createStatement();
        stmt->execute(select);
    }
    catch (sql::SQLException& e) {
        cout << HTTPRedirectHeader("/?error=Couldn't safe changes") << endl;
        exit(0);
    }
}

/* Make Query Function
    Used to execute SELECT statements, which give a SQL result back
    uses db user: user

    @param select
    SQL query

    @return SQL::ResultSet
    the SQL result
*/
sql::ResultSet* MakeQuery(string select) {
    try {
        sql::Driver* driver;
        sql::Connection* con;
        sql::Statement* stmt;
        sql::ResultSet* res;

        driver = get_driver_instance();
        con = driver->connect("tcp://127.0.0.1:3306", "user", "d9pifetoyesad2cekipoyolis");
        con->setSchema("markmanager");

        stmt = con->createStatement();
        return res = stmt->executeQuery(select);
    }
    catch (sql::SQLException& e) {
        cout << HTTPRedirectHeader("/?error=Session expired") << endl;
        exit(0);
    }
}

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
        string select = "SELECT fk_user,SessionID,active FROM sessions WHERE IP='"+ENV+"' AND SessionID = '" + SessionID + "' AND expires > '" + timenow + "'";
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
    if (DBusername == username) { //usernam and SessionID fit
        if (DBSessionStatus) //sessionID active
            if (username == "admin") {
                cout << HTTPRedirectHeader("/auth.cgi") << endl;
                exit(0);
            }
            else {
                return true;
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

/* Course name valid Function
    Checks if it is a course of the lecturer

    @param username
    the username of the user, that tries to access the function

    @param courseid
    the courseid of the course that should be opened

    @return true, if the course is one of the courses of the lectuere
*/
bool CnameValid(string username, string courseid) {
    bool RESULT = false;
    sql::ResultSet* res = MakeQuery("SELECT idmodules FROM modules WHERE active='1' AND idmodules='"+ courseid +"' AND idmodules IN (SELECT modules_idmodules FROM modules_has_lectures WHERE login_name='" + username + "')");
    while (res->next()) {
        if (res->getString("idmodules") == courseid) RESULT = true;
    }
    return RESULT;
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
    It receives the GET, POST and Cookies paramters and allows to edit the grade of the students within the course

    @GETparam courseid
    the courseid of the course that the user wants to accesse

    @POSTparam student
    the id of the student, whos grade should be edited
    @POSTparam grade
    the new grade

    @COOKIEparam username
    the username
    @COOKIEparam SessionID
    the sesssionID
*/
int main()
{
    string username = "";
    string SessionID = "";
    string courseid = "";
    string coursename = "";

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

    #pragma region GetCourse
    form_iterator fi = cgi.getElement("c");
    if (!fi->isEmpty() && fi != (*cgi).end())
        courseid = escape_sql(**fi);

    sql::ResultSet* res = MakeQuery("SELECT name FROM modules WHERE idmodules='"+courseid+"'");
    if(res->next())
        coursename = res->getString("name");

    #pragma endregion

    if (SessionAlive(username, SessionID)&&CnameValid(username, courseid)) {

        #pragma region SafeGrade
        string newgrade = "";
        string student = "";
        form_iterator fi = cgi.getElement("student");
        if (!fi->isEmpty() && fi != (*cgi).end()) {
            student = escape_sql(**fi);
            form_iterator fi = cgi.getElement("grade");
            if (!fi->isEmpty() && fi != (*cgi).end()) 
                newgrade = escape_sql(**fi);
            if(!student.empty()&&!newgrade.empty())
                InsterQuery("UPDATE students_has_modules LEFT JOIN students ON students_has_modules.students_idstudents = students.idstudents SET grade='" + newgrade + "' WHERE active='1' AND modules_idmodules ='" + courseid + "' AND students_idstudents='"+student+"'");
        }

        #pragma endregion


        sql::ResultSet* res = MakeQuery("SELECT Name, grade, idstudents FROM students LEFT JOIN students_has_modules ON students_has_modules.students_idstudents = students.idstudents WHERE active='1' AND modules_idmodules ='"+courseid+"';");
        try {

            cout << HTTPHTMLHeader() << endl;
            cout << html() << head(title("Mark Manager")).add(link().set("rel", "stylesheet").set("href", "/css/main.css").set("style", "text/css")) << endl;
            cout << link().set("rel", "stylesheet").set("href", "/css/list.css").set("style", "text/css") << endl;
            cout << body();
            cout << p("user: " + username) << endl;
            cout << a("back").set("href", "/lecturer/") << endl;
            cout << cgicc::div().set("class", "sublogotop") << endl;
            cout << p();
            cout << a("M").set("class", "capital");
            cout << "ark";
            cout << a("M").set("class", "capital");
            cout << "anager";
            cout << p();
            cout << cgicc::div() << endl;
            cout << cgicc::div().set("class", "sublogobot") << endl;
            cout << p(coursename);
            cout << cgicc::div() << endl;

            while (res->next()) {
                cout << cgicc::div().set("class", "listelement") << endl;
                cout << table().set("class", "listtable") << endl;
                cout << "<tr>" << endl;
                cout << form().set("method", "post") << endl;
                cout << "<td class='listtableleft'>" << endl;
                cout << p(res->getString("name")+", id: "+res->getString("idstudents")).set("class", "listlabel") << endl;
                cout << "</td>" << endl;
                cout << "<td class='listtablemiddle'>" << endl;
                cout << input().set("type", "number").set("name", "grade").set("value", res->getString("grade")).set("class", "listinput") << endl;
                cout << "</td>" << endl;
                cout << "<td class='listtabelright'>" << endl;
                cout << button().set("type", "submit").set("value", res->getString("idstudents")).set("class", "listbutton").set("name", "student");
                cout << "Safe";
                cout << button() << endl;
                cout << "</td>" << endl;
                cout << form() << endl;
                cout << "</tr>" << endl;
                cout << table() << endl;
                cout << cgicc::div() << endl;
                cout << "<br>" << endl;
            }

            cout << body() << endl;
            cout << html();
        }
        catch (exception& e) {
            cout << "This should not have happend. Please try to reload the webpage." << endl; 
        }
    }
}