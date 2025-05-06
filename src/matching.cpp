#include "matching.hpp"
#include <algorithm>


MatchingManager::MatchingManager(const UserManage& userManage) : userManage(userManage) {
    // 构建索引
    for (const auto& pair : userManage.GetTeachers()) {
        const Teacher* teacher = pair.second.get();
        if (!teacher->subjects.empty()) {
            for (const auto& subject : teacher->subjects) {
                subject_index[subject].push_back(teacher);
            }
        }
        if (!teacher->allow_location.empty()) {
            for (const auto& location : teacher->allow_location) {
                location_index[location].push_back(teacher);
            }
        }
    }
}

double MatchingManager::calculateScore(const Teacher& teacher, const MatchingCriteria& criteria) {
    double score = 0.0;

    // 学历匹配30%
    double degree_score = (teacher.education == criteria.education) ? 1.0 : 0.7;
    score += 0.3 * degree_score;

    // 性格匹配20%
    double character_score = (teacher.character == criteria.character) ? 1.0 : 0.5;
    score += 0.2 * character_score;

    // 时间重叠度20%
    double time_score = timeOverlap(teacher.available_times, criteria.time_slot) ? 1.0 : 0.0;
    score += 0.2 * time_score;

    // 价格匹配度20%
    double price_mid_teacher = (teacher.price_min + teacher.price_high) / 2.0;
    double price_mid_student = (criteria.price_min + criteria.price_high) / 2.0;
    double price_range = criteria.price_high - criteria.price_min;
    double price_score = price_range > 0 ? 1.0 - std::abs(price_mid_teacher - price_mid_student) / price_range : 1.0;
    score += 0.2 * price_score;

    // 地区匹配10%
    double location_score = (std::find(teacher.allow_location.begin(), teacher.allow_location.end(), criteria.location) != teacher.allow_location.end()) ? 1.0 : 0.0;
    score += 0.1 * location_score;

    return score;
}

bool MatchingManager::timeOverlap(const std::vector<std::pair<std::string, std::pair<int, int>>>& teacher_times,
                                 const std::pair<std::string, std::pair<int, int>>& student_time) {
    for (const auto& teacher_time : teacher_times) {
        if (teacher_time.first == student_time.first) { // 星期几匹配
            int t_start = teacher_time.second.first;
            int t_end = teacher_time.second.second;
            int s_start = student_time.second.first;
            int s_end = student_time.second.second;
            if (std::max(t_start, s_start) <= std::min(t_end, s_end)) {
                return true;
            }
        }
    }
    return false;
}

bool MatchingManager::priceOverlap(uint16_t teacher_min, uint16_t teacher_max,
                                  uint16_t student_min, uint16_t student_max) {
    return std::max(teacher_min, student_min) <= std::min(teacher_max, student_max);
}

std::vector<MatchResult> MatchingManager::matchTeachers(const MatchingCriteria& criteria) {
    std::vector<MatchResult> results;

    // 初筛：学科和地区
    auto subject_teachers = subject_index[criteria.subject];
    std::vector<const Teacher*> filtered_teachers;
    for (const auto* teacher : subject_teachers) {
        if (std::find(teacher->allow_location.begin(), teacher->allow_location.end(), criteria.location) != teacher->allow_location.end()) {
            filtered_teachers.push_back(teacher);
        }
    }

    // 进一步过滤：学历、性格、时间、价格
    for (const auto* teacher : filtered_teachers) {
        if (teacher->education != criteria.education) continue;
        if (teacher->character != criteria.character) continue;
        if (!timeOverlap(teacher->available_times, criteria.time_slot)) continue;
        if (!priceOverlap(teacher->price_min, teacher->price_high, criteria.price_min, criteria.price_high)) continue;

        double score = calculateScore(*teacher, criteria);
        if (score > 0.0) { // 最低分数阈值
            results.push_back({teacher, score});
        }
    }

    // 排序
    std::sort(results.begin(), results.end(), [](const MatchResult& a, const MatchResult& b) {
        return a.score > b.score;
    });

    // 返回 Top 5
    if (results.size() > 5) results.resize(5);
    return results;
}