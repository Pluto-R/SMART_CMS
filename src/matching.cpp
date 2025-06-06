#include "matching.hpp"
#include <algorithm>
#include <cmath>
#include <set>
#include <QDebug>
#include <limits>

PriceRBTree::Node* PriceRBTree::rotateLeft(Node* h) {
    Node* x = h->right;
    h->right = x->left;
    if (x->left) x->left->parent = h;
    x->left = h;
    x->parent = h->parent;
    h->parent = x;
    x->isRed = h->isRed;
    h->isRed = true;
    return x;
}

PriceRBTree::Node* PriceRBTree::rotateRight(Node* h) {
    Node* x = h->left;
    h->left = x->right;
    if (x->right) x->right->parent = h;
    x->right = h;
    x->parent = h->parent;
    h->parent = x;
    x->isRed = h->isRed;
    h->isRed = true;
    return x;
}

void PriceRBTree::flipColors(Node* h) {
    h->isRed = !h->isRed;
    if (h->left) h->left->isRed = !h->left->isRed;
    if (h->right) h->right->isRed = !h->right->isRed;
}

PriceRBTree::Node* PriceRBTree::insert(Node* h, uint16_t price, Teacher* teacher) {
    if (h == nullptr) {
        Node* newNode = new Node(price);
        newNode->teachers.push_back(teacher);
        return newNode;
    }

    if (price < h->price) {
        h->left = insert(h->left, price, teacher);
        if (h->left) h->left->parent = h;
    } else if (price > h->price) {
        h->right = insert(h->right, price, teacher);
        if (h->right) h->right->parent = h;
    } else {
        h->teachers.push_back(teacher);
        return h;
    }

    if (h->right && h->right->isRed && (!h->left || !h->left->isRed)) {
        h = rotateLeft(h);
    }
    if (h->left && h->left->isRed && h->left->left && h->left->left->isRed) {
        h = rotateRight(h);
    }
    if (h->left && h->left->isRed && h->right && h->right->isRed) {
        flipColors(h);
    }

    return h;
}

void PriceRBTree::insert(Teacher* teacher) {
    root = insert(root, teacher->price_min, teacher);
    if (root) root->isRed = false;
}

void PriceRBTree::queryRange(Node* h, uint16_t min_price, uint16_t max_price, std::vector<Teacher*>& result) {
    if (h == nullptr) return;

    if (h->price > min_price && h->left) {
        queryRange(h->left, min_price, max_price, result);
    }

    if (h->price >= min_price && h->price <= max_price) {
        result.insert(result.end(), h->teachers.begin(), h->teachers.end());
    }

    if (h->price < max_price && h->right) {
        queryRange(h->right, min_price, max_price, result);
    }
}

std::vector<Teacher*> PriceRBTree::queryRange(uint16_t min_price, uint16_t max_price) {
    std::vector<Teacher*> result;
    queryRange(root, min_price, max_price, result);
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
    return result;
}

void PriceRBTree::destroyTree(Node* node) {
    if (node) {
        destroyTree(node->left);
        destroyTree(node->right);
        delete node;
    }
}

PriceRBTree::~PriceRBTree() {
    destroyTree(root);
    root = nullptr;
}

void PriceRBTree::reset() {
    destroyTree(root);
    root = nullptr;
}

void IntervalTree::insert(std::unique_ptr<Node>& h, Interval interval, const std::string& weekday, Teacher* teacher) {
    if (!h) {
        h = std::make_unique<Node>(interval, weekday);
        h->teachers.push_back(teacher);
        return;
    }

    h->maxEnd = std::max({h->interval.end,
                          h->left ? h->left->maxEnd : std::numeric_limits<int>::min(),
                          h->right ? h->right->maxEnd : std::numeric_limits<int>::min()});

    if (interval.start < h->interval.start) {
        insert(h->left, interval, weekday, teacher);
    } else {
        insert(h->right, interval, weekday, teacher);
    }

    h->maxEnd = std::max({h->interval.end,
                          h->left ? h->left->maxEnd : std::numeric_limits<int>::min(),
                          h->right ? h->right->maxEnd : std::numeric_limits<int>::min()});
}

