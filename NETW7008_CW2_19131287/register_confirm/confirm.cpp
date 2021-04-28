/* Copyright(c) 2021  Alexander Sworski
*  All Rights Reserved
*  Brookes ID: 19131287
*/
#include <Poco/Net/MailMessage.h>
#include <Poco/Net/MailRecipient.h>
#include <Poco/Net/NetException.h>
#include <Poco/Net/SMTPClientSession.h>

#include <iostream>
#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTMLClasses.h"
#include "mysql/mysql.h"
#include <cgicc/HTTPRedirectHeader.h>

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

#include <CkCrypt2.h>
#include <openssl/sha.h>
#include <iomanip>

using namespace cgicc;
using namespace std;

using namespace Poco::Net;
using namespace Poco;

Cgicc cgi;

/* user Page Function
    Prints the confirm registration page for the regular users

    @param optional message to be display on the website
*/
void UserPage(string message = "") {
    try {

        cout << HTTPHTMLHeader() << endl;
        cout << html() << head(title("Mark Manager")) << endl;
        cout << link().set("rel", "stylesheet").set("href", "../css/main.css").set("style", "text/css") << endl;
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
        cout << p("Confirm Email");
        cout << cgicc::div() << endl;

        cout << form().set("method", "post") << endl;
        cout << p("OTP Code send by E-Mail").set("class", "smalllable") << endl;
        cout << input().set("type", "password").set("name", "2FA").set("class", "input") << endl;
        cout << "<br>" << endl;
        if (!message.empty()) {
            cout << p(message).set("class", "error") << endl;
            cout << "<br>" << endl;
        }
        cout << input().set("type", "submit").set("value", "Confirm").set("class", "button") << endl;
        cout << "<br>" << endl;
        cout << form() << endl;

        cout << body() << endl;

        cout << html();
    }
    catch (exception& e) {
        cout << "This should not have happend. Please try to reload the webpage." << endl; 
    }
}

