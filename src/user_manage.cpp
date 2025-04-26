#include "user_manage.hpp"

std::string hashPasswd(const std::string& password, const std::string& salt) {
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    const EVP_MD* md = EVP_sha256();
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digest_len;
    
    EVP_DigestInit_ex(ctx, md, nullptr);
    EVP_DigestUpdate(ctx, (salt + password).c_str(), (salt + password).size());
    EVP_DigestFinal_ex(ctx, digest, &digest_len);
    EVP_MD_CTX_free(ctx);
    
    return binaryToHex(std::string(reinterpret_cast<char*>(digest), digest_len));
}

inline std::string generateSalt(size_t length) {  
    static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    std::string salt;
    salt.reserve(length);
    
    for (size_t i = 0; i < length; ++i) {
        salt += alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    return salt;
}

std::string binaryToHex(const std::string& binary) {
    static const char hexDigits[] = "0123456789abcdef";
    std::string hex;
    for (unsigned char c : binary) {
        hex += hexDigits[c >> 4];
        hex += hexDigits[c & 0xF];
    }
    return hex;
}

void UserManage::Registered(const std::string& type, const std::string& name, const std::string& passwd){
    if(users.find(name) == users.end()){
        //users.insert({name,new User(type,name,passwd)});  //直接尝试插入 new User 的裸指针或初始化列表 {name, new User(...)}，这与 unique_ptr 的独占所有权语义冲突
        //编译器无法找到合适的pair构造函数来插入元素。
        users.emplace(name,std::make_unique<User>(type,name,passwd));
        std::cout << "Success register!" << std::endl;
        SaveUsers();        
        // for(auto it = users.begin();it != users.end();it++){
        //     std::cout << it->second->Gethash_passwd()<< std::endl;
        // }
        //return true;
        return;
    }
    //return false;
    std::cout << "faile to register!" << std::endl;
}


std::unique_ptr<User>& UserManage::FindUser(const std::string& name) {
    auto it = users.find(name);
    if(it != users.end()){
        return it -> second;
    }
    throw std::runtime_error("User not found: " + name); 
}


//这样会导致内存泄露，直接使用已存储的用户对象进行验证
//容器find(键)得到的是指向值的指针吗？此外像下面这种直接使用it不行吗？为什么还需要创建一个新的User指针？这样还没有删除不会导致内存泄露吗？it作为一个指针，还是作为一个迭代器？不会导致内存泄露吗？是不是只有new一个指针才需要考虑内存泄露的问题？
bool UserManage::Login(const std::string& type, const std::string& name, const std::string& passwd) const{
    auto it = users.find(name);
    //std::string name2 = it->second->GetName();
    //std::cout << "This is a test about " << name2 << std::endl; 
    if(it != users.end()){
        
        std::cout << it->second->Gethash_passwd() << " " << hashPasswd(passwd,it->second->Getsalt()) << " " << passwd << std::endl;
        bool success_passwd = (it->second->Gethash_passwd() == binaryToHex(hashPasswd(passwd,it->second->Getsalt())));
        bool success_type = (it->second->GetType() == type); 
        if(success_passwd && success_type){
            std::cout << "Success to login!" << std::endl;
            return true;}
        else{
            if(!success_type) std::cout << "The type is error!" << std::endl;
            if(!success_passwd) std::cout << "The passwd is error!" << std::endl;
            return false;
        }
    }
    std::cout << "Faile to login!Not exist user!" << std::endl;
    return false;
}

//一个从文件字符串反序列化创建User对象
std::unique_ptr<User> UserManage::fromFile(const std::string& data){
    std::istringstream iss(data);
    std::string name, type, hash, salt;
    if(std::getline(iss, name, ',') &&
    std::getline(iss, type, ',') &&
    std::getline(iss, hash, ',') &&
    std::getline(iss, salt)){
        std::unique_ptr<User> ptr = std::make_unique<User>(type,name,hash);
        ptr->ChangeName(name);
        ptr->ChangeType(type);
        
        ptr->ChangeHash(hash);
        ptr->ChangeSalt(salt);
        std::cout << "name: " << ptr->GetName() << ' ' << "salt: " << ptr->Getsalt()  << ' ' << "type: "<< ptr->GetType() << std::endl;
        return ptr;
    }
    return nullptr;
}

//在UserManage对象构造函数时就将数据加载到哈希表
void UserManage::LoadUsers(std::string user_file){
    std::ifstream file(user_file);
    if(!file) return;
    std::string line;
    while(std::getline(file,line)){
        //!!!unique_ptr 是不可复制的，只能移动  直接获取 unique_ptr
        //if(std::unique_ptr<User> user = fromFile(line)){
        if(auto user = fromFile(line)){
            //string std::unique_ptr<User>
            
            //使用 try_emplace 或 insert 替代 emplace,std::move???
            //users.emplace(user->GetName(),user); 
            users.insert({user->GetName(),std::move(user)});
        }
    }
}



//在UserManage对象登记用户后把数据信息存入txt文件
void UserManage::SaveUsers(){
    std::ofstream file(user_file);
    for (const auto& pair : users) {
        file << pair.second->Tofilestring() << "\n"; 
    }
}

//User类的函数，主要想把当前user的成员转换成想要传入txt文件中的格式
std::string User::Tofilestring() {
    return user_name + ',' + user_type + ',' + hash_passwd + ',' + salt; 
}