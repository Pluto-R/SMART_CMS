#include "matching.hpp"
#include <algorithm>
#include <cmath>

MatchingManager::MatchingManager(UserManage& userManage) : userManage(userManage) {
    for (auto& pair : userManage.GetTeachers()) {
        Teacher* teacher = pair.second.get();
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
        price_tree.insert(teacher);
        for (const auto& time : teacher->available_times) {
            time_tree.insert(time.second.first, time.second.second, teacher);
        }
    }
}

double MatchingManager::calculateScore(const Teacher& teacher, const MatchingCriteria& criteria) {
    double score = 0.0;

    // 学历匹配 (30%)
    double degree_score = (teacher.education == criteria.education) ? 1.0 : 0.7;
    score += 0.3 * degree_score;

    // 性格匹配 (20%)
    double character_score = (teacher.character == criteria.character) ? 1.0 : 0.5;
    score += 0.2 * character_score;

    // 时间重叠 (20%)
    double time_score = timeOverlap(teacher.available_times, criteria.time_slot) ? 1.0 : 0.0;
    score += 0.2 * time_score;

    // 价格匹配 (20%)
    double price_mid_teacher = (teacher.price_min + teacher.price_high) / 2.0;
    double price_mid_student = (criteria.price_min + criteria.price_high) / 2.0;
    double price_range = criteria.price_high - criteria.price_min;
    double price_score = price_range > 0 ? 1.0 - std::abs(price_mid_teacher - price_mid_student) / price_range : 1.0;
    score += 0.2 * price_score;

    // 地区匹配 (10%)
    double location_score = (std::find(teacher.allow_location.begin(), teacher.allow_location.end(), criteria.location) != teacher.allow_location.end()) ? 1.0 : 0.0;
    score += 0.1 * location_score;

    return score;
}

