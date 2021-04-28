/* Copyright(c) 2021  Alexander Sworski
*  All Rights Reserved
*  Brookes ID: 19131287
*/
#define _CRT_SECURE_NO_WARNINGS

#include <Poco/Net/MailMessage.h>
#include <Poco/Net/MailRecipient.h>
#include <Poco/Net/NetException.h>
#include <Poco/Net/SMTPClientSession.h>


#include <iostream>
#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTMLClasses.h"
#include <cgicc/HTTPRedirectHeader.h>
#include "mysql/mysql.h"
#include "mysql_connection.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

#include <sstream>
#include <iomanip>
#include <stdio.h>
#include <openssl/sha.h>

/*https://www.chilkatsoft.com/downloads_vcpp.asp*/
#include <CkCrypt2.h>

#include <chrono>

#include<cstdlib>


using namespace Poco::Net;
using namespace Poco;
using namespace cgicc;
using namespace std;

using namespace std::chrono;

Cgicc cgi;

/* Error Functinon
    Displays a error page
    Will only be called if a unexpected error occurs
*/
void error() {
    try {

        cout << HTTPHTMLHeader() << endl;
        cout << html() << head(title("Mark Manager")).add(link().set("rel", "stylesheet").set("href", "/css/main.css").set("style", "text/css")) << endl;
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
        cout << p("Login Error");
        cout << cgicc::div() << endl;
        cout << p("A error occured while login in. Please go back and try again.").set("classs","error") << endl;
        cout << body() << endl;
        cout << html();
    }
    catch (exception& e) {
        cout << "This should not have happend. Please try to reload the webpage." << endl; 
    }
    exit(0);
}

/* SHA256 Function
    Generates a SHA256 hash from the input and returns it

    @param str
    the input string, that will be hashed

    @return
    returns the hash of str
*/
string sha256(string str)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, str.c_str(), str.size());
    SHA256_Final(hash, &sha256);
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    return ss.str();
}

/* Set Session ID valid Function
    Sets the SessionID given to active within the DB
    Uses the DB user login

    @param SessionID
    the SessionID, that should be set to active

    @return true if operation succesfull

*/
bool setSessionIDValid(string SessionID) {
    try {
        sql::Driver* driver;
        sql::Connection* con;
        sql::Statement* stmt;
        sql::ResultSet* res;

        driver = get_driver_instance();
        con = driver->connect("tcp://127.0.0.1:3306", "login", "n9ras1yay4aey8yoterec2vex");
        con->setSchema("markmanager");

        stmt = con->createStatement();
        string select = "UPDATE sessions SET active='1' WHERE SessionID='"+SessionID+"'";
        stmt->execute(select);
        return true;
    }
    catch (sql::SQLException& e) {
        return false;
    }

}

/* Safe Session ID  Function
    Safes the SessionID to the DB
    Uses the DB user login

    @param SessionID
    the SessionID, that has been created earlier
    
    @param username
    the username the SessionID belongs to

    @param intenmin
    The DATETIME string of now + 10min, which respresents the expiration time of the session

    @param IP
    The IP adress of the user

    @return true if operation succesfull

*/
bool safeSessionID(string sessionID, string username,string intenmin, string IP) {
    try {
        sql::Driver* driver;
        sql::Connection* con;
        sql::Statement* stmt;
        sql::ResultSet* res;

        driver = get_driver_instance();
        con = driver->connect("tcp://127.0.0.1:3306", "login", "n9ras1yay4aey8yoterec2vex");
        con->setSchema("markmanager");

        stmt = con->createStatement();
        string select = "INSERT INTO sessions(SessionID,fk_user,IP,expires, active) VALUES(\"" + sessionID + "\",\"" + username + "\",\"" + IP + "\",\"" + intenmin + "\", \"false\")";
        stmt->execute(select);
        return true;
    }
    catch (sql::SQLException& e) {
        return false;
    }

}

/* get Session ID Function
    Creates a SessionID for the user and calls safeSessionID() to safe it

    @param username
    the username the SessionID will belong to

    @return
    returns the SessionID

    @deprecated
    the time function used is deprecated, but had to be used to get the format needed for MySQL

*/
string getSessionID(string username) {
    system_clock::time_point now = system_clock::now();
    time_t tt = system_clock::to_time_t(now);
    string nowstring = ctime(&tt);
    time_t     ten = time(0) + 600;
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&ten);
    strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);

    const CgiEnvironment& env = cgi.getEnvironment();
    string ENV = env.getRemoteAddr();

    srand(time(0));
    string random = to_string(rand());
    string sessionID = sha256(username+ random + nowstring + ENV + random);
    if (!safeSessionID(sessionID,username, buf,ENV))
        error();
    return sessionID;
}

