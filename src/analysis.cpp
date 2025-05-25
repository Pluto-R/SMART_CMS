#include "analysis.hpp"
#include <fstream>
#include <algorithm>
#include <sstream>
#include <iostream>

void ScoreManager::addScore(const std::string& date, const std::string& subject, double score) {
    if (score < 0 || score > 100) {
        std::cerr << "[ScoreManager] Invalid score: " << score << ". Score must be between 0 and 100." << std::endl;
        return;
    }
    scores.push_back({date, subject, score});
    std::cout << "[ScoreManager] Added score: user=" << user_id << ", subject=" << subject 
              << ", score=" << score << ", date=" << date << std::endl;
}

std::vector<ScoreEntry> ScoreManager::getScoresBySubject(const std::string& subject) const {
    std::vector<ScoreEntry> result;
    for (const auto& entry : scores) {
        if (entry.subject == subject) {
            result.push_back(entry);
        }
    }
    std::sort(result.begin(), result.end(), [](const ScoreEntry& a, const ScoreEntry& b) {
        return a.date < b.date;
    });
    std::cout << "[ScoreManager] Found " << result.size() << " scores for subject: " << subject << std::endl;
    return result;
}

void ScoreManager::saveToFile(const std::string& filename) const {
    std::cerr << "[ScoreManager] saveToFile not used; scores saved via UserManage." << std::endl;
}

void ScoreManager::loadFromFile(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) {
        std::cerr << "[ScoreManager] Failed to open file: " << filename << std::endl;
        return;
    }
    scores.clear();
    std::string line;
    int line_count = 0;
    while (std::getline(in, line)) {
        line_count++;
        std::stringstream ss(line);
        std::string student_id, teacher_id, subject, score_str, date;
        if (std::getline(ss, student_id, '|') &&
            std::getline(ss, teacher_id, '|') &&
            std::getline(ss, subject, '|') &&
            std::getline(ss, score_str, '|') &&
            std::getline(ss, date)) {
            if (student_id != user_id) continue;
            try {
                double score = std::stod(score_str);
                scores.push_back({date, subject, score});
                //std::cout << "[ScoreManager] Loaded score: user=" << user_id << ", subject=" << subject 
                  //        << ", score=" << score << ", date=" << date << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "[ScoreManager] Invalid score format in line " << line_count << ": " << line << std::endl;
            }
        } else {
            std::cerr << "[ScoreManager] Malformed line " << line_count << ": " << line << std::endl;
        }
    }
    std::cout << "[ScoreManager] Loaded " << scores.size() << " scores for user: " << user_id << std::endl;
    in.close();
}

void ScoreManager::plotStudentScores(const std::string& subject, sf::RenderWindow& window) const {
    auto subject_scores = getScoresBySubject(subject);
    if (subject_scores.empty()) {
        std::cerr << "[ScoreManager] No scores found for subject: " << subject << " for user: " << user_id << std::endl;
        return;
    }

    sf::Font font;
    // Try loading a Chinese-supporting font (e.g., SimSun or Noto Sans CJK)
    bool font_loaded = font.loadFromFile("/usr/share/fonts/truetype/noto/NotoSansCJK-Regular.ttc") ||
                       font.loadFromFile("/usr/share/fonts/truetype/simsun/simsun.ttc") ||
                       font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
    if (!font_loaded) {
        std::cerr << "[ScoreManager] Failed to load any font, skipping text rendering" << std::endl;
    }

    std::cout << "[ScoreManager] Plotting scores for subject: " << subject << ", scores count: " << subject_scores.size() << std::endl;

    // Ensure title is in Chinese
    std::string title = subject.empty() ? "未知学科成绩趋势" : subject + "成绩趋势";
    std::cout << "[ScoreManager] Using title: " << title << std::endl;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                std::cout << "[ScoreManager] Closing plot window for subject: " << subject << std::endl;
                window.close();
                return; // Return to menu instead of crashing
            }
        }

        window.clear(sf::Color::White);

        // Set up axes
        sf::VertexArray x_axis(sf::Lines, 2);
        x_axis[0].position = sf::Vector2f(50, 550);
        x_axis[1].position = sf::Vector2f(750, 550);
        x_axis[0].color = x_axis[1].color = sf::Color::Black;

        sf::VertexArray y_axis(sf::Lines, 2);
        y_axis[0].position = sf::Vector2f(50, 50);
        y_axis[1].position = sf::Vector2f(50, 550);
        y_axis[0].color = y_axis[1].color = sf::Color::Black;

        // Plot scores
        sf::VertexArray line(sf::LineStrip, subject_scores.size());
        float x_step = subject_scores.size() > 1 ? 700.0f / (subject_scores.size() - 1) : 700.0f;
        for (size_t i = 0; i < subject_scores.size(); ++i) {
            float x = 50 + i * x_step;
            float y = 550 - (subject_scores[i].score / 100.0f) * 500;
            line[i].position = sf::Vector2f(x, y);
            line[i].color = sf::Color::Blue;
        }

        // Draw axes and line
        window.draw(x_axis);
        window.draw(y_axis);
        window.draw(line);

        // Add title only if font loaded
        if (font_loaded) {
            sf::Text text;
            text.setFont(font);
            text.setString(title);
            text.setCharacterSize(20);
            text.setFillColor(sf::Color::Black);
            text.setPosition(300, 10);
            window.draw(text);
        }

        window.display();
    }
}

