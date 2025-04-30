#ifndef USER_MANAGE_HPP
#define USER_MANAGE_HPP

#include "config.hpp"

const std::string user_file = "/home/jqz/Desktop/Smart_CMS/users.txt";
const std::string teacher_file = "/home/jqz/Desktop/Smart_CMS/teacher_info.txt";

// SHA-256加密哈希算法
std::string hashPasswd(const std::string& password, const std::string& salt);
// 生成随机盐值
std::string generateSalt(size_t length = 16);

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

//后续考虑使用std::bitset来减小内存
//李四|在职教师|数学,物理,化学,生物|400-700|渝中区,北碚区|Tue[12:00-14:00],Wed[14:00-17:00]
class Teacher : public User{
    public:
        Teacher() : User("1","","") {}
        Teacher(std::string education_,std::vector<std::string> subjects_,uint16_t price_min_,uint16_t price_high_,std::vector<std::string> allow_location_,std::vector<std::pair<std::string, std::pair<int, int>>> available_times_) :  
        User("1", "", ""), education(education_),subjects(subjects_),price_min(price_min_),price_high(price_high_),allow_location(allow_location_),available_times(available_times_){};
    
        std::string education;
        std::vector<std::string> subjects;
        uint16_t price_min,price_high;
        std::vector<std::string> allow_location;
        
        std::vector<std::pair<std::string, std::pair<int, int>>> available_times; 
             
        //add available_time
        void add_avaliabe_time(const std::string& weekday, int time_begin, int time_end){
        //    available_times.emplace(available_times.begin(),weekday,std::make_pair(time_begin,time_end));
            available_times.emplace_back(weekday,std::make_pair(time_begin,time_end));
        }
    
        std::string ToTeachfilestring();
    };

class UserManage{
private:    
    //创造使用用户名作为键的哈希表,
    //std::unordered_map<std::string,User*> users;
    std::unordered_map<std::string,std::unique_ptr<User>> users;
    std::unordered_map<std::string,std::unique_ptr<Teacher>> teachers;
public:
    UserManage(std::string user_file, std::string teacher_file){
        LoadUsers(user_file);  
        LoadTeachers(teacher_file);
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

    std::unique_ptr<Teacher> fromTeachFile(const std::string& data);   
     
    void LoadTeachers(std::string teacher_file);
    void SaveTeachers();

};



//数据存入txt文件,实现永久性存储
//非成员函数上不允许使用类型限定符

#endif