/* Set Cookie Function
    sets two Cookies, one with the Username and one with the SesssionID, for this it calls getSessionID()

    @param username
    the username user to login

*/
void setCookie(string username) {
    cout << "Set-Cookie:SessionID=";
    cout << getSessionID(username);
    cout << "; Domain=localhost; ";
    cout << "Path = /; ";
    cout << "HTTPOnly=true; ";
    cout << "SameSite=Strict; ";
    cout << "Max-Age=600;\n";
    cout << "Set-Cookie:username=";
    cout << username;
    cout << "; Domain=localhost; ";
    cout << "Path=/; ";
    cout << "HTTPOnly=true; ";
    cout << "SameSite=Strict; ";
    cout << "Max-Age=600;\n";
}

/* Check Login Function
    checks if the password and username match
    it also hashes the password by calling sha256()
    uses the DB user login

    @param username
    the username

    @parama password
    the password

    @return true if passwor and username correct

*/
bool checkLogin(string username, string password) {
    password = sha256(password);
    try {
        sql::Driver* driver;
        sql::Connection* con;
        sql::Statement* stmt;
        sql::ResultSet* res;

        driver = get_driver_instance();
        con = driver->connect("tcp://127.0.0.1:3306", "login", "n9ras1yay4aey8yoterec2vex");
        con->setSchema("markmanager");

        stmt = con->createStatement();
        string select = "SELECT name,password FROM login WHERE active='1' AND name='"+username+"'";
        res = stmt->executeQuery(select);
        if (res->next()) {
            string dbname = res->getString("name");
            string dbpassword = res->getString("password");
            if (dbname == username && dbpassword == password) {
                delete res;
                delete stmt;
                delete con;
                return true;
            }
            else
                return false;
        }
        else {
            delete res;
            delete stmt;
            delete con;
            return false;
        }  
    }
    catch (sql::SQLException& e) {
        return false;
    }
    return false;
}

/* Check Session ID Function
    Checks if the SessionID is valid and belongs to the user requested
    uses the DB user login

    @param username
    the username

    @param SessionID
    the sessionID

    @return true, if username and SessionID valid 
*/
bool checkSessionID(string username, string SessionID) {
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
        string select = "SELECT fk_user,SessionID  FROM sessions WHERE fk_user ='" + username + "' AND expires > '" + timenow + "'";
        res = stmt->executeQuery(select);
        while (res->next()) {
            string dbname = res->getString("fk_user");
            string dbSession = res->getString("SessionID");
            if (dbname == username && dbSession == SessionID) {
                delete res;
                delete stmt;
                delete con;
                return true;
            }
        }
    }
    catch (sql::SQLException& e) {
        return false;
    }
    return false;
}

/* Check 2FA return Function
    checks if the OTP code entered is valid and belongs to the user
    uses the DB user login

    @param username
    the username

    @param return2fa
    the OTP code

    @return true, if OTP code and username match and valid

*/
bool check2afReturn(string username, string return2fa) {
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
        con = driver->connect("tcp://127.0.0.1:3306", "login", "n9ras1yay4aey8yoterec2vex");
        con->setSchema("markmanager");

        stmt = con->createStatement();
        string select = "SELECT fk_user,code FROM 2FA WHERE fk_user ='" + username + "' AND expires > '"+timenow+"'";
        res = stmt->executeQuery(select);
        while (res->next()) {
            string dbname = res->getString("fk_user");
            string dbcode = res->getString("code");
            if (dbname == username && dbcode == return2fa) {
                delete res;
                delete stmt;
                delete con;
                return true;
            }
        }
    }
    catch (sql::SQLException& e) {
        return false;
    }
    return false;
}

/* Check CR return Function
    checks if the TOTP code entered is valid and blongs to the admin
    uses the DB user login

    @param username
    the username; should awalys be admin

    @param crReturn
    the TOTP code

    @return true, if TOTP code and username match and valid

*/
bool checkCrReturn(string username, string crReturn) {
    if (username == "admin") {
        // https://example-code.com/cpp/totp_one_time_password.asp
        try {
            sql::Driver* driver;
            sql::Connection* con;
            sql::Statement* stmt;
            sql::ResultSet* res;

            driver = get_driver_instance();
            con = driver->connect("tcp://127.0.0.1:3306", "login", "n9ras1yay4aey8yoterec2vex");
            con->setSchema("markmanager");

            stmt = con->createStatement();
            string select = "SELECT seed FROM TOTP WHERE fk_user='admin' AND active='1'";
            res = stmt->executeQuery(select);
            while (res->next()) {
                CkCrypt2 crypt;
                const char* seed = res->getString("seed").c_str();
                string totp = crypt.totp(seed, "base32", "0", "", 30, 6, -1, "sha1");
                if (totp == crReturn) {
                    delete res;
                    delete stmt;
                    delete con;
                    return true;
                }
            }
        }
        catch (sql::SQLException& e) {
            return false;
        }
        return false;
    }
    else {
        return false;
    }
    return false;;
}

