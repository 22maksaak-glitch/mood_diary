// MoodDiary.cs
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.Json;
using System.Text.Json.Serialization;

public record MoodEntry(
    int Id,
    string Timestamp,
    int Mood,
    string Note
);

public class MoodDiary
{
    private List<MoodEntry> entries = new();
    private int nextId = 1;

    public IReadOnlyList<MoodEntry> Entries => entries.AsReadOnly();

    public MoodEntry AddEntry(int mood, string note)
    {
        if (mood < 1 || mood > 10)
            throw new ArgumentException("Оценка должна быть от 1 до 10");
        var entry = new MoodEntry(
            nextId,
            DateTime.Now.ToString("o"),
            mood,
            note
        );
        entries.Add(entry);
        nextId++;
        return entry;
    }

    public MoodEntry? FindEntry(int id) => entries.FirstOrDefault(e => e.Id == id);

    public bool DeleteEntry(int id) => entries.RemoveAll(e => e.Id == id) > 0;

    public Dictionary<string, object> GetStats()
    {
        int total = entries.Count;
        if (total == 0)
            return new Dictionary<string, object> { ["total"] = 0, ["average"] = null, ["min"] = null, ["max"] = null };
        var moods = entries.Select(e => e.Mood);
        return new Dictionary<string, object>
        {
            ["total"] = total,
            ["average"] = moods.Average(),
            ["min"] = moods.Min(),
            ["max"] = moods.Max()
        };
    }

    public void SaveToFile(string filename)
    {
        var options = new JsonSerializerOptions { WriteIndented = true };
        string json = JsonSerializer.Serialize(entries, options);
        File.WriteAllText(filename, json);
    }

    public void LoadFromFile(string filename)
    {
        if (!File.Exists(filename)) return;
        string json = File.ReadAllText(filename);
        var loaded = JsonSerializer.Deserialize<List<MoodEntry>>(json);
        if (loaded != null)
        {
            entries = loaded;
            nextId = entries.Any() ? entries.Max(e => e.Id) + 1 : 1;
        }
    }
}

public static class Program
{
    private static string ReadString(string prompt)
    {
        Console.Write(prompt);
        return Console.ReadLine()?.Trim() ?? "";
    }

    private static int ReadInt(string prompt)
    {
        while (true)
        {
            Console.Write(prompt);
            if (int.TryParse(Console.ReadLine(), out int result))
                return result;
            Console.WriteLine("Введите число.");
        }
    }

    private static void PrintEntry(MoodEntry entry)
    {
        string emoji = entry.Mood >= 7 ? "😄" : entry.Mood >= 4 ? "😐" : "😞";
        Console.WriteLine($"{emoji} #{entry.Id} - {entry.Timestamp} - Оценка: {entry.Mood}/10");
        Console.WriteLine($"   Заметка: {entry.Note}");
    }

    public static void Main()
    {
        var diary = new MoodDiary();
        try { diary.LoadFromFile("mood_data.json"); }
        catch { Console.WriteLine("Не удалось загрузить данные."); }

        while (true)
        {
            Console.WriteLine("\n===== ДНЕВНИК НАСТРОЕНИЯ (C#) =====");
            Console.WriteLine("1. Добавить запись");
            Console.WriteLine("2. Показать все записи");
            Console.WriteLine("3. Показать статистику");
            Console.WriteLine("4. Удалить запись");
            Console.WriteLine("5. Сохранить в файл");
            Console.WriteLine("6. Загрузить из файла");
            Console.WriteLine("0. Выход");
            string choice = ReadString("Выберите действие: ");

            switch (choice)
            {
                case "0": return;
                case "1":
                    int mood = ReadInt("Оценка настроения (от 1 до 10): ");
                    if (mood < 1 || mood > 10)
                    {
                        Console.WriteLine("Оценка должна быть от 1 до 10.");
                        continue;
                    }
                    string note = ReadString("Заметка: ");
                    var entry = diary.AddEntry(mood, note);
                    Console.WriteLine($"Запись добавлена с ID {entry.Id}");
                    break;
                case "2":
                    if (!diary.Entries.Any()) Console.WriteLine("Нет записей.");
                    else foreach (var e in diary.Entries) PrintEntry(e);
                    break;
                case "3":
                    var stats = diary.GetStats();
                    if ((int)stats["total"] == 0)
                        Console.WriteLine("Нет записей для статистики.");
                    else
                    {
                        Console.WriteLine("\n=== СТАТИСТИКА ===");
                        Console.WriteLine($"Всего записей: {stats["total"]}");
                        Console.WriteLine($"Средняя оценка: {stats["average"]:F2}");
                        Console.WriteLine($"Минимальная оценка: {stats["min"]}");
                        Console.WriteLine($"Максимальная оценка: {stats["max"]}");
                    }
                    break;
                case "4":
                    int id = ReadInt("Введите ID записи для удаления: ");
                    if (diary.DeleteEntry(id)) Console.WriteLine("Запись удалена.");
                    else Console.WriteLine("Запись не найдена.");
                    break;
                case "5":
                    try { diary.SaveToFile("mood_data.json"); Console.WriteLine("Сохранено."); }
                    catch (Exception ex) { Console.WriteLine($"Ошибка: {ex.Message}"); }
                    break;
                case "6":
                    try { diary.LoadFromFile("mood_data.json"); Console.WriteLine("Загружено."); }
                    catch (Exception ex) { Console.WriteLine($"Ошибка: {ex.Message}"); }
                    break;
                default: Console.WriteLine("Неизвестная команда."); break;
            }
        }
    }
}
