// mood_diary.js
const fs = require('fs').promises;
const readline = require('readline');

const rl = readline.createInterface({
    input: process.stdin,
    output: process.stdout
});

const question = (prompt) => new Promise(resolve => rl.question(prompt, resolve));

class MoodEntry {
    constructor(id, mood, note, timestamp = new Date().toISOString()) {
        this.id = id;
        this.timestamp = timestamp;
        this.mood = mood;
        this.note = note;
    }
}

class MoodDiary {
    constructor() {
        this.entries = [];
        this.nextId = 1;
    }

    addEntry(mood, note) {
        if (mood < 1 || mood > 10) {
            throw new Error('Оценка должна быть от 1 до 10');
        }
        const entry = new MoodEntry(this.nextId, mood, note);
        this.entries.push(entry);
        this.nextId++;
        return entry;
    }

    findEntry(id) {
        return this.entries.find(e => e.id === id);
    }

    deleteEntry(id) {
        const index = this.entries.findIndex(e => e.id === id);
        if (index === -1) return false;
        this.entries.splice(index, 1);
        return true;
    }

    getStats() {
        const total = this.entries.length;
        if (total === 0) {
            return { total: 0, average: null, min: null, max: null };
        }
        const moods = this.entries.map(e => e.mood);
        const sum = moods.reduce((a, b) => a + b, 0);
        return {
            total,
            average: sum / total,
            min: Math.min(...moods),
            max: Math.max(...moods)
        };
    }

    async saveToFile(filename = 'mood_data.json') {
        await fs.writeFile(filename, JSON.stringify(this.entries, null, 2));
    }

    async loadFromFile(filename = 'mood_data.json') {
        try {
            const data = await fs.readFile(filename, 'utf8');
            const loaded = JSON.parse(data);
            this.entries = loaded.map(e => Object.assign(new MoodEntry(0), e));
            this.nextId = this.entries.reduce((max, e) => Math.max(max, e.id), 0) + 1;
        } catch (err) {
            if (err.code !== 'ENOENT') throw err;
        }
    }
}

function printEntry(entry) {
    const emoji = entry.mood >= 7 ? '😄' : entry.mood >= 4 ? '😐' : '😞';
    console.log(`${emoji} #${entry.id} - ${entry.timestamp} - Оценка: ${entry.mood}/10`);
    console.log(`   Заметка: ${entry.note}`);
}

async function main() {
    const diary = new MoodDiary();
    await diary.loadFromFile();

    while (true) {
        console.log('\n===== ДНЕВНИК НАСТРОЕНИЯ (JavaScript) =====');
        console.log('1. Добавить запись');
        console.log('2. Показать все записи');
        console.log('3. Показать статистику');
        console.log('4. Удалить запись');
        console.log('5. Сохранить в файл');
        console.log('6. Загрузить из файла');
        console.log('0. Выход');
        const choice = await question('Выберите действие: ');

        if (choice === '0') break;

        switch (choice) {
            case '1': {
                const mood = parseInt(await question('Оценка настроения (от 1 до 10): '));
                if (isNaN(mood) || mood < 1 || mood > 10) {
                    console.log('Оценка должна быть от 1 до 10.');
                    continue;
                }
                const note = await question('Заметка: ');
                try {
                    const entry = diary.addEntry(mood, note);
                    console.log(`Запись добавлена с ID ${entry.id}`);
                } catch (err) {
                    console.log(err.message);
                }
                break;
            }
            case '2': {
                if (diary.entries.length === 0) {
                    console.log('Нет записей.');
                } else {
                    diary.entries.forEach(printEntry);
                }
                break;
            }
            case '3': {
                const stats = diary.getStats();
                if (stats.total === 0) {
                    console.log('Нет записей для статистики.');
                } else {
                    console.log('\n=== СТАТИСТИКА ===');
                    console.log(`Всего записей: ${stats.total}`);
                    console.log(`Средняя оценка: ${stats.average.toFixed(2)}`);
                    console.log(`Минимальная оценка: ${stats.min}`);
                    console.log(`Максимальная оценка: ${stats.max}`);
                }
                break;
            }
            case '4': {
                const id = parseInt(await question('Введите ID записи для удаления: '));
                if (diary.deleteEntry(id)) {
                    console.log('Запись удалена.');
                } else {
                    console.log('Запись не найдена.');
                }
                break;
            }
            case '5':
                try {
                    await diary.saveToFile();
                    console.log('Сохранено.');
                } catch (err) {
                    console.log('Ошибка сохранения:', err.message);
                }
                break;
            case '6':
                try {
                    await diary.loadFromFile();
                    console.log('Загружено.');
                } catch (err) {
                    console.log('Ошибка загрузки:', err.message);
                }
                break;
            default:
                console.log('Неизвестная команда.');
        }
    }
    rl.close();
}

main().catch(console.error);