/* Send Email Function
    Send a email containing a OTP to a adress given and a username given
    uses a SendinBlue SMTP server
    
    The function is taken from:
    https://stackoverflow.com/questions/19767431/how-to-send-email-with-c

    @param name
    the full name of the user

    @param email
    the email of the user

    @param Code
    the OTP code

*/
void sendByEmail(string name, string email,string Code) {
    MailMessage msg;
    msg.addRecipient(MailRecipient(MailRecipient::PRIMARY_RECIPIENT,email, name));
    msg.setSender("MarkManager <noreply@markmanager.com>");
    msg.setSubject("OTP Code");
    msg.setContent(Code);
    SMTPClientSession smtp("smtp-relay.sendinblue.com",587);
    smtp.login(Poco::Net::SMTPClientSession::LoginMethod::AUTH_LOGIN,"19131287@brookes.ac.uk","xsmtpsib-cefff66855de9173a88de69b6dbc1de87e6724f0c27495bbd602e22f58db5f2b-SPTdqzU7y91wHvFN");
    smtp.sendMessage(msg);
    smtp.close();
}

/* Cretae 2FA Token Function
    Creates a OTP, saves it to the DB and calls sendByEmail() to send it to the user
    uses DB user login

    @param username
    the username

*/
bool create2FAToken(string username) {
    time_t     ten = time(0) + 600;
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&ten);
    strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
    srand(time(0));
    int random = rand()%10000;
    string email = "";
    try {
        sql::Driver* driver;
        sql::Connection* con;
        sql::Statement* stmt;
        sql::ResultSet* res;

        driver = get_driver_instance();
        con = driver->connect("tcp://127.0.0.1:3306", "login", "n9ras1yay4aey8yoterec2vex");
        con->setSchema("markmanager");

        stmt = con->createStatement();
        string select = "INSERT INTO 2FA(code,fk_user,expires) VALUES(\"" + to_string(random) + "\",\"" + username + "\",\"" + buf + "\")";
        stmt->execute(select);

        stmt = con->createStatement();
        select = "SELECT email FROM login WHERE name='"+username+"'";
        res = stmt->executeQuery(select);
        if (res->next()) {
            email = res->getString("email");
        }
        else
            throw "No Email";

        sendByEmail(username,email,to_string(random));
        return true;
    }
    catch (sql::SQLException& e) {
        return false;
    }
}

/* Show 2FA Function
    Prints the 2FA page for the regular users

    @param optional message to be display on the website
*/
void show2fa(string message="") {
    try {

        cout << HTTPHTMLHeader() << endl;
        cout << html() << head(title("Mark Manager")).add(link().set("rel", "stylesheet").set("href", "/css/main.css").set("style", "text/css")) << endl;
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
        cout << p("2FA");
        cout << cgicc::div() << endl;

        cout << form().set("method", "post") << endl;
        cout << p("OTP Code send by E-Mail").set("class", "smalllable") << endl;
        cout << input().set("type", "password").set("name", "2FA").set("class", "input") << endl;
        cout << "<br>" << endl;
        if (!message.empty()) {
            cout << p(message).set("class", "error") << endl;
            cout << "<br>" << endl;
        }
        cout << input().set("type", "submit").set("value", "Login").set("class", "button") << endl;
        cout << "<br>" << endl;
        cout << form() << endl;

        cout << body() << endl;
        cout << html();
    }
    catch (exception& e) {
        cout << "This should not have happend. Please try to reload the webpage." << endl; // handle if applicable
    }
}

/* Show CR Function
    Prints the 3FA page for the admin

    @param optional message to be display on the website
*/
void showCR(string message="") {
    try {

        cout << HTTPHTMLHeader() << endl;
        cout << html() << head(title("Mark Manager")).add(link().set("rel", "stylesheet").set("href", "/css/main.css").set("style", "text/css")) << endl;
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
        cout << p("2FA");
        cout << cgicc::div() << endl;

        cout << form().set("method", "post") << endl;
        cout << p("OTP Code send by E-Mail").set("class", "smalllable") << endl;
        cout << input().set("type", "password").set("name", "2FA").set("class", "input") << endl;
        cout << "<br>" << endl;
        cout << p("OTP Code generated by App").set("class", "smalllable") << endl;
        cout << input().set("type", "password").set("name", "CR").set("class", "input") << endl;
        cout << "<br>" << endl;
        if (!message.empty()) {
            cout << p(message).set("class", "error") << endl;
            cout << "<br>" << endl;
        }
        cout << input().set("type", "submit").set("value", "Login").set("class", "button") << endl;
        cout << "<br>" << endl;
        cout << form() << endl;

        cout << body() << endl;
        cout << html();
    }
    catch (exception& e) {
        cout << "This should not have happend. Please try to reload the webpage." << endl; 
    }
}