void TeacherScoreAnalyzer::addStudentScores(const std::string& student_id, const std::vector<ScoreEntry>& scores) {
    student_scores[student_id] = scores;
    std::cout << "[TeacherScoreAnalyzer] Added scores for student: " << student_id 
              << ", count: " << scores.size() << std::endl;
}

void TeacherScoreAnalyzer::plotTeacherAnalysis(const std::string& subject, sf::RenderWindow& window) const {
    std::vector<std::vector<double>> all_scores;
    std::vector<std::string> student_ids;
    for (const auto& [student_id, scores] : student_scores) {
        std::vector<double> student_subject_scores;
        for (const auto& entry : scores) {
            if (entry.subject == subject) {
                student_subject_scores.push_back(entry.score);
            }
        }
        if (!student_subject_scores.empty()) {
            all_scores.push_back(student_subject_scores);
            student_ids.push_back(student_id);
        }
    }

    if (all_scores.empty()) {
        std::cerr << "[TeacherScoreAnalyzer] No scores found for subject: " << subject 
                  << " for teacher: " << teacher_id << std::endl;
        return;
    }

    sf::Font font;
    // Try loading a Chinese-supporting font
    bool font_loaded = font.loadFromFile("/usr/share/fonts/truetype/noto/NotoSansCJK-Regular.ttc") ||
                       font.loadFromFile("/usr/share/fonts/truetype/simsun/simsun.ttc") ||
                       font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
    if (!font_loaded) {
        std::cerr << "[TeacherScoreAnalyzer] Failed to load any font, skipping text rendering" << std::endl;
    }

    std::cout << "[TeacherScoreAnalyzer] Plotting scores for subject: " << subject 
              << ", student count: " << all_scores.size() << std::endl;

    // Ensure title is in Chinese
    std::string title = subject.empty() ? "未知学科学生平均成绩" : subject + "学生平均成绩";
    std::cout << "[TeacherScoreAnalyzer] Using title: " << title << std::endl;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                std::cout << "[TeacherScoreAnalyzer] Closing plot window for subject: " << subject << std::endl;
                window.close();
                return; // Return to menu instead of crashing
            }
        }

        window.clear(sf::Color::White);

        // Set up axes ;;;
        sf::VertexArray x_axis(sf::Lines, 2);
        x_axis[0].position = sf::Vector2f(50, 550);
        x_axis[1].position = sf::Vector2f(750, 550);
        x_axis[0].color = x_axis[1].color = sf::Color::Black;

        sf::VertexArray y_axis(sf::Lines, 2);
        y_axis[0].position = sf::Vector2f(50, 50);
        y_axis[1].position = sf::Vector2f(50, 550);
        y_axis[0].color = y_axis[1].color = sf::Color::Black;

        // Calculate average score per student
        std::vector<double> avg_scores;
        for (const auto& scores : all_scores) {
            double sum = 0;
            for (double score : scores) {
                sum += score;
            }
            avg_scores.push_back(sum / scores.size());
        }

        // Plot average scores
        sf::VertexArray line(sf::LineStrip, avg_scores.size());
        float x_step = avg_scores.size() > 1 ? 700.0f / (avg_scores.size() - 1) : 700.0f;
        for (size_t i = 0; i < avg_scores.size(); ++i) {
            float x = 50 + i * x_step;
            float y = 550 - (avg_scores[i] / 100.0f) * 500;
            line[i].position = sf::Vector2f(x, y);
            line[i].color = sf::Color::Red;
        }

        window.draw(x_axis);
        window.draw(y_axis);
        window.draw(line);

        // Add title only if font loaded
        if (font_loaded) {
            sf::Text text;
            text.setFont(font);
            text.setString(title);
            text.setCharacterSize(20);
            text.setFillColor(sf::Color::Black);
            text.setPosition(300, 10);
            window.draw(text);
        }

        window.display();
    }
}

void TeacherScoreAnalyzer::saveToFile(const std::string& filename) const {
    std::cerr << "[TeacherScoreAnalyzer] saveToFile not used; scores saved via UserManage." << std::endl;
}

void TeacherScoreAnalyzer::loadFromFile(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) {
        std::cerr << "[TeacherScoreAnalyzer] Failed to open file: " << filename << std::endl;
        return;
    }
    student_scores.clear();
    std::string line;
    int line_count = 0;
    while (std::getline(in, line)) {
        line_count++;
        std::stringstream ss(line);
        std::string student_id, teacher_id, subject, score_str, date;
        if (std::getline(ss, student_id, '|') &&
            std::getline(ss, teacher_id, '|') &&
            std::getline(ss, subject, '|') &&
            std::getline(ss, score_str, '|') &&
            std::getline(ss, date)) {
            if (teacher_id != this->teacher_id) continue;
            try {
                double score = std::stod(score_str);
                student_scores[student_id].push_back({date, subject, score});
                std::cout << "[TeacherScoreAnalyzer] Loaded score: student=" << student_id 
                          << ", subject=" << subject << ", score=" << score << ", date=" << date << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "[TeacherScoreAnalyzer] Invalid score format in line " << line_count << ": " << line << std::endl;
            }
        } else {
            std::cerr << "[TeacherScoreAnalyzer] Malformed line " << line_count << ": " << line << std::endl;
        }
    }
    std::cout << "[TeacherScoreAnalyzer] Loaded scores for " << student_scores.size() 
              << " students for teacher: " << teacher_id << std::endl;
    in.close();
}