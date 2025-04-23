#ifndef USER_MANAGE_HPP
#define USER_MANAGE_HPP

#include "config.hpp"

// ? SHA-256加密哈希算法
inline std::string hashPasswd(const std::string& password, const std::string& salt);

// 生成随机盐值
inline std::string generateSalt(size_t length = 16);


class User{
protected:
    std::string user_name;
    std::string hash_passwd;
    std::string salt;
public:
    User(const std::string& name,const std::string&passwd) : user_name(name),hash_passwd(passwd),salt(generateSalt()){
        hash_passwd = hashPasswd(passwd,salt);
    }

    virtual ~User() = default;

    virtual bool login(const std::string& name,const std::string& passwd);

    virtual std::string toFileString() const;//{
    //     return user_name + ',' + hash_passwd + ',' + salt;
    // }

    static User* fromFileString(const std::string& data);

    std::string get_username(){
        return user_name;
    }
};


class UserManage{
private:
    //内存索引是什么？什么是std::unordered_map？前面string是索引，后面是实际类型吗？
    std::unordered_map<std::string,User*> users; //内存索引
    const std::string user_file = "users.txt";

    void loadUsers();
    void saveUsers();

public:
    UserManage(){
        loadUsers();
    }
//this this this
    ~UserManage(){
        saveUsers();
        for(auto& pair : users){
            delete pair.second;
        }
    }

    bool Registered(const std::string& username, const std::string& password);
    bool Delete(const std::string& username);
    User* findUser(const std::string& username);
};


#endif