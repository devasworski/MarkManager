# Coursework 2 (NETW7008 - Secure Programming)

<img src="https://external-content.duckduckgo.com/iu/?u=http%3A%2F%2Fstatic.brookes.ac.uk%2Fabout%2Fidentity%2Flogos%2Fbrookes_logo_charcoal_cmyk.jpg&f=1&nofb=1" height=80>


Module Leader: Ian Bayley

Semester 2 

Due: 30th april 2021.

### Final Mark: Distinction (88/100)

## Assignment
A web interface with the function of a student record management system. Lecturers will use it to view a list of the students for their modules and they will be able to view the marks for those students and change them. A separate person called the administrator will be responsible for deciding which lecturers oversee which modules.

## Requirements

### Functional Requirements 
1. There are two kinds of users: lecturers and administrators. Both can register an account and set a password.
2. Lecturers can see a list of their modules and a list of the students on each module. They can also enter and change marks.
3. Administrators can assign lecturers to modules and students to modules. There is only one administrator account.
4. The process of logging in should use two-factor authentication. The user must enter a second password sent by email after the main password has been entered. The email address to be used is the one entered when registering the account. If you are not able to install the relevant mail library, you can simulate the process of emailing by appending to a “mail spool” text file representing all the emails that have been sent.
5. The administrator account, in addition to the protections of FR4, must also be authenticated by a “hardware” token, which should be implemented as a piece of challenge-response software. 

### Non-Functional Requirements
1. The web server can be running on your own machine if you wish. However, the department has provided the SOTS server, which you can use instead. Your login details will be the same username and password you use for Moodle and Google Suite. 
2. The system must be developed in C/C++. You may use CGI to interact with the web pages. You may use the C/C++ CGI libraries, which have been installed on SOTS, if you are using SOTS. Here is one of many tutorials on them:
https://www.tutorialspoint.com/cplusplus/cpp_web_programming.htm
3. The system must be robust and secure. Specifically, it should be capable of mitigating many kinds of attacks covered in the module, as detailed in the marking scheme. SSL must not be the sole means of preventing these attacks.
4. The system must be designed with maintainability, security and reliability in mind and according to best practice in designing and implementing secure software. Defensive software practices should be used throughout
5. The code should be commented and have sensible and consistent naming
6. The system should be responsive and easy to use
7. Cryptographic libraries can be used
8. The report must explain why you believe you have satisfied NFR3, NFR4, NFR6.
9. The report must explain why you believe you have satisfied FR1, FR2, FR3, FR4, FR5.
