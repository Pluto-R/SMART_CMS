#ifndef USER_MANAGE_HPP
#define USER_MANAGE_HPP

#include "config.hpp"

const std::string user_file = "/home/jqz/Desktop/Smart_CMS/users.txt";

// SHA-256加密哈希算法
inline std::string hashPasswd(const std::string& password, const std::string& salt);
// 生成随机盐值
inline std::string generateSalt(size_t length = 16);

//
std::string binaryToHex(const std::string& binary);
//UserManage 通过 std::unordered_map 管理多个 User*

class User{
protected:
    std::string user_type;        //0: student 1:teacher
    std::string user_name;
    std::string hash_passwd;
    std::string salt;
    public:
        friend std::unique_ptr<User> fromFileString(const std::string&);
        User(const std::string type,const std::string& name,const std::string&passwd) : user_type(type),user_name(name),hash_passwd(passwd),salt(generateSalt()){
            hash_passwd = binaryToHex(hashPasswd(passwd,salt));
        }

    std::string GetType(){
        return user_type;
    }
    std::string Gethash_passwd(){
        return hash_passwd;
    }
    std::string Getsalt(){
        return salt;
    }
    std::string GetName(){
        return user_name;
    }

    void ChangeType(std::string type){
        user_type = type;
    }
    void ChangeName(std::string name){
        user_name = name;
    }
    void ChangeHash(std::string hash){
        hash_passwd = hash;
    }
    void ChangeSalt(std::string salt_){
        salt = salt_;
    }
    virtual ~User() = default;

    std::string Tofilestring();
};


class UserManage{
private:    
    //创造使用用户名作为键的哈希表,
    //std::unordered_map<std::string,User*> users;
    std::unordered_map<std::string,std::unique_ptr<User>> users;
public:
    UserManage(std::string user_file){
        LoadUsers(user_file);  
    }

    bool Login(const std::string& type, const std::string& name, const std::string& passwd) const;
    void Registered(const std::string& type, const std::string& name, const std::string& passwd);
    bool Delete(const std::string& name);
    std::unique_ptr<User>& FindUser(const std::string& name);

    //
    // const std::unordered_map<std::string, std::unique_ptr<User>>& GetUsers() const {
    //     return users;
    // }


    std::unique_ptr<User> fromFile(const std::string& data);
    void LoadUsers(std::string user_file);
    void SaveUsers();
};



//数据存入txt文件,实现永久性存储
//非成员函数上不允许使用类型限定符

#endif