bool MatchingManager::timeOverlap(const std::vector<std::pair<std::string, std::pair<int, int>>>& teacher_times,
                                 const std::pair<std::string, std::pair<int, int>>& student_time) {
    for (const auto& teacher_time : teacher_times) {
        if (teacher_time.first == student_time.first) {
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

    auto subject_it = subject_index.find(criteria.subject);
    std::vector<Teacher*> candidates = (subject_it != subject_index.end()) ? subject_it->second : std::vector<Teacher*>();

    auto location_it = location_index.find(criteria.location);
    if (location_it != location_index.end()) {
        candidates = price_tree.intersect(candidates, location_it->second);
    } else {
        candidates.clear();
    }

    candidates = price_tree.queryRange(criteria.price_min, criteria.price_high, candidates);

    candidates = time_tree.queryOverlap(criteria.time_slot.second, candidates);

    for (Teacher* teacher_ptr : candidates) {
        if (teacher_ptr->education != criteria.education) continue;
        if (teacher_ptr->character != criteria.character) continue;
        if (!timeOverlap(teacher_ptr->available_times, criteria.time_slot)) continue;
        if (!priceOverlap(teacher_ptr->price_min, teacher_ptr->price_high, 
                          criteria.price_min, criteria.price_high)) continue;

        double score = calculateScore(*teacher_ptr, criteria);
        if (score > 0.0) {
            results.push_back({std::make_unique<Teacher>(*teacher_ptr), score});
        }
    }

    std::sort(results.begin(), results.end(), [](const auto& a, const auto& b) {
        return a.score > b.score;
    });
    if (results.size() > 5) results.resize(5);

    return results;
}

// PriceRBTree Implementations
PriceRBTree::Node* PriceRBTree::rotateLeft(Node* h) {
    Node* x = h->right;
    h->right = x->left;
    x->left = h;
    x->isRed = h->isRed;
    h->isRed = true;
    return x;
}

PriceRBTree::Node* PriceRBTree::rotateRight(Node* h) {
    Node* x = h->left;
    h->left = x->right;
    x->right = h;
    x->isRed = h->isRed;
    h->isRed = true;
    return x;
}

void PriceRBTree::flipColors(Node* h) {
    h->isRed = !h->isRed;
    h->left->isRed = !h->left->isRed;
    h->right->isRed = !h->right->isRed;
}

PriceRBTree::Node* PriceRBTree::insert(Node* h, uint16_t price, Teacher* teacher) {
    if (!h) {
        Node* node = new Node(price);
        node->teachers.push_back(teacher);
        return node;
    }
    if (price < h->price) h->left = insert(h->left, price, teacher);
    else if (price > h->price) h->right = insert(h->right, price, teacher);
    else h->teachers.push_back(teacher);

    if (isRed(h->right) && !isRed(h->left)) h = rotateLeft(h);
    if (isRed(h->left) && isRed(h->left->left)) h = rotateRight(h);
    if (isRed(h->left) && isRed(h->right)) flipColors(h);

    return h;
}

void PriceRBTree::insert(Teacher* teacher) {
    root = insert(root, teacher->price_min, teacher);
    if (root) root->isRed = false;
}

void PriceRBTree::queryRange(Node* h, uint16_t low, uint16_t high, std::vector<Teacher*>& result) {
    if (!h) return;
    if (low <= h->price) queryRange(h->left, low, high, result);
    if (low <= h->price && h->price <= high) result.insert(result.end(), h->teachers.begin(), h->teachers.end());
    if (h->price <= high) queryRange(h->right, low, high, result);
}

std::vector<Teacher*> PriceRBTree::intersect(const std::vector<Teacher*>& a, const std::vector<Teacher*>& b) const {
    std::vector<Teacher*> result;
    for (Teacher* t : a) {
        if (std::find(b.begin(), b.end(), t) != b.end()) {
            result.push_back(t);
        }
    }
    return result;
}

std::vector<Teacher*> PriceRBTree::queryRange(uint16_t low, uint16_t high, const std::vector<Teacher*>& candidates) {
    std::vector<Teacher*> result;
    queryRange(root, low, high, result);
    return intersect(result, candidates);
}

// TimeIntervalTree Implementations
TimeIntervalTree::Node* TimeIntervalTree::insert(Node* h, Interval interval, Teacher* teacher) {
    if (!h) {
        Node* node = new Node(interval.start, interval.end);
        node->interval.teachers.push_back(teacher);
        return node;
    }
    if (interval.start < h->interval.start) h->left = insert(h->left, interval, teacher);
    else if (interval.start > h->interval.start) h->right = insert(h->right, interval, teacher);
    else h->interval.teachers.push_back(teacher);

    h->maxEnd = std::max(h->maxEnd, interval.end);
    if (h->left) h->maxEnd = std::max(h->maxEnd, h->left->maxEnd);
    if (h->right) h->maxEnd = std::max(h->maxEnd, h->right->maxEnd);

    return h;
}

void TimeIntervalTree::insert(int start, int end, Teacher* teacher) {
    root = insert(root, {start, end}, teacher);
}

void TimeIntervalTree::queryOverlap(Node* h, int start, int end, std::vector<Teacher*>& result) {
    if (!h) return;
    if (h->left && h->left->maxEnd >= start) queryOverlap(h->left, start, end, result);
    if (h->interval.start <= end && start <= h->interval.end) {
        result.insert(result.end(), h->interval.teachers.begin(), h->interval.teachers.end());
    }
    if (h->interval.start <= end) queryOverlap(h->right, start, end, result);
}

std::vector<Teacher*> TimeIntervalTree::intersect(const std::vector<Teacher*>& a, const std::vector<Teacher*>& b) const {
    std::vector<Teacher*> result;
    for (Teacher* t : a) {
        if (std::find(b.begin(), b.end(), t) != b.end()) {
            result.push_back(t);
        }
    }
    return result;
}

std::vector<Teacher*> TimeIntervalTree::queryOverlap(const std::pair<int, int>& slot, const std::vector<Teacher*>& candidates) {
    std::vector<Teacher*> result;
    queryOverlap(root, slot.first, slot.second, result);
    return intersect(result, candidates);
}