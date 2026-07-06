# mood_diary.rb
require 'json'
require 'date'

class MoodEntry
  attr_accessor :id, :timestamp, :mood, :note

  def initialize(id, mood, note, timestamp = DateTime.now.iso8601)
    @id = id
    @timestamp = timestamp
    @mood = mood
    @note = note
  end

  def to_h
    { id: @id, timestamp: @timestamp, mood: @mood, note: @note }
  end

  def self.from_h(hash)
    MoodEntry.new(hash[:id], hash[:mood], hash[:note], hash[:timestamp])
  end
end

class MoodDiary
  attr_reader :entries

  def initialize
    @entries = []
    @next_id = 1
  end

  def add_entry(mood, note)
    raise ArgumentError, "Оценка должна быть от 1 до 10" unless (1..10).include?(mood)
    entry = MoodEntry.new(@next_id, mood, note)
    @entries << entry
    @next_id += 1
    entry
  end

  def find_entry(id)
    @entries.find { |e| e.id == id }
  end

  def delete_entry(id)
    entry = find_entry(id)
    return false unless entry
    @entries.delete(entry)
    true
  end

  def stats
    total = @entries.size
    if total == 0
      { total: 0, average: nil, min: nil, max: nil }
    else
      moods = @entries.map(&:mood)
      {
        total: total,
        average: moods.sum.to_f / total,
        min: moods.min,
        max: moods.max
      }
    end
  end

  def save_to_file(filename = "mood_data.json")
    File.write(filename, JSON.pretty_generate(@entries.map(&:to_h)))
  end

  def load_from_file(filename = "mood_data.json")
    return unless File.exist?(filename)
    data = JSON.parse(File.read(filename), symbolize_names: true)
    @entries.clear
    data.each do |item|
      entry = MoodEntry.from_h(item)
      @entries << entry
      @next_id = entry.id + 1 if entry.id >= @next_id
    end
  rescue JSON::ParserError
    puts "Ошибка чтения файла."
  end
end

def print_entry(entry)
  emoji = if entry.mood >= 7 then "😄" elsif entry.mood >= 4 then "😐" else "😞" end
  puts "#{emoji} ##{entry.id} - #{entry.timestamp} - Оценка: #{entry.mood}/10"
  puts "   Заметка: #{entry.note}"
end

def main
  diary = MoodDiary.new
  diary.load_from_file

  loop do
    puts "\n===== ДНЕВНИК НАСТРОЕНИЯ (Ruby) ====="
    puts "1. Добавить запись"
    puts "2. Показать все записи"
    puts "3. Показать статистику"
    puts "4. Удалить запись"
    puts "5. Сохранить в файл"
    puts "6. Загрузить из файла"
    puts "0. Выход"
    print "Выберите действие: "
    choice = gets.chomp

    case choice
    when "0"
      break
    when "1"
      print "Оценка настроения (от 1 до 10): "
      mood = gets.chomp.to_i
      unless (1..10).include?(mood)
        puts "Оценка должна быть от 1 до 10."
        next
      end
      print "Заметка: "
      note = gets.chomp
      entry = diary.add_entry(mood, note)
      puts "Запись добавлена с ID #{entry.id}"
    when "2"
      if diary.entries.empty?
        puts "Нет записей."
      else
        diary.entries.each { |e| print_entry(e) }
      end
    when "3"
      stats = diary.stats
      if stats[:total] == 0
        puts "Нет записей для статистики."
      else
        puts "\n=== СТАТИСТИКА ==="
        puts "Всего записей: #{stats[:total]}"
        puts "Средняя оценка: #{'%.2f' % stats[:average]}"
        puts "Минимальная оценка: #{stats[:min]}"
        puts "Максимальная оценка: #{stats[:max]}"
      end
    when "4"
      print "Введите ID записи для удаления: "
      id = gets.chomp.to_i
      if diary.delete_entry(id)
        puts "Запись удалена."
      else
        puts "Запись не найдена."
      end
    when "5"
      diary.save_to_file
      puts "Сохранено."
    when "6"
      diary.load_from_file
      puts "Загружено."
    else
      puts "Неизвестная команда."
    end
  end
end

main if __FILE__ == $0
