// mood_diary.go
package main

import (
	"bufio"
	"encoding/json"
	"fmt"
	"os"
	"strconv"
	"strings"
	"time"
)

type MoodEntry struct {
	ID        int       `json:"id"`
	Timestamp time.Time `json:"timestamp"`
	Mood      int       `json:"mood"`
	Note      string    `json:"note"`
}

type MoodDiary struct {
	entries []MoodEntry
	nextID  int
}

func NewMoodDiary() *MoodDiary {
	return &MoodDiary{
		entries: []MoodEntry{},
		nextID:  1,
	}
}

func (d *MoodDiary) AddEntry(mood int, note string) (MoodEntry, error) {
	if mood < 1 || mood > 10 {
		return MoodEntry{}, fmt.Errorf("оценка должна быть от 1 до 10")
	}
	entry := MoodEntry{
		ID:        d.nextID,
		Timestamp: time.Now(),
		Mood:      mood,
		Note:      note,
	}
	d.entries = append(d.entries, entry)
	d.nextID++
	return entry, nil
}

func (d *MoodDiary) FindEntry(id int) *MoodEntry {
	for i := range d.entries {
		if d.entries[i].ID == id {
			return &d.entries[i]
		}
	}
	return nil
}

func (d *MoodDiary) DeleteEntry(id int) bool {
	for i, e := range d.entries {
		if e.ID == id {
			d.entries = append(d.entries[:i], d.entries[i+1:]...)
			return true
		}
	}
	return false
}

func (d *MoodDiary) GetStats() map[string]interface{} {
	total := len(d.entries)
	if total == 0 {
		return map[string]interface{}{"total": 0, "average": nil, "min": nil, "max": nil}
	}
	sum := 0
	min := 11
	max := 0
	for _, e := range d.entries {
		sum += e.Mood
		if e.Mood < min {
			min = e.Mood
		}
		if e.Mood > max {
			max = e.Mood
		}
	}
	return map[string]interface{}{
		"total":   total,
		"average": float64(sum) / float64(total),
		"min":     min,
		"max":     max,
	}
}

func (d *MoodDiary) SaveToFile(filename string) error {
	data, err := json.MarshalIndent(d.entries, "", "  ")
	if err != nil {
		return err
	}
	return os.WriteFile(filename, data, 0644)
}

func (d *MoodDiary) LoadFromFile(filename string) error {
	data, err := os.ReadFile(filename)
	if err != nil {
		if os.IsNotExist(err) {
			return nil
		}
		return err
	}
	var loaded []MoodEntry
	if err := json.Unmarshal(data, &loaded); err != nil {
		return err
	}
	d.entries = loaded
	for _, e := range loaded {
		if e.ID >= d.nextID {
			d.nextID = e.ID + 1
		}
	}
	return nil
}

func readString(prompt string) string {
	fmt.Print(prompt)
	reader := bufio.NewReader(os.Stdin)
	input, _ := reader.ReadString('\n')
	return strings.TrimSpace(input)
}

func readInt(prompt string) int {
	for {
		input := readString(prompt)
		if val, err := strconv.Atoi(input); err == nil {
			return val
		}
		fmt.Println("Введите число.")
	}
}

func printEntry(entry MoodEntry) {
	emoji := "😄"
	if entry.Mood >= 7 {
		emoji = "😄"
	} else if entry.Mood >= 4 {
		emoji = "😐"
	} else {
		emoji = "😞"
	}
	fmt.Printf("%s #%d - %s - Оценка: %d/10\n", emoji, entry.ID, entry.Timestamp.Format("2006-01-02 15:04:05"), entry.Mood)
	fmt.Printf("   Заметка: %s\n", entry.Note)
}

func main() {
	diary := NewMoodDiary()
	if err := diary.LoadFromFile("mood_data.json"); err != nil {
		fmt.Println("Ошибка загрузки:", err)
	}

	for {
		fmt.Println("\n===== ДНЕВНИК НАСТРОЕНИЯ (Go) =====")
		fmt.Println("1. Добавить запись")
		fmt.Println("2. Показать все записи")
		fmt.Println("3. Показать статистику")
		fmt.Println("4. Удалить запись")
		fmt.Println("5. Сохранить в файл")
		fmt.Println("6. Загрузить из файла")
		fmt.Println("0. Выход")
		choice := readString("Выберите действие: ")

		switch choice {
		case "0":
			return
		case "1":
			mood := readInt("Оценка настроения (от 1 до 10): ")
			if mood < 1 || mood > 10 {
				fmt.Println("Оценка должна быть от 1 до 10.")
				continue
			}
			note := readString("Заметка: ")
			entry, err := diary.AddEntry(mood, note)
			if err != nil {
				fmt.Println(err)
				continue
			}
			fmt.Printf("Запись добавлена с ID %d\n", entry.ID)
		case "2":
			if len(diary.entries) == 0 {
				fmt.Println("Нет записей.")
			} else {
				for _, e := range diary.entries {
					printEntry(e)
				}
			}
		case "3":
			stats := diary.GetStats()
			if stats["total"] == 0 {
				fmt.Println("Нет записей для статистики.")
			} else {
				fmt.Println("\n=== СТАТИСТИКА ===")
				fmt.Printf("Всего записей: %d\n", stats["total"])
				fmt.Printf("Средняя оценка: %.2f\n", stats["average"])
				fmt.Printf("Минимальная оценка: %d\n", stats["min"])
				fmt.Printf("Максимальная оценка: %d\n", stats["max"])
			}
		case "4":
			id := readInt("Введите ID записи для удаления: ")
			if diary.DeleteEntry(id) {
				fmt.Println("Запись удалена.")
			} else {
				fmt.Println("Запись не найдена.")
			}
		case "5":
			if err := diary.SaveToFile("mood_data.json"); err != nil {
				fmt.Println("Ошибка сохранения:", err)
			} else {
				fmt.Println("Сохранено.")
			}
		case "6":
			if err := diary.LoadFromFile("mood_data.json"); err != nil {
				fmt.Println("Ошибка загрузки:", err)
			} else {
				fmt.Println("Загружено.")
			}
		default:
			fmt.Println("Неизвестная команда.")
		}
	}
}
