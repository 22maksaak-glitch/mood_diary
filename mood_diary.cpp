// mood_diary.cpp
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <ctime>
#include <memory>
#include <variant>
#include <map>

using namespace std;

struct MoodEntry {
    int id;
    string timestamp;
    int mood;
    string note;

    MoodEntry(int id, int mood, const string& note, const string& timestamp = "")
        : id(id), mood(mood), note(note) {
        if (timestamp.empty()) {
            time_t now = time(nullptr);
            char buf[20];
            tm* tm_info = localtime(&now);
            strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", tm_info);
            this->timestamp = string(buf);
        } else {
            this->timestamp = timestamp;
        }
    }
};

class MoodDiary {
private:
    vector<MoodEntry> entries;
    int nextId = 1;

public:
    MoodEntry addEntry(int mood, const string& note) {
        if (mood < 1 || mood > 10) {
            throw invalid_argument("Оценка должна быть от 1 до 10");
        }
        MoodEntry entry(nextId, mood, note);
        entries.push_back(entry);
        nextId++;
        return entry;
    }

    MoodEntry* findEntry(int id) {
        auto it = find_if(entries.begin(), entries.end(), [id](const MoodEntry& e) { return e.id == id; });
        return it != entries.end() ? &(*it) : nullptr;
    }

    bool deleteEntry(int id) {
        auto it = find_if(entries.begin(), entries.end(), [id](const MoodEntry& e) { return e.id == id; });
        if (it == entries.end()) return false;
        entries.erase(it);
        return true;
    }

    map<string, variant<int, double, nullptr_t>> getStats() const {
        int total = entries.size();
        if (total == 0) {
            return {{"total", 0}, {"average", nullptr}, {"min", nullptr}, {"max", nullptr}};
        }
        int sum = 0, min = 11, max = 0;
        for (const auto& e : entries) {
            sum += e.mood;
            if (e.mood < min) min = e.mood;
            if (e.mood > max) max = e.mood;
        }
        return {
            {"total", total},
            {"average", static_cast<double>(sum) / total},
            {"min", min},
            {"max", max}
        };
    }

    void saveToFile(const string& filename = "mood_data.txt") {
        ofstream out(filename);
        if (!out) return;
        for (const auto& e : entries) {
            out << e.id << '|'
                << e.timestamp << '|'
                << e.mood << '|'
                << e.note << '\n';
        }
    }

    void loadFromFile(const string& filename = "mood_data.txt") {
        ifstream in(filename);
        if (!in) return;
        entries.clear();
        string line;
        while (getline(in, line)) {
            stringstream ss(line);
            string idStr, timestamp, moodStr, note;
            getline(ss, idStr, '|');
            getline(ss, timestamp, '|');
            getline(ss, moodStr, '|');
            getline(ss, note, '|');
            int id = stoi(idStr);
            int mood = stoi(moodStr);
            entries.emplace_back(id, mood, note, timestamp);
            if (id >= nextId) nextId = id + 1;
        }
    }

    const vector<MoodEntry>& getEntries() const { return entries; }
};

string readString(const string& prompt) {
    cout << prompt;
    string input;
    getline(cin, input);
    return input;
}

int readInt(const string& prompt) {
    while (true) {
        cout << prompt;
        string input;
        getline(cin, input);
        try {
            return stoi(input);
        } catch (...) {
            cout << "Введите число.\n";
        }
    }
}

void printEntry(const MoodEntry& entry) {
    string emoji = entry.mood >= 7 ? "😄" : entry.mood >= 4 ? "😐" : "😞";
    cout << emoji << " #" << entry.id << " - " << entry.timestamp << " - Оценка: " << entry.mood << "/10\n";
    cout << "   Заметка: " << entry.note << "\n";
}

int main() {
    MoodDiary diary;
    diary.loadFromFile();

    while (true) {
        cout << "\n===== ДНЕВНИК НАСТРОЕНИЯ (C++) =====" << endl;
        cout << "1. Добавить запись\n";
        cout << "2. Показать все записи\n";
        cout << "3. Показать статистику\n";
        cout << "4. Удалить запись\n";
        cout << "5. Сохранить в файл\n";
        cout << "6. Загрузить из файла\n";
        cout << "0. Выход\n";
        string choice = readString("Выберите действие: ");

        if (choice == "0") break;

        if (choice == "1") {
            int mood = readInt("Оценка настроения (от 1 до 10): ");
            if (mood < 1 || mood > 10) {
                cout << "Оценка должна быть от 1 до 10.\n";
                continue;
            }
            string note = readString("Заметка: ");
            try {
                MoodEntry entry = diary.addEntry(mood, note);
                cout << "Запись добавлена с ID " << entry.id << "\n";
            } catch (const exception& e) {
                cout << e.what() << "\n";
            }
        } else if (choice == "2") {
            if (diary.getEntries().empty()) {
                cout << "Нет записей.\n";
            } else {
                for (const auto& e : diary.getEntries()) {
                    printEntry(e);
                }
            }
        } else if (choice == "3") {
            auto stats = diary.getStats();
            if (get<int>(stats["total"]) == 0) {
                cout << "Нет записей для статистики.\n";
            } else {
                cout << "\n=== СТАТИСТИКА ===\n";
                cout << "Всего записей: " << get<int>(stats["total"]) << "\n";
                cout << "Средняя оценка: " << fixed << setprecision(2) << get<double>(stats["average"]) << "\n";
                cout << "Минимальная оценка: " << get<int>(stats["min"]) << "\n";
                cout << "Максимальная оценка: " << get<int>(stats["max"]) << "\n";
            }
        } else if (choice == "4") {
            int id = readInt("Введите ID записи для удаления: ");
            if (diary.deleteEntry(id)) {
                cout << "Запись удалена.\n";
            } else {
                cout << "Запись не найдена.\n";
            }
        } else if (choice == "5") {
            diary.saveToFile();
            cout << "Сохранено.\n";
        } else if (choice == "6") {
            diary.loadFromFile();
            cout << "Загружено.\n";
        } else {
            cout << "Неизвестная команда.\n";
        }
    }
    return 0;
}
