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

/* Make Query Function
    Used to execute SELECT statements, which give a SQL result back
    uses db user: admin

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
        con = driver->connect("tcp://127.0.0.1:3306", "admin", "nocikocofot9com4cif1boq6t");
        con->setSchema("markmanager");

        stmt = con->createStatement();
        return res = stmt->executeQuery(select);
    }
    catch (sql::SQLException& e) {
        cout << HTTPRedirectHeader("/?error=Data not available") << endl;
        exit(0);
    }
}

/* Insert Query Function
    Used to execute alter statements within the database
    uses db user: admin

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
        con = driver->connect("tcp://127.0.0.1:3306", "admin", "nocikocofot9com4cif1boq6t");
        con->setSchema("markmanager");

        stmt = con->createStatement();
        stmt->execute(select);
    }
    catch (sql::SQLException& e) {
        cout << HTTPRedirectHeader("/?error=Couldn't safe changes") << endl;
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
    if (DBusername == username) { //usernam and SessionID fit
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

/* Course name valid Function
    Checks if it is a valid course
    
    @param courseid
    the courseid of the course that should be opened

    @return true, if the courseid exists
*/
bool CnameValid(string courseid) {
    bool RESULT = false;
    sql::ResultSet* res = MakeQuery("SELECT idmodules FROM modules WHERE active='1' AND idmodules='" + courseid + "'");
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
    It receives the GET, POST and Cookies paramters and allows to remove or add lecturer or students to the course

    @GETparam courseid
    the courseid of the course that the user wants to accesse

    @POSTparam studentid
    the id of the student, that will be added or delted from the course
    @POSTparam lecturer
    the id of the lectuere that will be added or delted from the course
    @POSTparam action
    the action taken: add or delete

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

    sql::ResultSet* res = MakeQuery("SELECT name FROM modules WHERE idmodules='" + courseid + "'");
    if (res->next())
        coursename = res->getString("name");

    #pragma endregion

    if (SessionAlive(username, SessionID)&&CnameValid(courseid)) {
        try {

        #pragma region editStudent
            string action = "";
            string studentid = "";
            form_iterator fi = cgi.getElement("studentid");
            if (!fi->isEmpty() && fi != (*cgi).end()) {
                studentid = escape_sql(**fi);
                form_iterator fi = cgi.getElement("action");
                if (!fi->isEmpty() && fi != (*cgi).end())
                    action = escape_sql(**fi);
                if (!action.empty() && !studentid.empty())
                    if (action == "add") {
                        InsterQuery("INSERT INTO students_has_modules(students_idstudents,modules_idmodules) VALUES ('" + studentid + "','"+courseid+"')");
                    }
                    else if (action == "del") {
                        InsterQuery("DELETE FROM `students_has_modules` WHERE students_idstudents = '" + studentid + "' AND modules_idmodules = '"+courseid+"'");
                    }
            }

        #pragma endregion

        #pragma region editLecturer
            action = "";
            string lecturer = "";
            fi = cgi.getElement("lecturer");
            if (!fi->isEmpty() && fi != (*cgi).end()) {
                lecturer = escape_sql(**fi);
                form_iterator fi = cgi.getElement("action");
                if (!fi->isEmpty() && fi != (*cgi).end())
                    action = escape_sql(**fi);
                if (!action.empty() && !lecturer.empty())
                    if (action == "add") {
                        InsterQuery("INSERT INTO modules_has_lectures(modules_idmodules,login_name) VALUES ('" + courseid + "','" + lecturer + "')");
                    }
                    else if (action == "del") {
                        InsterQuery("DELETE FROM `modules_has_lectures` WHERE login_name = '" + lecturer + "' AND modules_idmodules ='"+ courseid +"'");
                    }
            }

        #pragma endregion

            cout << HTTPHTMLHeader() << endl;
            cout << html() << head(title("Mark Manager")).add(link().set("rel", "stylesheet").set("href", "/css/main.css").set("style", "text/css")) << endl;
            cout << link().set("rel", "stylesheet").set("href", "/css/list.css").set("style", "text/css") << endl;
            cout << head() << endl;
            cout << body();
            cout << a("back").set("href", "/admin/courses/") << endl;
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

            sql::ResultSet* res;

            #pragma region Lecturer


            res = MakeQuery("SELECT name,fullname FROM login WHERE name !='admin' AND name NOT IN (SELECT login_name FROM modules_has_lectures WHERE modules_idmodules='" + courseid + "')");

            cout << p("Lecturers").set("class", "lable") << endl;
            cout << "<br>" << endl;

            cout << cgicc::div().set("class", "listelement") << endl;
            cout << table().set("class", "listtable") << endl;
            cout << "<tr>" << endl;
            cout << form().set("method", "post") << endl;
            cout << "<td class='listtable-left'>" << endl;
            cout << cgicc::select().set("name", "lecturer").set("class", "listlabel") << endl;
            while (res->next()) {
                cout << option(res->getString("fullname")).set("value", res->getString("name")) << endl;
            }
            cout << cgicc::select() << endl;
            cout << "</td>" << endl;
            cout << "<td class='listtabel-right'>" << endl;
            cout << button().set("type", "submit").set("value", "add").set("class", "listbutton").set("name", "action");
            cout << "Add";
            cout << button() << endl;
            cout << "</td>" << endl;
            cout << form() << endl;
            cout << "</tr>" << endl;
            cout << table() << endl;
            cout << cgicc::div() << endl;
            cout << "<br>" << endl;

            res = MakeQuery("SELECT login_name,fullname FROM modules_has_lectures LEFT JOIN login ON modules_has_lectures.login_name = login.name WHERE modules_idmodules='"+courseid+"'");
            while (res->next()) {

                cout << cgicc::div().set("class", "listelement") << endl;
                cout << table().set("class", "listtable") << endl;
                cout << "<tr>" << endl;
                cout << form().set("method", "post") << endl;
                cout << "<td class='listtable-left'>" << endl;
                cout << p(res->getString("fullname")).set("class", "listlabel");
                cout << "</td>" << endl;
                cout << "<td>" << endl;
                cout << input().set("type", "hidden").set("name", "lecturer").set("value", res->getString("login_name")) << endl;
                cout << "</td>" << endl;
                cout << "<td class='listtabel-right'>" << endl;
                cout << button().set("type", "submit").set("value", "del").set("class", "listbutton").set("name", "action");
                cout << "Del";
                cout << button() << endl;
                cout << "</td>" << endl;
                cout << form() << endl;
                cout << "</tr>" << endl;
                cout << table() << endl;
                cout << cgicc::div() << endl;
                cout << "<br>" << endl;
            }

            #pragma endregion
            
            #pragma region Student

            res = MakeQuery("SELECT idstudents,name FROM students WHERE active ='1' AND idstudents NOT IN (SELECT students_idstudents FROM students_has_modules WHERE modules_idmodules='" + courseid + "')");

            cout << p("Students").set("class", "lable") << endl;
            cout << "<br>" << endl;

            cout << cgicc::div().set("class", "listelement") << endl;
            cout << table().set("class", "listtable") << endl;
            cout << "<tr>" << endl;
            cout << form().set("method", "post") << endl;
            cout << "<td class='listtable-left'>" << endl;
            cout << cgicc::select().set("name", "studentid").set("class", "listlabel") << endl;
            while (res->next()) {
                cout << option(res->getString("name")+", id: "+ res->getString("idstudents")).set("value", res->getString("idstudents")) << endl;
            }
            cout << cgicc::select() << endl;
            cout << "</td>" << endl;
            cout << "<td class='listtabel-right'>" << endl;
            cout << button().set("type", "submit").set("value", "add").set("class", "listbutton").set("name", "action");
            cout << "Add";
            cout << button() << endl;
            cout << "</td>" << endl;
            cout << form() << endl;
            cout << "</tr>" << endl;
            cout << table() << endl;
            cout << cgicc::div() << endl;
            cout << "<br>" << endl;

            res = MakeQuery("SELECT Name,students_idstudents FROM students_has_modules LEFT JOIN students ON students_has_modules.students_idstudents = students.idstudents WHERE modules_idmodules='" + courseid + "'");
            while (res->next()) {

                cout << cgicc::div().set("class", "listelement") << endl;
                cout << table().set("class", "listtable") << endl;
                cout << "<tr>" << endl;
                cout << form().set("method", "post") << endl;
                cout << "<td class='listtable-left'>" << endl;
                cout << p(res->getString("Name") + ", id:" + res->getString("students_idstudents")).set("class", "listlabel");
                cout << "</td>" << endl;
                cout << "<td>" << endl;
                cout << input().set("type", "hidden").set("name", "studentid").set("value", res->getString("students_idstudents")) << endl;
                cout << "</td>" << endl;
                cout << "<td class='listtabel-right'>" << endl;
                cout << button().set("type", "submit").set("value", "del").set("class", "listbutton").set("name", "action");
                cout << "Del";
                cout << button() << endl;
                cout << "</td>" << endl;
                cout << form() << endl;
                cout << "</tr>" << endl;
                cout << table() << endl;
                cout << cgicc::div() << endl;
                cout << "<br>" << endl;
            }

            #pragma endregion

            cout << body() << endl;
            cout << html();
        }
        catch (exception& e) {
            cout << "This should not have happend. Please try to reload the webpage." << endl;
        }
    }

}