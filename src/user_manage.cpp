#include "user_manage.hpp"
#include "config.hpp"

inline std::string hashPasswd(const std::string& password, const std::string& salt){
    unsigned char digest[SHA256_DIGEST_LENGTH];
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, (salt + password).c_str(), (salt + password).size());
    SHA256_Final(digest, &ctx);

    std::ostringstream oss;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
      oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(digest[i]);
    }
    return oss.str();
}

inline std::string generateSalt(size_t length = 16) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    std::string salt;
    salt.reserve(length);

    for (size_t i = 0; i < length; ++i) {
        salt += alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    return salt;
}

bool User::login(const std::string& name,const std::string& passwd){
    return passwd == hashPasswd(passwd,salt);
}

std::string User::toFileString() const{
    return user_name + ',' + hash_passwd + ',' + salt;
}

//? 
User* User::fromFileString(const std::string& data){
    std::istringstream iss(data);
    std::string name, hash, salt;
    if (std::getline(iss, name, ',') &&
        std::getline(iss, hash, ',') &&
        std::getline(iss, salt)) {
        User* user = new User(name, "");
        user->hash_passwd = hash;
        user->salt = salt;
        return user;
    }
    return nullptr;
}

//即使是private也可以在cpp文件中进行定义吗？什么时候不能使用private？
//这里的user_name不应该是保护变量吗？不能直接指针指向这个变量呀？
/*
友元类/函数：如果UserManage被声明为User的友元，可直接访问private成员。
同一类的成员函数直接访问该类的private成员。
可以在类中定义一个函数来提供公有的接口
*/
void UserManage::loadUsers(){
    std::ifstream file(user_file);
    if(!file) return;
    std::string line;
    while(std::getline(file,line)){
        if (User* user = User::fromFileString(line)) {
            users[user->get_username()] = user;
        }
    }
}

//const auto& pair : users
void UserManage::saveUsers(){
    std::ofstream file(user_file);
    for (const auto& pair : users) {
        file << pair.second->toFileString() << "\n";
    }
}

bool UserManage::Registered(const std::string& username, const std::string& passwd){
    if(users.count(username)) return false;
    users[username] = new User(username,passwd);
    return true;
}

//auto it = begin()是指向第一行吗？当auto it == end()时是指向最后一个的后面一位吗？此外为什么是users.erase(it)？
bool UserManage::Delete(const std::string& username){
    auto it = users.find(username);
    if (it == users.end()) return false;

    delete it->second;
    users.erase(it);
    return true;
}


//this this this
User* UserManage::findUser(const std::string& username){
    auto it = users.begin();
    return it != users.end() ? it -> second : nullptr;
}