/* admin Page Function
    Prints the confirm registration page for the admin

    @param optional message to be display on the website

    @param secret optional
    the seed for the TOTP, needed to generate the QR code
*/
void AdminPage(string message = "", string secret = "") {
    try {

        cout << HTTPHTMLHeader() << endl;
        cout << html() << head(title("Mark Manager")) << endl;
        cout << link().set("rel", "stylesheet").set("href", "../css/main.css").set("style", "text/css") << endl;
        cout << link().set("rel", "stylesheet").set("href", "../css/qrcode.css").set("style", "text/css") << endl;
        cout << script().set("type", "text/javascript").set("src", "../js/qrcode.js");
        cout << script() << endl;
        cout << script().set("type", "text/javascript").set("src", "../js/totp_qrcode.js");
        cout << script() << endl;
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
        cout << p("Confirm Email");
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
        cout << input().set("type", "submit").set("value", "Confirm").set("class", "button") << endl;
        cout << "<br>" << endl;
        cout << form() << endl;

        //cout << cgicc::div().set("class","qrcode") << endl;
        cout << p("Scann this code with a TOTP App as Google Authenticator").set("class","smalllable") << endl;
        cout << a().set("id", "urilink") << endl;
        cout << cgicc::div().set("id", "qrcode") << endl;
        cout << img().set("id", "preview").set("src", "img/freeotp.svg") << endl;
        cout << cgicc::div() << endl;
        cout << a() << endl;
        //cout << cgicc::div() << endl;

        cout << script() << endl;
        cout << "var qrcode = new QRCode(document.getElementById('qrcode'), { correctLevel : QRCode.CorrectLevel.H, text : window.location.href, colorLight : '#ffffff', colorDark : '#000000' }); " << endl;
        cout << "MakeQRCode('"+secret+"');" << endl;
        cout << script() << endl;

        cout << body() << endl;

        cout << html();
    }
    catch (exception& e) {
        cout << "This should not have happend. Please try to reload the webpage." << endl; // handle if applicable
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

/* Set Cookie Function
    sets one Cookie with the Username

    @param username
    the username who wants to register

*/
void setCookie(string username) {
    cout << "Set-Cookie:username=";
    cout << username;
    cout << "; Domain=localhost; ";
    cout << "Path=/; ";
    cout << "HTTPOnly=true; ";
    cout << "SameSite=Strict; ";
    cout << "Max-Age=600;\n";
}

/* Error Function
    redircets the user to the register page with the error message "Please Validate Input!"
    will be called in the case of unexpeced behaviour
*/
void error(string error="Please validate input!") {
    cout << HTTPRedirectHeader("/register?error="+error) << endl;
    exit(0);
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
void sendByEmail(string name, string email, string Code) {
    MailMessage msg;
    msg.addRecipient(MailRecipient(MailRecipient::PRIMARY_RECIPIENT, email, name));
    msg.setSender("MarkManager <noreply@markmanager.com>");
    msg.setSubject("OTP Code");
    msg.setContent(Code);
    SMTPClientSession smtp("smtp-relay.sendinblue.com", 587);
    smtp.login(Poco::Net::SMTPClientSession::LoginMethod::AUTH_LOGIN, "19131287@brookes.ac.uk", "xsmtpsib-cefff66855de9173a88de69b6dbc1de87e6724f0c27495bbd602e22f58db5f2b-SPTdqzU7y91wHvFN");
    smtp.sendMessage(msg);
    smtp.close();
}

/* Cretae 2FA Token Function
    Creates a OTP, saves it to the DB and calls sendByEmail() to send it to the user
    uses DB register

    @param username
    the username
*/
bool create2FAToken(string username, string email) {
    time_t     ten = time(0) + 600;
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&ten);
    strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
    srand(time(0));
    int random = rand() % 10000;
    try {
        sql::Driver* driver;
        sql::Connection* con;
        sql::Statement* stmt;
        sql::ResultSet* res;

        driver = get_driver_instance();
        con = driver->connect("tcp://127.0.0.1:3306", "register", "qaciy0t2kif5cul5d5quwanus");
        con->setSchema("markmanager");

        stmt = con->createStatement();
        string select = "INSERT INTO 2FA(code,fk_user,expires) VALUES(\"" + to_string(random) + "\",\"" + username + "\",\"" + buf + "\")";
        stmt->execute(select);

        sendByEmail(username, email, to_string(random));
        return true;
    }
    catch (sql::SQLException& e) {
        return false;
    }
}

/*INT to BASE32 Function
       converts a long to base32, which is used for the TOTP seed

       @param i
       i that will be converted

       @return Base32 string
*/
string itob32(long i)
{
    string b32 = "";
        int d = i % 32;
        if (d < 26)
        {
            b32.append(1,char((int('a')+d)));
        }
        else
        {
            b32.append(1, char(d-26+1+49));
        }

    return b32;
}

/* set TOTP Function
    Creates a radom seed by calling itob32() for the TOTP genertion, and saves it to the DB
    Is prepared to be used for other users too and not limited to work only with the admin
    uses DB register

    @param username
    the username
*/
string setTOTP(string username) {
    username = escape_sql(username);
    string secret;
    srand(time(0));
    for (int i = 0; i < 56; i++) {
        int random = rand() % 100;
        secret.append(itob32(random));
    }

    try {
        sql::Driver* driver;
        sql::Connection* con;
        sql::Statement* stmt;
        sql::ResultSet* res;

        driver = get_driver_instance();
        con = driver->connect("tcp://127.0.0.1:3306", "register", "qaciy0t2kif5cul5d5quwanus");
        con->setSchema("markmanager");

        stmt = con->createStatement();
        string select = "INSERT INTO TOTP(seed,fk_user,active) VALUES('"+secret+"','" + username + "','false')";
        stmt->execute(select);

    }
    catch (sql::SQLException& e) {
        sql::Driver* driver;
        sql::Connection* con;
        sql::Statement* stmt;
        sql::ResultSet* res;

        driver = get_driver_instance();
        con = driver->connect("tcp://127.0.0.1:3306", "register", "qaciy0t2kif5cul5d5quwanus");
        con->setSchema("markmanager");

        stmt = con->createStatement();
        string select = "UPDATE TOTP SET seed='" + secret + "' WHERE fk_user='"+username+"' AND active='0'";
        stmt->execute(select);
    }

    return secret;
}

/* Check TOTP return Function
    Checks of the TOTP entered by the user corresponds to the TOTP code genrated by the server

    @param username
    the username

    @param TOTP
    the TOTP code entered by the user

    @return true, if server genrated code and user entered code match
*/
bool check_TOTP_Return(string username, string TOTP) {
    if (username == "admin") {
        // https://example-code.com/cpp/totp_one_time_password.asp
        try {
            sql::Driver* driver;
            sql::Connection* con;
            sql::Statement* stmt;
            sql::ResultSet* res;

            driver = get_driver_instance();
            con = driver->connect("tcp://127.0.0.1:3306", "register", "qaciy0t2kif5cul5d5quwanus");
            con->setSchema("markmanager");

            stmt = con->createStatement();
            string select = "SELECT seed FROM TOTP WHERE fk_user='admin' AND active='0'";
            res = stmt->executeQuery(select);
            while (res->next()) {
                CkCrypt2 crypt;
                const char* seed = res->getString("seed").c_str();
                string totp = crypt.totp(seed, "base32", "0", "", 30, 6, -1, "sha1");
                if (totp == TOTP) {
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

/* Check Email return Function
    checks if the OTP code entered is valid and belongs to the user
    uses the DB user register

    @param username
    the username

    @param return2fa
    the OTP code

    @return true, if OTP code and username match and valid

*/
bool check_Email_Return(string username, string OTP) {
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
        con = driver->connect("tcp://127.0.0.1:3306", "register", "qaciy0t2kif5cul5d5quwanus");
        con->setSchema("markmanager");

        stmt = con->createStatement();
        string select = "SELECT fk_user,code FROM 2FA WHERE fk_user ='" + username + "' AND expires > '" + timenow + "'";
        res = stmt->executeQuery(select);
        while (res->next()) {
            string dbname = res->getString("fk_user");
            string dbcode = res->getString("code");
            if (dbname == username && dbcode == OTP) {
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

/* Make registration Function
    Saves the username, password, email and fullname if input valid
    Then send the OTP code for email verification and shows the TOTP QR Code of the admin is a user

    @param username
    the username

    @param password
    the plain password

    @param email
    the user email

    @param fullname
    the full name of the user
*/
void makeRegistration(string username, string password, string email, string fullname) {

    username = escape_sql(username);
    password = sha256(password);
    email = escape_sql(email);
    fullname = escape_sql(fullname);

    try {
        sql::Driver* driver;
        sql::Connection* con;
        sql::Statement* stmt;
        sql::ResultSet* res;

        driver = get_driver_instance();
        con = driver->connect("tcp://127.0.0.1:3306", "register", "qaciy0t2kif5cul5d5quwanus");
        con->setSchema("markmanager");

        if (username == "admin") {
            bool active = true;
            stmt = con->createStatement();
            string select = "SELECT active FROM login WHERE name='admin';";
            res = stmt->executeQuery(select);
            if (res->next()) {
                active = res->getBoolean("active");
            }
            else {
                delete res;
                delete stmt;
                delete con;
                error("User already exist");
            }

            if (!active) {
                //add admin info
                stmt = con->createStatement();
                select = "UPDATE login SET active='0',password='"+password+"',email='"+email+"',fullname='"+fullname+"' WHERE name='admin';";
                stmt->execute(select);
                create2FAToken(username, email);
                setCookie(username);
                AdminPage("", setTOTP(username));
                exit(0);
            }
        }
        else {

            bool active = true;
            stmt = con->createStatement();
            string select = "SELECT active FROM login WHERE name='" + username + "';";
            res = stmt->executeQuery(select);
            if (res->next()) {
                stmt = con->createStatement();
                string select = "SELECT active FROM login WHERE active='1' AND name='" + username + "';";
                res = stmt->executeQuery(select);
                if (res->next()) {
                    error("User already exist");
                }
                else {
                    stmt = con->createStatement();
                    select = "UPDATE login SET active='0',password='" + password + "',email='" + email + "',fullname='" + fullname + "' WHERE name='"+username+"';";
                    stmt->execute(select);
                    create2FAToken(username, email);
                    setCookie(username);
                    UserPage();
                    exit(0);
                }
            }
            else {
                // add user info
                stmt = con->createStatement();
                select = "INSERT INTO login (active,password,email,fullname,name) VALUES('0','" + password + "','" + email + "','" + fullname + "','"+username+"');";
                stmt->execute(select);
                create2FAToken(username, email);
                setCookie(username);
                UserPage();
                exit(0);
            }
        }
    }
    catch (sql::SQLException& e) {
        error();
    }
    error();
}

/* Confirm Registration Function
    Asks for the OTP and TOPT codes (only for admins), in order to active the acounts.
    If activated the user will be redirected to the login page

    @param username
    the username that should be actived

    @param returnemail
    the OTP code send via email

    @param returntotp
    the TOTP code generated by the token generator app
*/
void Confirmregistration(string username, string returnemail, string returntotp = "") {
    try {
        sql::Driver* driver;
        sql::Connection* con;
        sql::Statement* stmt;
        sql::ResultSet* res;

        driver = get_driver_instance();
        con = driver->connect("tcp://127.0.0.1:3306", "register", "qaciy0t2kif5cul5d5quwanus");
        con->setSchema("markmanager");

        if (username == "admin") {
            bool active = true;
            stmt = con->createStatement();
            string select = "SELECT active FROM login WHERE name='admin'";
            res = stmt->executeQuery(select);
            if (res->next()) {
                active = res->getBoolean("active");
            }
            if (!active) {
                if (check_TOTP_Return(username, returntotp) && check_Email_Return(username, returnemail)) {
                    // User.active = true
                    stmt = con->createStatement();
                    select = "UPDATE login SET active='1' WHERE name='admin'";
                    stmt->execute(select);
                    // TOPT.active = true
                    stmt = con->createStatement();
                    select = "UPDATE totp SET active='1' WHERE fk_user='admin' AND active='0'";
                    stmt->execute(select);
                    cout << HTTPRedirectHeader("/?error=Registration successful") << endl;
                    exit(0);
                }
            }
            else {
                error("User already exists!");
            }
        }
        else {
            bool active = true;
            stmt = con->createStatement();
            string select = "SELECT active FROM login WHERE name='"+escape_sql(username)+"'";
            res = stmt->executeQuery(select);
            if (res->next()) {
                active = res->getBoolean("active");
            }
            if (!active) {
                if (check_Email_Return(username, returnemail)) {
                    // User.active = true
                    stmt = con->createStatement();
                    select = "UPDATE login SET active='1' WHERE name='" + escape_sql(username) + "'";
                    stmt->execute(select);
                    cout << HTTPRedirectHeader("/?error=Registration successful") << endl;
                    exit(0);
                }
            }
        }
    }
    catch (sql::SQLException& e) {
        error();
    }
    error();
    cout << HTTPRedirectHeader("/register/confirm.cgi") << endl;
    exit(0);
}

/* Main Funcion
    the main function
    It receives the GET, POST and Cookies paramters and allows for a registration, email confirmation and TOTP confirmation
    If succesfully autheticated, it forwards to the user index page, where he can login

    @POSTparam username
    the username provied by the register page
    @POSTparam password
    the password provived by the register page
    @POSTparam email
    the email provived by the register page
    @POSTparam fullname
    the full name provived by the register page

    @POSTparam returnemail
    the OTP code
    @POSTparam returntotp
    the TOTP code

    @COOKIEparam username
    the username that wants to register
*/
int main()
{
    string username = "";
    string password = "";
    string email = "";
    string fullname = "";
    string returnemail = "";
    string returntotp = "";

    form_iterator fi = cgi.getElement("username");
    
    if (!fi->isEmpty() && fi != (*cgi).end()) {
        username = **fi;

        fi = cgi.getElement("password");
        if (!fi->isEmpty() && fi != (*cgi).end())
            password = **fi;

        fi = cgi.getElement("email");
        if (!fi->isEmpty() && fi != (*cgi).end())
            email = **fi;

        fi = cgi.getElement("fullname");
        if (!fi->isEmpty() && fi != (*cgi).end())
            fullname = **fi;

        makeRegistration(username, password, email, fullname);

    }
    else {

        const_cookie_iterator cci;
        const CgiEnvironment& env = cgi.getEnvironment();

        for (cci = env.getCookieList().begin(); cci != env.getCookieList().end(); ++cci) {
            if (cci->getName() == "username") {
                username = cci->getValue();
            }
        }
        if (!username.empty()) {

            fi = cgi.getElement("2FA");
            if (!fi->isEmpty() && fi != (*cgi).end()) {
                returnemail = **fi;
            }
            fi = cgi.getElement("CR");
            if (!fi->isEmpty() && fi != (*cgi).end()) {
                returntotp = **fi;
            }
            if (username == "admin" && (returnemail.empty() || returntotp.empty())) {
                AdminPage("authentication error");
                exit(0);
            }
            else if (returnemail.empty() && username != "admin") {
                UserPage("authentication error");
                exit(0);
            }
            else if (username == "admin" && !returnemail.empty() && !returntotp.empty()) {
                Confirmregistration(username, returnemail, returntotp);
            }
            else if (!returnemail.empty() && username != "admin" && returntotp.empty()) {
                Confirmregistration(username, returnemail);
            }
        }
        else {
            cout << HTTPRedirectHeader("/register/?error=No registration details provided") << endl;
            exit(0);
        }

    }
}