void IntervalTree::insert(int start, int end, const std::string& weekday, Teacher* teacher) {
    insert(root, Interval(start, end), weekday, teacher);
}

void IntervalTree::queryOverlap(Node* h, const std::string& weekday, int start, int end,
                                std::vector<std::pair<Teacher*, std::pair<std::string, std::pair<int, int>>>>& result) {
    if (h == nullptr) return;

    if (h->weekday == weekday &&
        h->interval.start <= end && h->interval.end >= start) { // Relaxed overlap condition
        for (Teacher* teacher : h->teachers) {
            for (const auto& time_slot : teacher->getAvailableTimes()) {
                if (time_slot.first == weekday &&
                    std::max(time_slot.second.first, start) <= std::min(time_slot.second.second, end)) {
                    result.push_back({teacher, time_slot});
                    qDebug() << "Time match found for teacher:" << QString::fromStdString(teacher->GetName())
                             << ", time:" << time_slot.second.first << "-" << time_slot.second.second;
                }
            }
        }
    }

    if (h->left && h->left->maxEnd >= start) {
        queryOverlap(h->left.get(), weekday, start, end, result);
    }

    if (h->right && h->interval.start <= end) {
        queryOverlap(h->right.get(), weekday, start, end, result);
    }
}

std::vector<std::pair<Teacher*, std::pair<std::string, std::pair<int, int>>>>
IntervalTree::queryOverlap(const std::string& weekday, int start, int end) {
    std::vector<std::pair<Teacher*, std::pair<std::string, std::pair<int, int>>>> result;
    queryOverlap(root.get(), weekday, start, end, result);
    std::sort(result.begin(), result.end(), [](const auto& a, const auto& b) {
        return a.first < b.first || (a.first == b.first && a.second < b.second);
    });
    result.erase(std::unique(result.begin(), result.end()), result.end());
    return result;
}

void IntervalTree::destroyTree(std::unique_ptr<Node>& node) {
    if (node) {
        node.reset();
    }
}

IntervalTree::~IntervalTree() {
    destroyTree(root);
}

void IntervalTree::reset() {
    root.reset();
}

MatchingManager::MatchingManager(UserManage& userManage) : userManage(userManage) {}

double MatchingManager::calculateScore(const Teacher& teacher, const MatchingCriteria& criteria) {
    double score = 0.0;
    double degree_score = criteria.education.empty() || teacher.education == criteria.education ? 1.0 : 0.7;
    score += 0.3 * degree_score;

    double character_score = criteria.character.empty() || teacher.character == criteria.character ? 1.0 : 0.5;
    score += 0.2 * character_score;

    double time_score = criteria.time_slot.first.empty() || timeOverlap(teacher.getAvailableTimes(), criteria.time_slot) ? 1.0 : 0.0;
    score += 0.2 * time_score;

    double price_mid_teacher = (teacher.price_min + teacher.price_max) / 2.0;
    double price_mid_student = (criteria.price_min + criteria.price_max) / 2.0;
    double price_range_student = criteria.price_max - criteria.price_min;
    double price_score;

    if (price_range_student > 0) {
        if (!priceOverlap(teacher.price_min, teacher.price_max, criteria.price_min, criteria.price_max)) {
            price_score = 0.0;
        } else {
            price_score = 1.0 - std::abs(price_mid_teacher - price_mid_student) / (price_range_student + 1);
            if (price_score < 0) price_score = 0;
        }
    } else {
        if (teacher.price_min <= criteria.price_min && teacher.price_max >= criteria.price_min) {
            price_score = 1.0;
        } else {
            price_score = 0.0;
        }
    }
    score += 0.2 * price_score;

    double location_score = criteria.location.empty() ||
        std::find(teacher.allow_location.begin(), teacher.allow_location.end(), criteria.location) != teacher.allow_location.end() ? 1.0 : 0.0;
    score += 0.1 * location_score;

    double subject_score = criteria.subject.empty() ||
        std::find(teacher.subjects.begin(), teacher.subjects.end(), criteria.subject) != teacher.subjects.end() ? 1.0 : 0.0;
    score += 0.1 * subject_score;

    return score;
}

