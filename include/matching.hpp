#ifndef MATCHING_HPP
#define MATCHING_HPP
#include "config.hpp"
#include "user_manage.hpp"
//teacher structure
/*
时间区间：区间树，适合高效处理时间段重叠查询。
价格区间：红黑树，适合高效处理范围查询。
初筛：unordered_map 索引，加速过滤。
*/

/*
load teacher_info ->
input expect info ->
Suanfa
result
*/

// 期望学历,性格,学科,地区,价格,期望时间段 (星期几, [开始, 结束])
// 匹配条件结构
struct MatchingCriteria {
    std::string education; // 期望学历
    std::string character; // 期望性格
    std::string subject;   // 期望学科
    std::string location;  // 期望地区
    uint16_t price_min;    // 最低可接受价格
    uint16_t price_high;   // 最高可接受价格
    std::pair<std::string, std::pair<int, int>> time_slot; // 期望时间段 (星期几, [开始, 结束])
};

// 匹配结果结构
struct MatchResult {
    std::unique_ptr<Teacher> teacher;
    double score;
};

// 匹配管理类
class MatchingManager {
public:
    MatchingManager(const UserManage& userManage);
    std::vector<MatchResult> matchTeachers(const MatchingCriteria& criteria);

private:
    const UserManage& userManage;
    std::unordered_map<std::string, std::vector<std::unique_ptr<Teacher>>> subject_index;
    std::unordered_map<std::string, std::vector<std::unique_ptr<Teacher>>> location_index;

    double calculateScore(const Teacher& teacher, const MatchingCriteria& criteria);
    bool timeOverlap(const std::vector<std::pair<std::string, std::pair<int, int>>>& teacher_times,
                     const std::pair<std::string, std::pair<int, int>>& student_time);
    bool priceOverlap(uint16_t teacher_min, uint16_t teacher_max, uint16_t student_min, uint16_t student_max);
};

#endif