/* Check Session alive Function
    checks if the Session is already active and redirects the user directly to his main user page
    uses db user: stateful

    @param username
    the username

    @param SessionID
    the sessionID
*/
void CheckSessionAlive(string username, string SessionID) {
    string DBusername = "";
    bool DBSessionStatus = false;

    string ENV = "";
    try { ENV = getenv("REMOTE_ADDR"); }
    catch (exception e) { exit(0); }
    
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
        string select = "SELECT fk_user,SessionID,active FROM sessions WHERE IP='" + ENV + "' AND SessionID = '"+ SessionID+"' AND expires > '" + timenow + "'";
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
                cout << HTTPRedirectHeader("/admin/") << endl;
                exit(0);
            }
            else {
                cout << HTTPRedirectHeader("/lecturer/") << endl;
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
        if (!isalnum(c)||c=='@' || c == '.' || c == '_')
        {
            input.erase(i);
            i--;
        }
    }
    return input;
}

/* Main Funcion
    the main function
    It receives the GET, POST and Cookies paramters and allows for a login and a 2FA or 3FA autheitcation
    If succesfully autheticated, it forwards to the user main page

    @POSTparam username
    the username provied by the login page
    @POSTparam password
    the password provived by the login page
    @POSTparam return2fa
    the OTP code
    @POSTparam crreturn
    the TOTP code
    @COOKIEparam username
    the username of a sessionID is already set
    @COOKIEparam SessionID
    the sessionID if sessionID is already set
*/
int main()
{
    string username = "";
    string password = "";
    string return2fa = "";
    string crreturn = "";
    string SessionID = "";

    form_iterator fi = cgi.getElement("username");
    if (!fi->isEmpty() && fi != (*cgi).end()) {
        username = **fi;
        fi = cgi.getElement("password");
        if (!fi->isEmpty() && fi != (*cgi).end()) {
            password = **fi;
        }
    }
    else  {

        const_cookie_iterator cci;
        const CgiEnvironment& env = cgi.getEnvironment();

        for (cci = env.getCookieList().begin(); cci != env.getCookieList().end(); ++cci) {
            if (cci->getName() == "SessionID") {
                SessionID = cci->getValue();
            }
            if (cci->getName() == "username") {
                username = cci->getValue();
            }
        }      
        if (!username.empty() && !SessionID.empty()) {


            CheckSessionAlive(escape_sql(username), escape_sql(SessionID));

            fi = cgi.getElement("2FA");
            if (!fi->isEmpty() && fi != (*cgi).end()) {
                return2fa = **fi;
            }
            fi = cgi.getElement("CR");
            if (!fi->isEmpty() && fi != (*cgi).end()) {
                crreturn = **fi;
            }
            if (username == "admin" && (return2fa.empty() || crreturn.empty())) {
                showCR("authentication error");
                exit(0);
            }
            else if (return2fa.empty() && username != "admin") {
                show2fa("authentication error");
                exit(0);
            }
        }
        else {
            cout << HTTPRedirectHeader("/?error=No login details provided") << endl;
            exit(0);
        }

    }

    if (!username.empty()) {
        if ((!crreturn.empty() && !return2fa.empty() && username == "admin") || (crreturn.empty() && !return2fa.empty() && username != "admin")) {
            if (username == "admin") {
                if (checkCrReturn(escape_sql(username), crreturn) && checkSessionID(escape_sql(username), escape_sql(SessionID)) && check2afReturn(escape_sql(username), escape_sql(return2fa))) {
                    setSessionIDValid(escape_sql(SessionID));
                    cout << HTTPRedirectHeader("/admin/") << endl;
                    exit(0);
                }
                else {
                    showCR("authentication error");
                    exit(0);
                }
            }
            else {
                if (checkSessionID(escape_sql(username), escape_sql(SessionID)) && check2afReturn(escape_sql(username), escape_sql(return2fa))) {
                    setSessionIDValid(escape_sql(SessionID));
                    cout << HTTPRedirectHeader("/lecturer/") << endl;
                    exit(0);
                }
                else {
                    show2fa("authentication error");
                    exit(0);
                }
            }
        }
        else {
            if (checkLogin(escape_sql(username), escape_sql(password))) {
                if (username == "admin") {
                    create2FAToken(escape_sql(username));
                    setCookie(escape_sql(username));
                    showCR();
                    exit(0);
                }
                else
                {
                    create2FAToken(username);
                    setCookie(escape_sql(username));
                    show2fa();
                    exit(0);
                }
            }
            else {
                cout << HTTPRedirectHeader("/?error=Password or Username incorrect") << endl;
                exit(0);
            }
        }
    }
    else {
        //return to index
        cout << HTTPRedirectHeader("/?error=Username missing") << endl;
        exit(0);
    }
}