bool MatchingManager::timeOverlap(const std::vector<std::pair<std::string, std::pair<int, int>>>& teacher_times,
                                 const std::pair<std::string, std::pair<int, int>>& student_time) {
    if (student_time.first.empty()) {
        qDebug() << "No specific time preference from student, allowing all teacher times.";
        return true;
    }
    for (const auto& teacher_time : teacher_times) {
        if (teacher_time.first == student_time.first) {
            int t_start = teacher_time.second.first;
            int t_end = teacher_time.second.second;
            int s_start = student_time.second.first;
            int s_end = student_time.second.second;
            if (std::max(t_start, s_start) <= std::min(t_end, s_end)) {
                qDebug() << "Time overlap found on" << QString::fromStdString(student_time.first)
                         << ": Teacher" << t_start << "-" << t_end
                         << "Student" << s_start << "-" << s_end;
                return true;
            }
        }
    }
    qDebug() << "No time overlap found for student time:" << QString::fromStdString(student_time.first)
             << student_time.second.first << "-" << student_time.second.second;
    return false;
}

bool MatchingManager::priceOverlap(uint16_t teacher_min, uint16_t teacher_max,
                                   uint16_t student_min, uint16_t student_max) {
    return std::max(teacher_min, student_min) <= std::min(teacher_max, student_max);
}

