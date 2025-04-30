#include "user_manage.hpp"

using namespace std;
int main()
{
    //
    UserManage manage("/home/jqz/Desktop/Smart_CMS/users.txt","/home/jqz/Desktop/Smart_CMS/teacher_info.txt");
    while(1){
    cout << "choose function : [1] register  [2] login  [3]exit" << endl;
    int op;
    cin >> op;
    switch (op)
    {
    //case 后面应该直接跟常量值
    case 1:{
        
        cout << "please enter your type : [0] student [1] teacher" << endl;
        std::string type;
        cin >> type;
        cout << "please enter your name and passwd" << endl;
        string name,passwd;
        cin >> name >> passwd;
        manage.Registered(type,name,passwd);

        break;
    }
    case 2:{
        cout << "please enter your type , name and passwd" << endl;
        std::string type;
        string name,passwd;
        cin >> type >> name >> passwd;
        manage.Login(type,name,passwd);
        break;
    }
    case 3:{
        break;
    }
    } 
}   
}