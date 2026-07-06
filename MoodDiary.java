// MoodDiary.java
import java.io.*;
import java.nio.file.*;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.*;
import java.util.stream.Collectors;

record MoodEntry(int id, String timestamp, int mood, String note) implements Serializable {}

class MoodDiary implements Serializable {
    private static final long serialVersionUID = 1L;
    private List<MoodEntry> entries = new ArrayList<>();
    private int nextId = 1;

    public MoodEntry addEntry(int mood, String note) {
        if (mood < 1 || mood > 10) {
            throw new IllegalArgumentException("Оценка должна быть от 1 до 10");
        }
        MoodEntry entry = new MoodEntry(
            nextId,
            LocalDateTime.now().format(DateTimeFormatter.ISO_LOCAL_DATE_TIME),
            mood,
            note
        );
        entries.add(entry);
        nextId++;
        return entry;
    }

    public Optional<MoodEntry> findEntry(int id) {
        return entries.stream().filter(e -> e.id() == id).findFirst();
    }

    public boolean deleteEntry(int id) {
        return entries.removeIf(e -> e.id() == id);
    }

    public Map<String, Object> getStats() {
        int total = entries.size();
        if (total == 0) {
            return Map.of("total", 0, "average", null, "min", null, "max", null);
        }
        IntSummaryStatistics stats = entries.stream().mapToInt(MoodEntry::mood).summaryStatistics();
        return Map.of(
            "total", total,
            "average", stats.getAverage(),
            "min", stats.getMin(),
            "max", stats.getMax()
        );
    }

    public void saveToFile(String filename) throws IOException {
        try (ObjectOutputStream oos = new ObjectOutputStream(Files.newOutputStream(Paths.get(filename)))) {
            oos.writeObject(this);
        }
    }

    public void loadFromFile(String filename) throws IOException, ClassNotFoundException {
        try (ObjectInputStream ois = new ObjectInputStream(Files.newInputStream(Paths.get(filename)))) {
            MoodDiary loaded = (MoodDiary) ois.readObject();
            this.entries = loaded.entries;
            this.nextId = loaded.nextId;
        }
    }

    public List<MoodEntry> getEntries() { return Collections.unmodifiableList(entries); }
}

public class MoodDiaryApp {
    private static final Scanner scanner = new Scanner(System.in);

    private static String readString(String prompt) {
        System.out.print(prompt);
        return scanner.nextLine().trim();
    }

    private static int readInt(String prompt) {
        while (true) {
            try {
                System.out.print(prompt);
                return Integer.parseInt(scanner.nextLine().trim());
            } catch (NumberFormatException e) {
                System.out.println("Введите число.");
            }
        }
    }

    private static void printEntry(MoodEntry entry) {
        String emoji = entry.mood() >= 7 ? "😄" : entry.mood() >= 4 ? "😐" : "😞";
        System.out.printf("%s #%d - %s - Оценка: %d/10%n", emoji, entry.id(), entry.timestamp(), entry.mood());
        System.out.printf("   Заметка: %s%n", entry.note());
    }

    public static void main(String[] args) {
        MoodDiary diary = new MoodDiary();
        try {
            diary.loadFromFile("mood_data.ser");
        } catch (IOException | ClassNotFoundException e) {
            System.out.println("Не удалось загрузить данные, начинаем с пустого дневника.");
        }

        while (true) {
            System.out.println("\n===== ДНЕВНИК НАСТРОЕНИЯ (Java) =====");
            System.out.println("1. Добавить запись");
            System.out.println("2. Показать все записи");
            System.out.println("3. Показать статистику");
            System.out.println("4. Удалить запись");
            System.out.println("5. Сохранить в файл");
            System.out.println("6. Загрузить из файла");
            System.out.println("0. Выход");
            String choice = readString("Выберите действие: ");

            switch (choice) {
                case "0" -> { return; }
                case "1" -> {
                    int mood = readInt("Оценка настроения (от 1 до 10): ");
                    if (mood < 1 || mood > 10) {
                        System.out.println("Оценка должна быть от 1 до 10.");
                        continue;
                    }
                    String note = readString("Заметка: ");
                    MoodEntry entry = diary.addEntry(mood, note);
                    System.out.printf("Запись добавлена с ID %d%n", entry.id());
                }
                case "2" -> {
                    if (diary.getEntries().isEmpty()) {
                        System.out.println("Нет записей.");
                    } else {
                        diary.getEntries().forEach(MoodDiaryApp::printEntry);
                    }
                }
                case "3" -> {
                    var stats = diary.getStats();
                    if ((int)stats.get("total") == 0) {
                        System.out.println("Нет записей для статистики.");
                    } else {
                        System.out.println("\n=== СТАТИСТИКА ===");
                        System.out.printf("Всего записей: %d%n", stats.get("total"));
                        System.out.printf("Средняя оценка: %.2f%n", stats.get("average"));
                        System.out.printf("Минимальная оценка: %d%n", stats.get("min"));
                        System.out.printf("Максимальная оценка: %d%n", stats.get("max"));
                    }
                }
                case "4" -> {
                    int id = readInt("Введите ID записи для удаления: ");
                    if (diary.deleteEntry(id)) {
                        System.out.println("Запись удалена.");
                    } else {
                        System.out.println("Запись не найдена.");
                    }
                }
                case "5" -> {
                    try {
                        diary.saveToFile("mood_data.ser");
                        System.out.println("Сохранено.");
                    } catch (IOException e) {
                        System.out.println("Ошибка сохранения: " + e.getMessage());
                    }
                }
                case "6" -> {
                    try {
                        diary.loadFromFile("mood_data.ser");
                        System.out.println("Загружено.");
                    } catch (IOException | ClassNotFoundException e) {
                        System.out.println("Ошибка загрузки: " + e.getMessage());
                    }
                }
                default -> System.out.println("Неизвестная команда.");
            }
        }
    }
}
