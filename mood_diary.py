# mood_diary.py
import json
from dataclasses import dataclass, asdict
from datetime import datetime
from typing import List, Optional
from pathlib import Path

@dataclass
class MoodEntry:
    id: int
    timestamp: str
    mood: int  # 1-10
    note: str

class MoodDiary:
    def __init__(self):
        self.entries: List[MoodEntry] = []
        self.next_id = 1

    def add_entry(self, mood: int, note: str) -> MoodEntry:
        if not (1 <= mood <= 10):
            raise ValueError("Оценка должна быть от 1 до 10")
        entry = MoodEntry(
            id=self.next_id,
            timestamp=datetime.now().isoformat(),
            mood=mood,
            note=note
        )
        self.entries.append(entry)
        self.next_id += 1
        return entry

    def find_entry(self, entry_id: int) -> Optional[MoodEntry]:
        return next((e for e in self.entries if e.id == entry_id), None)

    def delete_entry(self, entry_id: int) -> bool:
        entry = self.find_entry(entry_id)
        if entry:
            self.entries.remove(entry)
            return True
        return False

    def get_stats(self) -> dict:
        total = len(self.entries)
        if total == 0:
            return {"total": 0, "average": None, "min": None, "max": None}
        moods = [e.mood for e in self.entries]
        return {
            "total": total,
            "average": sum(moods) / total,
            "min": min(moods),
            "max": max(moods)
        }

    def save_to_file(self, filename: str = "mood_data.json") -> None:
        with open(filename, "w", encoding="utf-8") as f:
            json.dump([asdict(e) for e in self.entries], f, ensure_ascii=False, indent=2)

    def load_from_file(self, filename: str = "mood_data.json") -> None:
        path = Path(filename)
        if not path.exists():
            return
        with open(filename, "r", encoding="utf-8") as f:
            data = json.load(f)
            self.entries.clear()
            for item in data:
                entry = MoodEntry(
                    id=item["id"],
                    timestamp=item["timestamp"],
                    mood=item["mood"],
                    note=item["note"]
                )
                self.entries.append(entry)
                if entry.id >= self.next_id:
                    self.next_id = entry.id + 1

def print_entry(entry: MoodEntry) -> None:
    emoji = "😄" if entry.mood >= 7 else "😐" if entry.mood >= 4 else "😞"
    print(f"{emoji} #{entry.id} - {entry.timestamp} - Оценка: {entry.mood}/10")
    print(f"   Заметка: {entry.note}")

def main():
    diary = MoodDiary()
    diary.load_from_file()

    while True:
        print("\n===== ДНЕВНИК НАСТРОЕНИЯ (Python) =====")
        print("1. Добавить запись")
        print("2. Показать все записи")
        print("3. Показать статистику")
        print("4. Удалить запись")
        print("5. Сохранить в файл")
        print("6. Загрузить из файла")
        print("0. Выход")
        choice = input("Выберите действие: ").strip()

        if choice == "0":
            break
        elif choice == "1":
            try:
                mood = int(input("Оценка настроения (от 1 до 10): ").strip())
            except ValueError:
                print("Введите число.")
                continue
            if not (1 <= mood <= 10):
                print("Оценка должна быть от 1 до 10.")
                continue
            note = input("Заметка: ").strip()
            entry = diary.add_entry(mood, note)
            print(f"Запись добавлена с ID {entry.id}")
        elif choice == "2":
            if not diary.entries:
                print("Нет записей.")
            else:
                for entry in diary.entries:
                    print_entry(entry)
        elif choice == "3":
            stats = diary.get_stats()
            if stats["total"] == 0:
                print("Нет записей для статистики.")
            else:
                print("\n=== СТАТИСТИКА ===")
                print(f"Всего записей: {stats['total']}")
                print(f"Средняя оценка: {stats['average']:.2f}")
                print(f"Минимальная оценка: {stats['min']}")
                print(f"Максимальная оценка: {stats['max']}")
        elif choice == "4":
            try:
                entry_id = int(input("Введите ID записи для удаления: ").strip())
            except ValueError:
                print("Некорректный ID.")
                continue
            if diary.delete_entry(entry_id):
                print("Запись удалена.")
            else:
                print("Запись не найдена.")
        elif choice == "5":
            diary.save_to_file()
            print("Сохранено.")
        elif choice == "6":
            diary.load_from_file()
            print("Загружено.")
        else:
            print("Неизвестная команда.")

if __name__ == "__main__":
    main()
