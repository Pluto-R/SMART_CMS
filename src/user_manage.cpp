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

std::string generateSalt(size_t length) {  
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

void UserManage::Registered(const std::string& type, const std::string& name, const std::string& passwd) {
    if(users.find(name) == users.end()) {
        //users.insert({name,new User(type,name,passwd)});  //直接尝试插入 new User 的裸指针或初始化列表 {name, new User(...)}，这与 unique_ptr 的独占所有权语义冲突
        //编译器无法找到合适的pair构造函数来插入元素。
        users.emplace(name, std::make_unique<User>(type, name, passwd));
        std::cout << "Success register!" << std::endl;
        SaveUsers();
        // for(auto it = users.begin();it != users.end();it++){
        //     std::cout << it->second->Gethash_passwd()<< std::endl;
        // }

        if (type == "1") {
            std::string education;
            std::vector<std::string> subjects;
            uint16_t price_min, price_high;
            std::vector<std::string> locations;
            std::vector<std::pair<std::string, std::pair<int, int>>> available_times;

            std::cout << "请输入老师的学历:  0:大学生家教  1:在职教师  2:特级教师: ";
            std::cin >> education;
            std::cin.ignore(); // Clear newline

            std::cout << "输入您教学科目（以逗号分隔）：";
            std::string subjects_str;
            std::getline(std::cin, subjects_str);
            std::istringstream subjects_ss(subjects_str);
            std::string subject;
            while (std::getline(subjects_ss, subject, ',')) {
                if (!subject.empty()) {
                    subjects.push_back(subject);
                }
            }

            std::cout << "输入最小价格: ";
            std::cin >> price_min;

            std::cout << "输入最大价格: ";
            std::cin >> price_high;
            std::cin.ignore(); // Clear newline

            std::cout << "输入可教学的地点（以逗号分隔）: ";
            std::string locations_str;
            std::getline(std::cin, locations_str);
            std::istringstream locations_ss(locations_str);
            std::string location;
            while (std::getline(locations_ss, location, ',')) {
                if (!location.empty()) {
                    locations.push_back(location);
                }
            }

            std::cout << "输入可用时间（格式: Tue,12,14/Wed,14,17）：";
            std::string times_str;
            std::getline(std::cin, times_str);
            std::istringstream times_ss(times_str);
            std::string time_entry;
            while (std::getline(times_ss, time_entry, '/')) {
                std::istringstream entry_ss(time_entry);
                std::string day, start_str, end_str;
                if (std::getline(entry_ss, day, ',') &&
                    std::getline(entry_ss, start_str, ',') &&
                    std::getline(entry_ss, end_str)) {
                    try {
                        int start = std::stoi(start_str);
                        int end = std::stoi(end_str);
                        available_times.emplace_back(day, std::make_pair(start, end));
                    } catch (const std::exception& e) {
                        std::cout << "Invalid time format: " << time_entry << ", skipping." << std::endl;
                    }
                }
            }

            auto teacher = std::make_unique<Teacher>(
                education, subjects, price_min, price_high, locations, available_times);
            teacher->ChangeName(name);
            teachers.emplace(name, std::move(teacher));
            SaveTeachers();
        }
        return;
    }
    std::cout << "Failed to register!" << std::endl;
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

void UserManage::LoadTeachers(std::string teacher_file){
    std::ifstream file(teacher_file);
    if(!file) return;
    std::string line;
    while(std::getline(file,line)){
        if(auto teacher = fromTeachFile(line)){
            teachers.insert({teacher->GetName(),std::move(teacher)});
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

void UserManage::SaveTeachers(){
    std::ofstream file(teacher_file);
    for (const auto& pair : teachers){
        file << pair.second->ToTeachfilestring() << "\n";
    }
}

//User类的函数，主要想把当前user的成员转换成想要传入txt文件中的格式
std::string User::Tofilestring() {
    return user_name + ',' + user_type + ',' + hash_passwd + ',' + salt; 
}

std::string Teacher::ToTeachfilestring(){
    std::string course;
    for(auto it = subjects.begin();it != subjects.end();it++){
        if(it != subjects.end()-1)
            course += *it + ','; 
        else
            course += *it; 
        
    }
    std::string locations;
    for(auto it = allow_location.begin();it != allow_location.end();it++){
        if(it != allow_location.end()-1)
            locations += *it + ','; 
        else
            locations += *it; 
        
    }
    std::string times;
    for(auto it = available_times.begin();it != available_times.end();it++){
        auto ti = it->second;
        if(it != available_times.end()-1)
            times += it->first + ',' + std::to_string(ti.first) + ',' + std::to_string(ti.second) + '/';
        else
            times += it->first + ',' + std::to_string(ti.first) + ',' + std::to_string(ti.second);
    }
    std::string price_m = std::to_string(price_min);
    std::string price_h = std::to_string(price_high);
    return user_name + '|' + education + '|' + course + '|' + price_m + '|' + price_h + '|' + locations + '|' + times;
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

std::unique_ptr<Teacher> UserManage::fromTeachFile(const std::string& data) {
    std::istringstream iss(data);
    std::string name, education, course, price_min, price_high, locations, times;

    auto teacher_info = std::make_unique<Teacher>();
    if (!std::getline(iss, name, '|') ||
        !std::getline(iss, education, '|') ||
        !std::getline(iss, course, '|') ||
        !std::getline(iss, price_min, '|') ||
        !std::getline(iss, price_high, '|') ||
        !std::getline(iss, locations, '|') ||
        !std::getline(iss, times)) {
        return nullptr; 
    }

    teacher_info->ChangeName(name);
    teacher_info->education = education;

    std::istringstream course_ss(course);
    std::string subject;
    while (std::getline(course_ss, subject, ',')) {
        if (!subject.empty()) {
            teacher_info->subjects.push_back(subject);
        }
    }

    try {
        teacher_info->price_min = std::stoi(price_min);
        teacher_info->price_high = std::stoi(price_high);
    } catch (const std::exception& e) {
        return nullptr; 
    }

    std::istringstream locations_ss(locations);
    std::string location;
    while (std::getline(locations_ss, location, ',')) {
        if (!location.empty()) {
            teacher_info->allow_location.push_back(location);
        }
    }

    std::istringstream times_ss(times);
    std::string time_entry;
    while (std::getline(times_ss, time_entry, '/')) {
        std::istringstream entry_ss(time_entry);
        std::string day, start_str, end_str;
        if (std::getline(entry_ss, day, ',') &&
            std::getline(entry_ss, start_str, ',') &&
            std::getline(entry_ss, end_str)) {
            try {
                int start = std::stoi(start_str);
                int end = std::stoi(end_str);
                teacher_info->available_times.emplace_back(day, std::make_pair(start, end));
            } catch (const std::exception& e) {
                continue; 
            }
        }
    }
    std::cout << "name: " << teacher_info->GetName() << ' ' << "education: " << teacher_info->education  << ' ' << "subjects: "<< course << ' ' << times << std::endl;

    return teacher_info;
}