std::vector<MatchResult> MatchingManager::matchTeachers(const MatchingCriteria& criteria) {
    std::vector<MatchResult> results;
    std::set<Teacher*> unique_matched_teachers;

    qDebug() << "Matching criteria: Subject=" << QString::fromStdString(criteria.subject)
             << ", Education=" << QString::fromStdString(criteria.education)
             << ", Character=" << QString::fromStdString(criteria.character)
             << ", Location=" << QString::fromStdString(criteria.location)
             << ", Price=" << criteria.price_min << "-" << criteria.price_max
             << ", Time=" << QString::fromStdString(criteria.time_slot.first)
             << "," << criteria.time_slot.second.first << "-" << criteria.time_slot.second.second;

    std::vector<Teacher*> initial_candidates;
    if (!criteria.subject.empty()) {
        auto subject_it = subject_index.find(criteria.subject);
        if (subject_it != subject_index.end()) {
            initial_candidates.insert(initial_candidates.end(), subject_it->second.begin(), subject_it->second.end());
            qDebug() << "Found" << subject_it->second.size() << "teachers for subject:" << QString::fromStdString(criteria.subject);
        } else {
            qDebug() << "No teachers found for subject:" << QString::fromStdString(criteria.subject);
        }
    } else {
        for (const auto& pair : userManage.GetTeachers()) {
            initial_candidates.push_back(pair.second.get());
        }
        qDebug() << "No subject specified, using all" << initial_candidates.size() << "teachers";
    }

    qDebug() << "Initial candidates after subject filter:" << initial_candidates.size();

    std::vector<Teacher*> after_education;
    for (Teacher* teacher_ptr : initial_candidates) {
        if (!teacher_ptr) continue;
        if (criteria.education.empty() || teacher_ptr->education == criteria.education) {
            after_education.push_back(teacher_ptr);
        }
    }
    qDebug() << "Candidates after education filter:" << after_education.size();

    std::vector<Teacher*> after_character;
    for (Teacher* teacher_ptr : after_education) {
        if (criteria.character.empty() || teacher_ptr->character == criteria.character) {
            after_character.push_back(teacher_ptr);
        }
    }
    qDebug() << "Candidates after character filter:" << after_character.size();

    std::vector<Teacher*> after_location;
    for (Teacher* teacher_ptr : after_character) {
        QStringList teacher_locations;
        for (const auto& loc : teacher_ptr->allow_location) {
            teacher_locations << QString::fromStdString(loc);
        }
        qDebug() << "Checking location for teacher:" << QString::fromStdString(teacher_ptr->GetName())
                 << ", allow_location:" << teacher_locations.join(",")
                 << ", criteria.location:" << QString::fromStdString(criteria.location);
        if (criteria.location.empty() || criteria.location == "任意" ||
            std::find(teacher_ptr->allow_location.begin(), teacher_ptr->allow_location.end(), criteria.location) != teacher_ptr->allow_location.end()) {
            after_location.push_back(teacher_ptr);
        }
    }
    qDebug() << "Candidates after location filter:" << after_location.size();

    std::vector<Teacher*> after_price;
    for (Teacher* teacher_ptr : after_location) {
        if (priceOverlap(teacher_ptr->price_min, teacher_ptr->price_max, criteria.price_min, criteria.price_max)) {
            after_price.push_back(teacher_ptr);
        }
    }
    qDebug() << "Candidates after price filter:" << after_price.size();

    for (Teacher* teacher_ptr : after_price) {
        std::pair<std::string, std::pair<int, int>> best_matched_time_slot;
        bool time_slot_matched = false;

        if (!criteria.time_slot.first.empty()) {
            std::vector<std::pair<Teacher*, std::pair<std::string, std::pair<int, int>>>> time_matches =
                time_tree.queryOverlap(criteria.time_slot.first, criteria.time_slot.second.first, criteria.time_slot.second.second);

            for (const auto& match : time_matches) {
                if (match.first == teacher_ptr) {
                    best_matched_time_slot = match.second;
                    time_slot_matched = true;
                    break;
                }
            }
        } else {
            if (!teacher_ptr->getAvailableTimes().empty()) {
                best_matched_time_slot = teacher_ptr->getAvailableTimes()[0];
                time_slot_matched = true;
            }
        }

        if (!time_slot_matched && !teacher_ptr->getAvailableTimes().empty()) {
            best_matched_time_slot = teacher_ptr->getAvailableTimes()[0];
            time_slot_matched = true;
            qDebug() << "No exact time match for teacher:" << QString::fromStdString(teacher_ptr->GetName())
                     << ", using first available time:" << QString::fromStdString(best_matched_time_slot.first)
                     << "," << best_matched_time_slot.second.first << "-" << best_matched_time_slot.second.second;
        }

        if (time_slot_matched) {
            double score = calculateScore(*teacher_ptr, criteria);
            if (unique_matched_teachers.find(teacher_ptr) == unique_matched_teachers.end()) {
                results.push_back(MatchResult(teacher_ptr, score, best_matched_time_slot));
                unique_matched_teachers.insert(teacher_ptr);
                qDebug() << "Added teacher:" << QString::fromStdString(teacher_ptr->GetName()) << ", score:" << score;
            }
        }
    }

    std::sort(results.begin(), results.end(), [](const MatchResult& a, const MatchResult& b) {
        return a.score > b.score;
    });

    qDebug() << "Final results:" << results.size() << "teachers";
    return results;
}

void MatchingManager::rebuildIndexes() {
    qDebug() << "Starting rebuildIndexes";
    subject_index.clear();
    location_index.clear();
    price_tree.reset();
    time_tree.reset();

    for (const auto& pair : userManage.GetTeachers()) {
        Teacher* teacher_ptr = pair.second.get();
        if (!teacher_ptr) continue;
        qDebug() << "Processing teacher for indexing:" << QString::fromStdString(teacher_ptr->GetName());
        for (const auto& subject : teacher_ptr->subjects) {
            if (!subject.empty()) {
                subject_index[subject].push_back(teacher_ptr);
                qDebug() << "Indexed subject:" << QString::fromStdString(subject);
            }
        }
        for (const auto& location : teacher_ptr->allow_location) {
            if (!location.empty()) {
                location_index[location].push_back(teacher_ptr);
            }
        }
        price_tree.insert(teacher_ptr);
        for (const auto& time : teacher_ptr->getAvailableTimes()) {
            qDebug() << "Inserting time for teacher:" << QString::fromStdString(teacher_ptr->GetName())
                     << ", weekday:" << QString::fromStdString(time.first)
                     << ", time:" << time.second.first << "-" << time.second.second;
            time_tree.insert(time.second.first, time.second.second, time.first, teacher_ptr);
        }
    }
    qDebug() << "Finished rebuildIndexes";
}